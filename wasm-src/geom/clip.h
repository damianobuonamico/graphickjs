/**
 * @file clip.h
 * @brief This file contains the declaration of methods for polygon clipping.
 */

#pragma once

#include "cubic_path.h"
#include "curve_ops.h"

#include "../math/rect.h"
#include "../math/vector.h"

namespace graphick::geom {

/**
 * @brief Calculates the x-coordinate of the intersection point between two lines defined by their
 * endpoints.
 *
 * If one of the lines is horizontal, use the x_intersect_horizontal() function.
 *
 * @param x1, y1, x2, y2 The coordinates of the first line.
 * @param x3, y3, x4, y4 The coordinates of the second line.
 * @return The x-coordinate of the intersection point.
 */
template<typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
inline T x_intersect(
    const T x1, const T y1, const T x2, const T y2, const T x3, const T y3, const T x4, const T y4)
{
  const T num = (x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4);
  const T den = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);

  return num / den;
}

/**
 * @brief Calculates the y-coordinate of the intersection point between two lines defined by their
 * endpoints.
 *
 * If one of the lines is vertical, use the y_intersect_vertical() function.
 *
 * @param x1, y1, x2, y2 The coordinates of the first line.
 * @param x3, y3, x4, y4 The coordinates of the second line.
 * @return The y-coordinate of the intersection point.
 */
template<typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
inline T y_intersect(
    const T x1, const T y1, const T x2, const T y2, const T x3, const T y3, const T x4, const T y4)
{
  const T num = (x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4);
  const T den = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);

  return num / den;
}

/**
 * @brief Calculates the x-coordinate of the intersection point between two lines.
 *
 * Uses int64_t as intermediate type to avoid overflow.
 *
 * @param y The y-coordinate of the horizontal line
 * @param x1, y1, x2, y2 The coordinates of the second line.
 * @return The x-coordinate of the intersection point.
 */
template<typename T>
inline T x_intersect_horizontal(const T y, const T x1, const T y1, const T x2, const T y2)
{
  const int64_t num = static_cast<int64_t>(x1) * static_cast<int64_t>(y2) -
                      static_cast<int64_t>(y1) * static_cast<int64_t>(x2) -
                      static_cast<int64_t>(y) * static_cast<int64_t>(x1 - x2);
  const int64_t den = static_cast<int64_t>(y2 - y1);

  return static_cast<T>(num / den);
}

/**
 * @brief Calculates the x-coordinate of the intersection point between two lines.
 *
 * If the first line is not horizontal, use the x_intersect() function.
 *
 * @param y The y-coordinate of the horizontal line
 * @param x1, y1, x2, y2 The coordinates of the second line.
 * @return The x-coordinate of the intersection point.
 */
template<typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
inline T x_intersect_horizontal(const T y, const T x1, const T y1, const T x2, const T y2)
{
  const T num = x1 * y2 - y1 * x2 - y * (x1 - x2);
  const T den = y2 - y1;

  return num / den;
}

/**
 * @brief Calculates the y-coordinate of the intersection point between two lines.
 *
 * Uses int64_t as intermediate type to avoid overflow.
 *
 * @param x The x-coordinate of the vertical line
 * @param x1, y1, x2, y2 The coordinates of the second line.
 * @return The y-coordinate of the intersection point.
 */
template<typename T>
inline T y_intersect_vertical(const T x, const T x1, const T y1, const T x2, const T y2)
{
  const int64_t num = static_cast<int64_t>(x1) * static_cast<int64_t>(y2) -
                      static_cast<int64_t>(y1) * static_cast<int64_t>(x2) +
                      static_cast<int64_t>(x) * static_cast<int64_t>(y1 - y2);
  const int64_t den = static_cast<int64_t>(x1 - x2);

  return static_cast<T>(num / den);
}

/**
 * @brief Calculates the y-coordinate of the intersection point between two lines.
 *
 * If the first line is not vertical, use the y_intersect() function.
 *
 * @param x The x-coordinate of the vertical line
 * @param x1, y1, x2, y2 The coordinates of the second line.
 * @return The y-coordinate of the intersection point.
 */
template<typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
inline T y_intersect_vertical(const T x, const T x1, const T y1, const T x2, const T y2)
{
  const T num = x1 * y2 - y1 * x2 + x * (y1 - y2);
  const T den = x1 - x2;

  return num / den;
}

/**
 * @brief Clips a polygon to the right of a given x value.
 *
 * Modifies the input points vector.
 *
 * @param points The points of the polygon.
 * @param x The x value to clip to.
 */
template<typename T>
inline void clip_to_left(std::vector<math::Vec2<T>>& points, const T x)
{
  if (points.empty())
    return;

  std::vector<math::Vec2<T>> new_points;

  for (size_t i = 0; i < points.size() - 1; i++) {
    const math::Vec2<T> point = points[i];

    if (point.x < x) {
      if (points[i + 1].x > x) {
        new_points.push_back(
            {x, y_intersect_vertical(x, point.x, point.y, points[i + 1].x, points[i + 1].y)});
      }
    } else {
      new_points.push_back(point);

      if (points[i + 1].x < x) {
        new_points.push_back(
            {x, y_intersect_vertical(x, point.x, point.y, points[i + 1].x, points[i + 1].y)});
      }
    }
  }

  if (new_points.size() > 2 && new_points.front() != new_points.back()) {
    new_points.push_back(new_points.front());
  }

  points = new_points;
}

/**
 * @brief Clips a polygon to the left of a given x value.
 *
 * Modifies the input points vector.
 *
 * @param points The points of the polygon.
 * @param x The x value to clip to.
 */
template<typename T>
inline void clip_to_right(std::vector<math::Vec2<T>>& points, const T x)
{
  if (points.empty())
    return;

  std::vector<math::Vec2<T>> new_points;

  for (size_t i = 0; i < points.size() - 1; i++) {
    const math::Vec2<T> point = points[i];

    if (point.x > x) {
      if (points[i + 1].x < x) {
        new_points.push_back(
            {x, y_intersect_vertical(x, point.x, point.y, points[i + 1].x, points[i + 1].y)});
      }
    } else {
      new_points.push_back(point);

      if (points[i + 1].x > x) {
        new_points.push_back(
            {x, y_intersect_vertical(x, point.x, point.y, points[i + 1].x, points[i + 1].y)});
      }
    }
  }

  if (new_points.size() > 2 && new_points.front() != new_points.back()) {
    new_points.push_back(new_points.front());
  }

  points = new_points;
}

/**
 * @brief Clips a polygon to the bottom of a given y value.
 *
 * Modifies the input points vector.
 *
 * @param points The points of the polygon.
 * @param y The y value to clip to.
 */
template<typename T>
inline void clip_to_top(std::vector<math::Vec2<T>>& points, const T y)
{
  if (points.empty())
    return;

  std::vector<math::Vec2<T>> new_points;

  for (size_t i = 0; i < points.size() - 1; i++) {
    const math::Vec2<T> point = points[i];

    if (point.y < y) {
      if (points[i + 1].y > y) {
        new_points.push_back(
            {x_intersect_horizontal(y, point.x, point.y, points[i + 1].x, points[i + 1].y), y});
      }
    } else {
      new_points.push_back(point);

      if (points[i + 1].y < y) {
        new_points.push_back(
            {x_intersect_horizontal(y, point.x, point.y, points[i + 1].x, points[i + 1].y), y});
      }
    }
  }

  if (new_points.size() > 2 && new_points.front() != new_points.back()) {
    new_points.push_back(new_points.front());
  }

  points = new_points;
}

/**
 * @brief Clips a polygon to the top of a given y value.
 *
 * Modifies the input points vector.
 *
 * @param points The points of the polygon.
 * @param y The y value to clip to.
 */
template<typename T>
inline void clip_to_bottom(std::vector<math::Vec2<T>>& points, const T y)
{
  if (points.empty())
    return;

  std::vector<math::Vec2<T>> new_points;

  for (size_t i = 0; i < points.size() - 1; i++) {
    const math::Vec2<T> point = points[i];

    if (point.y > y) {
      if (points[i + 1].y < y) {
        new_points.push_back(
            {x_intersect_horizontal(y, point.x, point.y, points[i + 1].x, points[i + 1].y), y});
      }
    } else {
      new_points.push_back(point);

      if (points[i + 1].y > y) {
        new_points.push_back(
            {x_intersect_horizontal(y, point.x, point.y, points[i + 1].x, points[i + 1].y), y});
      }
    }
  }

  if (new_points.size() > 2 && new_points.front() != new_points.back()) {
    new_points.push_back(new_points.front());
  }

  points = new_points;
}

/**
 * @brief Clips a polygon to a given rect.
 *
 * Modifies the input points vector.
 *
 * @param points The points of the polygon.
 * @param rect The rect to clip to.
 */
template<typename T>
inline void clip(std::vector<math::Vec2<T>>& points, const math::Rect<T>& rect)
{
  clip_to_left(points, rect.min.x);
  clip_to_right(points, rect.max.x);
  clip_to_top(points, rect.min.y);
  clip_to_bottom(points, rect.max.y);
}

/**
 * @brief Clips a cubic path to the right of a given x value.
 *
 * Modifies the input path.
 *
 * @param path The cubic path to clip.
 * @param x The x value to clip to.
 */
template<typename T>
inline void clip_to_left(CubicPath<T>& path, const T x)
{
  if (path.empty()) {
    return;
  }

  CubicPath<T> new_path;
  new_path.points.reserve(path.points.size());

  for (size_t i = 0; i < path.size(); i++) {
    /* Curves are monotonic, only p0 and p3 matter. */

    const math::Vec2<T> p0 = path[i * 3];
    const math::Vec2<T> p1 = path[i * 3 + 1];
    const math::Vec2<T> p2 = path[i * 3 + 2];
    const math::Vec2<T> p3 = path[i * 3 + 3];

    const CubicBezier<T> curve = CubicBezier<T>(p0, p1, p2, p3);

    const bool linear = curve.is_line(math::geometric_epsilon<T>);

    if (p0.x < x) {
      if (p3.x > x) {
        const double t0 = math::clamp((x - p0.x) / (p3.x - p0.x), 0.0, 1.0);

        if (linear) {
          new_path.line_to({x, math::lerp(p0.y, p3.y, t0)});
          new_path.line_to(p3);
        } else {
          const auto& [a, b, c, d] = curve.coefficients();
          const double t = math::is_almost_zero_or_one(t0) ?
                               t0 :
                               geom::cubic_line_intersect_approx(a.x, b.x, c.x, d.x, x, t0);

          const CubicBezier<T> new_curve = extract(curve, t, 1.0);

          new_path.line_to({x, new_curve.p0.y});
          new_path.cubic_to_monotonic(new_curve.p1, new_curve.p2, new_curve.p3);
        }
      }
    } else {
      if (new_path.empty() || new_path.back() != p0) {
        new_path.line_to(p0);
      }

      if (p3.x < x) {
        const double t0 = math::clamp((x - p0.x) / (p3.x - p0.x), 0.0, 1.0);

        if (linear) {
          new_path.line_to({x, math::lerp(p0.y, p3.y, t0)});
        } else {
          const auto& [a, b, c, d] = curve.coefficients();
          const double t = math::is_almost_zero_or_one(t0) ?
                               t0 :
                               geom::cubic_line_intersect_approx(a.x, b.x, c.x, d.x, x, t0);

          const CubicBezier<T> new_curve = extract(curve, 0.0, t);

          new_path.cubic_to_monotonic(new_curve.p1, new_curve.p2, new_curve.p3);
        }
      } else {
        new_path.cubic_to_monotonic(p1, p2, p3);
      }
    }
  }

  if (!new_path.empty() && new_path.front() != new_path.back()) {
    new_path.line_to(new_path.front());
  }

  path = new_path;
}

/**
 * @brief Clips a cubic path to the left of a given x value.
 *
 * Modifies the input path.
 *
 * @param path The cubic path to clip.
 * @param x The x value to clip to.
 */
template<typename T>
inline void clip_to_right(CubicPath<T>& path, const T x)
{
  if (path.empty()) {
    return;
  }

  CubicPath<T> new_path;
  new_path.points.reserve(path.points.size());

  for (size_t i = 0; i < path.size(); i++) {
    /* Curves are monotonic, only p0 and p3 matter. */

    const math::Vec2<T> p0 = path[i * 3];
    const math::Vec2<T> p1 = path[i * 3 + 1];
    const math::Vec2<T> p2 = path[i * 3 + 2];
    const math::Vec2<T> p3 = path[i * 3 + 3];

    const CubicBezier<T> curve = CubicBezier<T>(p0, p1, p2, p3);

    const bool linear = curve.is_line(math::geometric_epsilon<T>);

    if (p0.x > x) {
      if (p3.x < x) {
        const double t0 = math::clamp((x - p0.x) / (p3.x - p0.x), 0.0, 1.0);

        if (linear) {
          new_path.line_to({x, math::lerp(p0.y, p3.y, t0)});
          new_path.line_to(p3);
        } else {
          const auto& [a, b, c, d] = curve.coefficients();
          const double t = math::is_almost_zero_or_one(t0) ?
                               t0 :
                               geom::cubic_line_intersect_approx(a.x, b.x, c.x, d.x, x, t0);

          const CubicBezier<T> new_curve = extract(curve, t, 1.0);

          new_path.line_to({x, new_curve.p0.y});
          new_path.cubic_to_monotonic(new_curve.p1, new_curve.p2, new_curve.p3);
        }
      }
    } else {
      if (new_path.empty() || new_path.back() != p0) {
        new_path.line_to(p0);
      }

      if (p3.x > x) {
        const double t0 = math::clamp((x - p0.x) / (p3.x - p0.x), 0.0, 1.0);

        if (linear) {
          new_path.line_to({x, math::lerp(p0.y, p3.y, t0)});
        } else {
          const auto& [a, b, c, d] = curve.coefficients();
          const double t = math::is_almost_zero_or_one(t0) ?
                               t0 :
                               geom::cubic_line_intersect_approx(a.x, b.x, c.x, d.x, x, t0);

          const CubicBezier<T> new_curve = extract(curve, 0.0, t);

          new_path.cubic_to_monotonic(new_curve.p1, new_curve.p2, new_curve.p3);
        }
      } else {
        new_path.cubic_to_monotonic(p1, p2, p3);
      }
    }
  }

  if (!new_path.empty() && new_path.front() != new_path.back()) {
    new_path.line_to(new_path.front());
  }

  path = new_path;
}

/**
 * @brief Clips a cubic path to the bottom of a given y value.
 *
 * Modifies the input path.
 *
 * @param path The cubic path to clip.
 * @param y The y value to clip to.
 */
template<typename T>
inline void clip_to_top(CubicPath<T>& path, const T y)
{
  if (path.empty()) {
    return;
  }

  CubicPath<T> new_path;
  new_path.points.reserve(path.points.size());

  for (size_t i = 0; i < path.size(); i++) {
    /* Curves are monotonic, only p0 and p3 matter. */

    const math::Vec2<T> p0 = path[i * 3];
    const math::Vec2<T> p1 = path[i * 3 + 1];
    const math::Vec2<T> p2 = path[i * 3 + 2];
    const math::Vec2<T> p3 = path[i * 3 + 3];

    const CubicBezier<T> curve = CubicBezier<T>(p0, p1, p2, p3);

    const bool linear = curve.is_line(math::geometric_epsilon<T>);

    if (p0.y < y) {
      if (p3.y > y) {
        const double t0 = math::clamp((y - p0.y) / (p3.y - p0.y), 0.0, 1.0);

        if (linear) {
          new_path.line_to({math::lerp(p0.x, p3.x, t0), y});
          new_path.line_to(p3);
        } else {
          const auto& [a, b, c, d] = curve.coefficients();
          const double t = math::is_almost_zero_or_one(t0) ?
                               t0 :
                               geom::cubic_line_intersect_approx(a.y, b.y, c.y, d.y, y, t0);

          const CubicBezier<T> new_curve = extract(curve, t, 1.0);

          new_path.line_to({new_curve.p0.x, y});
          new_path.cubic_to_monotonic(new_curve.p1, new_curve.p2, new_curve.p3);
        }
      }
    } else {
      if (new_path.empty() || new_path.back() != p0) {
        new_path.line_to(p0);
      }

      if (p3.y < y) {
        const double t0 = math::clamp((y - p0.y) / (p3.y - p0.y), 0.0, 1.0);

        if (linear) {
          new_path.line_to({math::lerp(p0.x, p3.x, t0), y});
        } else {
          const auto& [a, b, c, d] = curve.coefficients();
          const double t = math::is_almost_zero_or_one(t0) ?
                               t0 :
                               geom::cubic_line_intersect_approx(a.y, b.y, c.y, d.y, y, t0);

          const CubicBezier<T> new_curve = extract(curve, 0.0, t);

          new_path.cubic_to_monotonic(new_curve.p1, new_curve.p2, new_curve.p3);
        }
      } else {
        new_path.cubic_to_monotonic(p1, p2, p3);
      }
    }
  }

  if (!new_path.empty() && new_path.front() != new_path.back()) {
    new_path.line_to(new_path.front());
  }

  path = new_path;
}

/**
 * @brief Clips a cubic path to the top of a given y value.
 *
 * Modifies the input path.
 *
 * @param path The cubic path to clip.
 * @param y The y value to clip to.
 */
template<typename T>
inline void clip_to_bottom(CubicPath<T>& path, const T y)
{
  if (path.empty()) {
    return;
  }

  CubicPath<T> new_path;
  new_path.points.reserve(path.points.size());

  for (size_t i = 0; i < path.size(); i++) {
    /* Curves are monotonic, only p0 and p3 matter. */

    const math::Vec2<T> p0 = path[i * 3];
    const math::Vec2<T> p1 = path[i * 3 + 1];
    const math::Vec2<T> p2 = path[i * 3 + 2];
    const math::Vec2<T> p3 = path[i * 3 + 3];

    const CubicBezier<T> curve = CubicBezier<T>(p0, p1, p2, p3);

    const bool linear = curve.is_line(math::geometric_epsilon<T>);

    if (p0.y > y) {
      if (p3.y < y) {
        const double t0 = math::clamp((y - p0.y) / (p3.y - p0.y), 0.0, 1.0);

        if (linear) {
          new_path.line_to({math::lerp(p0.x, p3.x, t0), y});
          new_path.line_to(p3);
        } else {
          const auto& [a, b, c, d] = curve.coefficients();
          const double t = math::is_almost_zero_or_one(t0) ?
                               t0 :
                               geom::cubic_line_intersect_approx(a.y, b.y, c.y, d.y, y, t0);

          const CubicBezier<T> new_curve = extract(curve, t, 1.0);

          new_path.line_to({new_curve.p0.x, y});
          new_path.cubic_to_monotonic(new_curve.p1, new_curve.p2, new_curve.p3);
        }
      }
    } else {
      if (new_path.empty() || new_path.back() != p0) {
        new_path.line_to(p0);
      }

      if (p3.y > y) {
        const double t0 = math::clamp((y - p0.y) / (p3.y - p0.y), 0.0, 1.0);

        if (linear) {
          new_path.line_to({math::lerp(p0.x, p3.x, t0), y});
        } else {
          const auto& [a, b, c, d] = curve.coefficients();
          const double t = math::is_almost_zero_or_one(t0) ?
                               t0 :
                               geom::cubic_line_intersect_approx(a.y, b.y, c.y, d.y, y, t0);

          const CubicBezier<T> new_curve = extract(curve, 0.0, t);

          new_path.cubic_to_monotonic(new_curve.p1, new_curve.p2, new_curve.p3);
        }
      } else {
        new_path.cubic_to_monotonic(p1, p2, p3);
      }
    }
  }

  if (!new_path.empty() && new_path.front() != new_path.back()) {
    new_path.line_to(new_path.front());
  }

  path = new_path;
}

/**
 * @brief Clips a cubic path to a given rect.
 *
 * Modifies the input path.
 *
 * @param path The cubic path to clip.
 * @param rect The rect to clip to.
 */
template<typename T>
inline void clip(CubicPath<T>& path, const math::Rect<T>& rect)
{
  clip_to_left(path, rect.min.x);
  clip_to_right(path, rect.max.x);
  clip_to_top(path, rect.min.y);
  clip_to_bottom(path, rect.max.y);
}

}  // namespace graphick::geom
