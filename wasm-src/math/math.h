/**
 * @file math.h
 * @brief Contains various math functions and structs used throughout the Graphick editor.
 *
 * @todo implement solve_cubic_approx()
 */

#pragma once

#include "scalar.h"
#include "vector.h"
#include "rect.h"

#include "f24x8.h"

#include <optional>
#include <vector>
#include <tuple>

namespace Graphick::Math {

  /**
   * @brief Struct containing solutions to a quadratic equation.
   *
   * @struct QuadraticSolutions
   */
  struct QuadraticSolutions {
    double solutions[2];    /* The solutions. */
    uint8_t count;          /* The number of solutions. */

    /**
     * @brief Constructs a QuadraticSolutions object.
     */
    QuadraticSolutions() : count(0), solutions{ 0.0f, 0.0f } {}
    QuadraticSolutions(const double x) : count(1), solutions{ x, 0.0f } {}
    QuadraticSolutions(const double x1, const double x2) : count(2), solutions{ x1, x2 } {}
  };

  /**
   * @brief Struct containing solutions to a cubic equation.
   *
   * @struct CubicSolutions
   */
  struct CubicSolutions {
    double solutions[3];    /* The solutions. */
    uint8_t count;          /* The number of solutions. */

    /**
     * @brief Constructs a CubicSolutions object.
     */
    CubicSolutions() : count(0), solutions{ 0.0f, 0.0f, 0.0f } {}
    CubicSolutions(const double x) : count(1), solutions{ x, 0.0f, 0.0f } {}
    CubicSolutions(const double x1, const double x2) : count(2), solutions{ x1, x2, 0.0f } {}
    CubicSolutions(const double x1, const double x2, const double x3) : count(3), solutions{ x1, x2, x3 } {}
    CubicSolutions(const QuadraticSolutions& quadratic) : count(quadratic.count), solutions{ quadratic.solutions[0], quadratic.solutions[1], 0.0f } {}
  };

  /**
   * @brief Solves a linear equation of the form ax + b = 0.
   *
   * Assumes that a != 0.
   *
   * @param a The coefficient of x.
   * @param b The constant term.
   * @return The solution for x.
   */
  inline double solve_linear(const double a, const double b) {
    return -b / a;
  }

  /**
   * @brief Solves a quadratic equation of the form ax^2 + bx + c = 0.
   *
   * @param a The coefficient of x^2.
   * @param b The coefficient of x.
   * @param c The constant term.
   * @return A struct containing the real solutions to the equation.
   */
  QuadraticSolutions solve_quadratic(double a, double b, double c);

  /**
   * @brief Solves a cubic equation of the form ax^3 + bx^2 + cx + d = 0.
   *
   * @param a The coefficient of x^3.
   * @param b The coefficient of x^2.
   * @param c The coefficient of x.
   * @param d The constant term.
   * @return A struct containing the real solutions of the cubic equation.
   */
  CubicSolutions solve_cubic(double a, double b, double c, double d);

  /**
   * @brief Converts a rotated rect to a rect.
   *
   * Returns a new rect that is the smallest rect that contains the rotated rect.
   *
   * @param r The rotated rect to convert.
   * @return The resulting rect.
   */
  rect rrect_to_rect(const rrect& r);

  /**
   * @brief Recalculates the minimum and maximum values of a rect.
   *
   * @param r The rect to straighten.
   * @return The resulting rect.
   */
  inline rect straighten_rect(const rect& r) {
    return {
      min(r.min, r.max),
      max(r.min, r.max)
    };
  }

  /**
   * @brief Determines if a point is inside a circle.
   *
   * @param point The point to check.
   * @param center The center of the circle.
   * @param radius The radius of the circle.
   * @return True if the point is inside the circle, false otherwise.
   */
  inline bool is_point_in_circle(const vec2 point, const vec2 center, const float radius) {
    return squared_distance(point, center) <= radius * radius;
  }

  /**
   * @brief Determines if a point is inside an ellipse.
   *
   * @param point The point to check.
   * @param center The center of the ellipse.
   * @param radius The radii of the ellipse.
   * @return True if the point is inside the ellipse, false otherwise.
   */
  inline bool is_point_in_ellipse(const vec2 point, const vec2 center, const vec2 radius) {
    return std::pow((point.x - center.x), 2) / std::pow(radius.x, 2) + std::pow((point.y - center.y), 2) / std::pow(radius.y, 2) <= 1.0f;
  }

  /**
   * @brief Determines if a point is inside a rect.
   *
   * @param point The point to check.
   * @param rect The rect to check.
   * @param threshold The threshold for the check.
   * @return True if the point is inside the rect, false otherwise.
   */
  inline bool is_point_in_rect(const vec2 point, const rect& rect, const float threshold = 0.0f) {
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
  inline bool is_point_in_rect(const vec2 point, const rect& rect, const vec2 threshold) {
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
  inline bool does_rect_intersect_rect(const rect& a, const rect& b) {
    return b.max.x >= a.min.x && a.max.x >= b.min.x && b.max.y >= a.min.y && a.max.y >= b.min.y;
  }

  /**
   * @brief Determines if one rect is inside another.
   *
   * @param a The first rect.
   * @param b The second rect.
   * @return True if the first rect is inside the second, false otherwise.
   */
  inline bool is_rect_in_rect(const rect& a, const rect& b) {
    return a.min.x >= b.min.x && a.max.x <= b.max.x && a.min.y >= b.min.y && a.max.y <= b.max.y;
  }

  /**
   * @brief Calculates the intersection area of two rects.
   *
   * @param a The first rect.
   * @param b The second rect.
   * @return The intersection area.
   */
  inline float rect_rect_intersection_area(const rect& a, const rect& b) {
    float x_left = std::max(a.min.x, b.min.x);
    float y_top = std::max(a.min.y, b.min.y);
    float x_right = std::min(a.max.x, b.max.x);
    float y_bottom = std::min(a.max.y, b.max.y);

    if (x_right < x_left || y_bottom < y_top) {
      return 0.0f;
    }

    return (x_right - x_left) * (y_bottom - y_top);
  }

  /**
   * @brief Calculates the intersection t value of two lines.
   *
   * The t value is relative to the first line.
   *
   * @param a The first line.
   * @param b The second line.
   * @return The intersection t value if it exists, std::nullopt otherwise.
   */
  std::optional<float> line_line_intersection(const rect& a, const rect& b);

  /**
   * @brief Calculates the intersection points of two lines.
   *
   * @param a The first line.
   * @param b The second line.
   * @return The intersection point if it exists, std::nullopt otherwise.
   */
  std::optional<vec2> line_line_intersection_point(const rect& a, const rect& b);

  /**
   * @brief Calculates the intersection points of a line and a circle.
   *
   * @param line The line.
   * @param center The center of the circle.
   * @param radius The radius of the circle.
   * @return The intersection points.
   */
  std::vector<vec2> line_circle_intersection_points(const rect& line, const vec2 center, const float radius);

  /**
   * @brief Calculates the intersection points between a line segment and a rect.
   *
   * @param p0 The starting point of the line segment.
   * @param p3 The ending point of the line segment.
   * @param rect The rect to check for intersection.
   * @return A vector containing the intersection points, if any.
   */
  std::vector<vec2> line_rect_intersection_points(const vec2 p0, const vec2 p3, const rect& rect);

  /**
   * @brief Calculates the intersection points between a quadratic curve and a rect.
   *
   * @param p0 The first control point of the quadratic curve.
   * @param p1 The second control point of the quadratic curve.
   * @param p2 The third control point of the quadratic curve.
   * @param rect The rect to intersect with the Bezier curve.
   * @return A vector containing the t values of the intersections between the quadratic curve and the rect.
   */
  std::vector<float> quadratic_rect_intersections(const vec2 p0, const vec2 p1, const vec2 p2, const rect& rect);

  /**
   * @brief Calculates the intersection points between a cubic Bezier curve and a rect.
   *
   * @param p0 The first control point of the Bezier curve.
   * @param p1 The second control point of the Bezier curve.
   * @param p2 The third control point of the Bezier curve.
   * @param p3 The fourth control point of the Bezier curve.
   * @param rect The rect to intersect with the Bezier curve.
   * @return A vector containing the t values of the intersections between the Bezier curve and the rect.
   */
  std::vector<float> cubic_rect_intersections(const vec2 p0, const vec2 p1, const vec2 p2, const vec2 p3, const rect& rect);

  /**
   * @brief Checks whether or not a linear segment intersects a rect.
   *
   * @param p0 The starting point of the line segment.
   * @param p1 The ending point of the line segment.
   * @param rect The rect to check for intersection.
   * @return true if they intersect, false otherwise.
   */
  bool does_line_intersect_rect(const vec2 p0, const vec2 p1, const rect& rect);

  /**
   * @brief Checks whether or not a quadratic segment intersects a rect.
   *
   * @param p0 The first control point of the quadratic curve.
   * @param p1 The second control point of the quadratic curve.
   * @param p2 The third control point of the quadratic curve.
   * @param rect The rect to check for intersection.
   * @return true if they intersect, false otherwise.
   */
  bool does_quadratic_intersect_rect(const vec2 p0, const vec2 p1, const vec2 p2, const rect& rect);

  /**
   * @brief Checks whether or not a cubic segment intersects a rect.
   *
  * @param p0 The first control point of the Bezier curve.
   * @param p1 The second control point of the Bezier curve.
   * @param p2 The third control point of the Bezier curve.
   * @param p3 The fourth control point of the Bezier curve.
   * @param rect The rect to check for intersection.
   * @return true if they intersect, false otherwise.
   */
  bool does_cubic_intersect_rect(const vec2 p0, const vec2 p1, const vec2 p3, const vec2 p4, const rect& rect);

  /**
   * @brief Calculates the closest t parameter to a point on a line.
   *
   * @param p0 The starting point of the line.
   * @param p1 The ending point of the line.
   * @param p The point to find the closest t to.
   * @return The closest t value, clamped to the range [0, 1].
   */
  float linear_closest_to(const vec2 p0, const vec2 p1, const vec2 p);

  /**
   * @brief Calculates the closest t parameter to a point on a quadratic bezier curve.
   *
   * @param p0 The first control point of the Bezier curve.
   * @param p1 The second control point of the Bezier curve.
   * @param p2 The third control point of the Bezier curve.
   * @param p The point to find the closest t to.
   * @return The closest t value, clamped to the range [0, 1].
   */
  float quadratic_closest_to(const vec2 p0, const vec2 p1, const vec2 p2, const vec2 p);

  /**
   * @brief Calculates the closest t parameter to a point on a cubic bezier curve.
   *
   * @param p0 The first control point of the Bezier curve.
   * @param p1 The second control point of the Bezier curve.
   * @param p2 The third control point of the Bezier curve.
   * @param p3 The fourth control point of the Bezier curve.
   * @param p The point to find the closest t to.
   * @return The closest t value, clamped to the range [0, 1].
   */
  float cubic_closest_to(const vec2 p0, const vec2 p1, const vec2 p2, const vec2 p3, const vec2 p);

  /**
   * @brief Calculates the lines that make up a rect.
   *
   * @param rect The rect.
   * @return The lines that make up the rect.
   */
  inline std::vector<rect> lines_from_rect(const rect& rect) {
    return {
      { rect.min, { rect.max.x, rect.min.y }},
      { { rect.max.x, rect.min.y }, rect.max },
      { rect.max, { rect.min.x, rect.max.y } },
      { { rect.min.x, rect.max.y }, rect.min }
    };
  }

  /**
   * @brief Clips a polygon to the right of a given x value.
   *
   * Modifies the input points vector.
   *
   * @param points The points of the polygon.
   * @param x The x value to clip to.
   */
  void clip_to_left(std::vector<vec2>& points, float x);
  void clip_to_left(std::vector<f24x8x2>& points, f24x8 x);

  /**
   * @brief Clips a polygon to the left of a given x value.
   *
   * Modifies the input points vector.
   *
   * @param points The points of the polygon.
   * @param x The x value to clip to.
   */
  void clip_to_right(std::vector<vec2>& points, float x);
  void clip_to_right(std::vector<f24x8x2>& points, f24x8 x);

  /**
   * @brief Clips a polygon to the bottom of a given y value.
   *
   * Modifies the input points vector.
   *
   * @param points The points of the polygon.
   * @param y The y value to clip to.
   */
  void clip_to_top(std::vector<vec2>& points, float y);
  void clip_to_top(std::vector<f24x8x2>& points, f24x8 y);

  /**
   * @brief Clips a polygon to the top of a given y value.
   *
   * Modifies the input points vector.
   *
   * @param points The points of the polygon.
   * @param y The y value to clip to.
   */
  void clip_to_bottom(std::vector<vec2>& points, float y);
  void clip_to_bottom(std::vector<f24x8x2>& points, f24x8 y);

  /**
   * @brief Clips a polygon to a given rect.
   *
   * Modifies the input points vector.
   *
   * @param points The points of the polygon.
   * @param rect The rect to clip to.
   */
  inline void clip(std::vector<vec2>& points, const rect& rect) {
    clip_to_left(points, rect.min.x);
    clip_to_right(points, rect.max.x);
    clip_to_top(points, rect.min.y);
    clip_to_bottom(points, rect.max.y);
  }
  inline void clip(std::vector<f24x8x2>& points, const f24x8x4& rect) {
    clip_to_left(points, rect.x0);
    clip_to_right(points, rect.x1);
    clip_to_top(points, rect.y0);
    clip_to_bottom(points, rect.y1);
  }

  /**
   * @brief Calculates the center of a circle given three points on its circumference.
   *
   * @param a The first point.
   * @param b The second point.
   * @param c The third point.
   * @return The center of the circle.
   */
  vec2 circle_center(const vec2 a, const vec2 b, const vec2 c);

  /**
   * @brief Determines if a set of points is in clockwise order.
   * @param points The points to check.
   * @return True if the points are in clockwise order, false otherwise.
   */
  bool clockwise(const std::vector<vec2>& points);

  /**
   * @brief Calculates the bounding rectangle of a line.
   *
   * @param p0 The first point.
   * @param p1 The second point.
   * @return The bounding rectangle.
   */
  rect linear_bounding_rect(const vec2 p0, const vec2 p1);

  /**
   * @brief Calculates the bounding rectangle of a quadratic bezier curve.
   *
   * @param p0 The first control point.
   * @param p1 The second control point.
   * @param p2 The third control point.
   * @return The bounding rectangle.
   */
  rect quadratic_bounding_rect(const vec2 p0, const vec2 p1, const vec2 p2);

  /**
   * @brief Calculates the bounding rectangle of a cubic bezier curve.
   *
   * @param p0 The first control point.
   * @param p1 The second control point.
   * @param p2 The third control point.
   * @param p3 The fourth control point.
   * @return The bounding rectangle.
   */
  rect cubic_bounding_rect(const vec2 p0, const vec2 p1, const vec2 p2, const vec2 p3);

  /**
   * @brief Splits a bezier curve into two at a given point.
   *
   * @param p0 The first control point.
   * @param p1 The second control point.
   * @param p2 The third control point.
   * @param p3 The fourth control point.
   * @param t The point at which to split the curve.
   * @return The resulting control points for the two curves.
   */
  std::tuple<vec2, vec2, vec2, vec2, vec2> split_bezier(const vec2 p0, const vec2 p1, const vec2 p2, const vec2 p3, const float t);

  /**
   * @brief Splits a bezier curve into two at two given points.
   *
   * @param p0 The first control point.
   * @param p1 The second control point.
   * @param p2 The third control point.
   * @param p3 The fourth control point.
   * @param t1 The first point at which to split the curve.
   * @param t2 The second point at which to split the curve.
   * @return The resulting control points for the two curves.
   */
  std::tuple<vec2, vec2, vec2, vec2> split_bezier(const vec2 p0, const vec2 p1, const vec2 p2, const vec2 p3, const float t1, const float t2);

  /**
   * @brief Splits a quadratic bezier curve into two at a given point.
   *
   * @param p0 The first control point.
   * @param p1 The second control point.
   * @param p2 The third control point.
   * @param t The point at which to split the curve.
   * @return The resulting control points for the two curves.
   */
  template <typename T>
  inline std::tuple<Vec2<T>, Vec2<T>, Vec2<T>> split_quadratic(const Vec2<T> p0, const Vec2<T> p1, const Vec2<T> p2, const T t) {
    Vec2<T> p = quadratic(p0, p1, p2, t);

    Vec2<T> q0 = lerp(p0, p1, t);
    Vec2<T> q1 = lerp(p1, p2, t);

    return { p, q0, q1 };
  }

  /**
   * @brief Splits a quadratic bezier curve into two at two given points.
   *
   * @param p0 The first control point.
   * @param p1 The second control point.
   * @param p2 The third control point.
   * @param t1 The first point at which to split the curve.
   * @param t2 The second point at which to split the curve.
   * @return The resulting control points for the two curves.
   */
  template <typename T>
  inline std::tuple<Vec2<T>, Vec2<T>, Vec2<T>, Vec2<T>, Vec2<T>> split_quadratic(const Vec2<T> p0, const Vec2<T> p1, const Vec2<T> p2, const T t1, const T t2) {
    Vec2<T> p01 = quadratic(p0, p1, p2, t1);
    Vec2<T> p02 = quadratic(p0, p1, p2, t2);

    Vec2<T> q1 = lerp(p0, p1, t1);
    Vec2<T> q2 = lerp(p1, p2, t2);

    Vec2<T> r1 = lerp(p1, p2, t1);
    Vec2<T> q = lerp(q1, r1, t2);

    return { q1, p01, q, p02, q2 };
  }

  /**
   * @brief Calculates a hash value for a list of floats.
   *
   * @param floats The list of floats.
   * @return The hash value.
   */
  inline int hash(std::initializer_list<float> floats) {
    int h = 1;

    for (float f : floats) {
      int i = *(int*)(&f);
      h = 31 * h + i;
    }

    h ^= (h >> 20) ^ (h >> 12);
    return h ^ (h >> 7) ^ (h >> 4);
  }

}
