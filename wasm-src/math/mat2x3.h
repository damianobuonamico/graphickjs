/**
 * @file math/mat2x3.h
 * @brief This file contains the Mat2x3 struct, a templated 2x3 matrix.
 */

#pragma once

#include "vec2.h"
#include "vec3.h"

namespace graphick::math {

/**
 * @brief A 2x3 matrix struct with 3 columns and 2 rows.
 *
 * This matrix is not mathematically correct, it is only used to reduce the memory footprint of 2D transforms.
 * The missing row is always interpreted as [0, 0, 1].
 * When multiplying a Vec2<T> with this matrix, the third component of the vector is always treated as 1.
 *
 * @struct Mat2x3
 */
template <typename T>
struct Mat2x3 {
  /* -- Component accesses -- */

  static constexpr uint8_t length() { return 2; }

  constexpr Vec3<T>& operator[](uint8_t i) { return this->value[i]; }

  constexpr Vec3<T> const& operator[](uint8_t i) const { return this->value[i]; }

  /* -- Constructors -- */

  constexpr Mat2x3() : value{Vec3<T>(1, 0, 0), Vec3<T>(0, 1, 0)} { }

  constexpr Mat2x3(const Mat2x3<T>& m) : value{Vec3<T>(m[0]), Vec3<T>(m[1])} { }

  constexpr explicit Mat2x3(T scalar) : value{Vec3<T>(scalar, 0, 0), Vec3<T>(0, scalar, 0)} { }

  constexpr Mat2x3(const Vec3<T>& v0, const Vec3<T>& v1) : value{Vec3<T>(v0), Vec3<T>(v1)} { }

  constexpr Mat2x3(T x0, T y0, T z0, T x1, T y1, T z1) : value{Vec3<T>(x0, y0, z0), Vec3<T>(x1, y1, z1)} { }

  template <typename U>
  constexpr Mat2x3(const Mat2x3<U>& m) : value{Vec3<T>(m[0]), Vec3<T>(m[1])} { }

  /* -- Static constructors -- */

  static constexpr Mat2x3<T> zero() { return Mat2x3<T>(T(0)); }

  static constexpr Mat2x3<T> identity() { return Mat2x3<T>(T(1)); }

  static constexpr Mat2x3<T> from_translation(const Vec2<T>& v) { return Mat2x3<T>(1, 0, v.x, 0, 1, v.y); }

  static constexpr Mat2x3<T> from_rotation(const T sin, const T cos) { return Mat2x3<T>(cos, -sin, 0, sin, cos, 0); }

  static constexpr Mat2x3<T> from_scale(const Vec2<T>& v) { return Mat2x3<T>(v.x, 0, 0, 0, v.y, 0); }

  static constexpr Mat2x3<T> from_translation_rotation_scale(
    const Vec2<T>& translation,
    const T sin,
    const T cos,
    const Vec2<T>& scale
  ) {
    return Mat2x3<T>(scale.x * cos, -scale.y * sin, translation.x, scale.x * sin, scale.y * cos, translation.y);
  }

  /* -- Assign operator -- */

  constexpr Mat2x3<T>& operator=(const Mat2x3<T>& m) {
    this->value[0] = m[0];
    this->value[1] = m[1];
    return *this;
  }

  /* -- Unary arithmetic operators -- */

  template <typename U>
  constexpr Mat2x3<T>& operator+=(U scalar) {
    this->value[0] += scalar;
    this->value[1] += scalar;
    return *this;
  }

  constexpr Mat2x3<T>& operator+=(const Mat2x3<T>& m) {
    this->value[0] += m[0];
    this->value[1] += m[1];
    return *this;
  }

  template <typename U>
  constexpr Mat2x3<T>& operator-=(U scalar) {
    this->value[0] -= scalar;
    this->value[1] -= scalar;
    return *this;
  }

  constexpr Mat2x3<T>& operator-=(const Mat2x3<T>& m) {
    this->value[0] -= m[0];
    this->value[1] -= m[1];
    return *this;
  }

  template <typename U>
  constexpr Mat2x3<T>& operator*=(U scalar) {
    this->value[0] *= scalar;
    this->value[1] *= scalar;
    return *this;
  }

  constexpr Mat2x3<T>& operator*=(const Mat2x3<T>& m) { return (*this = *this * m); }

  template <typename U>
  constexpr Mat2x3<T>& operator/=(U scalar) {
    this->value[0] /= scalar;
    this->value[1] /= scalar;
    return *this;
  }

  /* -- Increment/Decrement operators -- */

  constexpr Mat2x3<T>& operator++() {
    ++this->value[0];
    ++this->value[1];
    return *this;
  }

  constexpr Mat2x3<T>& operator--() {
    --this->value[0];
    --this->value[1];
    return *this;
  }
private:
  Vec3<T> value[2];
};

/* -- Unary operators */

template <typename T>
constexpr Mat2x3<T> operator+(const Mat2x3<T>& m) {
  return m;
}

template <typename T>
constexpr Mat2x3<T> operator-(const Mat2x3<T>& m) {
  return Mat2x3<T>(-m[0], -m[1]);
}

/* -- Binary operators -- */

template <typename T, typename U>
constexpr Mat2x3<T> operator+(const Mat2x3<T>& m, U scalar) {
  return Mat2x3<T>(m[0] + scalar, m[1] + scalar);
}

template <typename T, typename U>
constexpr Mat2x3<T> operator+(U scalar, const Mat2x3<T>& m) {
  return Mat2x3<T>(scalar + m[0], scalar + m[1]);
}

template <typename T>
constexpr Mat2x3<T> operator+(const Mat2x3<T>& m1, const Mat2x3<T>& m2) {
  return Mat2x3<T>(m1[0] + m2[0], m1[1] + m2[1]);
}

template <typename T, typename U>
constexpr Mat2x3<T> operator-(const Mat2x3<T>& m, U scalar) {
  return Mat2x3<T>(m[0] - scalar, m[1] - scalar);
}

template <typename T, typename U>
constexpr Mat2x3<T> operator-(U scalar, const Mat2x3<T>& m) {
  return Mat2x3<T>(scalar - m[0], scalar - m[1]);
}

template <typename T>
constexpr Mat2x3<T> operator-(const Mat2x3<T>& m1, const Mat2x3<T>& m2) {
  return Mat2x3<T>(m1[0] - m2[0], m1[1] - m2[1]);
}

template <typename T, typename U>
constexpr Mat2x3<T> operator*(const Mat2x3<T>& m, U scalar) {
  return Mat2x3<T>(m[0] * scalar, m[1] * scalar);
}

template <typename T, typename U>
constexpr Mat2x3<T> operator*(U scalar, const Mat2x3<T>& m) {
  return Mat2x3<T>(scalar * m[0], scalar * m[1]);
}

template <typename T>
constexpr Vec2<T> operator*(const Mat2x3<T>& m, const Vec2<T>& v) {
  return Vec2<T>(m[0][0] * v.x + m[0][1] * v.y + m[0][2], m[1][0] * v.x + m[1][1] * v.y + m[1][2]);
}

template <typename T>
constexpr Mat2x3<T> operator*(const Mat2x3<T>& m1, const Mat2x3<T>& m2) {
  return Mat2x3<T>(
    m1[0][0] * m2[0][0] + m1[0][1] * m2[1][0],
    m1[0][0] * m2[0][1] + m1[0][1] * m2[1][1],
    m1[0][0] * m2[0][2] + m1[0][1] * m2[1][2] + m1[0][2],
    m1[1][0] * m2[0][0] + m1[1][1] * m2[1][0],
    m1[1][0] * m2[0][1] + m1[1][1] * m2[1][1],
    m1[1][0] * m2[0][2] + m1[1][1] * m2[1][2] + m1[1][2]
  );
}

template <typename T, typename U>
constexpr Mat2x3<T> operator/(const Mat2x3<T>& m, U scalar) {
  return Mat2x3<T>(m[0] / scalar, m[1] / scalar);
}

template <typename T, typename U>
constexpr Mat2x3<T> operator/(U scalar, const Mat2x3<T>& m) {
  return Mat2x3<T>(scalar / m[0], scalar / m[1]);
}

template <typename T>
constexpr Mat2x3<T> operator/(const Mat2x3<T>& m1, const Mat2x3<T>& m2) {
  Mat2x3<T> m1_copy(m1);
  return m1_copy /= m2;
}

/* -- Boolean operators -- */

template <typename T>
constexpr bool operator==(const Mat2x3<T>& m1, const Mat2x3<T>& m2) {
  return m1[0] == m2[0] && m1[1] == m2[1];
}

template <typename T>
constexpr bool operator!=(const Mat2x3<T>& m1, const Mat2x3<T>& m2) {
  return !(m1 == m2);
}
}  // namespace graphick::math

namespace graphick::math {

using mat2x3 = math::Mat2x3<float>;
using dmat2x3 = math::Mat2x3<double>;
using imat2x3 = math::Mat2x3<int32_t>;
using umat2x3 = math::Mat2x3<uint32_t>;

}  // namespace graphick::math

namespace graphick {

using math::dmat2x3;
using math::imat2x3;
using math::mat2x3;
using math::umat2x3;

}  // namespace graphick
