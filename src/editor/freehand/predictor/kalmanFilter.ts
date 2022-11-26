import { mat4, vec4 } from '@/math';

/**
 * Generates a state estimation based upon observations which can then be used
 * to compute predicted values.
 */
class KalmanFilter {
  /**
   * Estimate of the latent state
   * Symbol: X
   * Dimension: state_vector_dim_
   */
  private m_stateEstimation: vec4 = vec4.create();

  /**
   * The covariance of the difference between prior predicted latent
   * state and posterior estimated latent state (the so-called "innovation".
   * Symbol: P
   */
  private m_errorCovarianceMatrix: mat4;

  /**
   * For position, state transition matrix is derived from basic physics:
   * new_x = x + v * dt + 1/2 * a * dt^2 + 1/6 * jerk * dt^3
   * new_v = v + a * dt + 1/2 * jerk * dt^2
   * ...
   * Matrix that transmit current state to next state
   * Symbol: F
   */
  private m_stateTransitionMatrix: mat4;

  /**
   * Process_noise_covariance_matrix_ is a time-varying parameter that will be
   * estimated as part of the Kalman filter process.
   * Symbol: Q
   */
  private m_processNoiseCovarianceMatrix: mat4;

  /**
   * Vector to transform estimate to measurement.
   * Symbol: H
   */
  private m_measurementVector: vec4 = vec4.create();

  /**
   * measurement_noise_ is a time-varying parameter that will be estimated as
   * part of the Kalman filter process.
   * Symbol: R
   */
  private m_measurementNoiseVariance: number;

  /**
   * The first iteration at which the Kalman filter is considered stable enough
   * to make a good estimate of the state.
   */
  private m_minStableIteration: number;

  /**
   * Tracks the number of update iterations that have occurred.
   */
  private m_iterNum: number;

  constructor(
    state_transition: mat4,
    process_noise_covariance: mat4,
    measurement_vector: vec4,
    measurement_noise_variance: number,
    min_stable_iteration: number
  ) {
    this.m_stateTransitionMatrix = state_transition;
    this.m_processNoiseCovarianceMatrix = process_noise_covariance;
    this.m_measurementVector = measurement_vector;
    this.m_measurementNoiseVariance = measurement_noise_variance;
    this.m_minStableIteration = min_stable_iteration;
    this.m_iterNum = 0;
  }

  /**
   * Get the estimation of current state.
   */
  get stateEstimation(): vec4 {
    return this.m_stateEstimation;
  }

  /**
   * Will return true only if the Kalman filter has seen enough data and is
   * considered as stable.
   */
  get stable(): boolean {
    return this.m_iterNum >= this.m_minStableIteration;
  }

  /**
   * Returns the number of times Update() has been called since the last time
   * the KalmanFilter was reset.
   */
  get numIterations(): number {
    return this.m_iterNum;
  }

  private predict(): void {
    // X = F * X
    this.m_stateEstimation = vec4.mulMat4(this.m_stateEstimation, this.m_stateTransitionMatrix);
    // P = F * P * F' + Q
    this.m_errorCovarianceMatrix = mat4.add(
      mat4.mul(
        this.m_stateTransitionMatrix,
        mat4.mul(this.m_errorCovarianceMatrix, mat4.transpose(this.m_stateTransitionMatrix))
      ),
      this.m_processNoiseCovarianceMatrix
    );
  }

  /**
   * Update the observation of the system.
   */
  update(observation: number): void {
    if (this.m_iterNum++ == 0) {
      // We only update the state estimation in the first iteration.
      this.m_stateEstimation[0] = observation;
      return;
    }
    this.predict();
    // Y = z - H * X
    const y = observation - vec4.dot(this.m_measurementVector, this.m_stateEstimation);
    // S = H * P * H' + R
    const S =
      vec4.dot(
        vec4.mulMat4(this.m_measurementVector, this.m_errorCovarianceMatrix),
        this.m_measurementVector
      ) + this.m_measurementNoiseVariance;
    // K = P * H' * inv(S)
    const kalman_gain = vec4.divS(
      vec4.mulMat4(this.m_measurementVector, this.m_errorCovarianceMatrix),
      S
    );

    // X = X + K * Y
    this.m_stateEstimation = vec4.mul(this.m_stateEstimation, vec4.mulS(kalman_gain, y));

    // I_HK = eye(P) - K * H
    const I_KH = mat4.subtract(mat4.create(), vec4.outer(kalman_gain, this.m_measurementVector));

    // P = I_KH * P * I_KH' + K * R * K'
    this.m_errorCovarianceMatrix = mat4.add(
      mat4.mul(mat4.mul(I_KH, this.m_errorCovarianceMatrix), mat4.transpose(I_KH)),
      mat4.mulS(vec4.outer(kalman_gain, kalman_gain), this.m_measurementNoiseVariance)
    );
  }

  reset(): void {
    this.m_stateEstimation = vec4.create();
    this.m_errorCovarianceMatrix = mat4.create(); // identity
    this.m_iterNum = 0;
  }
}

export default KalmanFilter;
