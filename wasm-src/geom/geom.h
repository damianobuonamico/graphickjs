/**
 * @file geom/geom.h
 * @brief This file contains geometric utility functions.
 */

#pragma once

#include "line.h"

#include "../math/rect.h"
#include "../math/vector.h"

#include <array>

namespace graphick::geom {

/* -- Ellipse -- */

/**
 * @brief Calculates the center of a circle given three points on its circumference.
 *
 * @param a The first point.
 * @param b The second point.
 * @param c The third point.
 * @return The center of the circle.
 */
template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
inline math::Vec2<T> circle_center(const math::Vec2<T> a, const math::Vec2<T> b, const math::Vec2<T> c) {
  const T offset = math::squared_length(b);
  const T bc = (math::squared_length(a) - offset) / T(2);
  const T cd = (offset - (math::squared_length(c))) / T(2);
  const T det = (a.x - b.x) * (b.y - c.y) - (b.x - c.x) * (a.y - b.y);

  if (math::is_almost_zero(det)) {
    return math::Vec2<T>::zero();
  }

  const T inverse_det = T(1) / det;

  return {(bc * (b.y - c.y) - cd * (a.y - b.y)) * inverse_det, (cd * (a.x - b.x) - bc * (b.x - c.x)) * inverse_det};
}

/* -- Rectangle -- */

/**
 * @brief Converts a rotated rect to a rect.
 *
 * Returns a new rect that is the smallest rect that contains the rotated rect.
 *
 * @param r The rotated rect to convert.
 * @return The resulting rect.
 */
template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
inline math::Rect<T> rrect_to_rect(const math::RRect<T>& r) {
  const math::Vec2<T> center = r.center();

  const T sin = std::sin(r.angle);
  const T cos = std::cos(r.angle);

  math::Vec2<T> r1 = math::rotate(r.min, center, sin, cos);
  math::Vec2<T> r2 = math::rotate({r.min.x, r.max.y}, center, sin, cos);
  math::Vec2<T> r3 = math::rotate(r.max, center, sin, cos);
  math::Vec2<T> r4 = math::rotate({r.max.x, r.min.y}, center, sin, cos);

  return {math::min(math::min(r1, r2), math::min(r3, r4)), math::max(math::max(r1, r2), math::max(r3, r4))};
}

/**
 * @brief Recalculates the minimum and maximum values of a rect.
 *
 * @param r The rect to straighten.
 * @return The resulting rect.
 */
template <typename T>
inline math::Rect<T> straighten_rect(const math::Rect<T>& r) {
  return {math::min(r.min, r.max), math::max(r.min, r.max)};
}

/**
 * @brief Calculates the lines that make up a rect.
 *
 * @param rect The rect.
 * @return The lines that make up the rect.
 */
template <typename T>
inline std::array<Line<T>, 4> lines_from_rect(const math::Rect<T>& rect) {
  return {
    {rect.min, {rect.max.x, rect.min.y}},
    {{rect.max.x, rect.min.y}, rect.max},
    {rect.max, {rect.min.x, rect.max.y}},
    {{rect.min.x, rect.max.y}, rect.min}
  };
}

/* -- Polygon -- */

/**
 * @brief Value returned when trying to determine orientation of 3 points.
 */
enum class TriangleOrientation : uint8_t {
  Clockwise,        /* Points are in clockwise orientation. */
  CounterClockwise, /* Points are in counter-clockwise orientation. */
  Collinear         /* Points are collinear and orientation cannot be determined. */
};

/**
 * @brief Determines the orientation of a triangle.
 *
 * @param p0 The first point.
 * @param p1 The second point.
 * @param p2 The third point.
 * @return The orientation of the triangle.
 */
template <typename T>
inline TriangleOrientation triangle_orientation(const math::Vec2<T> p0, const math::Vec2<T> p1, const math::Vec2<T> p2) {
  const T turn = math::cross(p1 - p0, p2 - p0);

  if (math::is_almost_zero(turn)) {
    return TriangleOrientation::Collinear;
  } else if (turn > T(0)) {
    return TriangleOrientation::Clockwise;
  }

  return TriangleOrientation::CounterClockwise;
}

/**
 * @brief Determines if a set of points is in clockwise order.
 *
 * @param p0 The first point.
 * @param p1 The second point.
 * @param p2 The third point.
 * @return true if the points are in clockwise order, false otherwise.
 */
template <typename T>
inline bool clockwise(const math::Vec2<T> p0, const math::Vec2<T> p1, const math::Vec2<T> p2) {
  return triangle_orientation(p0, p1, p2) == TriangleOrientation::Clockwise;
}

/**
 * @brief Determines if a set of points is in clockwise order.
 *
 * @param points The points to check.
 * @return true if the points are in clockwise order, false otherwise.
 */
template <typename T>
inline bool clockwise(const std::vector<math::Vec2<T>>& points) {
  T sum = (points.front().x - points.back().x) * (points.front().y + points.back().y);

  for (size_t i = 0; i < points.size() - 1; i++) {
    sum += (points[i + 1].x - points[i].x) * (points[i + 1].y + points[i].y);
  }

  return sum >= T(0);
}

template <typename T>
inline bool collinear(const math::Vec2<T> p0, const math::Vec2<T> p1, const math::Vec2<T> p2, const T eps = math::geometric_epsilon<T>) {
  return math::is_almost_zero(math::cross(p1 - p0, p2 - p0), eps);
}

}  // namespace graphick::geom
