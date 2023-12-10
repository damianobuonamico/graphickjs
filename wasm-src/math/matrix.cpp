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

  double determinant(const dmat2x3& m) {
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

  dmat2x3 inverse(const dmat2x3& m) {
    double one_over_determinant = 1.0f / determinant(m);

    dmat2x3 inverse;
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

  dmat2x3 translate(const dmat2x3& m, const dvec2 v) {
    const double src_b00 = m[0][0];
    const double src_b01 = m[0][1];
    const double src_b02 = m[0][2];
    const double src_b10 = m[1][0];
    const double src_b11 = m[1][1];
    const double src_b12 = m[1][2];

    dmat2x3 result;
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

  dmat2x3 scale(const dmat2x3& m, const dvec2 v) {
    const double src_b00 = m[0][0];
    const double src_b01 = m[0][1];
    const double src_b02 = m[0][2];
    const double src_b10 = m[1][0];
    const double src_b11 = m[1][1];
    const double src_b12 = m[1][2];

    dmat2x3 result;
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
    return rotate(m, std::sinf(t), std::cosf(t));
  }

  mat2x3 rotate(const mat2x3& m, const float sin_t, const float cos_t) {
    const float src_b00 = m[0][0];
    const float src_b01 = m[0][1];
    const float src_b02 = m[0][2];
    const float src_b10 = m[1][0];
    const float src_b11 = m[1][1];
    const float src_b12 = m[1][2];

    mat2x3 result;
    result[0][0] = cos_t * src_b00 - sin_t * src_b10;
    result[0][1] = cos_t * src_b01 - sin_t * src_b11;
    result[0][2] = cos_t * src_b02 - sin_t * src_b12;
    result[1][0] = sin_t * src_b00 + cos_t * src_b10;
    result[1][1] = sin_t * src_b01 + cos_t * src_b11;
    result[1][2] = sin_t * src_b02 + cos_t * src_b12;
    return result;
  }

  mat2x3 rotate(const mat2x3& m, const vec2 c, const float t) {
    return translate(rotate(translate(m, -c), t), c);
  }

  mat2x3 rotate(const mat2x3& m, const vec2 c, const float sin_t, const float cos_t) {
    return translate(rotate(translate(m, -c), sin_t, cos_t), c);
  }

  /* -- decompose -- */

  DecomposedTransform decompose(const mat2x3& m) {
    DecomposedTransform decomposed;

    decomposed.rotation = std::atan2f(m[1][0], m[0][0]);

    decomposed.shear = std::atan2f(m[1][1], m[0][1]) - MATH_F_PI / 2.0f - decomposed.rotation;

    decomposed.translation.x = m[0][2];
    decomposed.translation.y = m[1][2];

    decomposed.scale.x = std::hypotf(m[0][0], m[1][0]);
    decomposed.scale.y = std::hypotf(m[0][1], m[1][1]) * std::cosf(decomposed.shear);

    return decomposed;
  }

  vec2 translation(const mat2x3& m) {
    return { m[0][2], m[1][2] };
  }

  float rotation(const mat2x3& m) {
    return std::atan2f(m[1][0], m[0][0]);
  }

  /* -- operators -- */

  rect operator*(const mat2x3& m, const rect& r) {
    vec2 r1 = m * r.min;
    vec2 r2 = m * vec2{ r.min.x, r.max.y };
    vec2 r3 = m * r.max;
    vec2 r4 = m * vec2{ r.max.x, r.min.y };

    return {
      min(min(r1, r2), min(r3, r4)),
      max(max(r1, r2), max(r3, r4))
    };
  }

  rect operator/(const mat2x3& m, const rect& r) {
    return inverse(m) * r;
  }

}
