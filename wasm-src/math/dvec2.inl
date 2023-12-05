/**
 * @file dvec2.inl
 * @brief This file contains the inline implementation of the dvec2 struct.
 */

#pragma once

#include <limits>

namespace Graphick::Math {

  /* -- Component accesses -- */

  constexpr double& dvec2::operator[](uint8_t i) {
    switch (i) {
    default:
    case 0:
      return x;
    case 1:
      return y;
    }
  }

  constexpr const double& dvec2::operator[](uint8_t i) const {
    switch (i) {
    default:
    case 0:
      return x;
    case 1:
      return y;
    }
  }

  /* -- Constructors -- */

  constexpr dvec2::dvec2(double scalar) : x(scalar), y(scalar) {}

  constexpr dvec2::dvec2(double x, double y) : x(x), y(y) {}

  /* -- Assign operator -- */

  constexpr dvec2& dvec2::operator=(const dvec2 v) {
    this->x = v.x;
    this->y = v.y;
    return *this;
  }

  /* -- Unary arithmetic operators -- */

  constexpr dvec2& dvec2::operator+=(double scalar) {
    this->x += scalar;
    this->y += scalar;
    return *this;
  }

  constexpr dvec2& dvec2::operator+=(const dvec2 v) {
    this->x += v.x;
    this->y += v.y;
    return *this;
  }

  constexpr dvec2& dvec2::operator-=(double scalar) {
    this->x -= scalar;
    this->y -= scalar;
    return *this;
  }

  constexpr dvec2& dvec2::operator-=(const dvec2 v) {
    this->x -= v.x;
    this->y -= v.y;
    return *this;
  }

  constexpr dvec2& dvec2::operator*=(double scalar) {
    this->x *= scalar;
    this->y *= scalar;
    return *this;
  }

  constexpr dvec2& dvec2::operator*=(const dvec2 v) {
    this->x *= v.x;
    this->y *= v.y;
    return *this;
  }

  constexpr dvec2& dvec2::operator/=(double scalar) {
    this->x /= scalar;
    this->y /= scalar;
    return *this;
  }

  constexpr dvec2& dvec2::operator/=(const dvec2 v) {
    this->x /= v.x;
    this->y /= v.y;
    return *this;
  }

  /* -- Increment/Decrement operators -- */

  constexpr dvec2& dvec2::operator++() {
    ++this->x;
    ++this->y;
    return *this;
  }

  constexpr dvec2& dvec2::operator--() {
    --this->x;
    --this->y;
    return *this;
  }

  /* -- Unary operators */

  constexpr dvec2 operator+(const dvec2 v) {
    return v;
  }

  constexpr dvec2 operator-(const dvec2 v) {
    return dvec2(-v.x, -v.y);
  }

  /* -- Binary operators -- */

  constexpr dvec2 operator+(const dvec2 v, double scalar) {
    return dvec2(v.x + scalar, v.y + scalar);
  }

  constexpr dvec2 operator+(double scalar, const dvec2 v) {
    return dvec2(scalar + v.x, scalar + v.y);
  }

  constexpr dvec2 operator+(const dvec2 v1, const dvec2 v2) {
    return dvec2(v1.x + v2.x, v1.y + v2.y);
  }

  constexpr dvec2 operator-(const dvec2 v, double scalar) {
    return dvec2(v.x - scalar, v.y - scalar);
  }

  constexpr dvec2 operator-(double scalar, const dvec2 v) {
    return dvec2(scalar - v.x, scalar - v.y);
  }

  constexpr dvec2 operator-(const dvec2 v1, const dvec2 v2) {
    return dvec2(v1.x - v2.x, v1.y - v2.y);
  }

  constexpr dvec2 operator*(const dvec2 v, double scalar) {
    return dvec2(v.x * scalar, v.y * scalar);
  }

  constexpr dvec2 operator*(double scalar, const dvec2 v) {
    return dvec2(scalar * v.x, scalar * v.y);
  }

  constexpr dvec2 operator*(const dvec2 v1, const dvec2 v2) {
    return dvec2(v1.x * v2.x, v1.y * v2.y);
  }

  constexpr dvec2 operator/(const dvec2 v, double scalar) {
    return dvec2(v.x / scalar, v.y / scalar);
  }

  constexpr dvec2 operator/(double scalar, const dvec2 v) {
    return dvec2(scalar / v.x, scalar / v.y);
  }

  constexpr dvec2 operator/(const dvec2 v1, const dvec2 v2) {
    return dvec2(v1.x / v2.x, v1.y / v2.y);
  }

  constexpr dvec2 operator%(const dvec2 v, double scalar) {
    return dvec2((double)((int)v.x % (int)scalar), (double)((int)v.y % (int)scalar));
  }

  constexpr dvec2 operator%(double scalar, const dvec2 v) {
    return dvec2((double)((int)scalar % (int)v.x), (double)((int)scalar % (int)v.y));
  }

  constexpr dvec2 operator%(const dvec2 v1, const dvec2 v2) {
    return dvec2((double)((int)v1.x % (int)v2.x), (double)((int)v1.y % (int)v2.y));
  }

  /* -- Boolean operators -- */

  constexpr bool operator==(const dvec2 v1, const dvec2 v2) {
    return v1.x == v2.x && v1.y == v2.y;
  }

  constexpr bool operator!=(const dvec2 v1, const dvec2 v2) {
    return !(v1 == v2);
  }

  constexpr dvec2 operator&&(const dvec2 v1, const dvec2 v2) {
    return dvec2(v1.x && v2.x, v1.y && v2.y);
  }

  constexpr dvec2 operator||(const dvec2 v1, const dvec2 v2) {
    return dvec2(v1.x || v2.x, v1.y || v2.y);
  }

  /* -- Address operator -- */

  constexpr const double* operator&(const dvec2& v) { return &(v.x); }

}

namespace std {

  /* -- numeric_limits -- */

  template<>
  class numeric_limits<Graphick::Math::dvec2> {
  public:
    static inline Graphick::Math::dvec2 min() {
      return Graphick::Math::dvec2{ numeric_limits<double>::min() };
    }

    static inline Graphick::Math::dvec2 max() {
      return Graphick::Math::dvec2{ numeric_limits<double>::max() };
    }

    static inline Graphick::Math::dvec2 lowest() {
      return Graphick::Math::dvec2{ numeric_limits<double>::lowest() };
    }
  };

}
