import { MATH_EPSILON } from '@/utils/constants';
import { round as nRound, clamp as nClamp } from './math';

// TODO: check methods where out = a

/**
 * 2 Dimensional Vector
 * @module vec2
 */

/**
 * Creates a new, empty vec2
 *
 * @returns {vec2} a new 2D vector
 */
export function create(): vec2 {
  return [0, 0];
}

/**
 * Creates a new vec2 initialized with values from an existing vector
 *
 * @param {ReadonlyVec2} a vector to clone
 * @returns {vec2} a new 2D vector
 */
export function clone(a: ReadonlyVec2): vec2 {
  return [a[0], a[1]];
}

/**
 * Creates a new vec2 initialized with the given values
 *
 * @param {Number} x X component
 * @param {Number} y Y component
 * @returns {vec2} a new 2D vector
 */
export function fromValues(x: number, y: number): vec2 {
  return [x, y];
}

/**
 * Copy the values from one vec2 to another
 *
 * @param {vec2} out the receiving vector
 * @param {ReadonlyVec2} a the source vector
 * @returns {vec2} out
 */
export function copy(out: vec2, a: ReadonlyVec2): vec2 {
  out[0] = a[0];
  out[1] = a[1];
  return out;
}

/**
 * Set the components of a vec2 to the given values
 *
 * @param {vec2} out the receiving vector
 * @param {Number} x X component
 * @param {Number} y Y component
 * @returns {vec2} out
 */
export function set(out: vec2, x: number, y: number): vec2 {
  out[0] = x;
  out[1] = y;
  return out;
}

/**
 * Adds two vec2's
 *
 * @param {ReadonlyVec2} a the first operand
 * @param {ReadonlyVec2} b the second operand
 * @param {vec2} out the receiving vector
 * @returns {vec2} out
 */
export function add(a: ReadonlyVec2, b: ReadonlyVec2, out: vec2 = [0, 0]): vec2 {
  out[0] = a[0] + b[0];
  out[1] = a[1] + b[1];
  return out;
}

/**
 * Adds a scalar to a vec2
 *
 * @param {ReadonlyVec2} a the first operand
 * @param {Number} b the second operand
 * @param {vec2} out the receiving vector
 * @returns {vec2} out
 */
export function addScalar(a: ReadonlyVec2, b: number, out: vec2 = [0, 0]): vec2 {
  out[0] = a[0] + b;
  out[1] = a[1] + b;
  return out;
}

/**
 * Subtracts vector b from vector a
 *
 * @param {ReadonlyVec2} a the first operand
 * @param {ReadonlyVec2} b the second operand
 * @param {vec2} out the receiving vector
 * @returns {vec2} out
 */
export function subtract(a: ReadonlyVec2, b: ReadonlyVec2, out: vec2 = [0, 0]): vec2 {
  out[0] = a[0] - b[0];
  out[1] = a[1] - b[1];
  return out;
}

/**
 * Subtracts a scalar to a vec2
 *
 * @param {ReadonlyVec2} a the first operand
 * @param {Number} b the second operand
 * @param {vec2} out the receiving vector
 * @returns {vec2} out
 */
export function subtractScalar(a: ReadonlyVec2, b: number, out: vec2 = [0, 0]): vec2 {
  out[0] = a[0] - b;
  out[1] = a[1] - b;
  return out;
}

/**
 * Multiplies two vec2's
 *
 * @param {ReadonlyVec2} a the first operand
 * @param {ReadonlyVec2} b the second operand
 * @param {vec2} out the receiving vector
 * @returns {vec2} out
 */
export function multiply(a: ReadonlyVec2, b: ReadonlyVec2, out: vec2 = [0, 0]): vec2 {
  out[0] = a[0] * b[0];
  out[1] = a[1] * b[1];
  return out;
}

/**
 * Multiplies a scalar and a vec2
 *
 * @param {ReadonlyVec2} a the first operand
 * @param {Number} b the second operand
 * @param {vec2} out the receiving vector
 * @returns {vec2} out
 */
export function multiplyScalar(a: ReadonlyVec2, b: number, out: vec2 = [0, 0]): vec2 {
  out[0] = a[0] * b;
  out[1] = a[1] * b;
  return out;
}

/**
 * Divides two vec2's
 *
 * @param {ReadonlyVec2} a the first operand
 * @param {ReadonlyVec2} b the second operand
 * @param {vec2} out the receiving vector
 * @returns {vec2} out
 */
export function divide(a: ReadonlyVec2, b: ReadonlyVec2, out: vec2 = [0, 0]): vec2 {
  out[0] = a[0] / b[0];
  out[1] = a[1] / b[1];
  return out;
}

/**
 * Divides a vec2 by a scalar
 *
 * @param {ReadonlyVec2} a the first operand
 * @param {Number} b the second operand
 * @param {vec2} out the receiving vector
 * @returns {vec2} out
 */
export function divideScalar(a: ReadonlyVec2, b: number, out: vec2 = [0, 0]): vec2 {
  out[0] = a[0] / b;
  out[1] = a[1] / b;
  return out;
}

/**
 * Math.ceil the components of a vec2
 *
 * @param {ReadonlyVec2} a vector to ceil
 * @param {vec2} out the receiving vector
 * @returns {vec2} out
 */
export function ceil(a: ReadonlyVec2, out: vec2 = [0, 0]): vec2 {
  out[0] = Math.ceil(a[0]);
  out[1] = Math.ceil(a[1]);
  return out;
}

/**
 * Math.floor the components of a vec2
 *
 * @param {ReadonlyVec2} a vector to floor
 * @param {vec2} out the receiving vector
 * @returns {vec2} out
 */
export function floor(a: ReadonlyVec2, out: vec2 = [0, 0]): vec2 {
  out[0] = Math.floor(a[0]);
  out[1] = Math.floor(a[1]);
  return out;
}

/**
 * Returns the minimum of two vec2's
 *
 * @param {ReadonlyVec2} a the first operand
 * @param {ReadonlyVec2} b the second operand
 * @param {vec2} out the receiving vector
 * @returns {vec2} out
 */
export function min(a: ReadonlyVec2, b: ReadonlyVec2, out: vec2 = [0, 0]): vec2 {
  out[0] = Math.min(a[0], b[0]);
  out[1] = Math.min(a[1], b[1]);
  return out;
}

/**
 * Returns the maximum of two vec2's
 *
 * @param {ReadonlyVec2} a the first operand
 * @param {ReadonlyVec2} b the second operand
 * @param {vec2} out the receiving vector
 * @returns {vec2} out
 */
export function max(a: ReadonlyVec2, b: ReadonlyVec2, out: vec2 = [0, 0]): vec2 {
  out[0] = Math.max(a[0], b[0]);
  out[1] = Math.max(a[1], b[1]);
  return out;
}

/**
 * Clamps a vec2's between two values
 *
 * @param {ReadonlyVec2} a the first operand
 * @param {ReadonlyVec2} min the minimum value
 * @param {ReadonlyVec2} max the maximum value
 * @param {vec2} out the receiving vector
 * @returns {vec2} out
 */
export function clamp(
  a: ReadonlyVec2,
  min: ReadonlyVec2,
  max: ReadonlyVec2,
  out: vec2 = [0, 0]
): vec2 {
  out[0] = nClamp(a[0], min[0], max[0]);
  out[1] = nClamp(a[1], min[1], max[1]);
  return out;
}

/**
 * Round the components of a vec2
 *
 * @param {ReadonlyVec2} a vector to round
 * @param {Number} decimals number of decimals
 * @param {vec2} out the receiving vector
 * @returns {vec2} out
 */
export function round(a: ReadonlyVec2, decimals: number = 0, out: vec2 = [0, 0]): vec2 {
  out[0] = nRound(a[0], decimals);
  out[1] = nRound(a[1], decimals);
  return out;
}

/**
 * Calculates the euclidian distance between two vec2's
 *
 * @param {ReadonlyVec2} a the first operand
 * @param {ReadonlyVec2} b the second operand
 * @returns {Number} distance between a and b
 */
export function distance(a: ReadonlyVec2, b: ReadonlyVec2): number {
  return Math.hypot(b[0] - a[0], b[1] - a[1]);
}

/**
 * Calculates the squared euclidian distance between two vec2's
 *
 * @param {ReadonlyVec2} a the first operand
 * @param {ReadonlyVec2} b the second operand
 * @returns {Number} squared distance between a and b
 */
export function squaredDistance(a: ReadonlyVec2, b: ReadonlyVec2): number {
  const x = b[0] - a[0],
    y = b[1] - a[1];
  return x * x + y * y;
}

/**
 * Calculates the length of a vec2
 *
 * @param {ReadonlyVec2} a vector to calculate length of
 * @returns {Number} length of a
 */
export function length(a: ReadonlyVec2): number {
  return Math.hypot(a[0], a[1]);
}

/**
 * Calculates the squared length of a vec2
 *
 * @param {ReadonlyVec2} a vector to calculate squared length of
 * @returns {Number} squared length of a
 */
export function squaredLength(a: ReadonlyVec2): number {
  return a[0] * a[0] + a[1] * a[1];
}

/**
 * Negates the components of a vec2
 *
 * @param {ReadonlyVec2} a vector to negate
 * @param {vec2} out the receiving vector
 * @returns {vec2} out
 */
export function negate(a: ReadonlyVec2, out: vec2 = [0, 0]): vec2 {
  out[0] = -a[0];
  out[1] = -a[1];
  return out;
}

/**
 * Perpendicular rotation of a vector
 *
 * @param {ReadonlyVec2} a vector to rotate
 * @param {vec2} out the receiving vector
 * @returns {vec2} out
 */
export function perpendicular(a: ReadonlyVec2, out: vec2 = [0, 0]): vec2 {
  out[0] = -a[1];
  out[1] = a[0];
  return out;
}

/**
 * Projects a vector in the direction of another vector by a scalar
 *
 * @param {ReadonlyVec2} a vector to project
 * @param {ReadonlyVec2} b direction vector
 * @param {Number} t projection amount
 * @param {vec2} out the receiving vector
 * @returns {vec2} out
 */
export function project(a: ReadonlyVec2, b: ReadonlyVec2, t: number, out: vec2 = [0, 0]): vec2 {
  out[0] = a[0] + b[0] * t;
  out[1] = a[1] + b[1] * t;
  return out;
}

/**
 * Returns the inverse of the components of a vec2
 *
 * @param {ReadonlyVec2} a vector to invert
 * @param {vec2} out the receiving vector
 * @returns {vec2} out
 */
export function inverse(a: ReadonlyVec2, out: vec2 = [0, 0]): vec2 {
  out[0] = 1 / a[0];
  out[1] = 1 / a[1];
  return out;
}

/**
 * Normalize a vec2
 *
 * @param {ReadonlyVec2} a vector to normalize
 * @param {vec2} out the receiving vector
 * @returns {vec2} out
 */
export function normalize(a: ReadonlyVec2, out: vec2 = [0, 0]): vec2 {
  let len = a[0] * a[0] + a[1] * a[1];
  if (len > 0) len = 1 / Math.sqrt(len);
  out[0] = a[0] * len;
  out[1] = a[1] * len;
  return out;
}

export function normalizeLength(a: ReadonlyVec2, t: number, out: vec2 = [0, 0]): vec2 {
  let len = a[0] * a[0] + a[1] * a[1];
  if (len > 0) len = 1 / Math.sqrt(len);
  out[0] = a[0] * len * t;
  out[1] = a[1] * len * t;
  return out;
}

/**
 * Calculates the absolute value of a given vec2
 *
 * @param {ReadonlyVec2} a vector
 * @param {vec2} out the receiving vector
 * @returns {vec2} out
 */
export function abs(a: ReadonlyVec2, out: vec2 = [0, 0]): vec2 {
  out[0] = Math.abs(a[0]);
  out[1] = Math.abs(a[1]);
  return out;
}

/**
 * Calculates the sign of each component of a given vec2
 *
 * @param {ReadonlyVec2} a vector
 * @param {vec2} out the receiving vector
 * @returns {vec2} out
 */
export function sign(a: ReadonlyVec2, out: vec2 = [0, 0]): vec2 {
  out[0] = Math.sign(a[0]);
  out[1] = Math.sign(a[1]);
  return out;
}

/**
 * Returns the midpoint of two vec2's
 *
 * @param {ReadonlyVec2} a first vector
 * @param {ReadonlyVec2} b second vector
 * @param {vec2} out the receiving vector
 * @returns {vec2} out
 */
export function midpoint(a: ReadonlyVec2, b: ReadonlyVec2, out: vec2 = [0, 0]): vec2 {
  out[0] = (a[0] + b[0]) / 2;
  out[1] = (a[1] + b[1]) / 2;
  return out;
}

/**
 * Calculates the dot product of two vec2's
 *
 * @param {ReadonlyVec2} a the first operand
 * @param {ReadonlyVec2} b the second operand
 * @returns {Number} dot product of a and b
 */
export function dot(a: ReadonlyVec2, b: ReadonlyVec2): number {
  return a[0] * b[0] + a[1] * b[1];
}

/**
 * Computes the cross product of two vec2's
 *
 * @param {ReadonlyVec2} a the first operand
 * @param {ReadonlyVec2} b the second operand
 * @param {vec3} out the receiving vector
 * @returns {vec3} out
 */
export function cross(a: ReadonlyVec2, b: ReadonlyVec2): number {
  return a[0] * b[1] - a[1] * b[0];
}

/**
 * Performs a linear interpolation between two vec2's
 *
 * @param {ReadonlyVec2} a the first operand
 * @param {ReadonlyVec2} b the second operand
 * @param {Number} t interpolation amount, in the range [0-1], between the two inputs
 * @param {vec2} out the receiving vector
 * @returns {vec2} out
 */
export function lerp(a: ReadonlyVec2, b: ReadonlyVec2, t: number, out: vec2 = [0, 0]): vec2 {
  out[0] = a[0] + t * (b[0] - a[0]);
  out[1] = a[1] + t * (b[1] - a[1]);
  return out;
}

/**
 * Performs a cubic bezier interpolation between four vec2's
 *
 * @param {ReadonlyVec2} p0 the start point
 * @param {ReadonlyVec2} p1 the first control point
 * @param {ReadonlyVec2} p2 the second control point
 * @param {ReadonlyVec2} p3 the end point
 * @param {Number} t interpolation amount, in the range [0-1], between the two inputs
 * @param {vec2} out the receiving vector
 * @returns {vec2} out
 */
export function bezier(
  p0: ReadonlyVec2,
  p1: ReadonlyVec2,
  p2: ReadonlyVec2,
  p3: ReadonlyVec2,
  t: number,
  out: vec2 = [0, 0]
): vec2 {
  const c0 = Math.pow(1 - t, 3),
    c1 = 3 * t * Math.pow(1 - t, 2),
    c2 = 3 * Math.pow(t, 2) * (1 - t),
    c3 = Math.pow(t, 3);

  out[0] = p0[0] * c0 + p1[0] * c1 + p2[0] * c2 + p3[0] * c3;
  out[1] = p0[1] * c0 + p1[1] * c1 + p2[1] * c2 + p3[1] * c3;

  return out;
}

/**
 * Generates a random vector with the given scale
 *
 * @param {Number} scale Length of the resulting vector. If omitted, a unit vector will be returned
 * @param {vec2} out the receiving vector
 * @returns {vec2} out
 */
export function random(scale: number = 1, out: vec2 = [0, 0]): vec2 {
  const r = Math.random() * 2 * Math.PI;
  out[0] = Math.cos(r) * scale;
  out[1] = Math.sin(r) * scale;
  return out;
}

/**
 * Scales a given vector
 *
 * @param {ReadonlyVec2} a point to scale
 * @param {ReadonlyVec2} b center of scale
 * @param {ReadonlyVec2} t magnitude of scale
 * @param {vec2} out the receiving vector
 * @returns {vec2} out
 */
export function scale(a: vec2, b: vec2, t: vec2, out: vec2 = [0, 0]): vec2 {
  out[0] = (a[0] - b[0]) * t[0] + b[0];
  out[1] = (a[1] - b[1]) * t[1] + b[1];
  return out;
}

// /**
//  * Transforms the vec2 with a mat2
//  *
//  * @param {ReadonlyVec2} a the vector to transform
//  * @param {ReadonlyMat2} m matrix to transform with
//  * @param {vec2} out the receiving vector
//  * @returns {vec2} out
//  */
// export function transformMat2(a: ReadonlyVec2, m: ReadonlyMat2, out: vec2 = [0, 0]): vec2 {
//   out[0] = m[0] * a[0] + m[2] * a[1];
//   out[1] = m[1] * a[0] + m[3] * a[1];
//   return out;
// }

// /**
//  * Transforms the vec2 with a mat2d
//  *
//  * @param {ReadonlyVec2} a the vector to transform
//  * @param {ReadonlyMat2d} m matrix to transform with
//  * @param {vec2} out the receiving vector
//  * @returns {vec2} out
//  */
// export function transformMat2d(a: ReadonlyVec2, m: ReadonlyMat2d, out: vec2 = [0, 0]): vec2 {
//   out[0] = m[0] * a[0] + m[2] * a[1] + m[4];
//   out[1] = m[1] * a[0] + m[3] * a[1] + m[5];
//   return out;
// }

/**
 * Transforms the vec2 with a mat3
 * 3rd vector component is implicitly '1'
 *
 * @param {ReadonlyVec2} a the vector to transform
 * @param {ReadonlyMat3} m matrix to transform with
 * @param {vec2} out the receiving vector
 * @returns {vec2} out
 */
export function transformMat3(a: ReadonlyVec2, m: ReadonlyMat3, out: vec2 = [0, 0]): vec2 {
  const x = a[0],
    y = a[1];

  out[0] = m[0] * x + m[1] * y + m[2];
  out[1] = m[3] * x + m[4] * y + m[5];
  return out;
}

/**
 * Transforms the vec2 with a mat4
 * 3rd vector component is implicitly '0'
 * 4th vector component is implicitly '1'
 *
 * @param {ReadonlyVec2} a the vector to transform
 * @param {ReadonlyMat4} m matrix to transform with
 * @param {vec2} out the receiving vector
 * @returns {vec2} out
 */
export function transformMat4(a: ReadonlyVec2, m: ReadonlyMat4, out: vec2 = [0, 0]): vec2 {
  out[0] = m[0] * a[0] + m[4] * a[1] + m[12];
  out[1] = m[1] * a[0] + m[5] * a[1] + m[13];
  return out;
}

/**
 * Rotate a 2D vector
 *
 * @param {ReadonlyVec2} a The vec2 point to rotate
 * @param {ReadonlyVec2} b The origin of the rotation
 * @param {Number} rad The angle of rotation in radians
 * @param {vec2} out The receiving vec2
 * @returns {vec2} out
 */
export function rotate(a: ReadonlyVec2, b: ReadonlyVec2, rad: number, out: vec2 = [0, 0]): vec2 {
  let c0 = a[0] - b[0],
    c1 = a[1] - b[1],
    sin = Math.sin(rad),
    cos = Math.cos(rad);

  out[0] = b[0] + c0 * cos - c1 * sin;
  out[1] = b[1] + c0 * sin + c1 * cos;

  return out;
}

export function rotateOrigin(a: ReadonlyVec2, rad: number, out: vec2 = [0, 0]): vec2 {
  const sin = Math.sin(rad);
  const cos = Math.cos(rad);

  out[0] = cos * a[0] - sin * a[1];
  out[1] = sin * a[0] + cos * a[1];

  return out;
}

/**
 * Get the angle between two 2D vectors
 * @param {ReadonlyVec2} a The first operand
 * @param {ReadonlyVec2} b The second operand
 * @param {Boolean} clamp Whether to clamp the resulting angle
 * @returns {Number} The angle in radians
 */
export function angle(a: ReadonlyVec2, b: ReadonlyVec2, clamp: boolean = false): number {
  if (clamp) return Math.atan2(b[1] - a[1], b[0] - a[0]);

  return (
    Math.sign(a[0] * b[1] - a[1] * b[0]) *
    Math.acos((a[0] * b[0] + a[1] * b[1]) / (Math.hypot(a[0], a[1]) * Math.hypot(b[0], b[1])))
  );
}

/**
 * Snaps the input vector to regular angle increments
 *
 * @param {ReadonlyVec2} a
 * @param {Number} intervals angle increments
 * @param {vec2} out The receiving vec2
 * @returns {vec2} out
 */
export function snap(a: ReadonlyVec2, intervals: number = 8, out: vec2 = [0, 0]): vec2 {
  const increment = (Math.PI * 2) / intervals;
  let ang = Math.round(angle(a, [0, 1], true) / increment) * increment;
  let len = length(a);
  let cos = Math.cos(ang);
  let sin = Math.sin(ang);

  if (Math.abs(sin) < MATH_EPSILON) sin = 0;
  if (Math.abs(cos) < MATH_EPSILON) cos = 0;

  out[0] = -cos * len;
  out[1] = -sin * len;
  return out;
}

/**
 * Set the components of a vec2 to zero
 *
 * @param {vec2} out the receiving vector
 * @returns {vec2} out
 */
export function zero(out: vec2): vec2 {
  out[0] = 0;
  out[1] = 0;
  return out;
}

export function isZero(a: vec2): boolean {
  return a[0] === 0 && a[1] === 0;
}

/**
 * Returns a string representation of a vector
 *
 * @param {ReadonlyVec2} a vector to represent as a string
 * @returns {String} string representation of the vector
 */
export function str(a: ReadonlyVec2): string {
  return 'vec2(' + a[0] + ', ' + a[1] + ')';
}

/**
 * Returns whether or not the vectors exactly have the same elements in the same position (when compared with ===)
 *
 * @param {ReadonlyVec2} a The first vector.
 * @param {ReadonlyVec2} b The second vector.
 * @returns {Boolean} True if the vectors are equal, false otherwise.
 */
export function exactEquals(a: ReadonlyVec2, b: ReadonlyVec2): boolean {
  return a[0] === b[0] && a[1] === b[1];
}

/**
 * Returns whether or not the vectors have approximately the same elements in the same position.
 *
 * @param {ReadonlyVec2} a The first vector.
 * @param {ReadonlyVec2} b The second vector.
 * @returns {Boolean} True if the vectors are equal, false otherwise.
 */
export function equals(a: ReadonlyVec2, b: ReadonlyVec2, epsilon: number = MATH_EPSILON): boolean {
  return (
    Math.abs(a[0] - b[0]) <= epsilon * Math.max(1, Math.abs(a[0]), Math.abs(b[0])) &&
    Math.abs(a[1] - b[1]) <= epsilon * Math.max(1, Math.abs(a[1]), Math.abs(b[1]))
  );
}

export function direction(a: ReadonlyVec2, b: ReadonlyVec2, out: vec2 = [0, 0]): vec2 {
  out[0] = b[0] - a[0];
  out[1] = b[1] - a[1];

  let len = out[0] * out[0] + out[1] * out[1];
  if (len > 0) len = 1 / Math.sqrt(len);

  out[0] *= len;
  out[1] *= len;

  return out;
}

/**
 * Alias for {@link vec2.length}
 * @function
 */
export const len = length;

/**
 * Alias for {@link vec2.addScalar}
 * @function
 */
export const addS = addScalar;

/**
 * Alias for {@link vec2.subtract}
 * @function
 */
export const sub = subtract;

/**
 * Alias for {@link vec2.subtractScalar}
 * @function
 */
export const subS = subtractScalar;

/**
 * Alias for {@link vec2.multiply}
 * @function
 */
export const mul = multiply;

/**
 * Alias for {@link vec2.multiplyScalar}
 * @function
 */
export const mulS = multiplyScalar;

/**
 * Alias for {@link vec2.divide}
 * @function
 */
export const div = divide;

/**
 * Alias for {@link vec2.divideScalar}
 * @function
 */
export const divS = divideScalar;

/**
 * Alias for {@link vec2.distance}
 * @function
 */
export const dist = distance;

/**
 * Alias for {@link vec2.squaredDistance}
 * @function
 */
export const sqrDist = squaredDistance;

/**
 * Alias for {@link vec2.squaredLength}
 * @function
 */
export const sqrLen = squaredLength;

/**
 * Alias for {@link vec2.negate}
 * @function
 */
export const neg = negate;

/**
 * Alias for {@link vec2.perpendicular}
 * @function
 */
export const perp = perpendicular;

/**
 * Alias for {@link vec2.project}
 * @function
 */
export const proj = project;

/**
 * Alias for {@link vec2.midpoint}
 * @function
 */
export const mid = midpoint;

/**
 * Alias for {@link vec2.direction}
 * @function
 */
export const dir = direction;
