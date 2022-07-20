import { vec2 } from '.';

/**
 * Creates a new, empty vec2
 *
 * @returns {vec2} a new 2D vector
 */
export function create(): vec2 {
  return new Float32Array(2);
}

/**
 * Creates a new vec2 initialized with the given values
 *
 * @param {Number} x X component
 * @param {Number} y Y component
 * @returns {vec2} a new 2D vector
 */
export function fromValues(x: number, y: number): vec2 {
  const out = create();
  out[0] = x;
  out[1] = y;
  return out;
}
