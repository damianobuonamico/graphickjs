#pragma once

#include "matrix.h"

mat3 inverse(const mat3& m) {
  float one_over_determinant = 1.0f / (
    +m[0][0] * (m[1][1] * m[2][2] - m[2][1] * m[1][2])
    - m[1][0] * (m[0][1] * m[2][2] - m[2][1] * m[0][2])
    + m[2][0] * (m[0][1] * m[1][2] - m[1][1] * m[0][2]));

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