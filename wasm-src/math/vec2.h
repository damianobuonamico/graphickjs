/**
 * @file vec2.h
 * @brief This file contains the definition of the vec2 struct, a 2D single precision vector.
 */

#pragma once

#include <cstdint>

namespace Graphick::Math {

  /**
   * @brief A 2D vector struct with x and y components.
   *
   * @struct vec2
   */
  struct vec2 {
    union { float x, r, s; };   /* The 0 component of the vector. */
    union { float y, g, t; };   /* The 1 component of the vector. */

    /* -- Component accesses -- */

    static constexpr uint8_t length() { return 2; }
    constexpr float& operator[](uint8_t i);
    constexpr float const& operator[](uint8_t i) const;

    /* -- Constructors -- */

    vec2() = default;
    constexpr vec2(const vec2& v) = default;
    constexpr explicit vec2(float scalar);
    constexpr vec2(float x, float y);

    /* -- Assign operator -- */

    constexpr vec2& operator=(const vec2 v);

    /* -- Unary arithmetic operators -- */

    constexpr vec2& operator+=(float scalar);
    constexpr vec2& operator+=(const vec2 v);
    constexpr vec2& operator-=(float scalar);
    constexpr vec2& operator-=(const vec2 v);
    constexpr vec2& operator*=(float scalar);
    constexpr vec2& operator*=(const vec2 v);
    constexpr vec2& operator/=(float scalar);
    constexpr vec2& operator/=(const vec2 v);

    /* -- Increment/Decrement operators -- */

    constexpr vec2& operator++();
    constexpr vec2& operator--();
  };

  /* -- Unary operators */

  constexpr vec2 operator+(const vec2 v);
  constexpr vec2 operator-(const vec2 v);

  /* -- Binary operators -- */

  constexpr vec2 operator+(const vec2 v, float scalar);
  constexpr vec2 operator+(float scalar, const vec2 v);
  constexpr vec2 operator+(const vec2 v1, const vec2 v2);
  constexpr vec2 operator-(const vec2 v, float scalar);
  constexpr vec2 operator-(float scalar, const vec2 v);
  constexpr vec2 operator-(const vec2 v1, const vec2 v2);
  constexpr vec2 operator*(const vec2 v, float scalar);
  constexpr vec2 operator*(float scalar, const vec2 v);
  constexpr vec2 operator*(const vec2 v1, const vec2 v2);
  constexpr vec2 operator/(const vec2 v, float scalar);
  constexpr vec2 operator/(float scalar, const vec2 v);
  constexpr vec2 operator/(const vec2 v1, const vec2 v2);
  constexpr vec2 operator%(const vec2 v, float scalar);
  constexpr vec2 operator%(float scalar, const vec2 v);
  constexpr vec2 operator%(const vec2 v1, const vec2 v2);

  /* -- Boolean operators -- */

  constexpr bool operator==(const vec2 v1, const vec2 v2);
  constexpr bool operator!=(const vec2 v1, const vec2 v2);
  constexpr vec2 operator&&(const vec2 v1, const vec2 v2);
  constexpr vec2 operator||(const vec2 v1, const vec2 v2);

  /* -- Address operator -- */

  constexpr const float* operator&(const vec2& v);

}

namespace Graphick {

  using vec2 = Math::vec2;

}

#include "vec2.inl"
