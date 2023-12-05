/**
 * @file mat2x3.inl
 * @brief This file contains the inline implementation of the mat2x3 struct.
 */

#pragma once

namespace Graphick::Math {

  /* -- Forward declarations -- */

  mat2x3 inverse(const mat2x3& m);

  /* -- Component accesses -- */

  inline vec3& mat2x3::operator[](uint8_t i) {
    return this->value[i];
  }

  constexpr const vec3& mat2x3::operator[](uint8_t i) const {
    return this->value[i];
  }

  /* -- Constructors -- */

  constexpr mat2x3::mat2x3()
    : value{ vec3(1, 0, 0), vec3(0, 1, 0) } {}

  constexpr mat2x3::mat2x3(const mat2x3& m)
    : value{ vec3(m[0]), vec3(m[1]) } {}

  constexpr mat2x3::mat2x3(float scalar)
    : value{ vec3(scalar, 0, 0), vec3(0, scalar, 0) } {}

  constexpr mat2x3::mat2x3(const vec3& v0, const vec3& v1)
    : value{ vec3(v0), vec3(v1) } {}

  constexpr mat2x3::mat2x3(
    float x0, float y0, float z0,
    float x1, float y1, float z1
  ) : value{ vec3(x0, y0, z0), vec3(x1, y1, z1) } {}

  /* -- Assign operator -- */

  inline mat2x3& mat2x3::operator=(const mat2x3& m) {
    this->value[0] = m[0];
    this->value[1] = m[1];
    return *this;
  }

  /* -- Unary arithmetic operators -- */

  inline mat2x3& mat2x3::operator+=(float scalar) {
    this->value[0] += scalar;
    this->value[1] += scalar;
    return *this;
  }

  inline mat2x3& mat2x3::operator+=(const mat2x3& m) {
    this->value[0] += m[0];
    this->value[1] += m[1];
    return *this;
  }

  inline mat2x3& mat2x3::operator-=(float scalar) {
    this->value[0] -= scalar;
    this->value[1] -= scalar;
    return *this;
  }

  inline mat2x3& mat2x3::operator-=(const mat2x3& m) {
    this->value[0] -= m[0];
    this->value[1] -= m[1];
    return *this;
  }

  inline mat2x3& mat2x3::operator*=(float scalar) {
    this->value[0] *= scalar;
    this->value[1] *= scalar;
    return *this;
  }

  inline mat2x3& mat2x3::operator*=(const mat2x3& m) {
    return (*this = *this * m);
  }

  inline mat2x3& mat2x3::operator/=(float scalar) {
    this->value[0] /= scalar;
    this->value[1] /= scalar;
    return *this;
  }

  inline mat2x3& mat2x3::operator/=(const mat2x3& m) {
    return *this *= inverse(m);
  }

  /* -- Increment/Decrement operators -- */

  inline mat2x3& mat2x3::operator++() {
    ++this->value[0];
    ++this->value[1];
    return *this;
  }

  inline mat2x3& mat2x3::operator--() {
    --this->value[0];
    --this->value[1];
    return *this;
  }

  /* -- Unary operators */

  inline mat2x3 operator+(const mat2x3& m) {
    return m;
  }

  inline mat2x3 operator-(const mat2x3& m) {
    return mat2x3(-m[0], -m[1]);
  }

  /* -- Binary operators -- */

  inline mat2x3 operator+(const mat2x3& m, float scalar) {
    return mat2x3(
      m[0] + scalar,
      m[1] + scalar
    );
  }

  inline mat2x3 operator+(float scalar, const mat2x3& m) {
    return mat2x3(
      scalar + m[0],
      scalar + m[1]
    );
  }

  inline mat2x3 operator+(const mat2x3& m1, const mat2x3& m2) {
    return mat2x3(
      m1[0] + m2[0],
      m1[1] + m2[1]
    );
  }

  inline mat2x3 operator-(const mat2x3& m, float scalar) {
    return mat2x3(
      m[0] - scalar,
      m[1] - scalar
    );
  }

  inline mat2x3 operator-(float scalar, const mat2x3& m) {
    return mat2x3(
      scalar - m[0],
      scalar - m[1]
    );
  }

  inline mat2x3 operator-(const mat2x3& m1, const mat2x3& m2) {
    return mat2x3(
      m1[0] - m2[0],
      m1[1] - m2[1]
    );
  }

  inline mat2x3 operator*(const mat2x3& m, float scalar) {
    return mat2x3(
      m[0] * scalar,
      m[1] * scalar
    );
  }

  inline mat2x3 operator*(float scalar, const mat2x3& m) {
    return mat2x3(
      scalar * m[0],
      scalar * m[1]
    );
  }

  inline vec2 operator*(const mat2x3& m, const vec2& v) {
    return vec2(
      m[0][0] * v.x + m[0][1] * v.y + m[0][2],
      m[1][0] * v.x + m[1][1] * v.y + m[1][2]
    );
  }

  inline mat2x3 operator*(const mat2x3& m1, const mat2x3& m2) {
    const float src_a00 = m1[0][0];
    const float src_a01 = m1[0][1];
    const float src_a02 = m1[0][2];
    const float src_a10 = m1[1][0];
    const float src_a11 = m1[1][1];
    const float src_a12 = m1[1][2];

    const float src_b00 = m2[0][0];
    const float src_b01 = m2[0][1];
    const float src_b02 = m2[0][2];
    const float src_b10 = m2[1][0];
    const float src_b11 = m2[1][1];
    const float src_b12 = m2[1][2];

    mat2x3 result;
    result[0][0] = src_a00 * src_b00 + src_a01 * src_b10;
    result[0][1] = src_a00 * src_b01 + src_a01 * src_b11;
    result[0][2] = src_a00 * src_b02 + src_a01 * src_b12 + src_a02;
    result[1][0] = src_a10 * src_b00 + src_a11 * src_b10;
    result[1][1] = src_a10 * src_b01 + src_a11 * src_b11;
    result[1][2] = src_a10 * src_b02 + src_a11 * src_b12 + src_a12;
    return result;
  }

  inline mat2x3 operator/(const mat2x3& m, float scalar) {
    return mat2x3(
      m[0] / scalar,
      m[1] / scalar
    );
  }

  inline mat2x3 operator/(float scalar, const mat2x3& m) {
    return mat2x3(
      scalar / m[0],
      scalar / m[1]
    );
  }

  inline vec2 operator/(const mat2x3& m, const vec2& v) {
    return inverse(m) * v;
  }

  inline mat2x3 operator/(const mat2x3& m1, const mat2x3& m2) {
    mat2x3 m1_copy(m1);
    return m1_copy /= m2;
  }

  /* -- Boolean operators -- */

  inline bool operator==(const mat2x3& m1, const mat2x3& m2) {
    return (m1[0] == m2[0]) && (m1[1] == m2[1]);
  }

  inline bool operator!=(const mat2x3& m1, const mat2x3& m2) {
    return (m1[0] != m2[0]) || (m1[1] != m2[1]);
  }

  /* -- Address operator -- */

  inline const float* operator&(const mat2x3& m) { return &(m[0].x); }

}
