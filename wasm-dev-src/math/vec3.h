#pragma once

#include <stdint.h>
#include <ostream>

namespace Graphick::Math {

  struct vec3 {
    union { float x, r, s; };
    union { float y, g, t; };
    union { float z, b, p; };

    /* -- Component accesses -- */

    static constexpr uint8_t length() { return 3; }
    constexpr float& operator[](uint8_t i);
    constexpr const float& operator[](uint8_t i) const;

    /* -- Constructors -- */

    vec3() = default;
    constexpr vec3(const vec3& v) = default;
    constexpr explicit vec3(float scalar);
    constexpr vec3(float x, float y, float z);

    /* -- Assign operator -- */

    constexpr vec3& operator=(const vec3& v);

    /* -- Unary arithmetic operators -- */

    constexpr vec3& operator+=(float scalar);
    constexpr vec3& operator+=(const vec3& v);
    constexpr vec3& operator-=(float scalar);
    constexpr vec3& operator-=(const vec3& v);
    constexpr vec3& operator*=(float scalar);
    constexpr vec3& operator*=(const vec3& v);
    constexpr vec3& operator/=(float scalar);
    constexpr vec3& operator/=(const vec3& v);

    /* -- Increment/Decrement operators -- */

    constexpr vec3& operator++();
    constexpr vec3& operator--();
  };

  /* -- Unary operators */

  constexpr vec3 operator+(const vec3& v);
  constexpr vec3 operator-(const vec3& v);

  /* -- Binary operators -- */

  constexpr vec3 operator+(const vec3& v, float scalar);
  constexpr vec3 operator+(float scalar, const vec3& v);
  constexpr vec3 operator+(const vec3& v1, const vec3& v2);
  constexpr vec3 operator-(const vec3& v, float scalar);
  constexpr vec3 operator-(float scalar, const vec3& v);
  constexpr vec3 operator-(const vec3& v1, const vec3& v2);
  constexpr vec3 operator*(const vec3& v, float scalar);
  constexpr vec3 operator*(float scalar, const vec3& v);
  constexpr vec3 operator*(const vec3& v1, const vec3& v2);
  constexpr vec3 operator/(const vec3& v, float scalar);
  constexpr vec3 operator/(float scalar, const vec3& v);
  constexpr vec3 operator/(const vec3& v1, const vec3& v2);
  constexpr vec3 operator%(const vec3& v, float scalar);
  constexpr vec3 operator%(float scalar, const vec3& v);
  constexpr vec3 operator%(const vec3& v1, const vec3& v2);

  /* -- Boolean operators -- */

  constexpr bool operator==(const vec3& v1, const vec3& v2);
  constexpr bool operator!=(const vec3& v1, const vec3& v2);
  constexpr vec3 operator&&(const vec3& v1, const vec3& v2);
  constexpr vec3 operator||(const vec3& v1, const vec3& v2);

  /* -- Address operator -- */

  constexpr const float* operator&(const vec3& v);

}

namespace Graphick {

  using vec3 = Math::vec3;

}

#include "vec3.inl"
