import PositionModeler from './positionModeler';
import KalmanPredictor from './predictor/kalmanPredictor';
import StrokeEndPredictor from './predictor/strokeEndPredictor';
import StylusStateModeler from './stylusStateModeler';
import { Status } from './utils';
import WobbleSmoother from './wobbleSmoother';

function modelStylus(
  tipStates: TipState[],
  stylusStateModeler: StylusStateModeler,
  result: Result[]
): void {
  for (const tipState of tipStates) {
    const stylusState = stylusStateModeler.query(tipState.position);

    result.push({
      position: tipState.position,
      velocity: tipState.velocity,
      time: tipState.time,
      pressure: stylusState.pressure,
      tilt: stylusState.tilt,
      orientation: stylusState.orientation
    });
  }
}

function getNumberOfSteps(
  startTime: number,
  endTime: number,
  samplingParams: SamplingParams
): Status.Error | number {
  const floatDelta = endTime - startTime;
  const nSteps = Math.ceil(floatDelta * samplingParams.minOutputRate);

  if (nSteps > samplingParams.maxOutputsPerCall) return Status.Error;
  return nSteps;
}

/**
 * This class models a stroke from a raw input stream. The modeling is performed
 * in several stages, which are delegated to component classes:
 * - Wobble Smoothing: Dampens high-frequency noise from quantization error.
 * - Position Modeling: Models the pen tip as a mass, connected by a spring, to
 *   a moving anchor.
 * - Stylus State Modeling: Constructs stylus states for modeled positions by
 *   interpolating over the raw input.
 *
 * Additionally, this class provides prediction of the modeled stroke.
 *
 * StrokeModeler is completely unit-agnostic. That is, it doesn't matter what
 * units or coordinate-system the input is given in; the output will be given in
 * the same coordinate-system and units.
 */
abstract class StrokeModeler {
  private static m_predictor: InputPredictor;

  private static m_strokeModelParams: StrokeModelParams | null;

  private static m_wobbleSmoother: WobbleSmoother = new WobbleSmoother();
  private static m_positionModeler: PositionModeler = new PositionModeler();
  private static m_stylusStateModeler: StylusStateModeler = new StylusStateModeler();

  private static m_tipStateBuffer: TipState[] = [];

  private static m_lastInput: InputAndCorrectedPosition | null;

  /**
   * Clears any in-progress stroke, and initializes (or re-initializes) the
   * model with the given parameters. Returns an error if the parameters are
   * invalid.
   */
  static reset(strokeModelParams: StrokeModelParams): Status {
    // MAYBE: Implement parameter validation
    // if (this.validateStrokeModelParams(strokeModelParams) !== Status.OK) return Status.Error;

    // Note that many of the sub-modelers require some knowledge about the stroke
    // (e.g. start position, input type) when resetting, and as such are reset in
    // ProcessTDown() instead.
    this.m_strokeModelParams = strokeModelParams;
    this.m_lastInput = null;

    // TODO: Check StrokeEndPredictor;
    if (false) {
      this.m_predictor = new KalmanPredictor(
        strokeModelParams.predictionParams,
        strokeModelParams.samplingParams
      );
    } else {
      this.m_predictor = new StrokeEndPredictor(
        strokeModelParams.positionModelerParams,
        strokeModelParams.samplingParams
      );
    }

    return Status.OK;
  }

  /**
   * Updates the model with a raw input, and then clears and fills the results
   * parameter with newly generated Results. Any previously generated Result
   * values remain valid.
   *
   * The function fills an out parameter instead of returning by value to allow
   * the caller to reuse allocations. Update is expected to be called 10s to
   * 100s of times over the course of a stroke, producing a relatively small
   * result each time.
   *
   * Returns an error if the the model has not yet been initialized (via Reset)
   * or if the input stream is malformed (e.g decreasing time, Up event before
   * Down event). In that case, results will be empty after the call.
   *
   * If this does not return an error, results will contain at least one Result,
   * and potentially more than one if the inputs are slower than the minimum
   * output rate.
   */
  static update(input: PencilInput, results: Result[]): Status {
    results.length = 0;

    if (!this.m_strokeModelParams) return Status.Error;

    // MAYBE: Implement parameter validation
    // if (this.validateInput(input) !== Status.OK) return Status.Error;

    if (this.m_lastInput) {
      if (
        this.m_lastInput.input.eventType == input.eventType &&
        this.m_lastInput.input.position == input.position &&
        this.m_lastInput.input.time == input.time &&
        this.m_lastInput.input.pressure == input.pressure &&
        this.m_lastInput.input.tilt == input.tilt &&
        this.m_lastInput.input.orientation == input.orientation
      )
        return Status.Error;

      if (input.time < this.m_lastInput.input.time) return Status.Error;
    }

    switch (input.eventType) {
      case 'down':
        return this.processDownEvent(input, results);
      case 'move':
        return this.processMoveEvent(input, results);
      case 'up':
        return this.processUpEvent(input, results);
    }

    return Status.Error;
  }

  /**
   * Models the given input prediction without changing the internal model
   * state, and then clears and fills the results parameter with the new
   * predicted Results. Any previously generated prediction Results are no
   * longer valid.
   *
   * Returns an error if the the model has not yet been initialized (via Reset),
   * or if there is no stroke in progress. In that case, results will be empty
   * after the call.
   *
   * The output is limited to results where the predictor has sufficient
   * confidence.
   */
  static predict(results: Result[]): Status {
    results.length = 0;

    if (!this.m_strokeModelParams) return Status.Error;

    if (this.m_lastInput === null) return Status.Error;

    this.m_predictor.constructPrediction(
      this.m_positionModeler.currentState,
      this.m_tipStateBuffer
    );
    modelStylus(this.m_tipStateBuffer, this.m_stylusStateModeler, results);

    return Status.OK;
  }

  private static processDownEvent(input: PencilInput, result: Result[]): Status {
    if (this.m_lastInput || !this.m_strokeModelParams) return Status.Error;

    // Note that many of the sub-modelers require some knowledge about the stroke
    // (e.g. start position, input type) when resetting, and as such are reset
    // here instead of in Reset().
    this.m_wobbleSmoother.reset(
      this.m_strokeModelParams.wobbleSmootherParams,
      input.position,
      input.time
    );
    this.m_positionModeler.reset(
      { position: input.position, velocity: [0, 0], time: input.time },
      this.m_strokeModelParams.positionModelerParams
    );
    this.m_stylusStateModeler.reset(this.m_strokeModelParams.stylusStateModelerParams);
    this.m_stylusStateModeler.update(input.position, {
      pressure: input.pressure,
      tilt: input.tilt,
      orientation: input.orientation
    });

    const tip_state = this.m_positionModeler.currentState;
    this.m_predictor.reset();
    this.m_predictor.update(input.position, input.time);

    // We don't correct the position on the down event, so we set
    // corrected_position to use the input position.
    this.m_lastInput = { input: input, correctedPosition: input.position };
    result.push({
      position: tip_state.position,
      velocity: tip_state.velocity,
      time: tip_state.time,
      pressure: input.pressure,
      tilt: input.tilt,
      orientation: input.orientation
    });

    return Status.OK;
  }

  private static processMoveEvent(input: PencilInput, results: Result[]): Status {
    if (!this.m_lastInput || !this.m_strokeModelParams) return Status.Error;

    const correctedPosition = this.m_wobbleSmoother.update(input.position, input.time);
    this.m_stylusStateModeler.update(correctedPosition, {
      pressure: input.pressure,
      tilt: input.tilt,
      orientation: input.orientation
    });

    const n_steps = getNumberOfSteps(
      this.m_lastInput.input.time,
      input.time,
      this.m_strokeModelParams.samplingParams
    );
    if (n_steps === Status.Error) return n_steps;

    this.m_tipStateBuffer.length = 0;

    this.m_positionModeler.updateAlongLinearPath(
      this.m_lastInput.correctedPosition,
      this.m_lastInput.input.time,
      correctedPosition,
      input.time,
      n_steps,
      this.m_tipStateBuffer
    );

    this.m_predictor.update(correctedPosition, input.time);
    this.m_lastInput = { input, correctedPosition };
    modelStylus(this.m_tipStateBuffer, this.m_stylusStateModeler, results);

    return Status.OK;
  }

  private static processUpEvent(input: PencilInput, results: Result[]): Status {
    if (!this.m_lastInput || !this.m_strokeModelParams) return Status.Error;

    const n_steps = getNumberOfSteps(
      this.m_lastInput.input.time,
      input.time,
      this.m_strokeModelParams?.samplingParams
    );
    if (n_steps === Status.Error) return n_steps;

    this.m_tipStateBuffer.length = 0;

    this.m_positionModeler.updateAlongLinearPath(
      this.m_lastInput.correctedPosition,
      this.m_lastInput.input.time,
      input.position,
      input.time,
      n_steps,
      this.m_tipStateBuffer
    );

    this.m_positionModeler.modelEndOfStroke(
      input.position,
      1 / this.m_strokeModelParams.samplingParams.minOutputRate,
      this.m_strokeModelParams.samplingParams.endOfStrokeMaxIterations,
      this.m_strokeModelParams.samplingParams.endOfStrokeStoppingDistance,
      this.m_tipStateBuffer
    );

    if (!this.m_tipStateBuffer.length) {
      // If we haven't generated any new states, add the current state. This can
      // happen if the TUp has the same timestamp as the last in-contact input.
      this.m_tipStateBuffer.push(this.m_positionModeler.currentState);
    }

    this.m_stylusStateModeler.update(input.position, {
      pressure: input.pressure,
      tilt: input.tilt,
      orientation: input.orientation
    });

    // This indicates that we've finished the stroke.
    this.m_lastInput = null;

    modelStylus(this.m_tipStateBuffer, this.m_stylusStateModeler, results);

    return Status.OK;
  }
}

export default StrokeModeler;
