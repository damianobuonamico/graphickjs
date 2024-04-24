/**
 * @file algorithms/fit.h
 * @brief Contains functions for fitting cubic Bezier curves to sets of points.
 *
 * The functions use the least-squares method to find the best-fitting curve,
 * which minimizes the sum of the squared distances between the curve and the points.
 */

#pragma once

#include "../geom/cubic_bezier.h"

#include <vector.h>

namespace graphick::algorithms {

  /**
   * @brief Fits a cubic Bezier curve to a set of points using the least-squares method.
   *
   * @param points The points to fit the curve to.
   * @param tolerance The maximum allowed error between the curve and the points.
   * @return A cubic Bezier curve that approximates the given points.
   */
  template <typename T, std::enable_if<std::is_floating_point_v<T>>>
  geom::CubicBezier<T> fit_points_to_cubic(
    const std::vector<math::Vec2<T>>& points,
    const T tolerance
  );

}
