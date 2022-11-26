/**
 * 4x4 Matrix
 * Format: row-major
 * @module mat4
 */

/**
 * Creates a new empty mat4
 *
 * @returns {mat4} a new 4x4 matrix
 */
export function zero(): mat4 {
  return [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
}

/**
 * Creates a new identity mat4
 *
 * @returns {mat4} a new 4x4 matrix
 */
export function create(): mat4 {
  return [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1];
}

// TODO: optimize
/**
 * Transpose the values of a mat4
 *
 * @param {ReadonlyMat4} a the source matrix
 * @param {mat4} out the receiving matrix
 * @returns {mat4} out
 */
export function transpose(a: ReadonlyMat4, out: mat4 = zero()): mat4 {
  for (let i = 0; i < 4; ++i) {
    for (let j = 0; j < 4; ++j) {
      out[4 * i + j] = a[4 * j + i];
    }
  }

  return out;
}

/**
 * Adds two mat4s
 *
 * @param {ReadonlyMat4} a the first operand
 * @param {ReadonlyMat4} b the second operand
 * @param {mat4} out the receiving matrix
 * @returns {mat4} out
 */
export function add(a: ReadonlyMat4, b: ReadonlyMat4, out: mat4 = zero()): mat4 {
  for (let i = 0; i < 4; ++i) {
    for (let j = 0; j < 4; ++j) {
      out[4 * i + j] = a[4 * i + j] + b[4 * i + j];
    }
  }

  return out;
}

/**
 * Subtracts two mat4s
 *
 * @param {ReadonlyMat4} a the first operand
 * @param {ReadonlyMat4} b the second operand
 * @param {mat4} out the receiving matrix
 * @returns {mat4} out
 */
export function subtract(a: ReadonlyMat4, b: ReadonlyMat4, out: mat4 = zero()): mat4 {
  for (let i = 0; i < 4; ++i) {
    for (let j = 0; j < 4; ++j) {
      out[4 * i + j] = a[4 * i + j] - b[4 * i + j];
    }
  }

  return out;
}

/**
 * Multiplies two mat4s
 *
 * @param {ReadonlyMat4} a the first operand
 * @param {ReadonlyMat4} b the second operand
 * @param {mat4} out the receiving matrix
 * @returns {mat4} out
 */
export function multiply(a: ReadonlyMat4, b: ReadonlyMat4, out: mat4 = zero()): mat4 {
  for (let i = 0; i < 4; ++i) {
    for (let j = 0; j < 4; ++j) {
      for (let k = 0; k < 4; ++k) {
        out[4 * i + j] += a[4 * i + k] * b[4 * k + j];
      }
    }
  }
  return out;
}

/**
 * Multiplies a scalat and a mat4
 *
 * @param {ReadonlyMat4} a the first operand
 * @param {number} b the second operand
 * @param {mat4} out the receiving matrix
 * @returns {mat4} out
 */
export function multiplyScalar(a: ReadonlyMat4, b: number, out: mat4 = zero()): mat4 {
  for (let i = 0; i < 4; ++i) {
    for (let j = 0; j < 4; ++j) {
      out[4 * i + j] += a[4 * i + j] * b;
    }
  }
  return out;
}

/**
 * Alias for {@link mat4.multiply}
 * @function
 */
export const mul = multiply;

/**
 * Alias for {@link mat4.multiplyScalar}
 * @function
 */
export const mulS = multiplyScalar;

/**
 * Alias for {@link mat4.subtract}
 * @function
 */
export const sub = subtract;
