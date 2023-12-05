/**
 * 2x2 Matrix
 * Format: row-major
 * @module mat2
 */

/**
 * Creates a new identity mat2
 *
 * @returns {mat2} a new 2x2 matrix
 */
export function create(): mat2 {
  return [1, 0, 0, 1];
}

export function zero(): mat2 {
  return [0, 0, 0, 0];
}

export function inverse(a: mat2, out: mat2 = create()): mat2 {
  const [a11, a12, a21, a22] = a;

  const oneOverDet = 1 / (a11 * a22 - a12 * a21);

  out[0] = oneOverDet * a22;
  out[1] = -oneOverDet * a12;
  out[2] = -oneOverDet * a21;
  out[3] = oneOverDet * a11;

  return out;
}

export function multiplyVec2(a: mat2, b: vec2, out: vec2 = [0, 0]): vec2 {
  const [b1, b2] = b;

  out[0] = a[0] * b1 + a[1] * b2;
  out[1] = a[2] * b1 + a[3] * b2;

  return out;
}

/**
 * Alias for {@link mat3.inverse}
 * @function
 */
export const inv = inverse;

/**
 * Alias for {@link mat3.multiplyVec2}
 * @function
 */
export const mulVec2 = multiplyVec2;
