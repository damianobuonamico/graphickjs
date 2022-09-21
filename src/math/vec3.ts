/**
 * Creates a new, empty vec3
 *
 * @returns {vec3} a new 3D vector
 */
export function create(): vec3 {
  return new Float32Array(3);
}

/**
 * Creates a new vec3 initialized with values from an existing vector
 *
 * @param {ReadonlyVec3} a vector to clone
 * @returns {vec3} a new 4D vector
 */
export function clone(a: ReadonlyVec3): vec3 {
  const out = new Float32Array(3);
  out[0] = a[0];
  out[1] = a[1];
  out[2] = a[2];
  return out;
}

/**
 * Creates a new vec3 initialized with the given values
 *
 * @param {Number} x X component
 * @param {Number} y Y component
 * @param {Number} z Z component
 * @returns {vec3} a new 3D vector
 */
export function fromValues(x: number, y: number, z: number): vec3 {
  let out = create();
  out[0] = x;
  out[1] = y;
  out[2] = z;
  return out;
}
