/**
 * @file dvec3.inl
 * @brief This file contains the inline implementation of the dvec3 struct.
 */

#pragma once

#include <limits>

namespace Graphick::Math {

  /* -- Component accesses -- */

  constexpr double& dvec3::operator[](uint8_t i) {
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

  constexpr const double& dvec3::operator[](uint8_t i) const {
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

  constexpr dvec3::dvec3(double scalar) : x(scalar), y(scalar), z(scalar) {}

  constexpr dvec3::dvec3(double x, double y, double z) : x(x), y(y), z(z) {}

  /* -- Assign operator -- */

  constexpr dvec3& dvec3::operator=(const dvec3& v) {
    this->x = v.x;
    this->y = v.y;
    this->z = v.z;
    return *this;
  }

  /* -- Unary arithmetic operators -- */

  constexpr dvec3& dvec3::operator+=(double scalar) {
    this->x += scalar;
    this->y += scalar;
    this->z += scalar;
    return *this;
  }

  constexpr dvec3& dvec3::operator+=(const dvec3& v) {
    this->x += v.x;
    this->y += v.y;
    this->z += v.z;
    return *this;
  }

  constexpr dvec3& dvec3::operator-=(double scalar) {
    this->x -= scalar;
    this->y -= scalar;
    this->z -= scalar;
    return *this;
  }

  constexpr dvec3& dvec3::operator-=(const dvec3& v) {
    this->x -= v.x;
    this->y -= v.y;
    this->z -= v.z;
    return *this;
  }

  constexpr dvec3& dvec3::operator*=(double scalar) {
    this->x *= scalar;
    this->y *= scalar;
    this->z *= scalar;
    return *this;
  }

  constexpr dvec3& dvec3::operator*=(const dvec3& v) {
    this->x *= v.x;
    this->y *= v.y;
    this->z *= v.z;
    return *this;
  }

  constexpr dvec3& dvec3::operator/=(double scalar) {
    this->x /= scalar;
    this->y /= scalar;
    this->z /= scalar;
    return *this;
  }

  constexpr dvec3& dvec3::operator/=(const dvec3& v) {
    this->x /= v.x;
    this->y /= v.y;
    this->z /= v.z;
    return *this;
  }


  /* -- Increment/Decrement operators -- */

  constexpr dvec3& dvec3::operator++() {
    ++this->x;
    ++this->y;
    ++this->z;
    return *this;
  }

  constexpr dvec3& dvec3::operator--() {
    --this->x;
    --this->y;
    --this->z;
    return *this;
  }

  /* -- Unary operators */

  constexpr dvec3 operator+(const dvec3& v) {
    return v;
  }

  constexpr dvec3 operator-(const dvec3& v) {
    return dvec3(-v.x, -v.y, -v.z);
  }

  /* -- Binary operators -- */

  constexpr dvec3 operator+(const dvec3& v, double scalar) {
    return dvec3(v.x + scalar, v.y + scalar, v.z + scalar);
  }

  constexpr dvec3 operator+(double scalar, const dvec3& v) {
    return dvec3(scalar + v.x, scalar + v.y, scalar + v.z);
  }

  constexpr dvec3 operator+(const dvec3& v1, const dvec3& v2) {
    return dvec3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
  }

  constexpr dvec3 operator-(const dvec3& v, double scalar) {
    return dvec3(v.x - scalar, v.y - scalar, v.z - scalar);
  }

  constexpr dvec3 operator-(double scalar, const dvec3& v) {
    return dvec3(scalar - v.x, scalar - v.y, scalar - v.z);
  }

  constexpr dvec3 operator-(const dvec3& v1, const dvec3& v2) {
    return dvec3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
  }

  constexpr dvec3 operator*(const dvec3& v, double scalar) {
    return dvec3(v.x * scalar, v.y * scalar, v.z * scalar);
  }

  constexpr dvec3 operator*(double scalar, const dvec3& v) {
    return dvec3(scalar * v.x, scalar * v.y, scalar * v.z);
  }

  constexpr dvec3 operator*(const dvec3& v1, const dvec3& v2) {
    return dvec3(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z);
  }

  constexpr dvec3 operator/(const dvec3& v, double scalar) {
    return dvec3(v.x / scalar, v.y / scalar, v.z / scalar);
  }

  constexpr dvec3 operator/(double scalar, const dvec3& v) {
    return dvec3(scalar / v.x, scalar / v.y, scalar / v.z);
  }

  constexpr dvec3 operator/(const dvec3& v1, const dvec3& v2) {
    return dvec3(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z);
  }

  constexpr dvec3 operator%(const dvec3& v, double scalar) {
    return dvec3((double)((int)(v.x) % (int)(scalar)), (double)((int)(v.y) % (int)(scalar)), (double)((int)(v.z) % (int)(scalar)));
  }

  constexpr dvec3 operator%(double scalar, const dvec3& v) {
    return dvec3((double)((int)(scalar) % (int)(v.x)), (double)((int)(scalar) % (int)(v.y)), (double)((int)(scalar) % (int)(v.z)));
  }

  constexpr dvec3 operator%(const dvec3& v1, const dvec3& v2) {
    return dvec3((double)((int)(v1.x) % (int)(v2.x)), (double)((int)(v1.y) % (int)(v2.y)), (double)((int)(v1.z) % (int)(v2.z)));
  }

  /* -- Boolean operators -- */

  constexpr bool operator==(const dvec3& v1, const dvec3& v2) {
    return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z;
  }

  constexpr bool operator!=(const dvec3& v1, const dvec3& v2) {
    return !(v1 == v2);
  }

  constexpr dvec3 operator&&(const dvec3& v1, const dvec3& v2) {
    return dvec3(v1.x && v2.x, v1.y && v2.y, v1.z && v2.z);
  }

  constexpr dvec3 operator||(const dvec3& v1, const dvec3& v2) {
    return dvec3(v1.x || v2.x, v1.y || v2.y, v1.z || v2.z);
  }

  /* -- Address operator -- */

  constexpr const double* operator&(const dvec3& v) { return &(v.x); }

}

namespace std {

  /* -- numeric_limits -- */

  template<>
  class numeric_limits<Graphick::Math::dvec3> {
  public:
    static inline Graphick::Math::dvec3 min() {
      return Graphick::Math::dvec3{ numeric_limits<double>::min() };
    }

    static inline Graphick::Math::dvec3 max() {
      return Graphick::Math::dvec3{ numeric_limits<double>::max() };
    }

    static inline Graphick::Math::dvec3 lowest() {
      return Graphick::Math::dvec3{ numeric_limits<double>::lowest() };
    }
  };

}

