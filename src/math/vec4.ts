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
