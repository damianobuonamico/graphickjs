import { mat4, vec4 } from '@/math';
import KalmanFilter from './kalmanFilter';

const kPositionIndex = 0;
const kVelocityIndex = 1;
const kAccelerationIndex = 2;
const kJerkIndex = 3;

const kDt = 1;
const kDtSquared = kDt * kDt;
const kDtCubed = kDt * kDt * kDt;

/**
 * Class to predict on axis.
 *
 * This predictor use one instance of Kalman filter to predict one dimension of
 * stylus movement.
 */
class AxisPredictor {
  private m_kalmanFilter: KalmanFilter;

  constructor(process_noise: number, measurement_noise: number, min_stable_iteration: number) {
    // State translation matrix is basic physics.
    // new_pos = pre_pos + v * dt + 1/2 * a * dt^2 + 1/6 * J * dt^3.
    // new_v = v + a * dt + 1/2 * J * dt^2.
    // new_a = a + J * dt.
    // new_j = J.
    const state_transition: mat4 = [
      1,
      kDt,
      0.5 * kDtSquared,
      (1 / 6) * kDtCubed,
      0,
      1,
      kDt,
      0.5 * kDtSquared,
      0,
      0,
      1,
      kDt,
      0,
      0,
      0,
      1
    ];

    // We model the system noise as noisy force on the pen.
    // The following matrix describes the impact of that noise on each state.
    const process_noise_vector: vec4 = [(1 / 6) * kDtCubed, 0.5 * kDtSquared, kDt, 1];
    const process_noise_covariance: mat4 = mat4.mulS(
      vec4.outer(process_noise_vector, process_noise_vector),
      process_noise
    );

    // Sensor only detects location. Thus measurement only impact the position.
    const measurement_vector: vec4 = [1, 0, 0, 0];

    this.m_kalmanFilter = new KalmanFilter(
      state_transition,
      process_noise_covariance,
      measurement_vector,
      measurement_noise,
      min_stable_iteration
    );
  }

  /**
   * Return true if the underlying Kalman filter is stable.
   */
  get stable(): boolean {
    return this.m_kalmanFilter && this.m_kalmanFilter.stable;
  }

  /**
   * Returns the number of times Update() has been called since the last time
   * the AxisPredictor was reset.
   */
  get numIterations(): number {
    return this.m_kalmanFilter ? this.m_kalmanFilter.numIterations : 0;
  }

  /**
   * Get the predicted values from the underlying Kalman filter.
   */
  get position(): number {
    if (this.m_kalmanFilter) return this.m_kalmanFilter.stateEstimation[kPositionIndex];
    else return 0;
  }

  get velocity(): number {
    if (this.m_kalmanFilter) return this.m_kalmanFilter.stateEstimation[kVelocityIndex];
    else return 0.0;
  }

  get acceleration(): number {
    return this.m_kalmanFilter.stateEstimation[kAccelerationIndex];
  }

  get jerk(): number {
    return this.m_kalmanFilter.stateEstimation[kJerkIndex];
  }

  /**
   * Reset the underlying Kalman filter.
   */
  reset(): void {
    if (this.m_kalmanFilter) this.m_kalmanFilter.reset();
  }

  /**
   * Update the predictor with a new observation.
   */
  update(observation: number): void {
    if (this.m_kalmanFilter) this.m_kalmanFilter.update(observation);
  }
}

export default AxisPredictor;
