/**
 * // A modeled input produced by the stroke modeler.
 * */
interface Result {
  /**
   * // The position and velocity of the stroke tip.
   * */
  position: vec2;
  velocity: vec2;

  /**
   * // The time at which this input occurs.
   * */
  time: number;

  /**
   * // These pressure, tilt, and orientation of the stylus. See the
  // corresponding fields on the Input struct for more info.
  */
  pressure: number;
  tilt: number;
  orientation: number;
}

interface Sample {
  position: vec2;
  weightedPosition: vec2;
  distance: number;
  duration: number;
  time: number;
}

interface InputAndCorrectedPosition {
  input: PencilInput;
  correctedPosition: vec2;
}

interface PositionAndState {
  position: vec2;
  state: StylusState;
}

interface KalmanState {
  position: vec2;
  velocity: vec2;
  acceleration: vec2;
  jerk: vec2;
}

type PencilInputEventType = 'down' | 'move' | 'up';

interface PencilInput {
  /**
   * The type of event represented by the input. A "kDown" event represents
   * the beginning of the stroke, a "kUp" event represents the end of the
   * stroke, and all events in between are "kMove" events.
   */
  eventType: PencilInputEventType;

  /**
   * The position of the input.
   */
  position: vec2;

  /**
   * The time at which the input occurs.
   */
  time: number;

  /**
   * The amount of pressure applied to the stylus. This is expected to lie in
   * the range [0, 1]. A negative value indicates unknown pressure.
   */
  pressure: number;

  /**
   * The angle between the stylus and the plane of the device's screen. This
   * is expected to lie in the range [0, π/2]. A value of 0 indicates that the
   * stylus is perpendicular to the screen, while a value of π/2 indicates
   * that it is flush with the screen. A negative value indicates unknown
   * tilt.
   */
  tilt: number;

  /**
   * The angle between the projection of the stylus onto the screen and the
   * positive x-axis, measured counter-clockwise. This is expected to lie in
   * the range [0, 2π). A negative value indicates unknown orientation.
   */
  orientation: number;
}

/**
 * These parameters are used for modeling the position of the pen.
 */
interface PositionModelerParams {
  /**
   * The mass of the "weight" being pulled along the path, multiplied by the
   * spring constant.
   */
  springMassConstant: number;

  /**
   * The ratio of the pen's velocity that is subtracted from the pen's
   * acceleration, to simulate drag.
   */
  dragConstant: number;
}

/**
 * These parameters are used for sampling.
 */
interface SamplingParams {
  /**
   * The minimum number of modeled inputs to output per unit time. If inputs are
   * received at a lower rate, they will be upsampled to produce output of at
   * least minOutputRate. If inputs are received at a higher rate, the
   * output rate will match the input rate.
   */
  minOutputRate: number;

  /**
   * This determines stop condition for end-of-stroke modeling; if the position
   * is within this distance of the final raw input, or if the last update
   * iteration moved less than this distance, it stop iterating.
   *
   * This should be a small distance; a good starting point is 2-3 orders of
   * magnitude smaller than the expected distance between input points.
   */
  endOfStrokeStoppingDistance: number;

  /**
   * The maximum number of iterations to perform at the end of the stroke, if it
   * does not stop due to the constraints of endOfStrokeStoppingDistance.
   */
  endOfStrokeMaxIterations: number;

  /**
   * Maximum number of outputs to generate per call to Update or Predict.
   * This limit avoids crashes if input events are recieved with too long of
   * a time between, possibly because a client was suspended and resumed.
   */
  maxOutputsPerCall: number;
}

/**
 * These parameters are used modeling the state of the stylus once the position
 * has been modeled.
 */
interface StylusStateModelerParams {
  /**
   * The maximum number of raw inputs to look at when finding the nearest states
   * for interpolation.
   */
  maxInputSamples: number;
}

/**
 * These parameters are used for applying smoothing to the input to reduce
 * wobble in the prediction.
 */
interface WobbleSmootherParams {
  /**
   * The length of the window over which the moving average of speed and
   * position are calculated.
   *
   * A good starting point is 2.5 divided by the expected number of inputs per
   * unit time.
   */
  timeout: number;

  /**
   * The range of speeds considered for wobble smoothing. At speedFloor, the
   * maximum amount of smoothing is applied. At speedCeilingVno smoothing is
   * applied.
   *
   * Good starting points are 2% and 3% of the expected speed of the inputs.
   */
  speedFloor: number;
  speedCeiling: number;
}

/**
 * This struct indicates the "stroke end" prediction strategy should be used,
 * which models a prediction as though the last seen input was the
 * end-of-stroke. There aren't actually any tunable parameters for this; it uses
 * the same PositionModelerParams and SamplingParams as the overall model. Note
 * that this "prediction" doesn't actually predict substantially into the
 * future, it only allows for very quickly "catching up" to the position of the
 * raw input.
 */
interface StrokeEndPredictorParams {}

/**
 * This struct indicates that the Kalman filter-based prediction strategy should
 * be used, and provides the parameters for tuning it.
 *
 * Unlike the "stroke end" predictor, this strategy can predict an extension
 * of the stroke beyond the last Input position, in addition to the "catch up"
 * step.
 */
interface KalmanPredictorParams {
  /**
   * The variance of the noise inherent to the stroke itself.
   */
  processNoise: number;

  /**
   * The variance of the noise that rises from errors in measurement of the
   * stroke.
   */
  measurementNoise: number;

  /**
   * The minimum number of inputs received before the Kalman predictor is
   * considered stable enough to make a prediction.
   */
  minStableIteration: number;

  /**
   * The Kalman filter assumes that input is received in uniform time steps, but
   * this is not always the case. We hold on to the most recent input timestamps
   * for use in calculating the correction for this. This determines the maximum
   * number of timestamps to save.
   */
  maxTimeSamples: number;

  /**
   * The minimum allowed velocity of the "catch up" portion of the prediction,
   * which covers the distance between the last Result (the last corrected
   * position) and the
   *
   * A good starting point is 3 orders of magnitude smaller than the expected
   * speed of the inputs.
   */
  minCatchupVelocity: number;

  /**
   * These weights are applied to the acceleration (x²) and jerk (x³) terms of
   * the cubic prediction polynomial. The closer they are to zero, the more
   * linear the prediction will be.
   */
  accelerationWeight: number;
  jerkWeight: number;

  /**
   * This value is a hint to the predictor, indicating the desired duration of
   * of the portion of the prediction extending beyond the position of the last
   * input. The actual duration of that portion of the prediction may be less
   * than this, based on the predictor's confidence, but it will never be
   * greater.
   */
  predictionInterval: number;

  confidenceParams: ConfidenceParams;
}

/**
 * The Kalman predictor uses several heuristics to evaluate confidence in the
 * prediction. Each heuristic produces a confidence value between 0 and 1, and
 * then we take their product as the total confidence.
 * These parameters may be used to tune those heuristics.
 */
interface ConfidenceParams {
  /**
   * The first heuristic simply increases confidence as we receive more sample
   * (i.e. input points). It evaluates to 0 at no samples, and 1 at
   * desiredNumberOfSamples.
   */
  desiredNumberOfSamples: number;

  /**
   * The second heuristic is based on the distance between the last sample
   * and the current estimate. If the distance is 0, it evaluates to 1, and if
   * the distance is greater than or equal to maxEstimationDistance, it
   * evaluates to 0.
   *
   * A good starting point is 1.5 times measurementNoise.
   */
  maxEstimationDistance: number;

  /**
   * The third heuristic is based on the speed of the prediction, which is
   * approximated by measuring the from the start of the prediction to the
   * projected endpoint (if it were extended for the full
   * predictionInterval). It evaluates to 0 at minTravelSpeed, and 1
   * at maxTravelSpeed.
   *
   * Good starting points are 5% and 25% of the expected speed of the inputs.
   */
  minTravelSpeed: number;
  maxTravelSpeed: number;

  /**
   * The fourth heuristic is based on the linearity of the prediction, which
   * is approximated by comparing the endpoint of the prediction with the
   * endpoint of a linear prediction (again, extended for the full
   * predictionInterval). It evaluates to 1 at zero distance, and
   * baselineLinearityConfideVe at a distance of maxLinearDeviation.
   *
   * A good starting point is an 10 times the measurementNoise.
   */
  maxLinearDeviation: number;
  baselineLinearityConfidence: number;
}

interface PredictionParams extends StrokeEndPredictorParams, KalmanPredictorParams {}

/**
 * Temporary params governing experimental changes in behavior. Any params
 * here may be removed without warning in a future release.
 */
interface ExperimentalParams {}

/**
 * This convenience struct is a collection of the parameters for the individual
 * parameter structs.
 */
interface StrokeModelParams {
  wobbleSmootherParams: WobbleSmootherParams;
  positionModelerParams: PositionModelerParams;
  samplingParams: SamplingParams;
  stylusStateModelerParams: StylusStateModelerParams;
  predictionParams: PredictionParams;
  experimentalParams: ExperimentalParams;
}

/*
 * This struct contains the position and velocity of the modeled pen tip at
 * the indicated time.
 */
interface TipState {
  position: vec2;
  velocity: vec2;
  time: number;
}

/*
 * This struct contains information about the state of the stylus. See the
 * corresponding fields on the PencilInput struct for more info.
 */
interface StylusState {
  pressure: number;
  tilt: number;
  orientation: number;
}

/**
 * Interface for input predictors that generate points based on past input.
 */
interface InputPredictor {
  /**
   * Resets the predictor's internal model.
   */
  reset(): void;

  /**
   * Updates the predictor's internal model with the given input.
   */
  update(position: vec2, time: number): void;

  /**
   * Constructs a prediction into the output parameter based on the given
   * last_state, based on the predictor's internal model. The result may be
   * empty if the predictor has not yet accumulated enough data, via Update(),
   * to construct a reasonable prediction.
   *
   * Subclasses are expected to maintain the following invariants:
   * - The prediction parameter is expected to be cleared by this function.
   * - The given state must not appear in the prediction.
   * - The time delta between each state in the prediction, and between the
   *   given state and the first predicted state, must conform to
   *   SamplingParams::min_output_rate.
   */
  constructPrediction(lastState: TipState, prediction: TipState[]): void;
}
