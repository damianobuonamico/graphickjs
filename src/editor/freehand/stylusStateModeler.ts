import { lerp, lerpAngle, vec2 } from '@/math';
import { nearestPointOnSegment } from './utils';

/**
 * This class is used to model the state of the stylus for a given position,
 * based on the state of the stylus at the original input points.
 *
 * The stylus is modeled by storing the last max_input_samples positions and
 * states received via Update(); when queried, it treats the stored positions as
 * a polyline, and finds the closest segment. The returned stylus state is a
 * linear interpolation between the states associated with the endpoints of the
 * segment, correcting angles to account for the "wraparound" that occurs at 0
 * and 2π. The value used for interpolation is based on how far along the
 * segment the closest point lies.
 *
 * If Update() is called with a state in which a field (i.e. pressure, tilt, or
 * orientation) has a negative value (indicating no information), then the
 * results of Query() will be -1 for that field until Reset() is called. This is
 * tracked independently for each field; e.g., if you pass in tilt = -1, then
 * pressure and orientation will continue to be interpolated normally.
 */
class StylusStateModeler {
  private m_receivedUnknownPressure: boolean = false;
  private m_receivedUnknownTilt: boolean = false;
  private m_receivedUnknownOrientation: boolean = false;

  private m_positionsAndStates: PositionAndState[] = [];
  private m_params: StylusStateModelerParams;

  /**
   * Clear the model and reset.
   */
  reset(params: StylusStateModelerParams): void {
    this.m_params = params;
    this.m_positionsAndStates.length = 0;
    this.m_receivedUnknownPressure = false;
    this.m_receivedUnknownTilt = false;
    this.m_receivedUnknownOrientation = false;
  }

  /**
   * Adds a position and state pair to the model. During stroke modeling, these
   * values will be taken from the raw input.
   */
  update(position: vec2, state: StylusState): void {
    // Possibly NaN should be prohibited in ValidateInput, but due to current
    // consumers, that can't be tightened for these values currently.
    if (state.pressure < 0 || state.pressure === undefined) {
      this.m_receivedUnknownPressure = true;
    }
    if (state.tilt < 0 || state.tilt === undefined) {
      this.m_receivedUnknownTilt = true;
    }
    if (state.orientation < 0 || state.orientation === undefined) {
      this.m_receivedUnknownOrientation = true;
    }

    if (
      this.m_receivedUnknownPressure &&
      this.m_receivedUnknownTilt &&
      this.m_receivedUnknownOrientation
    ) {
      // We've stopped tracking all fields, so there's no need to keep updating.
      this.m_positionsAndStates.length = 0;
      return;
    }

    this.m_positionsAndStates.push({ position, state });

    if (
      this.m_params.maxInputSamples < 0 ||
      this.m_positionsAndStates.length > this.m_params.maxInputSamples
    ) {
      this.m_positionsAndStates.shift();
    }
  }

  /**
   * Query the model for the state at the given position. During stroke
   * modeling, the position will be taken from the modeled input.
   *
   * If no Update() calls have been received since the last Reset(), this will
   * return {.pressure = -1, .tilt = -1, .orientation = -1}.
   */
  query(position: vec2): StylusState {
    if (!this.m_positionsAndStates.length) return { pressure: -1, tilt: -1, orientation: -1 };

    let closestSegmentIndex = -1;
    let minDistance = Infinity;
    let interpValue = 0;

    for (let i = 0; i < this.m_positionsAndStates.length - 1; ++i) {
      const segmentStart = this.m_positionsAndStates[i].position;
      const segmentEnd = this.m_positionsAndStates[i + 1].position;
      const param = nearestPointOnSegment(segmentStart, segmentEnd, position);
      const distance = vec2.dist(position, vec2.lerp(segmentStart, segmentEnd, param));

      if (distance <= minDistance) {
        closestSegmentIndex = i;
        minDistance = distance;
        interpValue = param;
      }
    }

    if (closestSegmentIndex < 0) {
      const state = this.m_positionsAndStates[0].state;
      return {
        pressure: this.m_receivedUnknownPressure ? -1 : state.pressure,
        tilt: this.m_receivedUnknownTilt ? -1 : state.tilt,
        orientation: this.m_receivedUnknownOrientation ? -1 : state.orientation
      };
    }

    const from_state = this.m_positionsAndStates[closestSegmentIndex].state;
    const to_state = this.m_positionsAndStates[closestSegmentIndex + 1].state;
    return {
      pressure: this.m_receivedUnknownPressure
        ? -1
        : lerp(from_state.pressure, to_state.pressure, interpValue),
      tilt: this.m_receivedUnknownTilt ? -1 : lerp(from_state.tilt, to_state.tilt, interpValue),
      orientation: this.m_receivedUnknownOrientation
        ? -1
        : lerpAngle(from_state.orientation, to_state.orientation, interpValue)
    };
  }
}

export default StylusStateModeler;
