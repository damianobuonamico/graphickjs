import { lerp, vec2 } from '@/math';
import { normalize01 } from '../utils';
import AxisPredictor from './axisPredictor';

function evaluateCubic(startState: KalmanState, deltaTime: number): KalmanState {
  const dt = deltaTime;
  const dtSquared = dt * dt;
  const dtCubed = dtSquared * dt;

  const endState: KalmanState = {
    position: vec2.add(
      vec2.add(
        vec2.add(startState.position, vec2.mulS(startState.velocity, dt)),
        vec2.mulS(startState.acceleration, dtSquared / 2)
      ),
      vec2.mulS(startState.jerk, dtCubed / 6)
    ),
    velocity: vec2.add(
      vec2.add(startState.velocity, vec2.mulS(startState.acceleration, dt)),
      vec2.mulS(startState.jerk, dtSquared / 2)
    ),
    acceleration: vec2.add(startState.acceleration, vec2.mulS(startState.jerk, dt)),
    jerk: startState.jerk
  };

  return endState;
}

/**
 * This class constructs a prediction by using a pair of Kalman filters (one
 * each for the x- and y-dimension) to model the true state of the tip, assuming
 * that the data we receive contains some noise.
 * To construct a prediction, we first fetch the estimation of the position,
 * velocity, acceleration, and jerk from the Kalman filters. The prediction is
 * then constructed in two parts: one cubic spline that connects the last tip
 * state to the estimation, constructed from the positions and velocities at the
 * endpoints; and one cubic spline that extends into the future, constructed
 * from the estimated position, velocity, acceleration, and jerk.
 */
class KalmanPredictor implements InputPredictor {
  private m_predictorParams: KalmanPredictorParams;
  private m_samplingParams: SamplingParams;

  private m_lastPositionReceived: vec2 | null;

  private m_sampleTimes: number[] = [];

  private m_xPredictor: AxisPredictor;
  private m_yPredictor: AxisPredictor;

  constructor(predictorParams: KalmanPredictorParams, samplingParams: SamplingParams) {
    this.m_predictorParams = predictorParams;
    this.m_samplingParams = samplingParams;

    this.m_xPredictor = new AxisPredictor(
      predictorParams.processNoise,
      predictorParams.measurementNoise,
      predictorParams.minStableIteration
    );
    this.m_yPredictor = new AxisPredictor(
      predictorParams.processNoise,
      predictorParams.measurementNoise,
      predictorParams.minStableIteration
    );
  }

  get estimatedState(): KalmanState | null {
    if (!this.isStable() || !this.m_sampleTimes.length) return null;

    const estimatedState: KalmanState = {
      position: [this.m_xPredictor.position, this.m_yPredictor.position],
      velocity: [this.m_xPredictor.velocity, this.m_yPredictor.velocity],
      acceleration: [this.m_xPredictor.acceleration, this.m_yPredictor.acceleration],
      jerk: [this.m_xPredictor.jerk, this.m_yPredictor.jerk]
    };

    // The axis predictors are not time-aware, assuming that the time delta
    // between measurements is always 1. To correct for this, we divide the
    // velocity, acceleration, and jerk by the average observed time delta, raised
    // to the appropriate power.
    const dt =
      (this.m_sampleTimes[this.m_sampleTimes.length - 1] - this.m_sampleTimes[0]) /
      this.m_sampleTimes.length;
    const dtSquared = dt * dt;
    const dtCubed = dtSquared * dt;

    vec2.divS(estimatedState.velocity, dt, estimatedState.velocity);
    vec2.divS(estimatedState.acceleration, dtSquared, estimatedState.acceleration);
    vec2.divS(estimatedState.jerk, dtCubed, estimatedState.jerk);

    // We want our predictions to tend more towards linearity -- to achieve this,
    // we reduce the acceleration and jerk.
    vec2.mulS(
      estimatedState.acceleration,
      this.m_predictorParams.accelerationWeight,
      estimatedState.acceleration
    );
    vec2.mulS(estimatedState.jerk, this.m_predictorParams.jerkWeight, estimatedState.jerk);

    return estimatedState;
  }

  private isStable(): boolean {
    return this.m_xPredictor.stable && this.m_xPredictor.stable;
  }

  private constructCubicConnector(
    lastTipState: TipState,
    estimatedState: KalmanState,
    params: KalmanPredictorParams,
    sampleDt: number,
    output: TipState[]
  ): void {
    // Estimate how long it will take for the tip to travel from its last position
    // to the estimated position, based on the start and end velocities. We define
    // a minimum "reasonable" velocity to avoid division by zero.
    const distanceTraveled = vec2.dist(lastTipState.position, estimatedState.position);
    const maxVelocityAtEnds = Math.max(
      vec2.length(lastTipState.velocity),
      vec2.length(estimatedState.velocity)
    );
    const targetDuration =
      distanceTraveled / Math.max(maxVelocityAtEnds, params.minCatchupVelocity);

    // Determine how many samples this will give us, ensuring that there's always
    // at least one. Then, pick a duration that's a multiple of the sample dt.
    const nPoints = Math.max(Math.ceil(targetDuration / sampleDt), 1);
    const duration = nPoints * sampleDt;

    // We want to construct a cubic curve connecting the last tip state and the
    // estimated state. Given positions p₀ and p₁, velocities v₀ and v₁, and times
    // t₀ and t₁ at the start and end of the curve, we define a pair of functions,
    // f and g, such that the curve is described by the composite function
    // f(g(t)):
    //   f(x) = ax³ + bx² + cx + d
    //   g(t) = (t - t₀) / (t₁ - t₀)
    // We then find the derivatives:
    //   f'(x) = 3ax² + 2bx + c
    //   g'(t) = 1 / (t₁ - t₀)
    //   (f∘g)'(t) = f'(g(t)) ⋅ g'(t) = (3ax² + 2bx + c) / (t₁ - t₀)
    // We then plug in the given values:
    //   f(g(t₀)) = f(0) = p₀
    //   ax³ + bx² + cx + d
    //   f(g(t₁)) = f(1) = p₁
    //   (f∘g)'(t₀) = f'(0) ⋅ g'(t₀) = v₀
    //   (f∘g)'(t₁) = f'(1) ⋅ g'(t₁) = v₁
    // This gives us four linear equations:
    //   a⋅0³ + b⋅0² + c⋅0 + d = p₀
    //   a⋅1³ + b⋅1² + c⋅1 + d = p₁
    //   (3a⋅0² + 2b⋅0 + c) / (t₁ - t₀) = v₀
    //   (3a⋅1² + 2b⋅1 + c) / (t₁ - t₀) = v₁
    // Finally, we can solve for a, b, c, and d:
    //   a = 2p₀ - 2p₁ + (v₀ + v₁)(t₁ - t₀)
    //   b = -3p₀ + 3p₁ - (2v₀ + v₁)(t₁ - t₀)
    //   c = v₀(t₁ - t₀)
    //   d = p₀
    const floatDuration = duration;
    const a = vec2.add(
      vec2.sub(vec2.mulS(lastTipState.position, 2), vec2.mulS(estimatedState.position, 2)),
      vec2.mulS(vec2.add(lastTipState.velocity, estimatedState.velocity), floatDuration)
    );

    const b = vec2.sub(
      vec2.add(vec2.mulS(lastTipState.position, -3), vec2.mulS(estimatedState.position, 3)),
      vec2.mulS(
        vec2.add(vec2.mulS(lastTipState.velocity, 2), estimatedState.velocity),
        floatDuration
      )
    );

    const c = vec2.mulS(lastTipState.velocity, floatDuration);

    const d = lastTipState.position;

    for (let i = 1; i <= nPoints; ++i) {
      const t = i / nPoints;
      const tSquared = t * t;
      const tCubed = tSquared * t;
      const position = vec2.add(
        vec2.add(vec2.mulS(a, tCubed), vec2.mulS(b, tSquared)),
        vec2.add(vec2.mulS(c, t), d)
      );
      const velocity = vec2.add(vec2.add(vec2.mulS(a, 3 * tSquared), vec2.mulS(b, 2 * t)), c);
      const time = lastTipState.time + duration * t;
      output.push({ position, velocity: vec2.divS(velocity, floatDuration), time });
    }
  }

  private constructCubicPrediction(
    estimatedState: KalmanState,
    params: KalmanPredictorParams,
    startTime: number,
    sampleDt: number,
    nSamples: number,
    output: TipState[]
  ): void {
    let currentState = estimatedState;
    let currentTime = startTime;

    for (let i = 0; i < nSamples; ++i) {
      const nextState = evaluateCubic(currentState, sampleDt);
      currentTime += sampleDt;

      output.push({
        position: nextState.position,
        velocity: nextState.velocity,
        time: currentTime
      });

      currentState = nextState;
    }
  }

  private numberOfPointsToPredict(estimatedState: KalmanState): number {
    const confidenceParams = this.m_predictorParams.confidenceParams;

    const targetNumber =
      this.m_predictorParams.predictionInterval * this.m_samplingParams.minOutputRate;

    // The more samples we've received, the less effect the noise from each
    // individual input affects the result.
    const sampleRatio = Math.min(
      1,
      this.m_xPredictor.numIterations / confidenceParams.desiredNumberOfSamples
    );

    // The further the last given position is from the estimated position, the
    // less confidence we have in the result.
    const estimatedError = vec2.dist(
      this.m_lastPositionReceived || [0, 0],
      estimatedState.position
    );
    const normalizedError =
      1 - normalize01(0, confidenceParams.maxEstimationDistance, estimatedError);

    // This is the state that the prediction would end at if we predicted the full
    // interval (i.e. if confidence == 1).
    const endState = evaluateCubic(estimatedState, this.m_predictorParams.predictionInterval);

    // If the prediction is not traveling quickly, then changes in direction
    // become more apparent, making the prediction appear wobbly.
    const travelSpeed =
      vec2.dist(estimatedState.position, endState.position) /
      this.m_predictorParams.predictionInterval;
    const normalizedDistance = normalize01(
      confidenceParams.minTravelSpeed,
      confidenceParams.maxTravelSpeed,
      travelSpeed
    );

    // If the actual prediction differs too much from the linear prediction, it
    // suggests that the acceleration and jerk components overtake the velocity,
    // resulting in a prediction that flies far off from the stroke.
    const deviationFromLinearPrediction = vec2.dist(
      endState.position,
      vec2.add(
        estimatedState.position,
        vec2.mulS(estimatedState.velocity, this.m_predictorParams.predictionInterval)
      )
    );
    const linearity = lerp(
      confidenceParams.baselineLinearityConfidence,
      1,
      1 - normalize01(0, confidenceParams.maxLinearDeviation, deviationFromLinearPrediction)
    );

    const confidence = sampleRatio * normalizedError * normalizedDistance * linearity;
    return Math.ceil(targetNumber * confidence);
  }

  reset(): void {
    this.m_xPredictor.reset();
    this.m_xPredictor.reset();
    this.m_sampleTimes.length = 0;
    this.m_lastPositionReceived = null;
  }

  update(position: vec2, time: number): void {
    this.m_lastPositionReceived = position;
    this.m_sampleTimes.push(time);

    if (
      this.m_predictorParams.maxTimeSamples < 0 ||
      this.m_sampleTimes.length > this.m_predictorParams.maxTimeSamples
    )
      this.m_sampleTimes.shift();

    this.m_xPredictor.update(position[0]);
    this.m_xPredictor.update(position[1]);
  }

  constructPrediction(lastState: TipState, prediction: TipState[]): void {
    prediction.length = 0;

    const estimatedState = this.estimatedState;

    console.log(prediction);

    if (!estimatedState || !this.m_lastPositionReceived) {
      // We don't yet have enough data to construct a prediction.
      return;
    }

    const sampleDt = 1 / this.m_samplingParams.minOutputRate;

    this.constructCubicConnector(
      lastState,
      estimatedState,
      this.m_predictorParams,
      sampleDt,
      prediction
    );

    const startTime = !prediction.length ? lastState.time : prediction[prediction.length - 1].time;

    this.constructCubicPrediction(
      estimatedState,
      this.m_predictorParams,
      startTime,
      sampleDt,
      this.numberOfPointsToPredict(estimatedState),
      prediction
    );
  }
}

export default KalmanPredictor;
