#pragma once

#include <stdint.h>
#include <ostream>

namespace Graphick::Math {

  struct vec4 {
    union { float x, r, s; };
    union { float y, g, t; };
    union { float z, b, p; };
    union { float w, a, q; };

    /* -- Component accesses -- */

    static constexpr uint8_t length() { return 4; }
    constexpr float& operator[](uint8_t i);
    constexpr const float& operator[](uint8_t i) const;

    /* -- Constructors -- */

    vec4() = default;
    constexpr vec4(const vec4& v) = default;
    constexpr explicit vec4(float scalar);
    constexpr vec4(float x, float y, float z, float w);

    /* -- Assign operator -- */

    constexpr vec4& operator=(const vec4& v);

    /* -- Unary arithmetic operators -- */

    constexpr vec4& operator+=(float scalar);
    constexpr vec4& operator+=(const vec4& v);
    constexpr vec4& operator-=(float scalar);
    constexpr vec4& operator-=(const vec4& v);
    constexpr vec4& operator*=(float scalar);
    constexpr vec4& operator*=(const vec4& v);
    constexpr vec4& operator/=(float scalar);
    constexpr vec4& operator/=(const vec4& v);

    /* -- Increment/Decrement operators -- */

    constexpr vec4& operator++();
    constexpr vec4& operator--();
  };

  /* -- Unary operators */

  constexpr vec4 operator+(const vec4& v);
  constexpr vec4 operator-(const vec4& v);

  /* -- Binary operators -- */

  constexpr vec4 operator+(const vec4& v, float scalar);
  constexpr vec4 operator+(float scalar, const vec4& v);
  constexpr vec4 operator+(const vec4& v1, const vec4& v2);
  constexpr vec4 operator-(const vec4& v, float scalar);
  constexpr vec4 operator-(float scalar, const vec4& v);
  constexpr vec4 operator-(const vec4& v1, const vec4& v2);
  constexpr vec4 operator*(const vec4& v, float scalar);
  constexpr vec4 operator*(float scalar, const vec4& v);
  constexpr vec4 operator*(const vec4& v1, const vec4& v2);
  constexpr vec4 operator/(const vec4& v, float scalar);
  constexpr vec4 operator/(float scalar, const vec4& v);
  constexpr vec4 operator/(const vec4& v1, const vec4& v2);
  constexpr vec4 operator%(const vec4& v, float scalar);
  constexpr vec4 operator%(float scalar, const vec4& v);
  constexpr vec4 operator%(const vec4& v1, const vec4& v2);

  /* -- Boolean operators -- */

  constexpr bool operator==(const vec4& v1, const vec4& v2);
  constexpr bool operator!=(const vec4& v1, const vec4& v2);
  constexpr vec4 operator&&(const vec4& v1, const vec4& v2);
  constexpr vec4 operator||(const vec4& v1, const vec4& v2);

  /* -- Address operator -- */

  constexpr const float* operator&(const vec4& v);

}

namespace Graphick {

  using vec4 = Math::vec4;

}

#include "vec4.inl"
