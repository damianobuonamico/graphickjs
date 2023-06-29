#pragma once

#include "vec3.h"

namespace Graphick::Math {

  struct mat3 {
    /* -- Component accesses -- */

    static constexpr uint8_t length() { return 3; }
    vec3& operator[](uint8_t i);
    constexpr vec3 const& operator[](uint8_t i) const;

    /* -- Constructors -- */

    constexpr mat3();
    constexpr mat3(const mat3& m);
    constexpr explicit mat3(float scalar);
    constexpr mat3(const vec3& v0, const vec3& v1, const vec3& v2);
    constexpr mat3(
      float x0, float y0, float z0,
      float x1, float y1, float z1,
      float x2, float y2, float z2
    );

    /* -- Assign operator -- */

    mat3& operator=(const mat3& m);

    /* -- Unary arithmetic operators -- */

    mat3& operator+=(float scalar);
    mat3& operator+=(const mat3& m);
    mat3& operator-=(float scalar);
    mat3& operator-=(const mat3& m);
    mat3& operator*=(float scalar);
    mat3& operator*=(const mat3& m);
    mat3& operator/=(float scalar);
    mat3& operator/=(const mat3& m);

    /* -- Increment/Decrement operators -- */

    mat3& operator++();
    mat3& operator--();
  private:
    vec3 value[3];
  };

  /* -- Unary operators */

  mat3 operator+(const mat3& m);
  mat3 operator-(const mat3& m);

  /* -- Binary operators -- */

  mat3 operator+(const mat3& m, float scalar);
  mat3 operator+(float scalar, const mat3& m);
  mat3 operator+(const mat3& m1, const mat3& m2);
  mat3 operator-(const mat3& m, float scalar);
  mat3 operator-(float scalar, const mat3& m);
  mat3 operator-(const mat3& m1, const mat3& m2);
  mat3 operator*(const mat3& m, float scalar);
  mat3 operator*(float scalar, const mat3& m);
  vec3 operator*(const mat3& m, const vec3& v);
  mat3 operator*(const mat3& m1, const mat3& m2);
  mat3 operator/(const mat3& m, float scalar);
  mat3 operator/(float scalar, const mat3& m);
  vec3 operator/(const mat3& m, const vec3& v);
  mat3 operator/(const mat3& m1, const mat3& m2);

  /* -- Boolean operators -- */

  bool operator==(const mat3& m1, const mat3& m2);
  bool operator!=(const mat3& m1, const mat3& m2);

  /* -- Address operator -- */

  const float* operator&(const mat3& m);

  /* -- Stream operator -- */

  std::ostream& operator<<(std::ostream& os, const mat3& m);

}

namespace Graphick {

  using mat3 = Math::mat3;

}

#include "mat3.inl"
