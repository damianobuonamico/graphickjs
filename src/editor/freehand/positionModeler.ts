import { lerp, vec2 } from '@/math';
import { nearestPointOnSegment } from './utils';

/**
 * This class models the movement of the pen tip based on the laws of motion.
 * The pen tip is represented as a mass, connected by a spring to a moving
 * anchor; as the anchor moves, it drags the pen tip along behind it.
 */
class PositionModeler {
  private m_state: TipState;
  private m_params: PositionModelerParams;

  get currentState(): TipState {
    return this.m_state;
  }

  get positionModelerParams(): PositionModelerParams {
    return this.m_params;
  }

  reset(state: TipState, params: PositionModelerParams): void {
    this.m_state = state;
    this.m_params = params;
  }

  /**
   * Given the position of the anchor and the time, updates the model and
   * returns the state of the pen tip.
   */
  update(anchorPosition: vec2, time: number): TipState {
    const deltaTime = time - this.m_state.time;
    const floatDelta = deltaTime;

    const acceleration = vec2.sub(
      vec2.divS(vec2.sub(anchorPosition, this.m_state.position), this.m_params.springMassConstant),
      vec2.mulS(this.m_state.velocity, this.m_params.dragConstant)
    );

    vec2.add(this.m_state.velocity, vec2.mulS(acceleration, floatDelta), this.m_state.velocity);
    vec2.add(
      this.m_state.position,
      vec2.mulS(this.m_state.velocity, floatDelta),
      this.m_state.position
    );
    this.m_state.time = time;

    return this.m_state;
  }

  /**
   * This helper function linearly interpolates between the between the start
   * and end anchor position and time, updating the model at each step and
   * storing the result in the given output iterator.
   *
   * NOTE: Because the expected use case is to repeatedly call this function on
   * a sequence of anchor positions/times, the start position/time is not sent
   * to the model. This prevents us from duplicating those inputs, but it does
   * mean that the first input must be provided on its own, via either Reset()
   * or Update(). This also means that the interpolation values are
   * (1 ... n) / n, as opposed to (0 ... (n - 1)) / (n - 1).
   *
   * Template parameter OutputIt is expected to be an output iterator over
   * TipState.
   */
  updateAlongLinearPath(
    startAnchorPosition: vec2,
    startTime: number,
    endAnchorPosition: vec2,
    endTime: number,
    nSamples: number,
    output: TipState[]
  ): void {
    for (let i = 1; i <= nSamples; ++i) {
      const interp_value = i / nSamples;
      const position = vec2.lerp(startAnchorPosition, endAnchorPosition, interp_value);
      const time = lerp(startTime, endTime, interp_value);

      output.push(this.update(position, time));
    }
  }

  /**
   * This helper function models the end of the stroke, by repeatedly updating
   * with the final anchor position. It attempts to stop at the closest point to
   * the anchor, by checking if it has overshot, and retrying with successively
   * smaller time steps.
   *
   * It halts when any of these three conditions is met:
   * - It has taken more than max_iterations steps (including discarded steps)
   * - The distance between the current state and the anchor is less than
   *   stop_distance
   * - The distance between the previous state and the current state is less
   *   than stop_distance
   *
   * Template parameter OutputIt is expected to be an output iterator over
   * TipState.
   */
  modelEndOfStroke(
    anchor_position: vec2,
    delta_time: number,
    max_iterations: number,
    stop_distance: number,
    output: TipState[]
  ): void {
    for (let i = 0; i < max_iterations; ++i) {
      // The call to Update modifies the state, so we store a copy of the
      // previous state so we can retry with a smaller step if necessary.
      const previous_state: TipState = this.m_state;
      const candidate: TipState = this.update(anchor_position, previous_state.time + delta_time);
      if (vec2.dist(previous_state.position, candidate.position) < stop_distance) {
        // We're no longer making any significant progress, which means that
        // we're about as close as we can get without looping around.
        return;
      }

      const closest_t = nearestPointOnSegment(
        previous_state.position,
        candidate.position,
        anchor_position
      );

      if (closest_t < 1) {
        // We're overshot the anchor, retry with a smaller step.
        delta_time *= 0.5;
        this.m_state = previous_state;
        continue;
      }
      
      output.push(candidate);

      if (vec2.dist(candidate.position, anchor_position) < stop_distance) {
        // We're within tolerance of the anchor.
        return;
      }
    }
  }
}

export default PositionModeler;
