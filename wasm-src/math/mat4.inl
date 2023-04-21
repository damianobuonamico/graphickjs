#include <assert.h>

/* -- Forward declarations -- */

mat4 inverse(const mat4& m);

/* -- Component accesses -- */

inline vec4& mat4::operator[](uint8_t i) {
  assert(i >= 0 && i < this->length());
  return this->value[i];
}

constexpr const vec4& mat4::operator[](uint8_t i) const {
  assert(i >= 0 && i < this->length());
  return this->value[i];
}

/* -- Constructors -- */

constexpr mat4::mat4()
  : value{ vec4(1, 0, 0, 0), vec4(0, 1, 0, 0), vec4(0, 0, 1, 0), vec4(0, 0, 0, 1) } {}

constexpr mat4::mat4(const mat4& m)
  : value{ vec4(m[0]), vec4(m[1]), vec4(m[2]), vec4(m[3]) } {}

constexpr mat4::mat4(float scalar)
  : value{ vec4(scalar, 0, 0, 0), vec4(0, scalar, 0, 0), vec4(0, 0, scalar, 0), vec4(0, 0, 0, scalar) } {}

constexpr mat4::mat4(const vec4& v0, const vec4& v1, const vec4& v2, const vec4& v3)
  : value{ vec4(v0), vec4(v1), vec4(v2), vec4(v3) } {}

constexpr mat4::mat4(
  float x0, float y0, float z0, float w0,
  float x1, float y1, float z1, float w1,
  float x2, float y2, float z2, float w2,
  float x3, float y3, float z3, float w3
  ) : value{ vec4(x0, y0, z0, w0), vec4(x1, y1, z1, w1), vec4(x2, y2, z2, w2), vec4(x3, y3, z3, w3)} {}

/* -- Assign operator -- */

inline mat4& mat4::operator=(const mat4& m) {
  this->value[0] = m[0];
  this->value[1] = m[1];
  this->value[2] = m[2];
  this->value[3] = m[3];
  return *this;
}

/* -- Unary arithmetic operators -- */

inline mat4& mat4::operator+=(float scalar) {
  this->value[0] += scalar;
  this->value[1] += scalar;
  this->value[2] += scalar;
  this->value[3] += scalar;
  return *this;
}

inline mat4& mat4::operator+=(const mat4& m) {
  this->value[0] += m[0];
  this->value[1] += m[1];
  this->value[2] += m[2];
  this->value[3] += m[3];
  return *this;
}

inline mat4& mat4::operator-=(float scalar) {
  this->value[0] -= scalar;
  this->value[1] -= scalar;
  this->value[2] -= scalar;
  this->value[3] -= scalar;
  return *this;
}

inline mat4& mat4::operator-=(const mat4& m) {
  this->value[0] -= m[0];
  this->value[1] -= m[1];
  this->value[2] -= m[2];
  this->value[3] -= m[3];
  return *this;
}

inline mat4& mat4::operator*=(float scalar) {
  this->value[0] *= scalar;
  this->value[1] *= scalar;
  this->value[2] *= scalar;
  this->value[3] *= scalar;
  return *this;
}

inline mat4& mat4::operator*=(const mat4& m) {
  return (*this = *this * m);
}

inline mat4& mat4::operator/=(float scalar) {
  this->value[0] /= scalar;
  this->value[1] /= scalar;
  this->value[2] /= scalar;
  this->value[3] /= scalar;
  return *this;
}

inline mat4& mat4::operator/=(const mat4& m) {
  return *this *= inverse(m);
}

/* -- Increment/Decrement operators -- */

inline mat4& mat4::operator++() {
  ++this->value[0];
  ++this->value[1];
  ++this->value[2];
  ++this->value[3];
  return *this;
}

inline mat4& mat4::operator--() {
  --this->value[0];
  --this->value[1];
  --this->value[2];
  --this->value[3];
  return *this;
}

/* -- Unary operators */

inline mat4 operator+(const mat4& m) {
  return m;
}

inline mat4 operator-(const mat4& m) {
  return mat4(-m[0], -m[1], -m[2], -m[3]);
}

/* -- Binary operators -- */

inline mat4 operator+(const mat4& m, float scalar) {
  return mat4(
    m[0] + scalar,
    m[1] + scalar,
    m[2] + scalar,
    m[3] + scalar
  );
}

inline mat4 operator+(float scalar, const mat4& m) {
  return mat4(
    scalar + m[0],
    scalar + m[1],
    scalar + m[2],
    scalar + m[3]
  );
}

inline mat4 operator+(const mat4& m1, const mat4& m2) {
  return mat4(
    m1[0] + m2[0],
    m1[1] + m2[1],
    m1[2] + m2[2],
    m1[3] + m2[3]
  );
}

inline mat4 operator-(const mat4& m, float scalar) {
  return mat4(
    m[0] - scalar,
    m[1] - scalar,
    m[2] - scalar,
    m[3] - scalar
  );
}

inline mat4 operator-(float scalar, const mat4& m) {
  return mat4(
    scalar - m[0],
    scalar - m[1],
    scalar - m[2],
    scalar - m[3]
  );
}

inline mat4 operator-(const mat4& m1, const mat4& m2) {
  return mat4(
    m1[0] - m2[0],
    m1[1] - m2[1],
    m1[2] - m2[2],
    m1[3] - m2[3]
  );
}

inline mat4 operator*(const mat4& m, float scalar) {
  return mat4(
    m[0] * scalar,
    m[1] * scalar,
    m[2] * scalar,
    m[3] * scalar
  );
}

inline mat4 operator*(float scalar, const mat4& m) {
  return mat4(
    scalar * m[0],
    scalar * m[1],
    scalar * m[2],
    scalar * m[3]
  );
}

inline vec4 operator*(const mat4& m, const vec4& v) {
  return vec4(
    m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3] * v.w,
    m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3] * v.w,
    m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3] * v.w,
    m[3][0] * v.x + m[3][1] * v.y + m[3][2] * v.z + m[3][3] * v.w
  );
}

inline mat4 operator*(const mat4& m1, const mat4& m2) {
  const float src_a00 = m1[0][0];
  const float src_a01 = m1[0][1];
  const float src_a02 = m1[0][2];
  const float src_a03 = m1[0][3];
  const float src_a10 = m1[1][0];
  const float src_a11 = m1[1][1];
  const float src_a12 = m1[1][2];
  const float src_a13 = m1[1][3];
  const float src_a20 = m1[2][0];
  const float src_a21 = m1[2][1];
  const float src_a22 = m1[2][2];
  const float src_a23 = m1[2][3];
  const float src_a30 = m1[3][0];
  const float src_a31 = m1[3][1];
  const float src_a32 = m1[3][2];
  const float src_a33 = m1[3][3];

  const float src_b00 = m2[0][0];
  const float src_b01 = m2[0][1];
  const float src_b02 = m2[0][2];
  const float src_b03 = m2[0][3];
  const float src_b10 = m2[1][0];
  const float src_b11 = m2[1][1];
  const float src_b12 = m2[1][2];
  const float src_b13 = m2[1][3];
  const float src_b20 = m2[2][0];
  const float src_b21 = m2[2][1];
  const float src_b22 = m2[2][2];
  const float src_b23 = m2[2][3];
  const float src_b30 = m2[3][0];
  const float src_b31 = m2[3][1];
  const float src_b32 = m2[3][2];
  const float src_b33 = m2[3][3];

  mat4 result;
  result[0][0] = src_a00 * src_b00 + src_a01 * src_b10 + src_a02 * src_b20 + src_a03 * src_b30;
  result[0][1] = src_a00 * src_b01 + src_a01 * src_b11 + src_a02 * src_b21 + src_a03 * src_b31;
  result[0][2] = src_a00 * src_b02 + src_a01 * src_b12 + src_a02 * src_b22 + src_a03 * src_b32;
  result[0][3] = src_a00 * src_b03 + src_a01 * src_b13 + src_a02 * src_b23 + src_a03 * src_b33;
  result[1][0] = src_a10 * src_b00 + src_a11 * src_b10 + src_a12 * src_b20 + src_a13 * src_b30;
  result[1][1] = src_a10 * src_b01 + src_a11 * src_b11 + src_a12 * src_b21 + src_a13 * src_b31;
  result[1][2] = src_a10 * src_b02 + src_a11 * src_b12 + src_a12 * src_b22 + src_a13 * src_b32;
  result[1][3] = src_a10 * src_b03 + src_a11 * src_b13 + src_a12 * src_b23 + src_a13 * src_b33;
  result[2][0] = src_a20 * src_b00 + src_a21 * src_b10 + src_a22 * src_b20 + src_a23 * src_b30;
  result[2][1] = src_a20 * src_b01 + src_a21 * src_b11 + src_a22 * src_b21 + src_a23 * src_b31;
  result[2][2] = src_a20 * src_b02 + src_a21 * src_b12 + src_a22 * src_b22 + src_a23 * src_b32;
  result[2][3] = src_a20 * src_b03 + src_a21 * src_b13 + src_a22 * src_b23 + src_a23 * src_b33;
  result[3][0] = src_a30 * src_b00 + src_a31 * src_b10 + src_a32 * src_b20 + src_a33 * src_b30;
  result[3][1] = src_a30 * src_b01 + src_a31 * src_b11 + src_a32 * src_b21 + src_a33 * src_b31;
  result[3][2] = src_a30 * src_b02 + src_a31 * src_b12 + src_a32 * src_b22 + src_a33 * src_b32;
  result[3][3] = src_a30 * src_b03 + src_a31 * src_b13 + src_a32 * src_b23 + src_a33 * src_b33;
  return result;
}

inline mat4 operator/(const mat4& m, float scalar) {
  return mat4(
    m[0] / scalar,
    m[1] / scalar,
    m[2] / scalar,
    m[3] / scalar
  );
}

inline mat4 operator/(float scalar, const mat4& m) {
  return mat4(
    scalar / m[0],
    scalar / m[1],
    scalar / m[2],
    scalar / m[3]
  );
}

inline vec4 operator/(const mat4& m, const vec4& v) {
  return inverse(m) * v;
}

inline mat4 operator/(const mat4& m1, const mat4& m2) {
  mat4 m1_copy(m1);
  return m1_copy /= m2;
}

/* -- Boolean operators -- */

inline bool operator==(const mat4& m1, const mat4& m2) {
  return (m1[0] == m2[0]) && (m1[1] == m2[1]) && (m1[2] == m2[2]) && (m1[3] == m2[3]);
}

inline bool operator!=(const mat4& m1, const mat4& m2) {
  return (m1[0] != m2[0]) || (m1[1] != m2[1]) || (m1[2] != m2[2]) || (m1[3] != m2[3]);
}

/* -- Address operator -- */

inline const float* operator&(const mat4& m) { return &(m[0].x); }

/* -- Stream operator -- */

// TODO: Fix allineation
inline std::ostream& operator<<(std::ostream& os, const mat4& m) {
  os << "┌" << m[0].x << ", " << m[0].y << ", " << m[0].z << ", " << m[0].w <<
    "┐\n│" << m[1].x << ", " << m[1].y << ", " << m[1].z << ", " << m[1].w << "│\n" <<
    "┐\n│" << m[2].x << ", " << m[2].y << ", " << m[2].z << ", " << m[2].w << "│\n└" <<
    m[3].x << ", " << m[3].y << ", " << m[3].z << ", " << m[3].w << "┘";
  return os;
}
