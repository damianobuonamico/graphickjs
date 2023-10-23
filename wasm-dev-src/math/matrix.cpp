/**
 * @file matrix.cpp
 * @brief This file contains the implementation of the matrix functions.
 */

#include "matrix.h"

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
    const float src_a00 = m[0][0];
    const float src_a01 = m[0][1];
    const float src_a02 = m[0][2];
    const float src_a10 = m[1][0];
    const float src_a11 = m[1][1];
    const float src_a12 = m[1][2];

    mat2x3 result;
    result[0][0] = src_a00;
    result[0][1] = src_a01;
    result[0][2] = src_a00 * v.x + src_a01 * v.y + src_a02;
    result[1][0] = src_a10;
    result[1][1] = src_a11;
    result[1][2] = src_a10 * v.x + src_a11 * v.y + src_a12;
    return result;
  }

}
