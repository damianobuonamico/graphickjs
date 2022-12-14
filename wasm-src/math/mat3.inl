#include "matrix.h"

#include <assert.h>

/* -- Component accesses -- */

inline vec3& mat3::operator[](uint8_t i) {
  assert(i >= 0 && i < this->length());
  return this->value[i];
}

constexpr const vec3& mat3::operator[](uint8_t i) const {
  assert(i >= 0 && i < this->length());
  return this->value[i];
}

/* -- Constructors -- */

constexpr mat3::mat3()
  : value{ vec3(1, 0, 0), vec3(0, 1, 0), vec3(0, 0, 1) } {}

constexpr mat3::mat3(const mat3& m)
  : value{ vec3(m[0]), vec3(m[1]), vec3(m[2]) } {}

constexpr mat3::mat3(float scalar)
  : value{ vec3(scalar, 0, 0), vec3(0, scalar, 0), vec3(0, 0, scalar) } {}

constexpr mat3::mat3(const vec3& v0, const vec3& v1, const vec3& v2)
  : value{ vec3(v0), vec3(v1), vec3(v2) } {}

constexpr mat3::mat3(
  float x0, float y0, float z0,
  float x1, float y1, float z1,
  float x2, float y2, float z2
) : value{ vec3(x0, y0, z0), vec3(x1, y1, z1), vec3(x2, y2, z2) } {}

/* -- Assign operator -- */

inline mat3& mat3::operator=(const mat3& m) {
  this->value[0] = m[0];
  this->value[1] = m[1];
  this->value[2] = m[2];
  return *this;
}

/* -- Unary arithmetic operators -- */

inline mat3& mat3::operator+=(float scalar) {
  this->value[0] += scalar;
  this->value[1] += scalar;
  this->value[2] += scalar;
  return *this;
}

inline mat3& mat3::operator+=(const mat3& m) {
  this->value[0] += m[0];
  this->value[1] += m[1];
  this->value[2] += m[2];
  return *this;
}

inline mat3& mat3::operator-=(float scalar) {
  this->value[0] -= scalar;
  this->value[1] -= scalar;
  this->value[2] -= scalar;
  return *this;
}

inline mat3& mat3::operator-=(const mat3& m) {
  this->value[0] -= m[0];
  this->value[1] -= m[1];
  this->value[2] -= m[2];
  return *this;
}

inline mat3& mat3::operator*=(float scalar) {
  this->value[0] *= scalar;
  this->value[1] *= scalar;
  this->value[2] *= scalar;
  return *this;
}

inline mat3& mat3::operator*=(const mat3& m) {
  return (*this = *this * m);
}

inline mat3& mat3::operator/=(float scalar) {
  this->value[0] /= scalar;
  this->value[1] /= scalar;
  this->value[2] /= scalar;
  return *this;
}

inline mat3& mat3::operator/=(const mat3& m) {
  return *this *= inverse(m);
}

/* -- Increment/Decrement operators -- */

inline mat3& mat3::operator++() {
  ++this->value[0];
  ++this->value[1];
  ++this->value[2];
  return *this;
}

inline mat3& mat3::operator--() {
  --this->value[0];
  --this->value[1];
  --this->value[2];
  return *this;
}

/* -- Unary operators */

inline mat3 operator+(const mat3& m) {
  return m;
}

inline mat3 operator-(const mat3& m) {
  return mat3(-m[0], -m[1], -m[2]);
}

/* -- Binary operators -- */

inline mat3 operator+(const mat3& m, float scalar) {
  return mat3(
    m[0] + scalar,
    m[1] + scalar,
    m[2] + scalar
  );
}

inline mat3 operator+(float scalar, const mat3& m) {
  return mat3(
    scalar + m[0],
    scalar + m[1],
    scalar + m[2]
  );
}

inline mat3 operator+(const mat3& m1, const mat3& m2) {
  return mat3(
    m1[0] + m2[0],
    m1[1] + m2[1],
    m1[2] + m2[2]
  );
}

inline mat3 operator-(const mat3& m, float scalar) {
  return mat3(
    m[0] - scalar,
    m[1] - scalar,
    m[2] - scalar
  );
}

inline mat3 operator-(float scalar, const mat3& m) {
  return mat3(
    scalar - m[0],
    scalar - m[1],
    scalar - m[2]
  );
}

inline mat3 operator-(const mat3& m1, const mat3& m2) {
  return mat3(
    m1[0] - m2[0],
    m1[1] - m2[1],
    m1[2] - m2[2]
  );
}

inline mat3 operator*(const mat3& m, float scalar) {
  return mat3(
    m[0] * scalar,
    m[1] * scalar,
    m[2] * scalar
  );
}

inline mat3 operator*(float scalar, const mat3& m) {
  return mat3(
    scalar * m[0],
    scalar * m[1],
    scalar * m[2]
  );
}

inline vec3 operator*(const mat3& m, const vec3& v) {
  return vec3(
    m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z,
    m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z,
    m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z
  );
}

inline mat3 operator*(const mat3& m1, const mat3& m2) {
  const float src_a00 = m1[0][0];
  const float src_a01 = m1[0][1];
  const float src_a02 = m1[0][2];
  const float src_a10 = m1[1][0];
  const float src_a11 = m1[1][1];
  const float src_a12 = m1[1][2];
  const float src_a20 = m1[2][0];
  const float src_a21 = m1[2][1];
  const float src_a22 = m1[2][2];

  const float src_b00 = m2[0][0];
  const float src_b01 = m2[0][1];
  const float src_b02 = m2[0][2];
  const float src_b10 = m2[1][0];
  const float src_b11 = m2[1][1];
  const float src_b12 = m2[1][2];
  const float src_b20 = m2[2][0];
  const float src_b21 = m2[2][1];
  const float src_b22 = m2[2][2];

  mat3 result;
  result[0][0] = src_a00 * src_b00 + src_a01 * src_b10 + src_a02 * src_b20;
  result[0][1] = src_a00 * src_b01 + src_a01 * src_b11 + src_a02 * src_b21;
  result[0][2] = src_a00 * src_b02 + src_a01 * src_b12 + src_a02 * src_b22;
  result[1][0] = src_a10 * src_b00 + src_a11 * src_b10 + src_a12 * src_b20;
  result[1][1] = src_a10 * src_b01 + src_a11 * src_b11 + src_a12 * src_b21;
  result[1][2] = src_a10 * src_b02 + src_a11 * src_b12 + src_a12 * src_b22;
  result[2][0] = src_a20 * src_b00 + src_a21 * src_b10 + src_a22 * src_b20;
  result[2][1] = src_a20 * src_b01 + src_a21 * src_b11 + src_a22 * src_b21;
  result[2][2] = src_a20 * src_b02 + src_a21 * src_b12 + src_a22 * src_b22;
  return result;
}

inline mat3 operator/(const mat3& m, float scalar) {
  return mat3(
    m[0] / scalar,
    m[1] / scalar,
    m[2] / scalar
  );
}

inline mat3 operator/(float scalar, const mat3& m) {
  return mat3(
    scalar / m[0],
    scalar / m[1],
    scalar / m[2]
  );
}

inline vec3 operator/(const mat3& m, const vec3& v) {
  return inverse(m) * v;
}

inline mat3 operator/(const mat3& m1, const mat3& m2) {
  mat3 m1_copy(m1);
  return m1_copy /= m2;
}

/* -- Boolean operators -- */

inline bool operator==(const mat3& m1, const mat3& m2) {
  return (m1[0] == m2[0]) && (m1[1] == m2[1]) && (m1[2] == m2[2]);
}

inline bool operator!=(const mat3& m1, const mat3& m2) {
  return (m1[0] != m2[0]) || (m1[1] != m2[1]) || (m1[2] != m2[2]);
}

/* -- Address operator -- */

inline const float* operator&(const mat3& m) { return &(m[0].x); }

/* -- Stream operator -- */

// TODO: Fix allineation
inline std::ostream& operator<<(std::ostream& os, const mat3& m) {
  os << "┌" << m[0].x << ", " << m[0].y << ", " << m[0].z <<
    "┐\n│" << m[1].x << ", " << m[1].y << ", " << m[1].z << "│\n└" <<
    m[2].x << ", " << m[2].y << ", " << m[2].z << "┘";
  return os;
}
