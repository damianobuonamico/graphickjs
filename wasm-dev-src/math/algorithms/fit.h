/**
 * @file fit.h
 * @brief Contains functions for fitting cubic Bezier curves to sets of points.
 *
 * The functions use the least-squares method to find the best-fitting curve, which minimizes the sum of the squared distances between the curve and the points.
 */

#pragma once

#include "../vec2.h"

#include <vector>

namespace Graphick::Math::Algorithms {

  /**
   * @struct CubicBezier
   * @brief Represents a cubic Bezier curve.
   */
  struct CubicBezier {
    vec2 p0;              /* The first control point. */
    vec2 p1;              /* The second control point. */
    vec2 p2;              /* The third control point. */
    vec2 p3;              /* The fourth control point. */

    size_t start_index;   /* The index of the first point in the set that the curve approximates. */
    size_t end_index;     /* The index of the last point in the set that the curve approximates. */

    vec2& operator[](uint8_t i);

    const vec2& operator[](uint8_t i) const;

    const vec2* operator&() const;
  };


  /**
   * @brief Fits a cubic Bezier curve to a set of points using the least-squares method.
   *
   * @param points The points to fit the curve to.
   * @param error The maximum allowed error between the curve and the points.
   * @return A cubic Bezier curve that approximates the given points.
   */
  CubicBezier fit_points_to_cubic(
    const std::vector<vec2>& points,
    float error
  );

}
