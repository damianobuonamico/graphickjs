#pragma once

#include <assert.h>

namespace Graphick::Math {

  /* -- Forward declarations -- */

  mat2 inverse(const mat2& m);

  /* -- Component accesses -- */

  inline vec2& mat2::operator[](uint8_t i) {
    assert(i >= 0 && i < this->length());
    return this->value[i];
  }

  constexpr const vec2& mat2::operator[](uint8_t i) const {
    assert(i >= 0 && i < this->length());
    return this->value[i];
  }

  /* -- Constructors -- */

  constexpr mat2::mat2()
    : value{ vec2(1, 0), vec2(0, 1) } {}

  constexpr mat2::mat2(const mat2& m)
    : value{ vec2(m[0]), vec2(m[1]) } {}

  constexpr mat2::mat2(float scalar)
    : value{ vec2(scalar, 0), vec2(0, scalar) } {}

  constexpr mat2::mat2(const vec2 v0, const vec2 v1)
    : value{ vec2(v0), vec2(v1) } {}

  constexpr mat2::mat2(
    float x0, float y0,
    float x1, float y1
  ) : value{ vec2(x0, y0), vec2(x1, y1) } {}

  /* -- Assign operator -- */

  inline mat2& mat2::operator=(const mat2& m) {
    this->value[0] = m[0];
    this->value[1] = m[1];
    return *this;
  }

  /* -- Unary arithmetic operators -- */

  inline mat2& mat2::operator+=(float scalar) {
    this->value[0] += scalar;
    this->value[1] += scalar;
    return *this;
  }

  inline mat2& mat2::operator+=(const mat2& m) {
    this->value[0] += m[0];
    this->value[1] += m[1];
    return *this;
  }

  inline mat2& mat2::operator-=(float scalar) {
    this->value[0] -= scalar;
    this->value[1] -= scalar;
    return *this;
  }

  inline mat2& mat2::operator-=(const mat2& m) {
    this->value[0] -= m[0];
    this->value[1] -= m[1];
    return *this;
  }

  inline mat2& mat2::operator*=(float scalar) {
    this->value[0] *= scalar;
    this->value[1] *= scalar;
    return *this;
  }

  inline mat2& mat2::operator*=(const mat2& m) {
    return (*this = *this * m);
  }

  inline mat2& mat2::operator/=(float scalar) {
    this->value[0] /= scalar;
    this->value[1] /= scalar;
    return *this;
  }

  inline mat2& mat2::operator/=(const mat2& m) {
    return *this *= inverse(m);
  }

  /* -- Increment/Decrement operators -- */

  inline mat2& mat2::operator++() {
    ++this->value[0];
    ++this->value[1];
    return *this;
  }

  inline mat2& mat2::operator--() {
    --this->value[0];
    --this->value[1];
    return *this;
  }

  /* -- Unary operators */

  inline mat2 operator+(const mat2& m) {
    return m;
  }

  inline mat2 operator-(const mat2& m) {
    return mat2(-m[0], -m[1]);
  }

  /* -- Binary operators -- */

  inline mat2 operator+(const mat2& m, float scalar) {
    return mat2(
      m[0] + scalar,
      m[1] + scalar
    );
  }

  inline mat2 operator+(float scalar, const mat2& m) {
    return mat2(
      scalar + m[0],
      scalar + m[1]
    );
  }

  inline mat2 operator+(const mat2& m1, const mat2& m2) {
    return mat2(
      m1[0] + m2[0],
      m1[1] + m2[1]
    );
  }

  inline mat2 operator-(const mat2& m, float scalar) {
    return mat2(
      m[0] - scalar,
      m[1] - scalar
    );
  }

  inline mat2 operator-(float scalar, const mat2& m) {
    return mat2(
      scalar - m[0],
      scalar - m[1]
    );
  }

  inline mat2 operator-(const mat2& m1, const mat2& m2) {
    return mat2(
      m1[0] - m2[0],
      m1[1] - m2[1]
    );
  }

  inline mat2 operator*(const mat2& m, float scalar) {
    return mat2(
      m[0] * scalar,
      m[1] * scalar
    );
  }

  inline mat2 operator*(float scalar, const mat2& m) {
    return mat2(
      scalar * m[0],
      scalar * m[1]
    );
  }

  inline vec2 operator*(const mat2& m, const vec2& v) {
    return vec2(
      m[0][0] * v.x + m[0][1] * v.y,
      m[1][0] * v.x + m[1][1] * v.y
    );
  }

  inline mat2 operator*(const mat2& m1, const mat2& m2) {
    const float src_a00 = m1[0][0];
    const float src_a01 = m1[0][1];
    const float src_a10 = m1[1][0];
    const float src_a11 = m1[1][1];

    const float src_b00 = m2[0][0];
    const float src_b01 = m2[0][1];
    const float src_b10 = m2[1][0];
    const float src_b11 = m2[1][1];
    const float src_b12 = m2[1][2];

    mat2 result;
    result[0][0] = src_a00 * src_b00 + src_a01 * src_b10;
    result[0][1] = src_a00 * src_b01 + src_a01 * src_b11;
    result[1][0] = src_a10 * src_b00 + src_a11 * src_b10;
    result[1][1] = src_a10 * src_b01 + src_a11 * src_b11;
    return result;
  }

  inline mat2 operator/(const mat2& m, float scalar) {
    return mat2(
      m[0] / scalar,
      m[1] / scalar
    );
  }

  inline mat2 operator/(float scalar, const mat2& m) {
    return mat2(
      scalar / m[0],
      scalar / m[1]
    );
  }

  inline vec2 operator/(const mat2& m, const vec2& v) {
    return inverse(m) * v;
  }

  inline mat2 operator/(const mat2& m1, const mat2& m2) {
    mat2 m1_copy(m1);
    return m1_copy /= m2;
  }

  /* -- Boolean operators -- */

  inline bool operator==(const mat2& m1, const mat2& m2) {
    return (m1[0] == m2[0]) && (m1[1] == m2[1]);
  }

  inline bool operator!=(const mat2& m1, const mat2& m2) {
    return (m1[0] != m2[0]) || (m1[1] != m2[1]);
  }

  /* -- Address operator -- */

  inline const float* operator&(const mat2& m) { return &(m[0].x); }

}

/* -- std -- */

namespace std {

  // TODO: Fix allineation
  inline ostream& operator<<(ostream& os, const Graphick::Math::mat2& m) {
    os << "┌" << m[0].x << ", " << m[0].y << "┐\n└" <<
      m[1].x << ", " << m[1].y << "┘";
    return os;
  }

}