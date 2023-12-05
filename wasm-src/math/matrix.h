/**
 * @file matrix.h
 * @brief This file contains the definition of the matrix functions.
 */

#pragma once

#include "mat2.h"
#include "mat3.h"
#include "mat4.h"

#include "rect.h"

#include "mat2x3.h"

namespace Graphick::Math {

  /**
   * @brief A struct containing the decomposed components of a 2x3 matrix.
   *
   * @struct DecomposedTransform
   */
  struct DecomposedTransform {
    vec2 translation;    /* Position component. */
    vec2 scale;          /* Scale component. */

    float rotation;      /* Rotation component in radians. */
    float shear;         /* Shear component along the y-axis. */
  };

  /**
   * Sets all elements of the given 2x2 matrix to zero.
   *
   * @param m The matrix to be zeroed.
   */
  void zero(mat2& m);

  /**
   * Sets all elements of the given 3x3 matrix to zero.
   *
   * @param m The matrix to be zeroed.
   */
  void zero(mat3& m);

  /**
   * Sets all elements of the given 4x4 matrix to zero.
   *
   * @param m The matrix to be zeroed.
   */
  void zero(mat4& m);

  /**
   * Sets all elements of the given 2x3 matrix to zero.
   *
   * @param m The matrix to be zeroed.
   */
  void zero(mat2x3& m);

  /**
   * Checks if a 2x2 matrix is a zero matrix.
   *
   * @param m The matrix to check.
   * @return True if the matrix is a zero matrix, false otherwise.
   */
  bool is_zero(const mat2& m);

  /**
   * Checks if a 3x3 matrix is a zero matrix.
   *
   * @param m The matrix to check.
   * @return True if the matrix is a zero matrix, false otherwise.
   */
  bool is_zero(const mat3& m);

  /**
   * Checks if a 4x4 matrix is a zero matrix.
   *
   * @param m The matrix to check.
   * @return True if the matrix is a zero matrix, false otherwise.
   */
  bool is_zero(const mat4& m);

  /**
   * Checks if a 2x3 matrix is a zero matrix.
   *
   * @param m The matrix to check.
   * @return True if the matrix is a zero matrix, false otherwise.
   */
  bool is_zero(const mat2x3& m);

  /**
   * @brief Calculates the determinant of a 2x2 matrix.
   *
   * @param m The matrix to calculate the determinant of.
   * @return The determinant of the matrix.
   */
  float determinant(const mat2& m);

  /**
   * @brief Calculates the determinant of a 3x3 matrix.
   *
   * @param m The matrix to calculate the determinant of.
   * @return The determinant of the matrix.
   */
  float determinant(const mat3& m);

  /**
   * @brief Calculates the determinant of a 4x4 matrix.
   *
   * @param m The matrix to calculate the determinant of.
   * @return The determinant of the matrix.
   */
  float determinant(const mat4& m);

  /**
   * @brief Calculates the determinant of a 2x3 matrix.
   *
   * Although the inverse of a 2x3 matrix is not defined, this function treats
   * the matrix as a 3x3 matrix with the last row being [0, 0, 1].
   *
   * @param m The matrix to calculate the determinant of.
   * @return The determinant of the matrix.
   */
  float determinant(const mat2x3& m);

  /**
   * @brief Calculates the inverse of a 2x2 matrix.
   *
   * @param m The matrix to calculate the inverse of.
   * @return The inverse of the matrix.
   */
  mat2 inverse(const mat2& m);

  /**
   * @brief Calculates the inverse of a 3x3 matrix.
   *
   * @param m The matrix to calculate the inverse of.
   * @return The inverse of the matrix.
   */
  mat3 inverse(const mat3& m);

  /**
   * @brief Calculates the inverse of a 4x4 matrix.
   *
   * @param m The matrix to calculate the inverse
   * @return The inverse of the matrix.
   */
  mat4 inverse(const mat4& m);

  /**
   * @brief Calculates the inverse of a 2x3 matrix.
   *
   * Although the inverse of a 2x3 matrix is not defined, this function treats
   * the matrix as a 3x3 matrix with the last row being [0, 0, 1].
   *
   * @param m The matrix to calculate the inverse of.
   * @return The inverse of the matrix.
   */
  mat2x3 inverse(const mat2x3& m);

  /**
   * @brief Performs a 2D translation to a 2x3 matrix.
   *
   * @param m The matrix to translate.
   * @param v The vector to translate by.
   * @return The translated matrix.
   */
  mat2x3 translate(const mat2x3& m, const vec2 v);

  /**
   * @brief Performs a 2D scale to a 2x3 matrix.
   *
   * @param m The matrix to scale.
   * @param v The vector to scale by.
   * @return The scaled matrix.
   */
  mat2x3 scale(const mat2x3& m, const vec2 v);

  /**
   * @brief Performs a 2D scale from an origin point to a 2x3 matrix.
   *
   * @param m The matrix to scale.
   * @param c The scale origin.
   * @param v The vector to scale by.
   * @return The scaled matrix.
   */
  mat2x3 scale(const mat2x3& m, const vec2 c, const vec2 v);

  /**
   * @brief Performs a 2D rotation to a 2x3 matrix.
   *
   * @param m The matrix to rotate.
   * @param t The angle to rotate by.
   * @return The rotated matrix.
   */
  mat2x3 rotate(const mat2x3& m, const float t);

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
  mat2x3 rotate(const mat2x3& m, const float sin_t, const float cos_t);

  /**
   * @brief Performs a 2D rotation from an origin point to a 2x3 matrix.
   *
   * @param m The matrix to rotate.
   * @param c The rotation origin.
   * @param t The angle to rotate by.
   * @return The rotated matrix.
   */
  mat2x3 rotate(const mat2x3& m, const vec2 c, const float t);

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
  mat2x3 rotate(const mat2x3& m, const vec2 c, const float sin_t, const float cos_t);

  /**
   * @brief Decomposes a 2x3 matrix into its translation, scale, and rotation components.
   *
   * @param m The matrix to decompose.
   * @return The decomposed matrix.
   */
  DecomposedTransform decompose(const mat2x3& m);

  /**
   * @brief Calculates the translation component of a 2x3 matrix.
   *
   * @param m The matrix to calculate the translation component of.
   * @return The translation component of the matrix.
   */
  vec2 translation(const mat2x3& m);

  /**
   * @brief Calculates the rotation component of a 2x3 matrix.
   *
   * @param m The matrix to calculate the rotation component of.
   * @return The rotation component of the matrix.
   */
  float rotation(const mat2x3& m);

  /**
   * @brief Overloaded operators for transforming rects.
   */
  rect operator*(const mat2x3& m, const rect& r);
  rect operator/(const mat2x3& m, const rect& r);

}
