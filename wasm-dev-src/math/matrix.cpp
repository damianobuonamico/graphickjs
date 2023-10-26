/**
 * @file matrix.cpp
 * @brief This file contains the implementation of the matrix functions.
 */

#include "matrix.h"

#include "scalar.h"
#include "vector.h"

namespace Graphick::Math {

  /* -- zero -- */

  void zero(mat2& m) {
    zero(m[0]);
    zero(m[1]);
  }

  void zero(mat3& m) {
    zero(m[0]);
    zero(m[1]);
    zero(m[2]);
  }

  void zero(mat4& m) {
    zero(m[0]);
    zero(m[1]);
    zero(m[2]);
    zero(m[3]);
  }

  void zero(mat2x3& m) {
    zero(m[0]);
    zero(m[1]);
  }

  /* -- is_zero -- */

  bool is_zero(const mat2& m) {
    return (
      is_zero(m[0]) &&
      is_zero(m[1])
      );
  }

  bool is_zero(const mat3& m) {
    return (
      is_zero(m[0]) &&
      is_zero(m[1]) &&
      is_zero(m[2])
      );
  }

  bool is_zero(const mat4& m) {
    return (
      is_zero(m[0]) &&
      is_zero(m[1]) &&
      is_zero(m[2]) &&
      is_zero(m[3])
      );
  }

  bool is_zero(const mat2x3& m) {
    return (
      is_zero(m[0]) &&
      is_zero(m[1])
      );
  }

  /* -- determinant -- */

  float determinant(const mat2& m) {
    return (
      +m[0][0] * m[1][1]
      - m[0][1] * m[1][0]
      );
  }

  float determinant(const mat3& m) {
    return (
      +m[0][0] * (m[1][1] * m[2][2] - m[2][1] * m[1][2])
      - m[1][0] * (m[0][1] * m[2][2] - m[2][1] * m[0][2])
      + m[2][0] * (m[0][1] * m[1][2] - m[1][1] * m[0][2])
      );
  }

  float determinant(const mat4& m) {
    return (
      +m[0][0] * (
        +m[1][1] * (m[2][2] * m[3][3] - m[3][2] * m[2][3])
        - m[2][1] * (m[1][2] * m[3][3] - m[3][2] * m[1][3])
        + m[3][1] * (m[1][2] * m[2][3] - m[2][2] * m[1][3])
        )
      - m[1][0] * (
        +m[0][1] * (m[2][2] * m[3][3] - m[3][2] * m[2][3])
        - m[2][1] * (m[0][2] * m[3][3] - m[3][2] * m[0][3])
        + m[3][1] * (m[0][2] * m[2][3] - m[2][2] * m[0][3])
        )
      + m[2][0] * (
        +m[0][1] * (m[1][2] * m[3][3] - m[3][2] * m[1][3])
        - m[1][1] * (m[0][2] * m[3][3] - m[3][2] * m[0][3])
        + m[3][1] * (m[0][2] * m[1][3] - m[1][2] * m[0][3])
        )
      - m[3][0] * (
        +m[0][1] * (m[1][2] * m[2][3] - m[2][2] * m[1][3])
        - m[1][1] * (m[0][2] * m[2][3] - m[2][2] * m[0][3])
        + m[2][1] * (m[0][2] * m[1][3] - m[1][2] * m[0][3])
        )
      );
  }

  float determinant(const mat2x3& m) {
    return (
      +m[0][0] * m[1][1]
      - m[1][0] * m[0][1]
      );
  }

  /* -- inverse -- */

  mat2 inverse(const mat2& m) {
    float one_over_determinant = 1.0f / determinant(m);

    mat2 inverse;
    inverse[0][0] = m[1][1] * one_over_determinant;
    inverse[1][0] = -m[1][0] * one_over_determinant;
    inverse[0][1] = -m[0][1] * one_over_determinant;
    inverse[1][1] = m[0][0] * one_over_determinant;

    return inverse;
  }

  mat3 inverse(const mat3& m) {
    float one_over_determinant = 1.0f / determinant(m);

    mat3 inverse;
    inverse[0][0] = +(m[1][1] * m[2][2] - m[2][1] * m[1][2]) * one_over_determinant;
    inverse[1][0] = -(m[1][0] * m[2][2] - m[2][0] * m[1][2]) * one_over_determinant;
    inverse[2][0] = +(m[1][0] * m[2][1] - m[2][0] * m[1][1]) * one_over_determinant;
    inverse[0][1] = -(m[0][1] * m[2][2] - m[2][1] * m[0][2]) * one_over_determinant;
    inverse[1][1] = +(m[0][0] * m[2][2] - m[2][0] * m[0][2]) * one_over_determinant;
    inverse[2][1] = -(m[0][0] * m[2][1] - m[2][0] * m[0][1]) * one_over_determinant;
    inverse[0][2] = +(m[0][1] * m[1][2] - m[1][1] * m[0][2]) * one_over_determinant;
    inverse[1][2] = -(m[0][0] * m[1][2] - m[1][0] * m[0][2]) * one_over_determinant;
    inverse[2][2] = +(m[0][0] * m[1][1] - m[1][0] * m[0][1]) * one_over_determinant;

    return inverse;
  }

  mat4 inverse(const mat4& m) {
    float one_over_determinant = 1.0f / determinant(m);

    mat4 inverse;
    inverse[0][0] = +(
      +m[1][1] * (m[2][2] * m[3][3] - m[3][2] * m[2][3])
      - m[2][1] * (m[1][2] * m[3][3] - m[3][2] * m[1][3])
      + m[3][1] * (m[1][2] * m[2][3] - m[2][2] * m[1][3])
      ) * one_over_determinant;
    inverse[1][0] = -(
      +m[1][0] * (m[2][2] * m[3][3] - m[3][2] * m[2][3])
      - m[2][0] * (m[1][2] * m[3][3] - m[3][2] * m[1][3])
      + m[3][0] * (m[1][2] * m[2][3] - m[2][2] * m[1][3])
      ) * one_over_determinant;
    inverse[2][0] = +(
      +m[1][0] * (m[2][1] * m[3][3] - m[3][1] * m[2][3])
      - m[2][0] * (m[1][1] * m[3][3] - m[3][1] * m[1][3])
      + m[3][0] * (m[1][1] * m[2][3] - m[2][1] * m[1][3])
      ) * one_over_determinant;
    inverse[3][0] = -(
      +m[1][0] * (m[2][1] * m[3][2] - m[3][1] * m[2][2])
      - m[2][0] * (m[1][1] * m[3][2] - m[3][1] * m[1][2])
      + m[3][0] * (m[1][1] * m[2][2] - m[2][1] * m[1][2])
      ) * one_over_determinant;
    inverse[0][1] = -(
      +m[0][1] * (m[2][2] * m[3][3] - m[3][2] * m[2][3])
      - m[2][1] * (m[0][2] * m[3][3] - m[3][2] * m[0][3])
      + m[3][1] * (m[0][2] * m[2][3] - m[2][2] * m[0][3])
      ) * one_over_determinant;
    inverse[1][1] = +(
      +m[0][0] * (m[2][2] * m[3][3] - m[3][2] * m[2][3])
      - m[2][0] * (m[0][2] * m[3][3] - m[3][2] * m[0][3])
      + m[3][0] * (m[0][2] * m[2][3] - m[2][2] * m[0][3])
      ) * one_over_determinant;
    inverse[2][1] = -(
      +m[0][0] * (m[2][1] * m[3][3] - m[3][1] * m[2][3])
      - m[2][0] * (m[0][1] * m[3][3] - m[3][1] * m[0][3])
      + m[3][0] * (m[0][1] * m[2][3] - m[2][1] * m[0][3])
      ) * one_over_determinant;
    inverse[3][1] = +(
      +m[0][0] * (m[2][1] * m[3][2] - m[3][1] * m[2][2])
      - m[2][0] * (m[0][1] * m[3][2] - m[3][1] * m[0][2])
      + m[3][0] * (m[0][1] * m[2][2] - m[2][1] * m[0][2])
      ) * one_over_determinant;
    inverse[0][2] = +(
      +m[0][1] * (m[1][2] * m[3][3] - m[3][2] * m[1][3])
      - m[1][1] * (m[0][2] * m[3][3] - m[3][2] * m[0][3])
      + m[3][1] * (m[0][2] * m[1][3] - m[1][2] * m[0][3])
      ) * one_over_determinant;
    inverse[1][2] = -(
      +m[0][0] * (m[1][2] * m[3][3] - m[3][2] * m[1][3])
      - m[1][0] * (m[0][2] * m[3][3] - m[3][2] * m[0][3])
      + m[3][0] * (m[0][2] * m[1][3] - m[1][2] * m[0][3])
      ) * one_over_determinant;
    inverse[2][2] = +(
      +m[0][0] * (m[1][1] * m[3][3] - m[3][1] * m[1][3])
      - m[1][0] * (m[0][1] * m[3][3] - m[3][1] * m[0][3])
      + m[3][0] * (m[0][1] * m[1][3] - m[1][1] * m[0][3])
      ) * one_over_determinant;
    inverse[3][2] = -(
      +m[0][0] * (m[1][1] * m[3][2] - m[3][1] * m[1][2])
      - m[1][0] * (m[0][1] * m[3][2] - m[3][1] * m[0][2])
      + m[3][0] * (m[0][1] * m[1][2] - m[1][1] * m[0][2])
      ) * one_over_determinant;
    inverse[0][3] = -(
      +m[0][1] * (m[1][2] * m[2][3] - m[2][2] * m[1][3])
      - m[1][1] * (m[0][2] * m[2][3] - m[2][2] * m[0][3])
      + m[2][1] * (m[0][2] * m[1][3] - m[1][2] * m[0][3])
      ) * one_over_determinant;
    inverse[1][3] = +(
      +m[0][0] * (m[1][2] * m[2][3] - m[2][2] * m[1][3])
      - m[1][0] * (m[0][2] * m[2][3] - m[2][2] * m[0][3])
      + m[2][0] * (m[0][2] * m[1][3] - m[1][2] * m[0][3])
      ) * one_over_determinant;
    inverse[2][3] = -(
      +m[0][0] * (m[1][1] * m[2][3] - m[2][1] * m[1][3])
      - m[1][0] * (m[0][1] * m[2][3] - m[2][1] * m[0][3])
      + m[2][0] * (m[0][1] * m[1][3] - m[1][1] * m[0][3])
      ) * one_over_determinant;
    inverse[3][3] = +(
      +m[0][0] * (m[1][1] * m[2][2] - m[2][1] * m[1][2])
      - m[1][0] * (m[0][1] * m[2][2] - m[2][1] * m[0][2])
      + m[2][0] * (m[0][1] * m[1][2] - m[1][1] * m[0][2])
      ) * one_over_determinant;

    return inverse;
  }

  mat2x3 inverse(const mat2x3& m) {
    float one_over_determinant = 1.0f / determinant(m);

    mat2x3 inverse;
    inverse[0][0] = +(m[1][1]) * one_over_determinant;
    inverse[1][0] = -(m[1][0]) * one_over_determinant;
    inverse[0][1] = -(m[0][1]) * one_over_determinant;
    inverse[1][1] = +(m[0][0]) * one_over_determinant;
    inverse[0][2] = +(m[0][1] * m[1][2] - m[1][1] * m[0][2]) * one_over_determinant;
    inverse[1][2] = -(m[0][0] * m[1][2] - m[1][0] * m[0][2]) * one_over_determinant;

    return inverse;
  }

  /* -- transform -- */

  mat2x3 translate(const mat2x3& m, const vec2 v) {
    const float src_b00 = m[0][0];
    const float src_b01 = m[0][1];
    const float src_b02 = m[0][2];
    const float src_b10 = m[1][0];
    const float src_b11 = m[1][1];
    const float src_b12 = m[1][2];

    mat2x3 result;
    result[0][0] = src_b00;
    result[0][1] = src_b01;
    result[0][2] = src_b02 + v.x;
    result[1][0] = src_b10;
    result[1][1] = src_b11;
    result[1][2] = src_b12 + v.y;
    return result;
  }

  mat2x3 scale(const mat2x3& m, const vec2 v) {
    const float src_b00 = m[0][0];
    const float src_b01 = m[0][1];
    const float src_b02 = m[0][2];
    const float src_b10 = m[1][0];
    const float src_b11 = m[1][1];
    const float src_b12 = m[1][2];

    mat2x3 result;
    result[0][0] = v.x * src_b00;
    result[0][1] = v.x * src_b01;
    result[0][2] = v.x * src_b02;
    result[1][0] = v.y * src_b10;
    result[1][1] = v.y * src_b11;
    result[1][2] = v.y * src_b12;
    return result;
  }

  mat2x3 scale(const mat2x3& m, const vec2 c, const vec2 v) {
    return translate(scale(translate(m, -c), v), c);
  }

  mat2x3 rotate(const mat2x3& m, const float t) {
    mat2x3 result;
    return result;
  }

  mat2x3 rotate(const mat2x3& m, const vec2 c, const float t) {
    mat2x3 result;
    return result;
  }

  /* -- decompose -- */

  DecomposedTransform decompose(const mat2x3& m) {
    DecomposedTransform decomposed;

    decomposed.rotation = std::atan2f(m[1][0], m[0][0]);

    decomposed.shear = std::atan2f(m[1][1], m[0][1]) - MATH_PI / 2.0f - decomposed.rotation;

    decomposed.translation.x = m[0][2];
    decomposed.translation.y = m[1][2];

    decomposed.scale.x = std::hypotf(m[0][0], m[1][0]);
    decomposed.scale.y = std::hypotf(m[0][1], m[1][1]) * std::cosf(decomposed.shear);

    return decomposed;
  }

  /* -- operators -- */

  rect operator*(const mat2x3& m, const rect& r) {
    vec2 trans_min = m * r.min;
    vec2 trans_max = m * r.max;

    return {
      min(trans_min, trans_max),
      max(trans_min, trans_max)
    };
  }

  rect operator/(const mat2x3& m, const rect& r) {
    return inverse(m) * r;
  }

}
