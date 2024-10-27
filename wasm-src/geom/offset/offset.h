/**
 * @file geom/offset/cubic_offset.h
 * @brief Contains the definition of methods to offset bezier curves.
 */

#pragma once

#include "../cubic_bezier.h"
#include "../cubic_path.h"

namespace graphick::geom {

/**
 * @brief Find a set of segments that approximate parallel curve.
 *
 * @param curve The input cubic bezier curve.
 * @param offset If it is zero, resulting curve will be identical to input curve. Can be negative.
 * @param tolerance The maximum distance between the offset curve and its approximation.
 * @param sink The output quadratic path.
 */
template<typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
void offset_cubic(const dcubic_bezier &curve,
                  const double offset,
                  const double tolerance,
                  CubicPath<T> &sink);

}  // namespace graphick::geom
