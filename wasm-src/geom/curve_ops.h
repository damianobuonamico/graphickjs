/**
 * @file geom/curve_ops.cpp
 * @brief This file contains the declaration of methods related to bezier curves.
 */

#pragma once

#include "quadratic_bezier.h"
#include "quadratic_path.h"
#include "cubic_bezier.h"
#include "line.h"

#include "../math/math.h"

#include <vector>

namespace graphick::geom {

  /* -- Sample -- */

  /**
   * @brief Samples a quadratic bezier curve at a given t value.
   *
   * @param quad The quadratic bezier curve.
   * @param t The point at which to sample the curve.
   * @return The sampled point.
   */
  template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
  inline math::Vec2<T> quadratic(const QuadraticBezier<T>& quad, const T t) {
    const auto [a, b, c] = quad.coefficients();
    return a * t * t + b * t + c;
  }

  /**
   * @brief Samples a cubic bezier curve at a given t value.
   *
   * @param cubic The cubic bezier curve.
   * @param t The point at which to sample the curve.
   * @return The sampled point.
   */
  template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
  inline math::Vec2<T> cubic(const CubicBezier<T>& cubic, const T t) {
    const auto [a, b, c, d] = cubic.coefficients();
    const T t_sq = t * t;
    return a * t * t_sq + b * t_sq + c * t + d;
  }

  /* -- Curvature -- */

  // TODO: doc
  template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
  math::CubicSolutions<T> max_curvature(const CubicBezier<T>& cubic);

  // TODO: doc
  template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
  math::QuadraticSolutions<T> inflections(const CubicBezier<T>& cubic);

  /* -- Approximate Bounding Rectangle -- */

  /**
   * @brief Calculates an approximate bounding rectangle of a quadratic bezier curve.
   *
   * @param quad The quadratic bezier curve.
   * @return The approximate bounding rectangle.
   */
  template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
  inline math::Rect<T> approx_bounding_rect(const QuadraticBezier<T>& quad) {
    return math::Rect<T>::from_vectors({ quad.p0, quad.p1, quad.p2 });
  }

  /**
   * @brief Calculates an approximate bounding rectangle of a cubic bezier curve.
   *
   * @param cubic The cubic bezier curve.
   * @return The approximate bounding rectangle.
   */
  template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
  inline math::Rect<T> approx_bounding_rect(const CubicBezier<T>& cubic) {
    return math::Rect<T>::from_vectors({ cubic.p0, cubic.p1, cubic.p2, cubic.p3 });
  }

  /* -- Bounding Rectangle -- */

  /**
   * @brief Calculates the bounding rectangle of a quadratic bezier curve.
   *
   * @param quad The quadratic bezier curve.
   * @return The bounding rectangle.
   */
  template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
  math::Rect<T> bounding_rect(const QuadraticBezier<T>& quad);

  /**
   * @brief Calculates the bounding rectangle of a cubic bezier curve.
   *
   * @param cubic The cubic bezier curve.
   * @return The bounding rectangle.
   */
  template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
  math::Rect<T> bounding_rect(const CubicBezier<T>& cubic);

  /* -- Curve Splitting -- */

  /**
   * @brief Splits a quadratic bezier curve into two at a given t value.
   *
   * @param quad The quadratic bezier curve.
   * @param t The point at which to split the curve.
   * @return The resulting quadratic bezier curves.
   */
  template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
  std::array<QuadraticBezier<T>, 2> split(const QuadraticBezier<T>& quad, const T t);

  /**
   * @brief Splits a cubic bezier curve into three at two a given t values.
   *
   * @param cubic The cubic bezier curve.
   * @param t1 The first point at which to split the curve.
   * @param t2 The second point at which to split the curve.
   * @return The resulting cubic bezier curves.
   */
  template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
  std::array<QuadraticBezier<T>, 3> split(const QuadraticBezier<T>& quad, const T t1, const T t2);

  /**
   * @brief Splits a cubic bezier curve into two at a given t value.
   *
   * @param cubic The cubic bezier curve.
   * @param t The point at which to split the curve.
   * @return The resulting cubic bezier curves.
   */
  template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
  std::array<CubicBezier<T>, 2> split(const CubicBezier<T>& cubic, const T t);

  /**
   * @brief Splits a cubic bezier curve into three at two a given t values.
   *
   * @param cubic The cubic bezier curve.
   * @param t1 The first point at which to split the curve.
   * @param t2 The second point at which to split the curve.
   * @return The resulting cubic bezier curves.
   */
  template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
  std::array<CubicBezier<T>, 3> split(const CubicBezier<T>& cubic, const T t1, const T t2);

  /* -- Curve Extraction -- */

  /**
   * @brief Extracts the part of a quadratic bezier curve between two t values.
   *
   * @param quad The quadratic bezier curve.
   * @param t1 The first t value.
   * @param t2 The second t value.
   * @return The extracted quadratic bezier curve.
   */
  template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
  QuadraticBezier<T> extract(const QuadraticBezier<T>& cubic, const T t1, const T t2);

  /**
   * @brief Extracts the part of a cubic bezier curve between two t values.
   *
   * @param cubic The cubic bezier curve.
   * @param t1 The first t value.
   * @param t2 The second t value.
   * @return The extracted cubic bezier curve.
   */
  template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
  CubicBezier<T> extract(const CubicBezier<T>& cubic, const T t1, const T t2);

  /* -- Conversion -- */

  /**
   * @brief Converts a cubic bezier curve into a sequence of quadratic bezier curves.
   *
   * @param cubic The cubic bezier curve.
   * @param tolerance The maximum distance between the cubic and the resulting quadratics.
   * @param sink The quadratic path to store the resulting curves.
   */
  template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
  void cubic_to_quadratics(const CubicBezier<T>& cubic, const T tolerance, QuadraticPath<T>& sink);

  template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
  std::vector<std::pair<QuadraticBezier<T>, math::Vec2<T>>> cubic_to_quadratics_with_intervals(const CubicBezier<T>& cubic);

}
