/**
 * @file matrix.h
 * @brief This file contains the definition of the matrix functions.
 */

#pragma once

#include "mat2.h"
#include "mat3.h"
#include "mat4.h"

#include "mat2x3.h"

#include "rect.h"

#include "scalar.h"
#include "vector.h"

namespace Graphick::Math {

  /**
   * @brief A struct containing the decomposed components of a 2x3 matrix.
   *
   * @struct DecomposedTransform
   */
  template<typename T>
  struct DecomposedTransform {
    Vec2<T> translation;    /* Position component. */
    Vec2<T> scale;          /* Scale component. */

    T rotation;      /* Rotation component in radians. */
    T shear;         /* Shear component along the y-axis. */
  };

  /**
   * Sets all elements of the given 2x2 matrix to zero.
   *
   * @param m The matrix to be zeroed.
   */
  template<typename T>
  void zero(Mat2<T>& m) {
    zero(m[0]);
    zero(m[1]);
  }

  /**
   * Sets all elements of the given 3x3 matrix to zero.
   *
   * @param m The matrix to be zeroed.
   */
  template<typename T>
  void zero(Mat3<T>& m) {
    zero(m[0]);
    zero(m[1]);
    zero(m[2]);
  }

  /**
   * Sets all elements of the given 4x4 matrix to zero.
   *
   * @param m The matrix to be zeroed.
   */
  template<typename T>
  void zero(Mat4<T>& m) {
    zero(m[0]);
    zero(m[1]);
    zero(m[2]);
    zero(m[3]);
  }

  /**
   * Sets all elements of the given 2x3 matrix to zero.
   *
   * @param m The matrix to be zeroed.
   */
  template<typename T>
  void zero(Mat2x3<T>& m) {
    zero(m[0]);
    zero(m[1]);
  }

  /**
   * Checks if a 2x2 matrix is a zero matrix.
   *
   * @param m The matrix to check.
   * @return True if the matrix is a zero matrix, false otherwise.
   */
  template<typename T>
  bool is_zero(const Mat2<T>& m) {
    return (
      is_zero(m[0]) &&
      is_zero(m[1])
    );
  }

  /**
   * Checks if a 3x3 matrix is a zero matrix.
   *
   * @param m The matrix to check.
   * @return True if the matrix is a zero matrix, false otherwise.
   */
  template<typename T>
  bool is_zero(const Mat3<T>& m) {
    return (
      is_zero(m[0]) &&
      is_zero(m[1]) &&
      is_zero(m[2])
    );
  }

  /**
   * Checks if a 4x4 matrix is a zero matrix.
   *
   * @param m The matrix to check.
   * @return True if the matrix is a zero matrix, false otherwise.
   */
  template<typename T>
  bool is_zero(const Mat4<T>& m) {
    return (
      is_zero(m[0]) &&
      is_zero(m[1]) &&
      is_zero(m[2]) &&
      is_zero(m[3])
    );
  }

  /**
   * Checks if a 2x3 matrix is a zero matrix.
   *
   * @param m The matrix to check.
   * @return True if the matrix is a zero matrix, false otherwise.
   */
  template<typename T>
  bool is_zero(const Mat2x3<T>& m) {
    return (
      is_zero(m[0]) &&
      is_zero(m[1])
    );
  }

  /**
   * @brief Calculates the determinant of a 2x2 matrix.
   *
   * @param m The matrix to calculate the determinant of.
   * @return The determinant of the matrix.
   */
  template<typename T>
  T determinant(const Mat2<T>& m) {
    return (
      +m[0][0] * m[1][1]
      - m[0][1] * m[1][0]
    );
  }

  /**
   * @brief Calculates the determinant of a 3x3 matrix.
   *
   * @param m The matrix to calculate the determinant of.
   * @return The determinant of the matrix.
   */
  template<typename T>
  T determinant(const Mat3<T>& m) {
    return (
      +m[0][0] * (m[1][1] * m[2][2] - m[2][1] * m[1][2])
      - m[1][0] * (m[0][1] * m[2][2] - m[2][1] * m[0][2])
      + m[2][0] * (m[0][1] * m[1][2] - m[1][1] * m[0][2])
    );
  }

  /**
   * @brief Calculates the determinant of a 4x4 matrix.
   *
   * @param m The matrix to calculate the determinant of.
   * @return The determinant of the matrix.
   */
  template<typename T>
  T determinant(const Mat4<T>& m) {
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

  /**
   * @brief Calculates the determinant of a 2x3 matrix.
   *
   * Although the determinant of a 2x3 matrix is not defined, this function treats
   * the matrix as a 3x3 matrix with the last row being [0, 0, 1].
   *
   * @param m The matrix to calculate the determinant of.
   * @return The determinant of the matrix.
   */
  template<typename T>
  T determinant(const Mat2x3<T>& m) {
    return (
      +m[0][0] * m[1][1]
      - m[0][1] * m[1][0]
    );
  }

  /**
   * @brief Calculates the inverse of a 2x2 matrix.
   *
   * @param m The matrix to calculate the inverse of.
   * @return The inverse of the matrix.
   */
  template<typename T>
  Mat2<T> inverse(const Mat2<T>& m) {
    T one_over_determinant = 1.0f / determinant(m);

    Mat2<T> inverse;
    inverse[0][0] = m[1][1] * one_over_determinant;
    inverse[1][0] = -m[1][0] * one_over_determinant;
    inverse[0][1] = -m[0][1] * one_over_determinant;
    inverse[1][1] = m[0][0] * one_over_determinant;

    return inverse;
  }

  /**
   * @brief Calculates the inverse of a 3x3 matrix.
   *
   * @param m The matrix to calculate the inverse of.
   * @return The inverse of the matrix.
   */
  template<typename T>
  Mat3<T> inverse(const Mat3<T>& m) {
    T one_over_determinant = 1.0f / determinant(m);

    Mat3<T> inverse;
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

  /**
   * @brief Calculates the inverse of a 4x4 matrix.
   *
   * @param m The matrix to calculate the inverse
   * @return The inverse of the matrix.
   */
  template<typename T>
  Mat4<T> inverse(const Mat4<T>& m) {
    T one_over_determinant = 1.0f / determinant(m);

    Mat4<T> inverse;
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

  /**
   * @brief Calculates the inverse of a 2x3 matrix.
   *
   * Although the inverse of a 2x3 matrix is not defined, this function treats
   * the matrix as a 3x3 matrix with the last row being [0, 0, 1].
   *
   * @param m The matrix to calculate the inverse of.
   * @return The inverse of the matrix.
   */
  template<typename T>
  Mat2x3<T> inverse(const Mat2x3<T>& m) {
    T one_over_determinant = 1.0f / determinant(m);

    Mat2x3<T> inverse;
    inverse[0][0] = +(m[1][1]) * one_over_determinant;
    inverse[1][0] = -(m[1][0]) * one_over_determinant;
    inverse[0][1] = -(m[0][1]) * one_over_determinant;
    inverse[1][1] = +(m[0][0]) * one_over_determinant;
    inverse[0][2] = +(m[0][1] * m[1][2] - m[1][1] * m[0][2]) * one_over_determinant;
    inverse[1][2] = -(m[0][0] * m[1][2] - m[1][0] * m[0][2]) * one_over_determinant;

    return inverse;
  }

  /**
   * @brief Performs a 2D translation to a 2x3 matrix.
   *
   * @param m The matrix to translate.
   * @param v The vector to translate by.
   * @return The translated matrix.
   */
  template<typename T>
  Mat2x3<T> translate(const Mat2x3<T>& m, const Vec2<T> v) {
    const T src_b00 = m[0][0];
    const T src_b01 = m[0][1];
    const T src_b02 = m[0][2];
    const T src_b10 = m[1][0];
    const T src_b11 = m[1][1];
    const T src_b12 = m[1][2];

    Mat2x3<T> result;
    result[0][0] = src_b00;
    result[0][1] = src_b01;
    result[0][2] = src_b02 + v.x;
    result[1][0] = src_b10;
    result[1][1] = src_b11;
    result[1][2] = src_b12 + v.y;
    return result;
  }

  /**
   * @brief Performs a 2D scale to a 2x3 matrix.
   *
   * @param m The matrix to scale.
   * @param v The vector to scale by.
   * @return The scaled matrix.
   */
  template<typename T>
  Mat2x3<T> scale(const Mat2x3<T>& m, const Vec2<T> v) {
    const T src_b00 = m[0][0];
    const T src_b01 = m[0][1];
    const T src_b02 = m[0][2];
    const T src_b10 = m[1][0];
    const T src_b11 = m[1][1];
    const T src_b12 = m[1][2];

    Mat2x3<T> result;
    result[0][0] = v.x * src_b00;
    result[0][1] = v.x * src_b01;
    result[0][2] = v.x * src_b02;
    result[1][0] = v.y * src_b10;
    result[1][1] = v.y * src_b11;
    result[1][2] = v.y * src_b12;
    return result;
  }

  /**
   * @brief Performs a 2D scale from an origin point to a 2x3 matrix.
   *
   * @param m The matrix to scale.
   * @param c The scale origin.
   * @param v The vector to scale by.
   * @return The scaled matrix.
   */
  template<typename T>
  Mat2x3<T> scale(const Mat2x3<T>& m, const Vec2<T> c, const Vec2<T> v) {
    return translate(scale(translate(m, -c), v), c);
  }

  /**
   * @brief Performs a 2D rotation to a 2x3 matrix.
   *
   * @param m The matrix to rotate.
   * @param t The angle to rotate by.
   * @return The rotated matrix.
   */
  template<typename T>
  Mat2x3<T> rotate(const Mat2x3<T>& m, const T t) {
    return rotate(m, std::sinf(t), std::cosf(t));
  }

  /**
   * @brief Performs a 2D rotation to a 2x3 matrix.
   *
   * This function is faster than the regular rotate function because it does not need to calculate the sine and cosine of the angle.
   *
   * @param m The matrix to rotate.
   * @param sin_t The sine of the angle to rotate by.
   * @param cos_t The cosine of the angle to rotate by.
   * @return The rotated matrix.
   */
  template<typename T>
  Mat2x3<T> rotate(const Mat2x3<T>& m, const T sin_t, const T cos_t) {
    const T src_b00 = m[0][0];
    const T src_b01 = m[0][1];
    const T src_b02 = m[0][2];
    const T src_b10 = m[1][0];
    const T src_b11 = m[1][1];
    const T src_b12 = m[1][2];

    Mat2x3<T> result;
    result[0][0] = cos_t * src_b00 - sin_t * src_b10;
    result[0][1] = cos_t * src_b01 - sin_t * src_b11;
    result[0][2] = cos_t * src_b02 - sin_t * src_b12;
    result[1][0] = sin_t * src_b00 + cos_t * src_b10;
    result[1][1] = sin_t * src_b01 + cos_t * src_b11;
    result[1][2] = sin_t * src_b02 + cos_t * src_b12;
    return result;
  }

  /**
   * @brief Performs a 2D rotation from an origin point to a 2x3 matrix.
   *
   * @param m The matrix to rotate.
   * @param c The rotation origin.
   * @param t The angle to rotate by.
   * @return The rotated matrix.
   */
  template<typename T>
  Mat2x3<T> rotate(const Mat2x3<T>& m, const Vec2<T> c, const T t) {
    return translate(rotate(translate(m, -c), t), c);
  }
  /**
   * @brief Performs a 2D rotation to a 2x3 matrix.
   *
   * This function is faster than the regular rotate function because it does not need to calculate the sine and cosine of the angle.
   *
   * @param m The matrix to rotate.
   * @param sin_t The sine of the angle to rotate by.
   * @param cos_t The cosine of the angle to rotate by.
   * @return The rotated matrix.
   */
  template<typename T>
  Mat2x3<T> rotate(const Mat2x3<T>& m, const Vec2<T> c, const T sin_t, const T cos_t) {
    return translate(rotate(translate(m, -c), sin_t, cos_t), c);
  }

  /**
   * @brief Decomposes a 2x3 matrix into its translation, scale, and rotation components.
   *
   * @param m The matrix to decompose.
   * @return The decomposed matrix.
   */
  template<typename T>
  DecomposedTransform<T> decompose(const Mat2x3<T>& m) {
    DecomposedTransform<T> decomposed;

    decomposed.rotation = std::atan2f(m[1][0], m[0][0]);

    decomposed.shear = std::atan2f(m[1][1], m[0][1]) - MATH_F_PI / 2.0f - decomposed.rotation;

    decomposed.translation.x = m[0][2];
    decomposed.translation.y = m[1][2];

    decomposed.scale.x = std::hypotf(m[0][0], m[1][0]);
    decomposed.scale.y = std::hypotf(m[0][1], m[1][1]) * std::cosf(decomposed.shear);

    return decomposed;
  }

  /**
   * @brief Calculates the translation component of a 2x3 matrix.
   *
   * @param m The matrix to calculate the translation component of.
   * @return The translation component of the matrix.
   */
  template<typename T>
  Vec2<T> translation(const Mat2x3<T>& m) {
    return { m[0][2], m[1][2] };
  }


  /**
   * @brief Calculates the rotation component of a 2x3 matrix.
   *
   * @param m The matrix to calculate the rotation component of.
   * @return The rotation component of the matrix.
   */
  template<typename T>
  T rotation(const Mat2x3<T>& m) {
    return std::atan2f(m[1][0], m[0][0]);
  }

  /**
   * @brief Overloaded multiplication operator to multiply a 2x3 matrix by a 2D rect.
   */
  template<typename T>
  rect operator*(const Mat2x3<T>& m, const rect& r) {
    Vec2<T> r1 = m * r.min;
    Vec2<T> r2 = m * Vec2<T>{ r.min.x, r.max.y };
    Vec2<T> r3 = m * r.max;
    Vec2<T> r4 = m * Vec2<T>{ r.max.x, r.min.y };

    return {
      min(min(r1, r2), min(r3, r4)),
      max(max(r1, r2), max(r3, r4))
    };
  }

  /**
   * @brief Overloaded division operator to divide a 2x3 matrix by a 2D rect.
   */
  template<typename T>
  rect operator/(const Mat2x3<T>& m, const rect& r) {
    return inverse(m) * r;
  }

}
