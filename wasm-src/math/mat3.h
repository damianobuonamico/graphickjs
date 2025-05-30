/**
 * @file math/mat3.h
 * @brief This file contains the Mat3 struct, a templated 3D matrix.
 */

#pragma once

#include "vec3.h"

namespace graphick::math {

/**
 * @brief A 3D matrix struct.
 */
template<typename T>
struct Mat3 {
  /* -- Component accesses -- */

  static constexpr uint8_t length()
  {
    return 3;
  }

  constexpr Vec3<T> &operator[](uint8_t i)
  {
    return this->value[i];
  }

  constexpr Vec3<T> const &operator[](uint8_t i) const
  {
    return this->value[i];
  }

  /* -- Constructors -- */

  constexpr Mat3() : value{Vec3<T>(1, 0, 0), Vec3<T>(0, 1, 0), Vec3<T>(0, 0, 1)} {}

  constexpr Mat3(const Mat3<T> &m) : value{Vec3<T>(m[0]), Vec3<T>(m[1]), Vec3<T>(m[2])} {}

  constexpr explicit Mat3(T scalar)
      : value{Vec3<T>(scalar, 0, 0), Vec3<T>(0, scalar, 0), Vec3<T>(0, 0, scalar)}
  {
  }

  constexpr Mat3(const Vec3<T> &v0, const Vec3<T> &v1, const Vec3<T> &v2)
      : value{Vec3<T>(v0), Vec3<T>(v1), Vec3<T>(v2)}
  {
  }

  constexpr Mat3(T x0, T y0, T z0, T x1, T y1, T z1, T x2, T y2, T z2)
      : value{Vec3<T>(x0, y0, z0), Vec3<T>(x1, y1, z1), Vec3<T>(x2, y2, z2)}
  {
  }

  template<typename U>
  constexpr Mat3(const Mat3<U> &m) : value{Vec3<T>(m[0]), Vec3<T>(m[1]), Vec3<T>(m[2])}
  {
  }

  /* -- Static constructors -- */

  static constexpr Mat3<T> zero()
  {
    return Mat3<T>(T(0));
  }

  static constexpr Mat3<T> identity()
  {
    return Mat3<T>(T(1));
  }

  /* -- Assign operator -- */

  constexpr Mat3<T> &operator=(const Mat3<T> &m)
  {
    this->value[0] = m[0];
    this->value[1] = m[1];
    this->value[2] = m[2];
    return *this;
  }

  /* -- Unary arithmetic operators -- */

  template<typename U>
  constexpr Mat3<T> &operator+=(U scalar)
  {
    this->value[0] += scalar;
    this->value[1] += scalar;
    this->value[2] += scalar;
    return *this;
  }

  constexpr Mat3<T> &operator+=(const Mat3<T> &m)
  {
    this->value[0] += m[0];
    this->value[1] += m[1];
    this->value[2] += m[2];
    return *this;
  }

  template<typename U>
  constexpr Mat3<T> &operator-=(U scalar)
  {
    this->value[0] -= scalar;
    this->value[1] -= scalar;
    this->value[2] -= scalar;
    return *this;
  }

  constexpr Mat3<T> &operator-=(const Mat3<T> &m)
  {
    this->value[0] -= m[0];
    this->value[1] -= m[1];
    this->value[2] -= m[2];
    return *this;
  }

  template<typename U>
  constexpr Mat3<T> &operator*=(U scalar)
  {
    this->value[0] *= scalar;
    this->value[1] *= scalar;
    this->value[2] *= scalar;
    return *this;
  }

  constexpr Mat3<T> &operator*=(const Mat3<T> &m)
  {
    return (*this = *this * m);
  }

  template<typename U>
  constexpr Mat3<T> &operator/=(U scalar)
  {
    this->value[0] /= scalar;
    this->value[1] /= scalar;
    this->value[2] /= scalar;
    return *this;
  }

  /* -- Increment/Decrement operators -- */

  constexpr Mat3<T> &operator++()
  {
    ++this->value[0];
    ++this->value[1];
    ++this->value[2];
    return *this;
  }

  constexpr Mat3<T> &operator--()
  {
    --this->value[0];
    --this->value[1];
    --this->value[2];
    return *this;
  }

 private:
  Vec3<T> value[3];
};

/* -- Unary operators */

template<typename T>
constexpr Mat3<T> operator+(const Mat3<T> &m);

template<typename T>
constexpr Mat3<T> operator-(const Mat3<T> &m);

/* -- Binary operators -- */

template<typename T, typename U>
constexpr Mat3<T> operator+(const Mat3<T> &m, U scalar)
{
  return Mat3<T>(m[0] + scalar, m[1] + scalar, m[2] + scalar);
}

template<typename T, typename U>
constexpr Mat3<T> operator+(U scalar, const Mat3<T> &m)
{
  return Mat3<T>(scalar + m[0], scalar + m[1], scalar + m[2]);
}

template<typename T>
constexpr Mat3<T> operator+(const Mat3<T> &m1, const Mat3<T> &m2)
{
  return Mat3<T>(m1[0] + m2[0], m1[1] + m2[1], m1[2] + m2[2]);
}

template<typename T, typename U>
constexpr Mat3<T> operator-(const Mat3<T> &m, U scalar)
{
  return Mat3<T>(m[0] - scalar, m[1] - scalar, m[2] - scalar);
}

template<typename T, typename U>
constexpr Mat3<T> operator-(U scalar, const Mat3<T> &m)
{
  return Mat3<T>(scalar - m[0], scalar - m[1], scalar - m[2]);
}

template<typename T>
constexpr Mat3<T> operator-(const Mat3<T> &m1, const Mat3<T> &m2)
{
  return Mat3<T>(m1[0] - m2[0], m1[1] - m2[1], m1[2] - m2[2]);
}

template<typename T, typename U>
constexpr Mat3<T> operator*(const Mat3<T> &m, U scalar)
{
  return Mat3<T>(m[0] * scalar, m[1] * scalar, m[2] * scalar);
}

template<typename T, typename U>
constexpr Mat3<T> operator*(U scalar, const Mat3<T> &m)
{
  return Mat3<T>(scalar * m[0], scalar * m[1], scalar * m[2]);
}

template<typename T>
constexpr Vec3<T> operator*(const Mat3<T> &m, const Vec3<T> &v)
{
  return Vec3<T>(m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z,
                 m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z,
                 m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z);
}

template<typename T>
constexpr Mat3<T> operator*(const Mat3<T> &m1, const Mat3<T> &m2)
{
  return Mat3<T>(m1[0][0] * m2[0][0] + m1[0][1] * m2[1][0] + m1[0][2] * m2[2][0],
                 m1[0][0] * m2[0][1] + m1[0][1] * m2[1][1] + m1[0][2] * m2[2][1],
                 m1[0][0] * m2[0][2] + m1[0][1] * m2[1][2] + m1[0][2] * m2[2][2],
                 m1[1][0] * m2[0][0] + m1[1][1] * m2[1][0] + m1[1][2] * m2[2][0],
                 m1[1][0] * m2[0][1] + m1[1][1] * m2[1][1] + m1[1][2] * m2[2][1],
                 m1[1][0] * m2[0][2] + m1[1][1] * m2[1][2] + m1[1][2] * m2[2][2],
                 m1[2][0] * m2[0][0] + m1[2][1] * m2[1][0] + m1[2][2] * m2[2][0],
                 m1[2][0] * m2[0][1] + m1[2][1] * m2[1][1] + m1[2][2] * m2[2][1],
                 m1[2][0] * m2[0][2] + m1[2][1] * m2[1][2] + m1[2][2] * m2[2][2]);
}

template<typename T, typename U>
constexpr Mat3<T> operator/(const Mat3<T> &m, U scalar)
{
  return Mat3<T>(m[0] / scalar, m[1] / scalar, m[2] / scalar);
}

template<typename T, typename U>
constexpr Mat3<T> operator/(U scalar, const Mat3<T> &m)
{
  return Mat3<T>(scalar / m[0], scalar / m[1], scalar / m[2]);
}

template<typename T>
constexpr Mat3<T> operator/(const Mat3<T> &m1, const Mat3<T> &m2)
{
  Mat3<T> m1_copy(m1);
  return m1_copy /= m2;
}

/* -- Boolean operators -- */

template<typename T>
constexpr bool operator==(const Mat3<T> &m1, const Mat3<T> &m2)
{
  return m1[0] == m2[0] && m1[1] == m2[1] && m1[2] == m2[2];
}

template<typename T>
constexpr bool operator!=(const Mat3<T> &m1, const Mat3<T> &m2)
{
  return !(m1 == m2);
}
}  // namespace graphick::math

namespace graphick::math {

using mat3 = math::Mat3<float>;
using dmat3 = math::Mat3<double>;
using imat3 = math::Mat3<int32_t>;
using umat3 = math::Mat3<uint8_t>;

}  // namespace graphick::math

namespace graphick {

using math::dmat3;
using math::imat3;
using math::mat3;
using math::umat3;

}  // namespace graphick
