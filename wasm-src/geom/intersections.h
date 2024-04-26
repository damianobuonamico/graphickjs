/**
 * @file geom/intersections.h
 * @brief This file contains hit testing and intersection methods for geometric shapes.
 */

#pragma once

#include "quadratic_bezier.h"
#include "cubic_bezier.h"
#include "line.h"

#include "../math/vector.h"
#include "../math/rect.h"

#include <optional>

namespace graphick::geom {

  /* -- Ellipse -- */

  /**
   * @brief Determines if a point is inside an ellipse.
   *
   * @param point The point to check.
   * @param center The center of the ellipse.
   * @param radius The radii of the ellipse.
   * @param threshold The threshold for the check.
   * @return True if the point is inside the ellipse, false otherwise.
   */
  template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
  inline bool is_point_in_ellipse(const math::Vec2<T> point, const math::Vec2<T> center, const math::Vec2<T> radius, const T threshold = T(0)) {
    const math::Vec2<T> diff = point - center;
    const math::Vec2<T> rad = radius + threshold;

    return diff.x * diff.x / (rad.x * rad.x) + diff.y * diff.y / (rad.y * rad.y) <= T(1);
  }

  /**
   * @brief Determines if a point is inside a circle.
   *
   * @param point The point to check.
   * @param center The center of the circle.
   * @param radius The radius of the circle.
   * @param threshold The threshold for the check.
   * @return True if the point is inside the circle, false otherwise.
   */
  template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
  inline bool is_point_in_circle(const math::Vec2<T> point, const math::Vec2<T> center, const T radius, const T threshold = T(0)) {
    const T rad = radius + threshold;

    return math::squared_distance(point, center) <= rad * rad;
  }

  /* -- Rectangle -- */

  /**
   * @brief Determines if a point is inside a rect.
   *
   * @param point The point to check.
   * @param rect The rect to check.
   * @param threshold The threshold for the check.
   * @return True if the point is inside the rect, false otherwise.
   */
  template <typename T>
  inline bool is_point_in_rect(const math::Vec2<T> point, const math::Rect<T>& rect, const T threshold = T(0)) {
    return (
      point.x + threshold >= rect.min.x &&
      point.x - threshold <= rect.max.x &&
      point.y + threshold >= rect.min.y &&
      point.y - threshold <= rect.max.y
    );
  }

  /**
   * @brief Determines if a point is inside a rect.
   *
   * @param point The point to check.
   * @param rect The rect to check.
   * @param threshold The threshold for the check.
   * @return True if the point is inside the rect, false otherwise.
   */
  template <typename T>
  inline bool is_point_in_rect(const math::Vec2<T> point, const math::Rect<T>& rect, const math::Vec2<T> threshold) {
    return (
      point.x + threshold.x >= rect.min.x &&
      point.x - threshold.x <= rect.max.x &&
      point.y + threshold.y >= rect.min.y &&
      point.y - threshold.y <= rect.max.y
    );
  }

  /**
   * @brief Determines if two rects intersect.
   *
   * @param a The first rect.
   * @param b The second rect.
   * @return True if the rects intersect, false otherwise.
   */
  template <typename T>
  inline bool does_rect_intersect_rect(const math::Rect<T>& a, const math::Rect<T>& b) {
    return (
      b.max.x >= a.min.x &&
      a.max.x >= b.min.x &&
      b.max.y >= a.min.y &&
      a.max.y >= b.min.y
    );
  }

  /**
   * @brief Determines if one rect is inside another.
   *
   * @param a The first rect.
   * @param b The second rect.
   * @return True if the first rect is inside the second, false otherwise.
   */
  template <typename T>
  inline bool is_rect_in_rect(const math::Rect<T>& a, const math::Rect<T>& b) {
    return (
      a.min.x >= b.min.x &&
      a.max.x <= b.max.x &&
      a.min.y >= b.min.y &&
      a.max.y <= b.max.y
    );
  }

  /**
   * @brief Calculates the intersection area of two rects.
   *
   * @param a The first rect.
   * @param b The second rect.
   * @return The intersection area.
   */
  template <typename T>
  inline T rect_rect_intersection_area(const math::Rect<T>& a, const math::Rect<T>& b) {
    const T x_left = std::max(a.min.x, b.min.x);
    const T y_top = std::max(a.min.y, b.min.y);
    const T x_right = std::min(a.max.x, b.max.x);
    const T y_bottom = std::min(a.max.y, b.max.y);

    if (x_right < x_left || y_bottom < y_top) {
      return T(0);
    }

    return (x_right - x_left) * (y_bottom - y_top);
  }

  /* -- Line -- */

  /**
   * @brief Calculates the intersection t value of two lines.
   *
   * The t value is relative to the first line.
   *
   * @param a The first line segment.
   * @param b The second line segment.
   * @return The intersection t value if it exists, std::nullopt otherwise.
   */
  std::optional<double> line_line_intersection(const dline& a, const dline& b);

  /**
   * @brief Calculates the intersection t value of two lines.
   *
   * The t value is relative to the first line.
   *
   * @param a The first line segment.
   * @param b The second line segment.
   * @return The intersection t value if it exists, std::nullopt otherwise.
   */
  inline std::optional<float> line_line_intersection(const line& a, const line& b) {
    return line_line_intersection(dline(a), dline(b));
  }

  /**
   * @brief Calculates the intersection points of two lines.
   *
   * @param a The first line segment.
   * @param b The second line segment.
   * @return The intersection point if it exists, std::nullopt otherwise.
   */
  std::optional<dvec2> line_line_intersection_point(const dline& a, const dline& b);

  /**
   * @brief Calculates the intersection points of two lines.
   *
   * @param a The first line segment.
   * @param b The second line segment.
   * @return The intersection point if it exists, std::nullopt otherwise.
   */
  inline std::optional<vec2> line_line_intersection_point(const line& a, const line& b) {
    if (std::optional<dvec2> point = line_line_intersection_point(dline(a), dline(b)); point.has_value()) {
      return vec2(point.value());
    }

    return std::nullopt;
  }

  /**
   * @brief Calculates the intersection points of a line and a circle.
   *
   * @param line The line segment.
   * @param center The center of the circle.
   * @param radius The radius of the circle.
   * @return The intersection points.
   */
  std::vector<dvec2> line_circle_intersection_points(const dline& line, const dvec2 center, const double radius);

  /**
   * @brief Calculates the intersection points of a line and a circle.
   *
   * @param line The line segment.
   * @param center The center of the circle.
   * @param radius The radius of the circle.
   * @return The intersection points.
   */
  inline std::vector<vec2> line_circle_intersection_points(const line& line, const vec2 center, const float radius) {
    std::vector<dvec2> points = line_circle_intersection_points(dline(line), dvec2(center), radius);

    return std::vector<vec2>(points.begin(), points.end());
  }

  /**
   * @brief Calculates the intersection points between a line segment and a rect.
   *
   * @param line the line segment.
   * @return A vector containing the intersection points, if any.
   */
  std::vector<dvec2> line_rect_intersection_points(const dline& line, const drect& rect);

  /**
   * @brief Calculates the intersection points between a line segment and a rect.
   *
   * @param line the line segment.
   * @param rect The rect to check for intersection.
   * @return A vector containing the intersection points, if any.
   */
  inline std::vector<vec2> line_rect_intersection_points(const line& line, const rect& rect) {
    std::vector<dvec2> points = line_rect_intersection_points(dline(line), drect(rect));

    return std::vector<vec2>(points.begin(), points.end());
  }

  /**
   * @brief Checks whether or not a linear segment intersects a rect.
   *
   * @param line The line segment.
   * @param rect The rect to check for intersection.
   * @return true if they intersect, false otherwise.
   */
  template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
  inline bool does_line_intersect_rect(const Line<T>& line, const math::Rect<T>& rect) {
    return !line_rect_intersection_points(line, rect).empty();
  }

  /**
   * @brief Calculates the closest t parameter to a point on a line.
   *
   * @param line The line segment.
   * @param p The point to find the closest t to.
   * @return The closest t value, clamped to the range [0, 1].
   */
  double line_closest_to(const dline& line, const dvec2 p);

  /**
   * @brief Calculates the closest t parameter to a point on a line.
   *
   * @param line The line segment.
   * @param p The point to find the closest t to.
   * @return The closest t value, clamped to the range [0, 1].
   */
  inline float line_closest_to(const line& line, const vec2 p) {
    return static_cast<float>(line_closest_to(dline(line), dvec2(p)));
  }

  /* -- QuadraticBezier -- */

  /**
   * @brief Calculates the intersection points between a quadratic curve and a rect.
   *
   * @param quad The quadratic bezier curve.
   * @return A vector containing the t values of the intersections between the quadratic curve and the rect.
   */
  std::vector<double> quadratic_rect_intersections(const dquadratic_bezier& quad, const drect& rect);

  /**
   * @brief Calculates the intersection points between a quadratic curve and a rect.
   *
   * @param quad The quadratic bezier curve.
   * @param rect The rect to intersect with the Bezier curve.
   * @return A vector containing the t values of the intersections between the quadratic curve and the rect.
   */
  inline std::vector<float> quadratic_rect_intersections(const quadratic_bezier& quad, const rect& rect) {
    std::vector<double> intersections = quadratic_rect_intersections(dquadratic_bezier(quad), drect(rect));

    return std::vector<float>(intersections.begin(), intersections.end());
  }

  /**
   * @brief Checks whether or not a quadratic segment intersects a rect.
   *
   * @param quad The quadratic bezier curve.
   * @param rect The rect to check for intersection.
   * @return true if they intersect, false otherwise.
   */
  template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
  inline bool does_quadratic_intersect_rect(const QuadraticBezier<T>& quad, const math::Rect<T>& rect) {
    return !quadratic_rect_intersections(quad, rect).empty();
  }

  /**
   * @brief Calculates the closest t parameter to a point on a quadratic bezier curve.
   *
   * @param quad The quadratic bezier curve.
   * @param p The point to find the closest t to.
   * @return The closest t value, clamped to the range [0, 1].
   */
  double quadratic_closest_to(const dquadratic_bezier& quad, const dvec2 p);

  /**
   * @brief Calculates the closest t parameter to a point on a quadratic bezier curve.
   *
   * @param quad The quadratic bezier curve.
   * @param p The point to find the closest t to.
   * @return The closest t value, clamped to the range [0, 1].
   */
  inline float quadratic_closest_to(const quadratic_bezier& quad, const vec2 p) {
    return static_cast<float>(quadratic_closest_to(dquadratic_bezier(quad), dvec2(p)));
  }

  /* -- CubicBezier -- */

  /**
   * @brief Calculates the intersection points between a cubic Bezier curve and a rect.
   *
   * @param cubic The cubic bezier curve.
   * @param rect The rect to intersect with the Bezier curve.
   * @return A vector containing the t values of the intersections between the Bezier curve and the rect.
   */
  std::vector<double> cubic_rect_intersections(const dcubic_bezier& cubic, const drect& rect);

  /**
   * @brief Calculates the intersection points between a cubic Bezier curve and a rect.
   *
   * @param cubic The cubic bezier curve.
   * @param rect The rect to intersect with the Bezier curve.
   * @return A vector containing the t values of the intersections between the Bezier curve and the rect.
   */
  inline std::vector<float> cubic_rect_intersections(const cubic_bezier& cubic, const rect& rect) {
    std::vector<double> intersections = cubic_rect_intersections(dcubic_bezier(cubic), drect(rect));

    return std::vector<float>(intersections.begin(), intersections.end());
  }

  /**
   * @brief Checks whether or not a cubic segment intersects a rect.
   *
   * @param cubic The cubic bezier curve.
   * @param rect The rect to check for intersection.
   * @return true if they intersect, false otherwise.
   */
  template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
  inline bool does_cubic_intersect_rect(const CubicBezier<T>& cubic, const math::Rect<T>& rect) {
    return !cubic_rect_intersections(cubic, rect).empty();
  }

  /**
   * @brief Calculates the closest t parameter to a point on a cubic bezier curve.
   *
   * @param cubic The cubic bezier curve.
   * @param p The point to find the closest t to.
   * @return The closest t value, clamped to the range [0, 1].
   */
  double cubic_closest_to(const dcubic_bezier& cubic, const dvec2 p);

  /**
   * @brief Calculates the closest t parameter to a point on a cubic bezier curve.
   *
   * @param cubic The cubic bezier curve.
   * @param p The point to find the closest t to.
   * @return The closest t value, clamped to the range [0, 1].
   */
  inline float cubic_closest_to(const cubic_bezier& cubic, const vec2 p) {
    return static_cast<float>(cubic_closest_to(dcubic_bezier(cubic), dvec2(p)));
  }

}
