/**
 * 3x3 Matrix
 * Format: row-major
 * @module mat3
 */

/**
 * Creates a new identity mat3
 *
 * @returns {mat3} a new 3x3 matrix
 */
export function create(): mat3 {
  return [0, 0, 0, 0, 0, 0, 0, 0, 0];
}

/**
 * Copies the upper-left 3x3 values into the given mat3.
 *
 * @param {ReadonlyMat4} a the source 4x4 matrix
 * @param {mat3} out the receiving 3x3 matrix
 * @returns {mat3} out
 */
export function fromMat4(a: ReadonlyMat4, out: mat3 = create()): mat3 {
  out[0] = a[0];
  out[1] = a[1];
  out[2] = a[2];
  out[3] = a[4];
  out[4] = a[5];
  out[5] = a[6];
  out[6] = a[8];
  out[7] = a[9];
  out[8] = a[10];
  return out;
}

/**
 * Creates a new mat3 initialized with values from an existing matrix
 *
 * @param {ReadonlyMat3} a matrix to clone
 * @returns {mat3} a new 3x3 matrix
 */
export function clone(a: ReadonlyMat3): mat3 {
  return [a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7], a[8]];
}

/**
 * Copy the values from one mat3 to another
 *
 * @param {mat3} out the receiving matrix
 * @param {ReadonlyMat3} a the source matrix
 * @returns {mat3} out
 */
export function copy(a: ReadonlyMat3, out: mat3 = create()): mat3 {
  out[0] = a[0];
  out[1] = a[1];
  out[2] = a[2];
  out[3] = a[3];
  out[4] = a[4];
  out[5] = a[5];
  out[6] = a[6];
  out[7] = a[7];
  out[8] = a[8];
  return out;
}

/**
 * Create a new mat3 with the given values
 *
 * @param {Number} m11 Component in column 1, row 1 position (index 0)
 * @param {Number} m12 Component in column 1, row 2 position (index 1)
 * @param {Number} m13 Component in column 1, row 3 position (index 2)
 * @param {Number} m21 Component in column 2, row 1 position (index 3)
 * @param {Number} m22 Component in column 2, row 2 position (index 4)
 * @param {Number} m23 Component in column 2, row 3 position (index 5)
 * @param {Number} m31 Component in column 3, row 1 position (index 6)
 * @param {Number} m32 Component in column 3, row 2 position (index 7)
 * @param {Number} m33 Component in column 3, row 3 position (index 8)
 * @returns {mat3} A new mat3
 */
export function fromValues(
  m11: number,
  m12: number,
  m13: number,
  m21: number,
  m22: number,
  m23: number,
  m31: number,
  m32: number,
  m33: number
): mat3 {
  return [m11, m12, m13, m21, m22, m23, m31, m32, m33];
}

/**
 * Set the components of a mat3 to the given values
 *
 * @param {mat3} out the receiving matrix
 * @param {Number} m11 Component in column 1, row 1 position (index 0)
 * @param {Number} m12 Component in column 1, row 2 position (index 1)
 * @param {Number} m13 Component in column 1, row 3 position (index 2)
 * @param {Number} m21 Component in column 2, row 1 position (index 3)
 * @param {Number} m22 Component in column 2, row 2 position (index 4)
 * @param {Number} m23 Component in column 2, row 3 position (index 5)
 * @param {Number} m31 Component in column 3, row 1 position (index 6)
 * @param {Number} m32 Component in column 3, row 2 position (index 7)
 * @param {Number} m33 Component in column 3, row 3 position (index 8)
 * @returns {mat3} out
 */
export function set(
  out: mat3,
  m11: number,
  m12: number,
  m13: number,
  m21: number,
  m22: number,
  m23: number,
  m31: number,
  m32: number,
  m33: number
): mat3 {
  out[0] = m11;
  out[1] = m12;
  out[2] = m13;
  out[3] = m21;
  out[4] = m22;
  out[5] = m23;
  out[6] = m31;
  out[7] = m32;
  out[8] = m33;
  return out;
}

/**
 * Set a mat3 to the identity matrix
 *
 * @param {mat3} out the receiving matrix
 * @returns {mat3} out
 */
export function identity(out: mat3 = create()): mat3 {
  out[0] = 1;
  out[1] = 0;
  out[2] = 0;
  out[3] = 0;
  out[4] = 1;
  out[5] = 0;
  out[6] = 0;
  out[7] = 0;
  out[8] = 1;
  return out;
}

/**
 * Transpose the values of a mat3
 *
 * @param {ReadonlyMat3} a the source matrix
 * @param {mat3} out the receiving matrix
 * @returns {mat3} out
 */
export function transpose(a: ReadonlyMat3, out: mat3 = create()): mat3 {
  // TODO: Implement
  // if (out === a) {
  //   let a01 = a[1],
  //     a02 = a[2],
  //     a12 = a[5];
  //   out[1] = a[3];
  //   out[2] = a[6];
  //   out[3] = a01;
  //   out[5] = a[7];
  //   out[6] = a02;
  //   out[7] = a12;
  // } else {
  //   out[0] = a[0];
  //   out[1] = a[3];
  //   out[2] = a[6];
  //   out[3] = a[1];
  //   out[4] = a[4];
  //   out[5] = a[7];
  //   out[6] = a[2];
  //   out[7] = a[5];
  //   out[8] = a[8];
  // }
  return out;
}

/**
 * Multiplies two mat3's
 *
 * @param {ReadonlyMat3} a the first operand
 * @param {ReadonlyMat3} b the second operand
 * @param {mat3} out the receiving matrix
 * @returns {mat3} out
 */
export function multiply(a: ReadonlyMat3, b: ReadonlyMat3, out: mat3 = create()): mat3 {
  const a11 = a[0],
    a12 = a[1],
    a13 = a[2];
  const a21 = a[3],
    a22 = a[4],
    a23 = a[5];
  const a31 = a[6],
    a32 = a[7],
    a33 = a[8];
  const b11 = b[0],
    b12 = b[1],
    b13 = b[2];
  const b21 = b[3],
    b22 = b[4],
    b23 = b[5];
  const b31 = b[6],
    b32 = b[7],
    b33 = b[8];

  out[0] = a11 * b11 + a12 * b21 + a13 * b31;
  out[1] = a11 * b12 + a12 * b22 + a13 * b32;
  out[2] = a11 * b13 + a12 * b23 + a13 * b33;
  out[3] = a21 * b11 + a23 * b31 + b21 * a22;
  out[4] = a21 * b12 + a23 * b32 + b22 * a22;
  out[5] = a21 * b13 + a23 * b33 + b23 * a22;
  out[6] = a31 * b11 + a32 * b21 + a33 * b31;
  out[7] = a31 * b12 + a32 * b22 + a33 * b32;
  out[8] = a31 * b13 + a32 * b23 + a33 * b33;

  return out;
}

/**
 * Creates a matrix from a vector translation
 * This is equivalent to (but much faster than):
 *
 *     mat3.identity(dest);
 *     mat3.translate(dest, dest, vec);
 *
 * @param {mat3} out mat3 receiving operation result
 * @param {ReadonlyVec2} v Translation vector
 * @returns {mat3} out
 */
export function fromTranslation(v: ReadonlyVec2, out: mat3 = create()): mat3 {
  out[0] = 1;
  out[1] = 0;
  out[2] = v[0];
  out[3] = 0;
  out[4] = 1;
  out[5] = v[1];
  out[6] = 0;
  out[7] = 0;
  out[8] = 1;
  return out;
}

/**
 * Translate a mat3 by the given vector
 *
 * @param {ReadonlyMat3} a the matrix to translate
 * @param {ReadonlyVec2} v vector to translate by
 * @param {mat3} out the receiving matrix
 * @returns {mat3} out
 */
export function translate(a: ReadonlyMat3, v: ReadonlyVec2, out: mat3 = create()): mat3 {
  const a11 = a[0],
    a12 = a[1],
    a13 = a[2];
  const a21 = a[3],
    a22 = a[4],
    a23 = a[5];
  const a31 = a[6],
    a32 = a[7],
    a33 = a[8];
  const x = v[0],
    y = v[1];

  out[0] = a11;
  out[1] = a12;
  out[2] = a11 * x + a12 * y + a13;
  out[3] = a21;
  out[4] = a22;
  out[5] = a21 * x + a22 * y + a23;
  out[6] = a31;
  out[7] = a32;
  out[8] = a31 * x + a32 * y + a33;

  return out;
}

/**
 * Rotates a mat3 by the given angle
 *
 * @param {ReadonlyMat3} a the matrix to rotate
 * @param {Number} rad the angle to rotate the matrix by
 * @param {mat3} out the receiving matrix
 * @returns {mat3} out
 */
export function rotate(a: ReadonlyMat3, rad: number, out: mat3 = create()): mat3 {
  const a11 = a[0],
    a12 = a[1],
    a13 = a[2];
  const a21 = a[3],
    a22 = a[4],
    a23 = a[5];
  const a31 = a[6],
    a32 = a[7],
    a33 = a[8];
  const sin = Math.sin(rad),
    cos = Math.cos(rad);

  out[0] = a11 * cos + a12 * sin;
  out[1] = a12 * cos - a11 * sin;
  out[2] = a13;
  out[3] = a21 * cos + a22 * sin;
  out[4] = a22 * cos - a21 * sin;
  out[5] = a23;
  out[6] = a31 * cos + a32 * sin;
  out[7] = a32 * cos - a31 * sin;
  out[8] = a33;

  return out;
}

/**
 * Scales the mat3 by the dimensions in the given vec2
 *
 * @param {ReadonlyMat3} a the matrix to rotate
 * @param {ReadonlyVec2} v the vec2 to scale the matrix by
 * @param {mat3} out the receiving matrix
 * @returns {mat3} out
 **/
export function scale(a: ReadonlyMat3, v: ReadonlyVec2, out: mat3 = create()): mat3 {
  const x = v[0],
    y = v[1];

  out[0] = a[0] * x;
  out[1] = a[1] * y;
  out[2] = a[2];
  out[3] = a[3] * x;
  out[4] = a[4] * y;
  out[5] = a[5];
  out[6] = a[6] * x;
  out[7] = a[7] * y;
  out[8] = a[8];

  return out;
}

/**
 * Alias for {@link mat3.multiply}
 * @function
 */
export const mul = multiply;
