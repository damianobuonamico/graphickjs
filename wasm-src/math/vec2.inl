/**
 * @file vec2.inl
 * @brief This file contains the inline implementation of the vec2 struct.
 */

#pragma once

#include <limits>

namespace Graphick::Math {

  /* -- Component accesses -- */

  template<typename T>
  constexpr float& vec2<T>::operator[](uint8_t i) {
    switch (i) {
    default:
    case 0:
      return x;
    case 1:
      return y;
    }
  }

  constexpr const float& vec2::operator[](uint8_t i) const {
    switch (i) {
    default:
    case 0:
      return x;
    case 1:
      return y;
    }
  }

  /* -- Constructors -- */

  constexpr vec2::vec2(float scalar) : x(scalar), y(scalar) {}

  constexpr vec2::vec2(float x, float y) : x(x), y(y) {}

  /* -- Assign operator -- */

  constexpr vec2& vec2::operator=(const vec2 v) {
    this->x = v.x;
    this->y = v.y;
    return *this;
  }

  /* -- Unary arithmetic operators -- */

  constexpr vec2& vec2::operator+=(float scalar) {
    this->x += scalar;
    this->y += scalar;
    return *this;
  }

  constexpr vec2& vec2::operator+=(const vec2 v) {
    this->x += v.x;
    this->y += v.y;
    return *this;
  }

  constexpr vec2& vec2::operator-=(float scalar) {
    this->x -= scalar;
    this->y -= scalar;
    return *this;
  }

  constexpr vec2& vec2::operator-=(const vec2 v) {
    this->x -= v.x;
    this->y -= v.y;
    return *this;
  }

  constexpr vec2& vec2::operator*=(float scalar) {
    this->x *= scalar;
    this->y *= scalar;
    return *this;
  }

  constexpr vec2& vec2::operator*=(const vec2 v) {
    this->x *= v.x;
    this->y *= v.y;
    return *this;
  }

  constexpr vec2& vec2::operator/=(float scalar) {
    this->x /= scalar;
    this->y /= scalar;
    return *this;
  }

  constexpr vec2& vec2::operator/=(const vec2 v) {
    this->x /= v.x;
    this->y /= v.y;
    return *this;
  }

  /* -- Increment/Decrement operators -- */

  constexpr vec2& vec2::operator++() {
    ++this->x;
    ++this->y;
    return *this;
  }

  constexpr vec2& vec2::operator--() {
    --this->x;
    --this->y;
    return *this;
  }

  /* -- Unary operators */

  constexpr vec2 operator+(const vec2 v) {
    return v;
  }

  constexpr vec2 operator-(const vec2 v) {
    return vec2(-v.x, -v.y);
  }

  /* -- Binary operators -- */

  constexpr vec2 operator+(const vec2 v, float scalar) {
    return vec2(v.x + scalar, v.y + scalar);
  }

  constexpr vec2 operator+(float scalar, const vec2 v) {
    return vec2(scalar + v.x, scalar + v.y);
  }

  constexpr vec2 operator+(const vec2 v1, const vec2 v2) {
    return vec2(v1.x + v2.x, v1.y + v2.y);
  }

  constexpr vec2 operator-(const vec2 v, float scalar) {
    return vec2(v.x - scalar, v.y - scalar);
  }

  constexpr vec2 operator-(float scalar, const vec2 v) {
    return vec2(scalar - v.x, scalar - v.y);
  }

  constexpr vec2 operator-(const vec2 v1, const vec2 v2) {
    return vec2(v1.x - v2.x, v1.y - v2.y);
  }

  constexpr vec2 operator*(const vec2 v, float scalar) {
    return vec2(v.x * scalar, v.y * scalar);
  }

  constexpr vec2 operator*(float scalar, const vec2 v) {
    return vec2(scalar * v.x, scalar * v.y);
  }

  constexpr vec2 operator*(const vec2 v1, const vec2 v2) {
    return vec2(v1.x * v2.x, v1.y * v2.y);
  }

  constexpr vec2 operator/(const vec2 v, float scalar) {
    return vec2(v.x / scalar, v.y / scalar);
  }

  constexpr vec2 operator/(float scalar, const vec2 v) {
    return vec2(scalar / v.x, scalar / v.y);
  }

  constexpr vec2 operator/(const vec2 v1, const vec2 v2) {
    return vec2(v1.x / v2.x, v1.y / v2.y);
  }

  constexpr vec2 operator%(const vec2 v, float scalar) {
    return vec2((float)((int)v.x % (int)scalar), (float)((int)v.y % (int)scalar));
  }

  constexpr vec2 operator%(float scalar, const vec2 v) {
    return vec2((float)((int)scalar % (int)v.x), (float)((int)scalar % (int)v.y));
  }

  constexpr vec2 operator%(const vec2 v1, const vec2 v2) {
    return vec2((float)((int)v1.x % (int)v2.x), (float)((int)v1.y % (int)v2.y));
  }

  /* -- Boolean operators -- */

  constexpr bool operator==(const vec2 v1, const vec2 v2) {
    return v1.x == v2.x && v1.y == v2.y;
  }

  constexpr bool operator!=(const vec2 v1, const vec2 v2) {
    return !(v1 == v2);
  }

  constexpr vec2 operator&&(const vec2 v1, const vec2 v2) {
    return vec2(v1.x && v2.x, v1.y && v2.y);
  }

  constexpr vec2 operator||(const vec2 v1, const vec2 v2) {
    return vec2(v1.x || v2.x, v1.y || v2.y);
  }

  /* -- Address operator -- */

  constexpr const float* operator&(const vec2& v) { return &(v.x); }

}

namespace std {

  /* -- numeric_limits -- */

  template<>
  class numeric_limits<Graphick::Math::vec2> {
  public:
    static inline Graphick::Math::vec2 min() {
      return Graphick::Math::vec2{ numeric_limits<float>::min() };
    }

    static inline Graphick::Math::vec2 max() {
      return Graphick::Math::vec2{ numeric_limits<float>::max() };
    }

    static inline Graphick::Math::vec2 lowest() {
      return Graphick::Math::vec2{ numeric_limits<float>::lowest() };
    }
  };

}
