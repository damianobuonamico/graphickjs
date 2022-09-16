/**
 * Creates a new, empty vec4
 *
 * @returns {vec4} a new 4D vector
 */
export function create(): vec4 {
  return new Float32Array(4);
}

/**
 * Creates a new vec4 initialized with values from an existing vector
 *
 * @param {ReadonlyVec4} a vector to clone
 * @returns {vec4} a new 4D vector
 */
export function clone(a: ReadonlyVec4): vec4 {
  const out = create();
  out[0] = a[0];
  out[1] = a[1];
  out[2] = a[2];
  out[3] = a[3];
  return out;
}

/**
 * Creates a new vec4 initialized with the given values
 *
 * @param {Number} x X component
 * @param {Number} y Y component
 * @param {Number} z Z component
 * @param {Number} w W component
 * @returns {vec4} a new 4D vector
 */
export function fromValues(x: number, y: number, z: number, w: number): vec4 {
  let out = create();
  out[0] = x;
  out[1] = y;
  out[2] = z;
  out[3] = w;
  return out;
}

/**
 * Checks if two vectors are equal
 *
 * @param {ReadonlyVec4} a the first operand
 * @param {ReadonlyVec4} b the second operand
 * @returns {boolean} out
 */
export function equals(a: ReadonlyVec4, b: ReadonlyVec4): boolean {
  return a[0] === b[0] && a[1] === b[1] && a[2] === b[2] && a[3] === b[3];
}
