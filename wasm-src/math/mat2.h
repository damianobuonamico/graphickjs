/**
 * @file math/mat2.h
 * @brief This file contains the Mat2 struct, a templated 2D matrix.
 */

#pragma once

#include "vec2.h"

namespace graphick::math {

/**
 * @brief A 2D matrix struct.
 *
 * @struct Mat2
 */
template <typename T>
struct Mat2 {
  /* -- Component accesses -- */

  static constexpr uint8_t length() { return 2; }

  constexpr Vec2<T>& operator[](uint8_t i) { return this->value[i]; }

  constexpr Vec2<T> const& operator[](uint8_t i) const { return this->value[i]; }

  /* -- Constructors -- */

  constexpr Mat2() : value{Vec2<T>(1, 0), Vec2<T>(0, 1)} { }

  constexpr Mat2(const Mat2<T>& m) : value{Vec2<T>(m[0]), Vec2<T>(m[1])} { }

  constexpr explicit Mat2(T scalar) : value{Vec2<T>(scalar, 0), Vec2<T>(0, scalar)} { }

  constexpr Mat2(const Vec2<T> v0, const Vec2<T> v1) : value{Vec2<T>(v0), Vec2<T>(v1)} { }

  constexpr Mat2(T x0, T y0, T x1, T y1) : value{Vec2<T>(x0, y0), Vec2<T>(x1, y1)} { }

  template <typename U>
  constexpr Mat2(const Mat2<U>& m) : value{Vec2<T>(m[0]), Vec2<T>(m[1])} { }

  /* -- Static constructors -- */

  static constexpr Mat2<T> zero() { return Mat2<T>(T(0)); }

  static constexpr Mat2<T> identity() { return Mat2<T>(T(1)); }

  /* -- Assign operator -- */

  constexpr Mat2<T>& operator=(const Mat2<T>& m) {
    this->value[0] = m[0];
    this->value[1] = m[1];
    return *this;
  }

  /* -- Unary arithmetic operators -- */

  template <typename U>
  constexpr Mat2<T>& operator+=(U scalar) {
    this->value[0] += scalar;
    this->value[1] += scalar;
    return *this;
  }

  constexpr Mat2<T>& operator+=(const Mat2<T>& m) {
    this->value[0] += m[0];
    this->value[1] += m[1];
    return *this;
  }

  template <typename U>
  constexpr Mat2<T>& operator-=(U scalar) {
    this->value[0] -= scalar;
    this->value[1] -= scalar;
    return *this;
  }

  constexpr Mat2<T>& operator-=(const Mat2<T>& m) {
    this->value[0] -= m[0];
    this->value[1] -= m[1];
    return *this;
  }

  template <typename U>
  constexpr Mat2<T>& operator*=(U scalar) {
    this->value[0] *= scalar;
    this->value[1] *= scalar;
    return *this;
  }

  constexpr Mat2<T>& operator*=(const Mat2<T>& m) { return (*this = *this * m); }

  template <typename U>
  constexpr Mat2<T>& operator/=(U scalar) {
    this->value[0] /= scalar;
    this->value[1] /= scalar;
    return *this;
  }

  constexpr Mat2<T>& operator/=(const Mat2<T>& m) { return *this *= inverse(m); }

  /* -- Increment/Decrement operators -- */

  constexpr Mat2<T>& operator++() {
    ++this->value[0];
    ++this->value[1];
    return *this;
  }

  constexpr Mat2<T>& operator--() {
    --this->value[0];
    --this->value[1];
    return *this;
  }
private:
  Vec2<T> value[2];
};

/* -- Unary operators */

template <typename T>
constexpr Mat2<T> operator+(const Mat2<T>& m) {
  return m;
}

template <typename T>
constexpr Mat2<T> operator-(const Mat2<T>& m) {
  return Mat2<T>(-m[0], -m[1]);
}

/* -- Binary operators -- */

template <typename T, typename U>
constexpr Mat2<T> operator+(const Mat2<T>& m, U scalar) {
  return Mat2<T>(m[0] + scalar, m[1] + scalar);
}

template <typename T, typename U>
constexpr Mat2<T> operator+(U scalar, const Mat2<T>& m) {
  return Mat2<T>(scalar + m[0], scalar + m[1]);
}

template <typename T>
constexpr Mat2<T> operator+(const Mat2<T>& m1, const Mat2<T>& m2) {
  return Mat2<T>(m1[0] + m2[0], m1[1] + m2[1]);
}

template <typename T, typename U>
constexpr Mat2<T> operator-(const Mat2<T>& m, U scalar) {
  return Mat2<T>(m[0] - scalar, m[1] - scalar);
}

template <typename T, typename U>
constexpr Mat2<T> operator-(U scalar, const Mat2<T>& m) {
  return Mat2<T>(scalar - m[0], scalar - m[1]);
}

template <typename T>
constexpr Mat2<T> operator-(const Mat2<T>& m1, const Mat2<T>& m2) {
  return Mat2<T>(m1[0] - m2[0], m1[1] - m2[1]);
}

template <typename T, typename U>
constexpr Mat2<T> operator*(const Mat2<T>& m, U scalar) {
  return Mat2<T>(m[0] * scalar, m[1] * scalar);
}

template <typename T, typename U>
constexpr Mat2<T> operator*(U scalar, const Mat2<T>& m) {
  return Mat2<T>(scalar * m[0], scalar * m[1]);
}

template <typename T>
constexpr Vec2<T> operator*(const Mat2<T>& m, const Vec2<T>& v) {
  return Vec2<T>(m[0][0] * v.x + m[1][0] * v.y, m[0][1] * v.x + m[1][1] * v.y);
}

template <typename T>
constexpr Mat2<T> operator*(const Mat2<T>& m1, const Mat2<T>& m2) {
  return Mat2<T>(
    m1[0][0] * m2[0][0] + m1[0][1] * m2[1][0],
    m1[0][0] * m2[0][1] + m1[0][1] * m2[1][1],
    m1[1][0] * m2[0][0] + m1[1][1] * m2[1][0],
    m1[1][0] * m2[0][1] + m1[1][1] * m2[1][1]
  );
}

template <typename T, typename U>
constexpr Mat2<T> operator/(const Mat2<T>& m, U scalar) {
  return Mat2<T>(m[0] / scalar, m[1] / scalar);
}

template <typename T, typename U>
constexpr Mat2<T> operator/(U scalar, const Mat2<T>& m) {
  return Mat2<T>(scalar / m[0], scalar / m[1]);
}

template <typename T>
constexpr Vec2<T> operator/(const Mat2<T>& m, const Vec2<T>& v) {
  return inverse(m) * v;
}

template <typename T>
constexpr Mat2<T> operator/(const Mat2<T>& m1, const Mat2<T>& m2) {
  Mat2<T> m1_copy(m1);
  return m1_copy /= m2;
}

/* -- Boolean operators -- */

template <typename T>
constexpr bool operator==(const Mat2<T>& m1, const Mat2<T>& m2) {
  return m1[0] == m2[0] && m1[1] == m2[1];
}

template <typename T>
constexpr bool operator!=(const Mat2<T>& m1, const Mat2<T>& m2) {
  return !(m1 == m2);
}
}  // namespace graphick::math

namespace graphick::math {

using mat2 = math::Mat2<float>;
using dmat2 = math::Mat2<double>;
using imat2 = math::Mat2<int32_t>;
using umat2 = math::Mat2<uint8_t>;

}  // namespace graphick::math

namespace graphick {

using math::dmat2;
using math::imat2;
using math::mat2;
using math::umat2;

}  // namespace graphick
