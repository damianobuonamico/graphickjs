/**
 * @file mat2x3.h
 * @brief This file contains the definition of the mat2x3 struct.
 */

#pragma once

#include "vec2.h"
#include "vec3.h"

namespace Graphick::Math {

  /**
   * @brief A 2x3 matrix struct with 3 columns and 2 rows.
   *
   * This matrix is not mathematically correct, it is only used to reduce the memory footprint of 2D transforms.
   * The missing row is always interpreted as [0, 0, 1].
   * When multiplying a vec2 with this matrix, the third component of the vector is always treated as 1.
   *
   * @struct mat2x3
   */
  struct mat2x3 {
    /* -- Component accesses -- */

    static constexpr uint8_t length() { return 2; }
    vec3& operator[](uint8_t i);
    constexpr vec3 const& operator[](uint8_t i) const;

    /* -- Constructors -- */

    constexpr mat2x3();
    constexpr mat2x3(const mat2x3& m);
    constexpr explicit mat2x3(float scalar);
    constexpr mat2x3(const vec3& v0, const vec3& v1);
    constexpr mat2x3(
      float x0, float y0, float z0,
      float x1, float y1, float z1
    );

    /* -- Assign operator -- */

    mat2x3& operator=(const mat2x3& m);

    /* -- Unary arithmetic operators -- */

    mat2x3& operator+=(float scalar);
    mat2x3& operator+=(const mat2x3& m);
    mat2x3& operator-=(float scalar);
    mat2x3& operator-=(const mat2x3& m);
    mat2x3& operator*=(float scalar);
    mat2x3& operator*=(const mat2x3& m);
    mat2x3& operator/=(float scalar);
    mat2x3& operator/=(const mat2x3& m);

    /* -- Increment/Decrement operators -- */

    mat2x3& operator++();
    mat2x3& operator--();
  private:
    vec3 value[2];
  };

  /* -- Unary operators */

  mat2x3 operator+(const mat2x3& m);
  mat2x3 operator-(const mat2x3& m);

  /* -- Binary operators -- */

  mat2x3 operator+(const mat2x3& m, float scalar);
  mat2x3 operator+(float scalar, const mat2x3& m);
  mat2x3 operator+(const mat2x3& m1, const mat2x3& m2);
  mat2x3 operator-(const mat2x3& m, float scalar);
  mat2x3 operator-(float scalar, const mat2x3& m);
  mat2x3 operator-(const mat2x3& m1, const mat2x3& m2);
  mat2x3 operator*(const mat2x3& m, float scalar);
  mat2x3 operator*(float scalar, const mat2x3& m);
  vec2 operator*(const mat2x3& m, const vec2& v);
  mat2x3 operator*(const mat2x3& m1, const mat2x3& m2);
  mat2x3 operator/(const mat2x3& m, float scalar);
  mat2x3 operator/(float scalar, const mat2x3& m);
  vec2 operator/(const mat2x3& m, const vec2& v);
  mat2x3 operator/(const mat2x3& m1, const mat2x3& m2);

  /* -- Boolean operators -- */

  bool operator==(const mat2x3& m1, const mat2x3& m2);
  bool operator!=(const mat2x3& m1, const mat2x3& m2);

  /* -- Address operator -- */

  const float* operator&(const mat2x3& m);

}

namespace Graphick {

  using mat2x3 = Math::mat2x3;

}

#include "mat2x3.inl"
