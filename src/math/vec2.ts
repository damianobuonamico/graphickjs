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
 * Adds two vec2's
 *
 * @param {ReadonlyVec2} a the first operand
 * @param {ReadonlyVec2 | number} b the second operand
 * @param {boolean} self is a the receiving vector
 * @returns {vec2} out
 */
export function add(a: ReadonlyVec2, b: ReadonlyVec2 | number, self: boolean = false): vec2 {
  const out = self ? a : create();
  const [b0, b1] = typeof b === 'number' ? [b, b] : [b[0], b[1]];
  out[0] = a[0] + b0;
  out[1] = a[1] + b1;
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
export function subtract(a: ReadonlyVec2, b: ReadonlyVec2 | number, self: boolean = false): vec2 {
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
export function multiply(a: ReadonlyVec2, b: ReadonlyVec2 | number, self: boolean = false): vec2 {
  const out = self ? a : create();
  const [b0, b1] = typeof b === 'number' ? [b, b] : [b[0], b[1]];
  out[0] = a[0] * b0;
  out[1] = a[1] * b1;
  return out;
}

/**
 * Divides two vec2's
 *
 * @param {ReadonlyVec2} a the first operand
 * @param {ReadonlyVec2 | number} b the second operand
 * @param {boolean} self is a the receiving vector
 * @returns {vec2} out
 */
export function divide(a: ReadonlyVec2, b: ReadonlyVec2 | number, self: boolean = false): vec2 {
  const out = self ? a : create();
  const [b0, b1] = typeof b === 'number' ? [b, b] : [b[0], b[1]];
  out[0] = a[0] / b0;
  out[1] = a[1] / b1;
  return out;
}

/**
 * Calculates the length of a vec2
 *
 * @param {ReadonlyVec2} a vector to calculate length of
 * @returns {Number} length of a
 */
export function length(a: ReadonlyVec2): number {
  var x = a[0],
    y = a[1];
  return Math.hypot(x, y);
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
 * Returns the minimum between two vectors
 *
 * @param {ReadonlyVec2} a the first operand
 * @param {ReadonlyVec2} b the second operand
 * @param {boolean} self is a the receiving vector
 * @returns {vec2} out
 */
export function min(a: ReadonlyVec2, b: ReadonlyVec2, self: boolean = false): vec2 {
  const out = self ? a : create();
  out[0] = Math.min(a[0], b[0]);
  out[1] = Math.min(a[1], b[1]);
  return out;
}

/**
 * Returns the maximum between two vectors
 *
 * @param {ReadonlyVec2} a the first operand
 * @param {ReadonlyVec2} b the second operand
 * @param {boolean} self is a the receiving vector
 * @returns {vec2} out
 */
export function max(a: ReadonlyVec2, b: ReadonlyVec2, self: boolean = false): vec2 {
  const out = self ? a : create();
  out[0] = Math.max(a[0], b[0]);
  out[1] = Math.max(a[1], b[1]);
  return out;
}

/**
 * Transforms the vec2 with a mat4
 * 3rd vector component is implicitly '0'
 * 4th vector component is implicitly '1'
 *
 * @param {vec2} out the receiving vector
 * @param {ReadonlyVec2} a the vector to transform
 * @param {ReadonlyMat4} m matrix to transform with
 * @returns {vec2} out
 */
export function transformMat4(a: ReadonlyVec2, m: ReadonlyMat4, self: boolean = false): vec2 {
  const out = self ? a : create();
  out[0] = m[0] * a[0] + m[4] * a[1] + m[12];
  out[1] = m[1] * a[0] + m[5] * a[1] + m[13];
  return out;
}

/**
 * Checks if two vectors are equal
 *
 * @param {ReadonlyVec2} a the first operand
 * @param {ReadonlyVec2} b the second operand
 * @returns {boolean} out
 */
export function equals(a: ReadonlyVec2, b: ReadonlyVec2): boolean {
  return a[0] === b[0] && a[1] === b[1];
}

/**
 * Calculates the euclidian distance between two vec2's
 *
 * @param {ReadonlyVec2} a the first operand
 * @param {ReadonlyVec2} b the second operand
 * @returns {Number} distance between a and b
 */
export function distance(a: ReadonlyVec2, b: ReadonlyVec2): number {
  var x = b[0] - a[0],
    y = b[1] - a[1];
  return Math.hypot(x, y);
}

/**
 * Calculates the squared euclidian distance between two vec2's
 *
 * @param {ReadonlyVec2} a the first operand
 * @param {ReadonlyVec2} b the second operand
 * @returns {Number} squared distance between a and b
 */
export function squaredDistance(a: ReadonlyVec2, b: ReadonlyVec2): number {
  var x = b[0] - a[0],
    y = b[1] - a[1];
  return x * x + y * y;
}

/**
 * Performs a linear interpolation between two vec2's
 *
 * @param {ReadonlyVec2} a the first operand
 * @param {ReadonlyVec2} b the second operand
 * @param {Number} t interpolation amount, in the range [0-1], between the two inputs
 * @returns {vec2} out
 */
export function lerp(a: ReadonlyVec2, b: ReadonlyVec2, t: number): vec2 {
  return add(a, mul(sub(b, a), t));
}

/**
 * Negates a vec2
 *
 * @param {ReadonlyVec2} a the vector
 * @returns {vec2} out
 */
export function negate(a: ReadonlyVec2, self: boolean = false): vec2 {
  const out = self ? a : create();
  out[0] = -a[0];
  out[1] = -a[1];
  return out;
}

/**
 * Returns the unit vector of a given vec2
 *
 * @param {ReadonlyVec2} a the vector
 * @returns {vec2} out
 */
export function unit(a: ReadonlyVec2, self: boolean = false): vec2 {
  const out = self ? a : clone(a);
  div(out, length(out), true);
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
 * Calculates the angle between two vec2's
 *
 * @param {ReadonlyVec2} a the first operand
 * @param {ReadonlyVec2} b the second operand
 * @returns {Number} angle between a and b
 */
export function angle(a: ReadonlyVec2, b: ReadonlyVec2): number {
  return Math.sign(a[0] * b[1] - a[1] * b[0]) * Math.acos(dot(a, b) / (len(a) * len(b)));
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

/**
 * Alias for {@link vec2.divide}
 * @function
 */
export const div = divide;

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
 * Alias for {@link vec2.negate}
 * @function
 */
export const neg = negate;

/**
 * Alias for {@link vec2.length}
 * @function
 */
export const len = length;
