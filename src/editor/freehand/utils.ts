import { vec2 } from '@/math';

/**
 * Clamps the given value to the range [0, 1].
 */
export function clamp01(value: number): number {
  if (value < 0) return 0;
  if (value > 1) return 1;
  return value;
}

/**
 * Returns the ratio of the difference from `start` to `value` and the
 * difference from `start` to `end`, clamped to the range [0, 1]. If
 * `start` == `end`, returns 1 if `value` > `start`, 0 otherwise.
 */
export function normalize01(start: number, end: number, value: number): number {
  if (start === end) return value > start ? 1 : 0;
  return clamp01((value - start) / (end - start));
}

/**
 * Returns the point on the line segment from `segment_start` to `segment_end`
 * that is closest to `point`, represented as the ratio of the length along the
 * segment.
 */
export function nearestPointOnSegment(segmentStart: vec2, segmentEnd: vec2, point: vec2): number {
  if (vec2.equals(segmentStart, segmentEnd)) return 0;

  const segmentVector = vec2.sub(segmentEnd, segmentStart);
  const projectionVector = vec2.sub(point, segmentStart);

  return clamp01(
    vec2.dot(projectionVector, segmentVector) / vec2.dot(segmentVector, segmentVector)
  );
}
