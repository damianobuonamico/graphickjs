#pragma once

#include "vec2.h"

#include <ostream>

namespace Graphick::Math {

  struct mat2 {
    /* -- Component accesses -- */

    static constexpr uint8_t length() { return 2; }
    vec2& operator[](uint8_t i);
    constexpr vec2 const& operator[](uint8_t i) const;

    /* -- Constructors -- */

    constexpr mat2();
    constexpr mat2(const mat2& m);
    constexpr explicit mat2(float scalar);
    constexpr mat2(const vec2 v0, const vec2 v1);
    constexpr mat2(
      float x0, float y0,
      float x1, float y1
    );

    /* -- Assign operator -- */

    mat2& operator=(const mat2& m);

    /* -- Unary arithmetic operators -- */

    mat2& operator+=(float scalar);
    mat2& operator+=(const mat2& m);
    mat2& operator-=(float scalar);
    mat2& operator-=(const mat2& m);
    mat2& operator*=(float scalar);
    mat2& operator*=(const mat2& m);
    mat2& operator/=(float scalar);
    mat2& operator/=(const mat2& m);

    /* -- Increment/Decrement operators -- */

    mat2& operator++();
    mat2& operator--();
  private:
    vec2 value[2];
  };

  /* -- Unary operators */

  mat2 operator+(const mat2& m);
  mat2 operator-(const mat2& m);

  /* -- Binary operators -- */

  mat2 operator+(const mat2& m, float scalar);
  mat2 operator+(float scalar, const mat2& m);
  mat2 operator+(const mat2& m1, const mat2& m2);
  mat2 operator-(const mat2& m, float scalar);
  mat2 operator-(float scalar, const mat2& m);
  mat2 operator-(const mat2& m1, const mat2& m2);
  mat2 operator*(const mat2& m, float scalar);
  mat2 operator*(float scalar, const mat2& m);
  vec2 operator*(const mat2& m, const vec2& v);
  mat2 operator*(const mat2& m1, const mat2& m2);
  mat2 operator/(const mat2& m, float scalar);
  mat2 operator/(float scalar, const mat2& m);
  vec2 operator/(const mat2& m, const vec2& v);
  mat2 operator/(const mat2& m1, const mat2& m2);

  /* -- Boolean operators -- */

  bool operator==(const mat2& m1, const mat2& m2);
  bool operator!=(const mat2& m1, const mat2& m2);

  /* -- Address operator -- */

  const float* operator&(const mat2& m);

  /* -- Stream operator -- */

  std::ostream& operator<<(std::ostream& os, const mat2& m);

}

namespace Graphick {

  using mat2 = Math::mat2;

}

#include "mat2.inl"
