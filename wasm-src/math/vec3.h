/**
 * @file math/vec3.h
 * @brief This file contains the Vec3 struct, a templated 3D vector.
 */

#pragma once

#include <cstdint>
#include <limits>

namespace graphick::math {

/**
 * @brief A 3D vector struct with x, y and z components.
 */
template<typename T>
struct Vec3 {
  union {
    T x, r, s;
  };  // The 0 component of the vector.
  union {
    T y, g, t;
  };  // The 1 component of the vector.
  union {
    T z, b, p;
  };  // The 2 component of the vector.

  /* -- Component accesses -- */

  static constexpr uint8_t length()
  {
    return 3;
  }

  constexpr T &operator[](uint8_t i)
  {
    switch (i) {
      default:
      case 0:
        return x;
      case 1:
        return y;
      case 2:
        return z;
    }
  }

  constexpr T const &operator[](uint8_t i) const
  {
    switch (i) {
      default:
      case 0:
        return x;
      case 1:
        return y;
      case 2:
        return z;
    }
  }

  /* -- Constructors -- */

  Vec3() = default;

  constexpr Vec3(const Vec3<T> &v) : x(v.x), y(v.y), z(v.z) {}

  constexpr explicit Vec3(T scalar) : x(scalar), y(scalar), z(scalar) {}

  constexpr Vec3(T x, T y, T z) : x(x), y(y), z(z) {}

  template<typename U>
  constexpr explicit Vec3(const Vec3<U> &v)
      : x(static_cast<T>(v.x)), y(static_cast<T>(v.y)), z(static_cast<T>(v.z))
  {
  }

  /* -- Static constructors -- */

  static constexpr Vec3<T> zero()
  {
    return Vec3<T>(T(0));
  }

  static constexpr Vec3<T> identity()
  {
    return Vec3<T>(T(1));
  }

  /* -- Assign operator -- */

  constexpr Vec3<T> &operator=(const Vec3<T> &v)
  {
    this->x = v.x;
    this->y = v.y;
    this->z = v.z;
    return *this;
  }

  /* -- Unary arithmetic operators -- */

  template<typename U>
  constexpr Vec3<T> &operator+=(U scalar)
  {
    this->x += static_cast<T>(scalar);
    this->y += static_cast<T>(scalar);
    this->z += static_cast<T>(scalar);
    return *this;
  }

  constexpr Vec3<T> &operator+=(const Vec3<T> &v)
  {
    this->x += v.x;
    this->y += v.y;
    this->z += v.z;
    return *this;
  }

  template<typename U>
  constexpr Vec3<T> &operator-=(U scalar)
  {
    this->x -= static_cast<T>(scalar);
    this->y -= static_cast<T>(scalar);
    this->z -= static_cast<T>(scalar);
    return *this;
  }

  constexpr Vec3<T> &operator-=(const Vec3<T> &v)
  {
    this->x -= v.x;
    this->y -= v.y;
    this->z -= v.z;
    return *this;
  }

  template<typename U>
  constexpr Vec3<T> &operator*=(U scalar)
  {
    this->x *= static_cast<T>(scalar);
    this->y *= static_cast<T>(scalar);
    this->z *= static_cast<T>(scalar);
    return *this;
  }

  constexpr Vec3<T> &operator*=(const Vec3<T> &v)
  {
    this->x *= v.x;
    this->y *= v.y;
    this->z *= v.z;
    return *this;
  }

  template<typename U>
  constexpr Vec3<T> &operator/=(U scalar)
  {
    this->x /= static_cast<T>(scalar);
    this->y /= static_cast<T>(scalar);
    this->z /= static_cast<T>(scalar);
    return *this;
  }

  constexpr Vec3<T> &operator/=(const Vec3<T> &v)
  {
    this->x /= v.x;
    this->y /= v.y;
    this->z /= v.z;
    return *this;
  }

  /* -- Increment/Decrement operators -- */

  constexpr Vec3<T> &operator++()
  {
    ++this->x;
    ++this->y;
    ++this->z;
    return *this;
  }

  constexpr Vec3<T> &operator--()
  {
    --this->x;
    --this->y;
    --this->z;
    return *this;
  }
};

/* -- Unary operators */

template<typename T>
constexpr Vec3<T> operator+(const Vec3<T> &v)
{
  return v;
}

template<typename T>
constexpr Vec3<T> operator-(const Vec3<T> &v)
{
  return Vec3<T>(-v.x, -v.y, -v.z);
}

/* -- Binary operators -- */

template<typename T, typename U>
constexpr Vec3<T> operator+(const Vec3<T> &v, U scalar)
{
  return Vec3<T>(
      v.x + static_cast<T>(scalar), v.y + static_cast<T>(scalar), v.z + static_cast<T>(scalar));
}

template<typename T, typename U>
constexpr Vec3<T> operator+(U scalar, const Vec3<T> &v)
{
  return Vec3<T>(
      static_cast<T>(scalar) + v.x, static_cast<T>(scalar) + v.y, static_cast<T>(scalar) + v.z);
}

template<typename T>
constexpr Vec3<T> operator+(const Vec3<T> &v1, const Vec3<T> &v2)
{
  return Vec3<T>(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}

template<typename T, typename U>
constexpr Vec3<T> operator-(const Vec3<T> &v, U scalar)
{
  return Vec3<T>(
      v.x - static_cast<T>(scalar), v.y - static_cast<T>(scalar), v.z - static_cast<T>(scalar));
}

template<typename T, typename U>
constexpr Vec3<T> operator-(U scalar, const Vec3<T> &v)
{
  return Vec3<T>(
      static_cast<T>(scalar) - v.x, static_cast<T>(scalar) - v.y, static_cast<T>(scalar) - v.z);
}

template<typename T>
constexpr Vec3<T> operator-(const Vec3<T> &v1, const Vec3<T> &v2)
{
  return Vec3<T>(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}

template<typename T, typename U>
constexpr Vec3<T> operator*(const Vec3<T> &v, U scalar)
{
  return Vec3<T>(
      v.x * static_cast<T>(scalar), v.y * static_cast<T>(scalar), v.z * static_cast<T>(scalar));
}

template<typename T, typename U>
constexpr Vec3<T> operator*(U scalar, const Vec3<T> &v)
{
  return Vec3<T>(
      static_cast<T>(scalar) * v.x, static_cast<T>(scalar) * v.y, static_cast<T>(scalar) * v.z);
}

template<typename T>
constexpr Vec3<T> operator*(const Vec3<T> &v1, const Vec3<T> &v2)
{
  return Vec3<T>(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z);
}

template<typename T, typename U>
constexpr Vec3<T> operator/(const Vec3<T> &v, U scalar)
{
  return Vec3<T>(
      v.x / static_cast<T>(scalar), v.y / static_cast<T>(scalar), v.z / static_cast<T>(scalar));
}

template<typename T, typename U>
constexpr Vec3<T> operator/(U scalar, const Vec3<T> &v)
{
  return Vec3<T>(
      static_cast<T>(scalar) / v.x, static_cast<T>(scalar) / v.y, static_cast<T>(scalar) / v.z);
}

template<typename T>
constexpr Vec3<T> operator/(const Vec3<T> &v1, const Vec3<T> &v2)
{
  return Vec3<T>(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z);
}

template<typename T, typename U>
constexpr Vec3<T> operator%(const Vec3<T> &v, U scalar)
{
  return Vec3<T>(static_cast<T>(static_cast<int>(v.x) % static_cast<int>(scalar)),
                 static_cast<T>(static_cast<int>(v.y) % static_cast<int>(scalar)),
                 static_cast<T>(static_cast<int>(v.z) % static_cast<int>(scalar)));
}

template<typename T, typename U>
constexpr Vec3<T> operator%(U scalar, const Vec3<T> &v)
{
  return Vec3<T>(static_cast<T>(static_cast<int>(scalar) % static_cast<int>(v.x)),
                 static_cast<T>(static_cast<int>(scalar) % static_cast<int>(v.y)),
                 static_cast<T>(static_cast<int>(scalar) % static_cast<int>(v.z)));
}

template<typename T>
constexpr Vec3<T> operator%(const Vec3<T> &v1, const Vec3<T> &v2)
{
  return Vec3<T>(static_cast<T>(static_cast<int>(v1.x) % static_cast<int>(v2.x)),
                 static_cast<T>(static_cast<int>(v1.y) % static_cast<int>(v2.y)),
                 static_cast<T>(static_cast<int>(v1.z) % static_cast<int>(v2.z)));
}

/* -- Boolean operators -- */

template<typename T>
constexpr bool operator==(const Vec3<T> &v1, const Vec3<T> &v2)
{
  return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z;
}

template<typename T>
constexpr bool operator!=(const Vec3<T> &v1, const Vec3<T> &v2)
{
  return !(v1 == v2);
}

template<typename T>
constexpr Vec3<T> operator&&(const Vec3<T> &v1, const Vec3<T> &v2)
{
  return Vec3<T>(v1.x && v2.x, v1.y && v2.y, v1.z && v2.z);
}

template<typename T>
constexpr Vec3<T> operator||(const Vec3<T> &v1, const Vec3<T> &v2)
{
  return Vec3<T>(v1.x || v2.x, v1.y || v2.y, v1.z || v2.z);
}
}  // namespace graphick::math

/* -- Aliases -- */

namespace graphick::math {

using vec3 = math::Vec3<float>;
using dvec3 = math::Vec3<double>;
using ivec3 = math::Vec3<int32_t>;
using uvec3 = math::Vec3<uint8_t>;

}  // namespace graphick::math

namespace graphick {

using math::dvec3;
using math::ivec3;
using math::uvec3;
using math::vec3;

}  // namespace graphick
