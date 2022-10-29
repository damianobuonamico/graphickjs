import { MATH_EPSILON } from '@/utils/constants';
import { round as nRound } from './math';

/**
 * 3 Dimensional Vector
 * @module vec3
 */

/**
 * Creates a new, empty vec3
 *
 * @returns {vec3} a new 3D vector
 */
export function create(): vec3 {
  return [0, 0, 0];
}

/**
 * Creates a new vec3 initialized with values from an existing vector
 *
 * @param {ReadonlyVec3} a vector to clone
 * @returns {vec3} a new 3D vector
 */
export function clone(a: ReadonlyVec3): vec3 {
  return [a[0], a[1], a[2]];
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
  return [x, y, z];
}

/**
 * Copy the values from one vec3 to another
 *
 * @param {vec3} out the receiving vector
 * @param {ReadonlyVec3} a the source vector
 * @returns {vec3} out
 */
export function copy(out: vec3, a: ReadonlyVec3): vec3 {
  out[0] = a[0];
  out[1] = a[1];
  out[2] = a[2];
  return out;
}

/**
 * Set the components of a vec3 to the given values
 *
 * @param {vec3} out the receiving vector
 * @param {Number} x X component
 * @param {Number} y Y component
 * @param {Number} z Z component
 * @returns {vec3} out
 */
export function set(out: vec3, x: number, y: number, z: number): vec3 {
  out[0] = x;
  out[1] = y;
  out[2] = z;
  return out;
}

/**
 * Adds two vec3's
 *
 * @param {ReadonlyVec3} a the first operand
 * @param {ReadonlyVec3} b the second operand
 * @param {vec3} out the receiving vector
 * @returns {vec3} out
 */
export function add(a: ReadonlyVec3, b: ReadonlyVec3, out: vec3 = [0, 0, 0]): vec3 {
  out[0] = a[0] + b[0];
  out[1] = a[1] + b[1];
  out[2] = a[2] + b[2];
  return out;
}

/**
 * Adds a scalar to a vec3
 *
 * @param {ReadonlyVec3} a the first operand
 * @param {Number} b the second operand
 * @param {vec3} out the receiving vector
 * @returns {vec3} out
 */
export function addScalar(a: ReadonlyVec3, b: number, out: vec3 = [0, 0, 0]): vec3 {
  out[0] = a[0] + b;
  out[1] = a[1] + b;
  out[2] = a[2] + b;
  return out;
}

/**
 * Subtracts vector b from vector a
 *
 * @param {ReadonlyVec3} a the first operand
 * @param {ReadonlyVec3} b the second operand
 * @param {vec3} out the receiving vector
 * @returns {vec3} out
 */
export function subtract(a: ReadonlyVec3, b: ReadonlyVec3, out: vec3 = [0, 0, 0]): vec3 {
  out[0] = a[0] - b[0];
  out[1] = a[1] - b[1];
  out[2] = a[2] - b[2];
  return out;
}

/**
 * Subtracts a scalar to a vec3
 *
 * @param {ReadonlyVec3} a the first operand
 * @param {Number} b the second operand
 * @param {vec3} out the receiving vector
 * @returns {vec3} out
 */
export function subtractScalar(a: ReadonlyVec3, b: number, out: vec3 = [0, 0, 0]): vec3 {
  out[0] = a[0] - b;
  out[1] = a[1] - b;
  out[2] = a[2] - b;
  return out;
}

/**
 * Multiplies two vec3's
 *
 * @param {ReadonlyVec3} a the first operand
 * @param {ReadonlyVec3} b the second operand
 * @param {vec3} out the receiving vector
 * @returns {vec3} out
 */
export function multiply(a: ReadonlyVec3, b: ReadonlyVec3, out: vec3 = [0, 0, 0]): vec3 {
  out[0] = a[0] * b[0];
  out[1] = a[1] * b[1];
  out[2] = a[2] * b[2];
  return out;
}

/**
 * Multiplies a scalar and a vec3
 *
 * @param {ReadonlyVec3} a the first operand
 * @param {Number} b the second operand
 * @param {vec3} out the receiving vector
 * @returns {vec3} out
 */
export function multiplyScalar(a: ReadonlyVec3, b: number, out: vec3 = [0, 0, 0]): vec3 {
  out[0] = a[0] * b;
  out[1] = a[1] * b;
  out[2] = a[2] * b;
  return out;
}

/**
 * Divides two vec3's
 *
 * @param {ReadonlyVec3} a the first operand
 * @param {ReadonlyVec3} b the second operand
 * @param {vec3} out the receiving vector
 * @returns {vec3} out
 */
export function divide(a: ReadonlyVec3, b: ReadonlyVec3, out: vec3 = [0, 0, 0]): vec3 {
  out[0] = a[0] / b[0];
  out[1] = a[1] / b[1];
  out[2] = a[2] / b[2];
  return out;
}

/**
 * Divides a vec3 by a scalar
 *
 * @param {ReadonlyVec3} a the first operand
 * @param {Number} b the second operand
 * @param {vec3} out the receiving vector
 * @returns {vec3} out
 */
export function divideScalar(a: ReadonlyVec3, b: number, out: vec3 = [0, 0, 0]): vec3 {
  out[0] = a[0] / b;
  out[1] = a[1] / b;
  out[2] = a[2] / b;
  return out;
}

/**
 * Math.ceil the components of a vec3
 *
 * @param {ReadonlyVec3} a vector to ceil
 * @param {vec3} out the receiving vector
 * @returns {vec3} out
 */
export function ceil(a: ReadonlyVec3, out: vec3 = [0, 0, 0]): vec3 {
  out[0] = Math.ceil(a[0]);
  out[1] = Math.ceil(a[1]);
  out[2] = Math.ceil(a[2]);
  return out;
}

/**
 * Math.floor the components of a vec3
 *
 * @param {ReadonlyVec3} a vector to floor
 * @param {vec3} out the receiving vector
 * @returns {vec3} out
 */
export function floor(a: ReadonlyVec3, out: vec3 = [0, 0, 0]): vec3 {
  out[0] = Math.floor(a[0]);
  out[1] = Math.floor(a[1]);
  out[2] = Math.floor(a[2]);
  return out;
}

/**
 * Returns the minimum of two vec3's
 *
 * @param {ReadonlyVec3} a the first operand
 * @param {ReadonlyVec3} b the second operand
 * @param {vec3} out the receiving vector
 * @returns {vec3} out
 */
export function min(a: ReadonlyVec3, b: ReadonlyVec3, out: vec3 = [0, 0, 0]): vec3 {
  out[0] = Math.min(a[0], b[0]);
  out[1] = Math.min(a[1], b[1]);
  out[2] = Math.min(a[2], b[2]);
  return out;
}

/**
 * Returns the maximum of two vec3's
 *
 * @param {ReadonlyVec3} a the first operand
 * @param {ReadonlyVec3} b the second operand
 * @param {vec3} out the receiving vector
 * @returns {vec3} out
 */
export function max(a: ReadonlyVec3, b: ReadonlyVec3, out: vec3 = [0, 0, 0]): vec3 {
  out[0] = Math.max(a[0], b[0]);
  out[1] = Math.max(a[1], b[1]);
  out[2] = Math.max(a[2], b[2]);
  return out;
}

/**
 * Round the components of a vec3
 *
 * @param {ReadonlyVec3} a vector to round
 * @param {Number} decimals number of decimals
 * @param {vec3} out the receiving vector
 * @returns {vec3} out
 */
export function round(a: ReadonlyVec3, decimals: number = 0, out: vec3 = [0, 0, 0]): vec3 {
  out[0] = nRound(a[0], decimals);
  out[1] = nRound(a[1], decimals);
  out[2] = nRound(a[2], decimals);
  return out;
}

/**
 * Calculates the euclidian distance between two vec3's
 *
 * @param {ReadonlyVec3} a the first operand
 * @param {ReadonlyVec3} b the second operand
 * @returns {Number} distance between a and b
 */
export function distance(a: ReadonlyVec3, b: ReadonlyVec3): number {
  return Math.hypot(b[0] - a[0], b[1] - a[1], b[2] - a[2]);
}

/**
 * Calculates the squared euclidian distance between two vec3's
 *
 * @param {ReadonlyVec3} a the first operand
 * @param {ReadonlyVec3} b the second operand
 * @returns {Number} squared distance between a and b
 */
export function squaredDistance(a: ReadonlyVec3, b: ReadonlyVec3): number {
  const x = b[0] - a[0],
    y = b[1] - a[1],
    z = b[2] - a[2];
  return x * x + y * y + z * z;
}

/**
 * Calculates the length of a vec3
 *
 * @param {ReadonlyVec3} a vector to calculate length of
 * @returns {Number} length of a
 */
export function length(a: ReadonlyVec3): number {
  return Math.hypot(a[0], a[1], a[2]);
}

/**
 * Calculates the squared length of a vec3
 *
 * @param {ReadonlyVec3} a vector to calculate squared length of
 * @returns {Number} squared length of a
 */
export function squaredLength(a: ReadonlyVec3): number {
  return a[0] * a[0] + a[1] * a[1] + a[2] * a[2];
}

/**
 * Negates the components of a vec3
 *
 * @param {ReadonlyVec3} a vector to negate
 * @param {vec3} out the receiving vector
 * @returns {vec3} out
 */
export function negate(a: ReadonlyVec3, out: vec3 = [0, 0, 0]): vec3 {
  out[0] = -a[0];
  out[1] = -a[1];
  out[2] = -a[2];
  return out;
}

/**
 * Returns the inverse of the components of a vec3
 *
 * @param {ReadonlyVec3} a vector to invert
 * @param {vec3} out the receiving vector
 * @returns {vec3} out
 */
export function inverse(a: ReadonlyVec3, out: vec3 = [0, 0, 0]): vec3 {
  out[0] = 1 / a[0];
  out[1] = 1 / a[1];
  out[2] = 1 / a[2];
  return out;
}

/**
 * Normalize a vec3
 *
 * @param {ReadonlyVec3} a vector to normalize
 * @param {vec3} out the receiving vector
 * @returns {vec3} out
 */
export function normalize(a: ReadonlyVec3, out: vec3 = [0, 0, 0]): vec3 {
  let len = a[0] * a[0] + a[1] * a[1] + a[2] * a[2];
  if (len > 0) len = 1 / Math.sqrt(len);
  out[0] = a[0] * len;
  out[1] = a[1] * len;
  out[2] = a[2] * len;
  return out;
}

/**
 * Calculates the dot product of two vec3's
 *
 * @param {ReadonlyVec3} a the first operand
 * @param {ReadonlyVec3} b the second operand
 * @returns {Number} dot product of a and b
 */
export function dot(a: ReadonlyVec3, b: ReadonlyVec3): number {
  return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

/**
 * Computes the cross product of two vec3's
 *
 * @param {ReadonlyVec3} a the first operand
 * @param {ReadonlyVec3} b the second operand
 * @param {vec3} out the receiving vector
 * @returns {vec3} out
 */
export function cross(a: ReadonlyVec3, b: ReadonlyVec3, out: vec3 = [0, 0, 0]): vec3 {
  out[0] = a[1] * b[2] - a[2] * b[1];
  out[1] = a[2] * b[0] - a[0] * b[2];
  out[2] = a[0] * b[1] - a[1] * b[0];
  return out;
}

/**
 * Performs a linear interpolation between two vec3's
 *
 * @param {ReadonlyVec3} a the first operand
 * @param {ReadonlyVec3} b the second operand
 * @param {Number} t interpolation amount, in the range [0-1], between the two inputs
 * @param {vec3} out the receiving vector
 * @returns {vec3} out
 */
export function lerp(a: ReadonlyVec3, b: ReadonlyVec3, t: number, out: vec3 = [0, 0, 0]): vec3 {
  out[0] = a[0] + t * (b[0] - a[0]);
  out[1] = a[1] + t * (b[1] - a[1]);
  out[2] = a[2] + t * (b[2] - a[2]);
  return out;
}

/**
 * Performs a spherical linear interpolation between two vec3's
 *
 * @param {ReadonlyVec3} a the first operand
 * @param {ReadonlyVec3} b the second operand
 * @param {Number} t interpolation amount, in the range [0-1], between the two inputs
 * @param {vec3} out the receiving vector
 * @returns {vec3} out
 */
export function slerp(a: ReadonlyVec3, b: ReadonlyVec3, t: number, out: vec3 = [0, 0, 0]): vec3 {
  let angle = Math.acos(Math.min(Math.max(dot(a, b), -1), 1)),
    sinTotal = Math.sin(angle),
    ratioA = Math.sin((1 - t) * angle) / sinTotal,
    ratioB = Math.sin(t * angle) / sinTotal;

  out[0] = ratioA * a[0] + ratioB * b[0];
  out[1] = ratioA * a[1] + ratioB * b[1];
  out[2] = ratioA * a[2] + ratioB * b[2];

  return out;
}

/**
 * Performs a hermite interpolation with two control points
 *
 * @param {ReadonlyVec3} a the first operand
 * @param {ReadonlyVec3} b the second operand
 * @param {ReadonlyVec3} c the third operand
 * @param {ReadonlyVec3} d the fourth operand
 * @param {Number} t interpolation amount, in the range [0-1], between the two inputs
 * @param {vec3} out the receiving vector
 * @returns {vec3} out
 */
export function hermite(
  a: ReadonlyVec3,
  b: ReadonlyVec3,
  c: ReadonlyVec3,
  d: ReadonlyVec3,
  t: number,
  out: vec3 = [0, 0, 0]
): vec3 {
  let factorTimes2 = t * t,
    factor1 = factorTimes2 * (2 * t - 3) + 1,
    factor2 = factorTimes2 * (t - 2) + t,
    factor3 = factorTimes2 * (t - 1),
    factor4 = factorTimes2 * (3 - 2 * t);

  out[0] = a[0] * factor1 + b[0] * factor2 + c[0] * factor3 + d[0] * factor4;
  out[1] = a[1] * factor1 + b[1] * factor2 + c[1] * factor3 + d[1] * factor4;
  out[2] = a[2] * factor1 + b[2] * factor2 + c[2] * factor3 + d[2] * factor4;

  return out;
}

/**
 * Performs a bezier interpolation with two control points
 *
 * @param {ReadonlyVec3} a the first operand
 * @param {ReadonlyVec3} b the second operand
 * @param {ReadonlyVec3} c the third operand
 * @param {ReadonlyVec3} d the fourth operand
 * @param {Number} t interpolation amount, in the range [0-1], between the two inputs
 * @param {vec3} out the receiving vector
 * @returns {vec3} out
 */
export function bezier(
  a: ReadonlyVec3,
  b: ReadonlyVec3,
  c: ReadonlyVec3,
  d: ReadonlyVec3,
  t: number,
  out: vec3 = [0, 0, 0]
): vec3 {
  let inverseFactor = 1 - t,
    inverseFactorTimesTwo = inverseFactor * inverseFactor,
    factorTimes2 = t * t,
    factor1 = inverseFactorTimesTwo * inverseFactor,
    factor2 = 3 * t * inverseFactorTimesTwo,
    factor3 = 3 * factorTimes2 * inverseFactor,
    factor4 = factorTimes2 * t;

  out[0] = a[0] * factor1 + b[0] * factor2 + c[0] * factor3 + d[0] * factor4;
  out[1] = a[1] * factor1 + b[1] * factor2 + c[1] * factor3 + d[1] * factor4;
  out[2] = a[2] * factor1 + b[2] * factor2 + c[2] * factor3 + d[2] * factor4;

  return out;
}

/**
 * Generates a random vector with the given scale
 *
 * @param {Number} scale Length of the resulting vector. If omitted, a unit vector will be returned
 * @param {vec3} out the receiving vector
 * @returns {vec3} out
 */
export function random(scale: number = 1, out: vec3 = [0, 0, 0]): vec3 {
  let r = Math.random() * 2.0 * Math.PI;
  let z = Math.random() * 2.0 - 1.0;
  let zScale = Math.sqrt(1.0 - z * z) * scale;

  out[0] = Math.cos(r) * zScale;
  out[1] = Math.sin(r) * zScale;
  out[2] = z * scale;

  return out;
}

/**
 * Scales a given vector
 *
 * @param {ReadonlyVec3} a point to scale
 * @param {ReadonlyVec3} b center of scale
 * @param {ReadonlyVec3} t magnitude of scale
 * @param {vec3} out the receiving vector
 * @returns {vec3} out
 */
export function scale(a: vec3, b: vec3, t: vec3, out: vec3 = [0, 0, 0]): vec3 {
  out[0] = (a[0] - b[0]) * t[0] + b[0];
  out[1] = (a[1] - b[1]) * t[1] + b[1];
  out[2] = (a[2] - b[2]) * t[2] + b[2];
  return out;
}

/**
 * Transforms the vec3 with a mat3
 * 3rd vector component is implicitly '1'
 *
 * @param {ReadonlyVec3} a the vector to transform
 * @param {ReadonlyMat3} m matrix to transform with
 * @param {vec3} out the receiving vector
 * @returns {vec3} out
 */
export function transformMat3(a: ReadonlyVec3, m: ReadonlyMat3, out: vec3 = [0, 0, 0]): vec3 {
  out[0] = a[0] * m[0] + a[1] * m[3] + a[2] * m[6];
  out[1] = a[0] * m[1] + a[1] * m[4] + a[2] * m[7];
  out[2] = a[0] * m[2] + a[1] * m[5] + a[2] * m[8];
  return out;
}

/**
 * Transforms the vec3 with a mat4
 * 3rd vector component is implicitly '0'
 * 4th vector component is implicitly '1'
 *
 * @param {ReadonlyVec3} a the vector to transform
 * @param {ReadonlyMat4} m matrix to transform with
 * @param {vec3} out the receiving vector
 * @returns {vec3} out
 */
export function transformMat4(a: ReadonlyVec3, m: ReadonlyMat4, out: vec3 = [0, 0, 0]): vec3 {
  let w = m[3] * a[0] + m[7] * a[1] + m[11] * a[2] + m[15] || 1;
  out[0] = (m[0] * a[0] + m[4] * a[1] + m[8] * a[2] + m[12]) / w;
  out[1] = (m[1] * a[0] + m[5] * a[1] + m[9] * a[2] + m[13]) / w;
  out[2] = (m[2] * a[0] + m[6] * a[1] + m[10] * a[2] + m[14]) / w;
  return out;
}

// /**
//  * Transforms the vec3 with a quat
//  * Can also be used for dual quaternions. (Multiply it with the real part)
//  *
//  * @param {ReadonlyVec3} a the vector to transform
//  * @param {ReadonlyQuat} q quaternion to transform with
//  * @param {vec3} out the receiving vector
//  * @returns {vec3} out
//  */
// export function transformQuat(a: ReadonlyVec3, q: ReadonlyQuat, out: vec3 = [0, 0, 0]) {
//   let qx = q[0],
//     qy = q[1],
//     qz = q[2],
//     qw = q[3];

//   let x = a[0],
//     y = a[1],
//     z = a[2];

//   let uvx = qy * z - qz * y,
//     uvy = qz * x - qx * z,
//     uvz = qx * y - qy * x;

//   let uuvx = qy * uvz - qz * uvy,
//     uuvy = qz * uvx - qx * uvz,
//     uuvz = qx * uvy - qy * uvx;

//   let w2 = qw * 2;

//   uvx *= w2;
//   uvy *= w2;
//   uvz *= w2;

//   uuvx *= 2;
//   uuvy *= 2;
//   uuvz *= 2;

//   out[0] = x + uvx + uuvx;
//   out[1] = y + uvy + uuvy;
//   out[2] = z + uvz + uuvz;

//   return out;
// }

/**
 * Rotate a 3D vector around the x-axis
 * @param {ReadonlyVec3} a The vec3 point to rotate
 * @param {ReadonlyVec3} b The origin of the rotation
 * @param {Number} rad The angle of rotation in radians
 * @param {vec3} out The receiving vec3
 * @returns {vec3} out
 */
export function rotateX(
  a: ReadonlyVec3,
  b: ReadonlyVec3,
  rad: number,
  out: vec3 = [0, 0, 0]
): vec3 {
  let p: vec3 = [0, 0, 0],
    r: vec3 = [0, 0, 0];

  p[0] = a[0] - b[0];
  p[1] = a[1] - b[1];
  p[2] = a[2] - b[2];

  r[0] = p[0];
  r[1] = p[1] * Math.cos(rad) - p[2] * Math.sin(rad);
  r[2] = p[1] * Math.sin(rad) + p[2] * Math.cos(rad);

  out[0] = r[0] + b[0];
  out[1] = r[1] + b[1];
  out[2] = r[2] + b[2];

  return out;
}

/**
 * Rotate a 3D vector around the y-axis
 * @param {ReadonlyVec3} a The vec3 point to rotate
 * @param {ReadonlyVec3} b The origin of the rotation
 * @param {Number} rad The angle of rotation in radians
 * @param {vec3} out The receiving vec3
 * @returns {vec3} out
 */
export function rotateY(
  a: ReadonlyVec3,
  b: ReadonlyVec3,
  rad: number,
  out: vec3 = [0, 0, 0]
): vec3 {
  let p: vec3 = [0, 0, 0],
    r: vec3 = [0, 0, 0];

  p[0] = a[0] - b[0];
  p[1] = a[1] - b[1];
  p[2] = a[2] - b[2];

  r[0] = p[2] * Math.sin(rad) + p[0] * Math.cos(rad);
  r[1] = p[1];
  r[2] = p[2] * Math.cos(rad) - p[0] * Math.sin(rad);

  out[0] = r[0] + b[0];
  out[1] = r[1] + b[1];
  out[2] = r[2] + b[2];

  return out;
}

/**
 * Rotate a 3D vector around the z-axis
 * @param {ReadonlyVec3} a The vec3 point to rotate
 * @param {ReadonlyVec3} b The origin of the rotation
 * @param {Number} rad The angle of rotation in radians
 * @param {vec3} out The receiving vec3
 * @returns {vec3} out
 */
export function rotateZ(
  a: ReadonlyVec3,
  b: ReadonlyVec3,
  rad: number,
  out: vec3 = [0, 0, 0]
): vec3 {
  let p: vec3 = [0, 0, 0],
    r: vec3 = [0, 0, 0];

  p[0] = a[0] - b[0];
  p[1] = a[1] - b[1];
  p[2] = a[2] - b[2];

  r[0] = p[0] * Math.cos(rad) - p[1] * Math.sin(rad);
  r[1] = p[0] * Math.sin(rad) + p[1] * Math.cos(rad);
  r[2] = p[2];

  out[0] = r[0] + b[0];
  out[1] = r[1] + b[1];
  out[2] = r[2] + b[2];

  return out;
}

/**
 * Get the angle between two 3D vectors
 * @param {ReadonlyVec3} a The first operand
 * @param {ReadonlyVec3} b The second operand
 * @returns {Number} The angle in radians
 */
export function angle(a: ReadonlyVec3, b: ReadonlyVec3): number {
  let mag = Math.sqrt(
      (a[0] * a[0] + a[1] * a[1] + a[2] * a[2]) * (b[0] * b[0] + b[1] * b[1] + b[2] * b[2])
    ),
    cosine = mag && (a[0] * b[0] + a[1] * b[1] + a[2] * b[2]) / mag;
  return Math.acos(Math.min(Math.max(cosine, -1), 1));
}

/**
 * Set the components of a vec3 to zero
 *
 * @param {vec3} out the receiving vector
 * @returns {vec3} out
 */
export function zero(out: vec3): vec3 {
  out[0] = 0;
  out[1] = 0;
  out[2] = 0;
  return out;
}

/**
 * Returns a string representation of a vector
 *
 * @param {ReadonlyVec3} a vector to represent as a string
 * @returns {String} string representation of the vector
 */
export function str(a: ReadonlyVec3): string {
  return 'vec3(' + a[0] + ', ' + a[1] + ', ' + a[2] + ')';
}

/**
 * Returns whether or not the vectors exactly have the same elements in the same position (when compared with ===)
 *
 * @param {ReadonlyVec3} a The first vector.
 * @param {ReadonlyVec3} b The second vector.
 * @returns {Boolean} True if the vectors are equal, false otherwise.
 */
export function exactEquals(a: ReadonlyVec3, b: ReadonlyVec3): boolean {
  return a[0] === b[0] && a[1] === b[1] && a[2] === b[2];
}

/**
 * Returns whether or not the vectors have approximately the same elements in the same position.
 *
 * @param {ReadonlyVec3} a The first vector.
 * @param {ReadonlyVec3} b The second vector.
 * @returns {Boolean} True if the vectors are equal, false otherwise.
 */
export function equals(a: ReadonlyVec3, b: ReadonlyVec3): boolean {
  return (
    Math.abs(a[0] - b[0]) <= MATH_EPSILON * Math.max(1, Math.abs(a[0]), Math.abs(b[0])) &&
    Math.abs(a[1] - b[1]) <= MATH_EPSILON * Math.max(1, Math.abs(a[1]), Math.abs(b[1])) &&
    Math.abs(a[2] - b[2]) <= MATH_EPSILON * Math.max(1, Math.abs(a[2]), Math.abs(b[2]))
  );
}

/**
 * Alias for {@link vec3.length}
 * @function
 */
export const len = length;

/**
 * Alias for {@link vec3.addScalar}
 * @function
 */
export const addS = addScalar;

/**
 * Alias for {@link vec3.subtract}
 * @function
 */
export const sub = subtract;

/**
 * Alias for {@link vec3.subtractScalar}
 * @function
 */
export const subS = subtractScalar;

/**
 * Alias for {@link vec3.multiply}
 * @function
 */
export const mul = multiply;

/**
 * Alias for {@link vec3.multiplyScalar}
 * @function
 */
export const mulS = multiplyScalar;

/**
 * Alias for {@link vec3.divide}
 * @function
 */
export const div = divide;

/**
 * Alias for {@link vec3.divideScalar}
 * @function
 */
export const divS = divideScalar;

/**
 * Alias for {@link vec3.distance}
 * @function
 */
export const dist = distance;

/**
 * Alias for {@link vec3.squaredDistance}
 * @function
 */
export const sqrDist = squaredDistance;

/**
 * Alias for {@link vec3.squaredLength}
 * @function
 */
export const sqrLen = squaredLength;
