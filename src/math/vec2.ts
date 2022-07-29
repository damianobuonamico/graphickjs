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

/**
 * Creates a new vec2 initialized with values from an existing vector
 *
 * @param {ReadonlyVec2} a vector to clone
 * @returns {vec2} a new 2D vector
 */
export function clone(a: ReadonlyVec2): vec2 {
  const out = create();
  out[0] = a[0];
  out[1] = a[1];
  return out;
}

/**
 * Subtracts vector b from vector a
 *
 * @param {ReadonlyVec2} a the first operand
 * @param {ReadonlyVec2 | number} b the second operand
 * @param {boolean} self is a the receiving vector
 * @returns {vec2} out
 */
export function subtract(
  a: ReadonlyVec2,
  b: ReadonlyVec2 | number,
  self: boolean = false
): vec2 {
  const out = self ? a : create();
  const [b0, b1] = typeof b === 'number' ? [b, b] : [b[0], b[1]];
  out[0] = a[0] - b0;
  out[1] = a[1] - b1;
  return out;
}

/**
 * Multiplies two vec2's
 *
 * @param {ReadonlyVec2} a the first operand
 * @param {ReadonlyVec2 | number} b the second operand
 * @param {boolean} self is a the receiving vector
 * @returns {vec2} out
 */
export function multiply(
  a: ReadonlyVec2,
  b: ReadonlyVec2 | number,
  self: boolean = false
): vec2 {
  const out = self ? a : create();
  const [b0, b1] = typeof b === 'number' ? [b, b] : [b[0], b[1]];
  out[0] = a[0] * b0;
  out[1] = a[1] * b1;
  return out;
}

/**
 * Joins an array of vectors into a single vector
 *
 * @param {vec2[]} a vector to negate
 * @returns {Float32Array} out
 */
export function join(a: vec2[]): Float32Array {
  const out = new Float32Array(a.length * 2);
  for (let i = 0; i < a.length * 2; i++) {
    out[i] = a[Math.floor(i / 2)][i % 2];
  }
  return out;
}

/**
 * Alias for {@link vec2.subtract}
 * @function
 */
export const sub = subtract;

/**
 * Alias for {@link vec2.multiply}
 * @function
 */
export const mul = multiply;
