import { MATH_EPSILON } from '@/utils/constants';
import { round as nRound } from './math';

/**
 * 4 Dimensional Vector
 * @module vec4
 */

/**
 * Creates a new, empty vec4
 *
 * @returns {vec4} a new 4D vector
 */
export function create(): vec4 {
  return [0, 0, 0, 0];
}

/**
 * Creates a new vec4 initialized with values from an existing vector
 *
 * @param {ReadonlyVec4} a vector to clone
 * @returns {vec4} a new 4D vector
 */
export function clone(a: ReadonlyVec4): vec4 {
  return [a[0], a[1], a[2], a[3]];
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
  return [x, y, z, w];
}

/**
 * Copy the values from one vec4 to another
 *
 * @param {vec4} out the receiving vector
 * @param {ReadonlyVec4} a the source vector
 * @returns {vec4} out
 */
export function copy(out: vec4, a: ReadonlyVec4): vec4 {
  out[0] = a[0];
  out[1] = a[1];
  out[2] = a[2];
  out[3] = a[3];
  return out;
}

/**
 * Set the components of a vec4 to the given values
 *
 * @param {vec4} out the receiving vector
 * @param {Number} x X component
 * @param {Number} y Y component
 * @param {Number} z Z component
 * @param {Number} w W component
 * @returns {vec4} out
 */
export function set(out: vec4, x: number, y: number, z: number, w: number): vec4 {
  out[0] = x;
  out[1] = y;
  out[2] = z;
  out[3] = w;
  return out;
}

/**
 * Adds two vec4's
 *
 * @param {ReadonlyVec4} a the first operand
 * @param {ReadonlyVec4} b the second operand
 * @param {vec4} out the receiving vector
 * @returns {vec4} out
 */
export function add(a: ReadonlyVec4, b: ReadonlyVec4, out: vec4 = [0, 0, 0, 0]): vec4 {
  out[0] = a[0] + b[0];
  out[1] = a[1] + b[1];
  out[2] = a[2] + b[2];
  out[3] = a[3] + b[3];
  return out;
}

/**
 * Adds a scalar to a vec4
 *
 * @param {ReadonlyVec4} a the first operand
 * @param {Number} b the second operand
 * @param {vec4} out the receiving vector
 * @returns {vec4} out
 */
export function addScalar(a: ReadonlyVec4, b: number, out: vec4 = [0, 0, 0, 0]): vec4 {
  out[0] = a[0] + b;
  out[1] = a[1] + b;
  out[2] = a[2] + b;
  out[3] = a[3] + b;
  return out;
}

/**
 * Subtracts vector b from vector a
 *
 * @param {ReadonlyVec4} a the first operand
 * @param {ReadonlyVec4} b the second operand
 * @param {vec4} out the receiving vector
 * @returns {vec4} out
 */
export function subtract(a: ReadonlyVec4, b: ReadonlyVec4, out: vec4 = [0, 0, 0, 0]): vec4 {
  out[0] = a[0] - b[0];
  out[1] = a[1] - b[1];
  out[2] = a[2] - b[2];
  out[3] = a[3] - b[3];
  return out;
}

/**
 * Subtracts a scalar to a vec4
 *
 * @param {ReadonlyVec4} a the first operand
 * @param {Number} b the second operand
 * @param {vec4} out the receiving vector
 * @returns {vec4} out
 */
export function subtractScalar(a: ReadonlyVec4, b: number, out: vec4 = [0, 0, 0, 0]): vec4 {
  out[0] = a[0] - b;
  out[1] = a[1] - b;
  out[2] = a[2] - b;
  out[3] = a[3] - b;
  return out;
}

/**
 * Multiplies two vec4's
 *
 * @param {ReadonlyVec4} a the first operand
 * @param {ReadonlyVec4} b the second operand
 * @param {vec4} out the receiving vector
 * @returns {vec4} out
 */
export function multiply(a: ReadonlyVec4, b: ReadonlyVec4, out: vec4 = [0, 0, 0, 0]): vec4 {
  out[0] = a[0] * b[0];
  out[1] = a[1] * b[1];
  out[2] = a[2] * b[2];
  out[3] = a[3] * b[3];
  return out;
}

/**
 * Multiplies a scalar and a vec4
 *
 * @param {ReadonlyVec4} a the first operand
 * @param {Number} b the second operand
 * @param {vec4} out the receiving vector
 * @returns {vec4} out
 */
export function multiplyScalar(a: ReadonlyVec4, b: number, out: vec4 = [0, 0, 0, 0]): vec4 {
  out[0] = a[0] * b;
  out[1] = a[1] * b;
  out[2] = a[2] * b;
  out[3] = a[3] * b;
  return out;
}

/**
 * Divides two vec4's
 *
 * @param {ReadonlyVec4} a the first operand
 * @param {ReadonlyVec4} b the second operand
 * @param {vec4} out the receiving vector
 * @returns {vec4} out
 */
export function divide(a: ReadonlyVec4, b: ReadonlyVec4, out: vec4 = [0, 0, 0, 0]): vec4 {
  out[0] = a[0] / b[0];
  out[1] = a[1] / b[1];
  out[2] = a[2] / b[2];
  out[3] = a[3] / b[3];
  return out;
}

/**
 * Divides a vec4 by a scalar
 *
 * @param {ReadonlyVec4} a the first operand
 * @param {Number} b the second operand
 * @param {vec4} out the receiving vector
 * @returns {vec4} out
 */
export function divideScalar(a: ReadonlyVec4, b: number, out: vec4 = [0, 0, 0, 0]): vec4 {
  out[0] = a[0] / b;
  out[1] = a[1] / b;
  out[2] = a[2] / b;
  out[3] = a[3] / b;
  return out;
}

/**
 * Math.ceil the components of a vec4
 *
 * @param {ReadonlyVec4} a vector to ceil
 * @param {vec4} out the receiving vector
 * @returns {vec4} out
 */
export function ceil(a: ReadonlyVec4, out: vec4 = [0, 0, 0, 0]): vec4 {
  out[0] = Math.ceil(a[0]);
  out[1] = Math.ceil(a[1]);
  out[2] = Math.ceil(a[2]);
  out[3] = Math.ceil(a[3]);
  return out;
}

/**
 * Math.floor the components of a vec4
 *
 * @param {ReadonlyVec4} a vector to floor
 * @param {vec4} out the receiving vector
 * @returns {vec4} out
 */
export function floor(a: ReadonlyVec4, out: vec4 = [0, 0, 0, 0]): vec4 {
  out[0] = Math.floor(a[0]);
  out[1] = Math.floor(a[1]);
  out[2] = Math.floor(a[2]);
  out[3] = Math.floor(a[3]);
  return out;
}

/**
 * Returns the minimum of two vec4's
 *
 * @param {ReadonlyVec4} a the first operand
 * @param {ReadonlyVec4} b the second operand
 * @param {vec4} out the receiving vector
 * @returns {vec4} out
 */
export function min(a: ReadonlyVec4, b: ReadonlyVec4, out: vec4 = [0, 0, 0, 0]): vec4 {
  out[0] = Math.min(a[0], b[0]);
  out[1] = Math.min(a[1], b[1]);
  out[2] = Math.min(a[2], b[2]);
  out[3] = Math.min(a[3], b[3]);
  return out;
}

/**
 * Returns the maximum of two vec4's
 *
 * @param {ReadonlyVec4} a the first operand
 * @param {ReadonlyVec4} b the second operand
 * @param {vec4} out the receiving vector
 * @returns {vec4} out
 */
export function max(a: ReadonlyVec4, b: ReadonlyVec4, out: vec4 = [0, 0, 0, 0]): vec4 {
  out[0] = Math.max(a[0], b[0]);
  out[1] = Math.max(a[1], b[1]);
  out[2] = Math.max(a[2], b[2]);
  out[3] = Math.max(a[3], b[3]);
  return out;
}

/**
 * Round the components of a vec4
 *
 * @param {ReadonlyVec4} a vector to round
 * @param {Number} decimals number of decimals
 * @param {vec4} out the receiving vector
 * @returns {vec4} out
 */
export function round(a: ReadonlyVec4, decimals: number = 0, out: vec4 = [0, 0, 0, 0]): vec4 {
  out[0] = nRound(a[0], decimals);
  out[1] = nRound(a[1], decimals);
  out[2] = nRound(a[2], decimals);
  out[3] = nRound(a[3], decimals);
  return out;
}

/**
 * Calculates the euclidian distance between two vec4's
 *
 * @param {ReadonlyVec4} a the first operand
 * @param {ReadonlyVec4} b the second operand
 * @returns {Number} distance between a and b
 */
export function distance(a: ReadonlyVec4, b: ReadonlyVec4): number {
  return Math.hypot(b[0] - a[0], b[1] - a[1], b[2] - a[2], b[3] - a[3]);
}

/**
 * Calculates the squared euclidian distance between two vec4's
 *
 * @param {ReadonlyVec4} a the first operand
 * @param {ReadonlyVec4} b the second operand
 * @returns {Number} squared distance between a and b
 */
export function squaredDistance(a: ReadonlyVec4, b: ReadonlyVec4): number {
  const x = b[0] - a[0],
    y = b[1] - a[1],
    z = b[2] - a[2],
    w = b[3] - a[3];
  return x * x + y * y + z * z + w * w;
}

/**
 * Calculates the length of a vec4
 *
 * @param {ReadonlyVec4} a vector to calculate length of
 * @returns {Number} length of a
 */
export function length(a: ReadonlyVec4): number {
  return Math.hypot(a[0], a[1], a[2], a[3]);
}

/**
 * Calculates the squared length of a vec4
 *
 * @param {ReadonlyVec4} a vector to calculate squared length of
 * @returns {Number} squared length of a
 */
export function squaredLength(a: ReadonlyVec4): number {
  return a[0] * a[0] + a[1] * a[1] + a[2] * a[2], a[3] * a[3];
}

/**
 * Negates the components of a vec4
 *
 * @param {ReadonlyVec4} a vector to negate
 * @param {vec4} out the receiving vector
 * @returns {vec4} out
 */
export function negate(a: ReadonlyVec4, out: vec4 = [0, 0, 0, 0]): vec4 {
  out[0] = -a[0];
  out[1] = -a[1];
  out[2] = -a[2];
  out[3] = -a[3];
  return out;
}

/**
 * Returns the inverse of the components of a vec4
 *
 * @param {ReadonlyVec4} a vector to invert
 * @param {vec4} out the receiving vector
 * @returns {vec4} out
 */
export function inverse(a: ReadonlyVec4, out: vec4 = [0, 0, 0, 0]): vec4 {
  out[0] = 1 / a[0];
  out[1] = 1 / a[1];
  out[2] = 1 / a[2];
  out[3] = 1 / a[3];
  return out;
}

/**
 * Normalize a vec4
 *
 * @param {ReadonlyVec4} a vector to normalize
 * @param {vec4} out the receiving vector
 * @returns {vec4} out
 */
export function normalize(a: ReadonlyVec4, out: vec4 = [0, 0, 0, 0]): vec4 {
  let len = a[0] * a[0] + a[1] * a[1] + a[2] * a[2] + a[3] * a[3];
  if (len > 0) len = 1 / Math.sqrt(len);
  out[0] = a[0] * len;
  out[1] = a[1] * len;
  out[2] = a[2] * len;
  return out;
}

/**
 * Calculates the dot product of two vec4's
 *
 * @param {ReadonlyVec4} a the first operand
 * @param {ReadonlyVec4} b the second operand
 * @returns {Number} dot product of a and b
 */
export function dot(a: ReadonlyVec4, b: ReadonlyVec4): number {
  return a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3];
}

/**
 * Computes the cross product of two vec4's
 *
 * @param {ReadonlyVec4} a the first operand
 * @param {ReadonlyVec4} b the second operand
 * @param {ReadonlyVec4} c the second operand
 * @param {vec4} out the receiving vector
 * @returns {vec4} out
 */
export function cross(
  a: ReadonlyVec4,
  b: ReadonlyVec4,
  c: ReadonlyVec4,
  out: vec4 = [0, 0, 0, 0]
): vec4 {
  let A = b[0] * c[1] - b[1] * c[0],
    B = b[0] * c[2] - b[2] * c[0],
    C = b[0] * c[3] - b[3] * c[0],
    D = b[1] * c[2] - b[2] * c[1],
    E = b[1] * c[3] - b[3] * c[1],
    F = b[2] * c[3] - b[3] * c[2];

  out[0] = a[1] * F - a[2] * E + a[3] * D;
  out[1] = -(a[0] * F) + a[2] * C - a[3] * B;
  out[2] = a[0] * E - a[1] * C + a[3] * A;
  out[3] = -(a[0] * D) + a[1] * B - a[2] * A;
  return out;
}

/**
 * Performs a linear interpolation between two vec4's
 *
 * @param {ReadonlyVec4} a the first operand
 * @param {ReadonlyVec4} b the second operand
 * @param {Number} t interpolation amount, in the range [0-1], between the two inputs
 * @param {vec4} out the receiving vector
 * @returns {vec4} out
 */
export function lerp(a: ReadonlyVec4, b: ReadonlyVec4, t: number, out: vec4 = [0, 0, 0, 0]): vec4 {
  out[0] = a[0] + t * (b[0] - a[0]);
  out[1] = a[1] + t * (b[1] - a[1]);
  out[2] = a[2] + t * (b[2] - a[2]);
  out[3] = a[3] + t * (b[3] - a[3]);
  return out;
}

/**
 * Generates a random vector with the given scale
 *
 * @param {Number} scale Length of the resulting vector. If omitted, a unit vector will be returned
 * @param {vec4} out the receiving vector
 * @returns {vec4} out
 */
export function random(scale: number = 1, out: vec4 = [0, 0, 0, 0]): vec4 {
  let v1, v2, v3, v4;
  let s1, s2;
  do {
    v1 = Math.random() * 2 - 1;
    v2 = Math.random() * 2 - 1;
    s1 = v1 * v1 + v2 * v2;
  } while (s1 >= 1);

  do {
    v3 = Math.random() * 2 - 1;
    v4 = Math.random() * 2 - 1;
    s2 = v3 * v3 + v4 * v4;
  } while (s2 >= 1);

  let d = Math.sqrt((1 - s1) / s2);

  out[0] = scale * v1;
  out[1] = scale * v2;
  out[2] = scale * v3 * d;
  out[3] = scale * v4 * d;

  return out;
}

/**
 * Scales a given vector
 *
 * @param {ReadonlyVec4} a point to scale
 * @param {ReadonlyVec4} b center of scale
 * @param {ReadonlyVec4} t magnitude of scale
 * @param {vec4} out the receiving vector
 * @returns {vec4} out
 */
export function scale(a: vec4, b: vec4, t: vec4, out: vec4 = [0, 0, 0, 0]): vec4 {
  out[0] = (a[0] - b[0]) * t[0] + b[0];
  out[1] = (a[1] - b[1]) * t[1] + b[1];
  out[2] = (a[2] - b[2]) * t[2] + b[2];
  out[3] = (a[3] - b[3]) * t[3] + b[3];
  return out;
}

/**
 * Transforms the vec4 with a mat4
 * 3rd vector component is implicitly '0'
 * 4th vector component is implicitly '1'
 *
 * @param {ReadonlyVec4} a the vector to transform
 * @param {ReadonlyMat4} m matrix to transform with
 * @param {vec4} out the receiving vector
 * @returns {vec4} out
 */
export function transformMat4(a: ReadonlyVec4, m: ReadonlyMat4, out: vec4 = [0, 0, 0, 0]): vec4 {
  out[0] = m[0] * a[0] + m[4] * a[1] + m[8] * a[2] + m[12] * a[3];
  out[1] = m[1] * a[0] + m[5] * a[1] + m[9] * a[2] + m[13] * a[3];
  out[2] = m[2] * a[0] + m[6] * a[1] + m[10] * a[2] + m[14] * a[3];
  out[3] = m[3] * a[0] + m[7] * a[1] + m[11] * a[2] + m[15] * a[3];
  return out;
}

// -----------------------------------------------------------------------------

// /**
//  * Transforms the vec4 with a quat
//  * Can also be used for dual quaternions. (Multiply it with the real part)
//  *
//  * @param {ReadonlyVec4} a the vector to transform
//  * @param {ReadonlyQuat} q quaternion to transform with
//  * @param {vec4} out the receiving vector
//  * @returns {vec4} out
//  */
// export function transformQuat(a: ReadonlyVec4, q: ReadonlyQuat, out: vec4 = [0, 0, 0, 0]) {
//   let ix = q[3] * a[0] + q[1] * a[2] - q[2] * a[1];
//   let iy = q[3] * a[1] + q[2] * a[0] - q[0] * a[2];
//   let iz = q[3] * a[2] + q[0] * a[1] - q[1] * a[0];
//   let iw = -q[0] * a[0] - q[1] * a[1] - q[2] * a[2];

//   out[0] = ix * q[3] + iw * -q[0] + iy * -q[2] - iz * -q[1];
//   out[1] = iy * q[3] + iw * -q[1] + iz * -q[0] - ix * -q[2];
//   out[2] = iz * q[3] + iw * -q[2] + ix * -q[1] - iy * -q[0];
//   out[3] = a[3];
//   return out;
// }

/**
 * Set the components of a vec4 to zero
 *
 * @param {vec4} out the receiving vector
 * @returns {vec4} out
 */
export function zero(out: vec4): vec4 {
  out[0] = 0;
  out[1] = 0;
  out[2] = 0;
  out[3] = 0;
  return out;
}

/**
 * Returns a string representation of a vector
 *
 * @param {ReadonlyVec4} a vector to represent as a string
 * @returns {String} string representation of the vector
 */
export function str(a: ReadonlyVec4): string {
  return 'vec4(' + a[0] + ', ' + a[1] + ', ' + a[2] + ', ' + a[3] + ')';
}

/**
 * Returns whether or not the vectors exactly have the same elements in the same position (when compared with ===)
 *
 * @param {ReadonlyVec4} a The first vector.
 * @param {ReadonlyVec4} b The second vector.
 * @returns {Boolean} True if the vectors are equal, false otherwise.
 */
export function exactEquals(a: ReadonlyVec4, b: ReadonlyVec4): boolean {
  return a[0] === b[0] && a[1] === b[1] && a[2] === b[2] && a[3] === b[3];
}

/**
 * Returns whether or not the vectors have approximately the same elements in the same position.
 *
 * @param {ReadonlyVec4} a The first vector.
 * @param {ReadonlyVec4} b The second vector.
 * @returns {Boolean} True if the vectors are equal, false otherwise.
 */
export function equals(a: ReadonlyVec4, b: ReadonlyVec4): boolean {
  return (
    Math.abs(a[0] - b[0]) <= MATH_EPSILON * Math.max(1, Math.abs(a[0]), Math.abs(b[0])) &&
    Math.abs(a[1] - b[1]) <= MATH_EPSILON * Math.max(1, Math.abs(a[1]), Math.abs(b[1])) &&
    Math.abs(a[2] - b[2]) <= MATH_EPSILON * Math.max(1, Math.abs(a[2]), Math.abs(b[2])) &&
    Math.abs(a[3] - b[3]) <= MATH_EPSILON * Math.max(1, Math.abs(a[3]), Math.abs(b[3]))
  );
}

/**
 * Alias for {@link vec4.length}
 * @function
 */
export const len = length;

/**
 * Alias for {@link vec4.addScalar}
 * @function
 */
export const addS = addScalar;

/**
 * Alias for {@link vec4.subtract}
 * @function
 */
export const sub = subtract;

/**
 * Alias for {@link vec4.subtractScalar}
 * @function
 */
export const subS = subtractScalar;

/**
 * Alias for {@link vec4.multiply}
 * @function
 */
export const mul = multiply;

/**
 * Alias for {@link vec4.multiplyScalar}
 * @function
 */
export const mulS = multiplyScalar;

/**
 * Alias for {@link vec4.divide}
 * @function
 */
export const div = divide;

/**
 * Alias for {@link vec4.divideScalar}
 * @function
 */
export const divS = divideScalar;

/**
 * Alias for {@link vec4.distance}
 * @function
 */
export const dist = distance;

/**
 * Alias for {@link vec4.squaredDistance}
 * @function
 */
export const sqrDist = squaredDistance;

/**
 * Alias for {@link vec4.squaredLength}
 * @function
 */
export const sqrLen = squaredLength;
