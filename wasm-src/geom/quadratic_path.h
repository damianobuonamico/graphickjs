/**
 * @file geom/quadratic_path.h
 * @brief Contains the definition of the QuadraticPath struct.
 */

#pragma once

#include "../math/rect.h"

#include "../utils/assert.h"

#include <vector>

namespace graphick::geom {

/**
 * @brief A quadratic path is a series of control points that are connected by quadratic curves.
 *
 * The last control point of a curve is the first control point of the next curve.
 * Linear segments are treated as quadratic curves with p1 = p2.
 *
 * All curves are splitted in monotone segments for efficient winding number computation (and
 * rendering).
 */
template<typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
struct QuadraticPath {
  std::vector<math::Vec2<T>> points;  // The control points of the path.

  /**
   * @brief Returns whether the path is empty.
   *
   * A path is considered empty if it has less than 3 control points (i.e. less than 1 curve).
   *
   * @return true if the path is empty, false otherwise.
   */
  inline bool empty() const
  {
    return points.size() < 3;
  }

  /**
   * @brief Returns the number of curves in the path.
   *
   * @return The number of curves in the path.
   */
  inline virtual size_t size() const
  {
    return empty() ? 0 : (points.size() - 1) / 2;
  }

  /**
   * @brief Returns whether the path is closed.
   *
   * A path is considered closed if the first and last control points are the same.
   *
   * @return true if the path is closed, false otherwise.
   */
  inline bool closed() const
  {
    return !empty() && points.front() == points.back();
  }

  /**
   * @brief Returns the first control point of the path.
   *
   * @return The first control point.
   */
  inline math::Vec2<T>& front()
  {
    return points.front();
  }

  /**
   * @brief Returns the last control point of the path.
   *
   * @return The last control point.
   */
  inline math::Vec2<T>& back()
  {
    return points.back();
  }

  /**
   * @brief Returns the i-th control point of the path.
   *
   * @param i The index of the control point.
   * @return The i-th control point.
   */
  inline math::Vec2<T> operator[](const size_t i) const
  {
    return points[i];
  }

  /**
   * @brief Returns an approximate bounding rectangle of the path.
   *
   * The bounding rectangle is computed by taking the minimum and maximum x and y values of the
   * control points.
   *
   * @return An approximate bounding rectangle of the path.
   */
  inline math::Rect<T> approx_bounding_rect() const
  {
    if (empty()) {
      return math::Rect<T>{};
    }

    math::Rect<T> bounds{points[0], points[0]};

    for (size_t i = 1; i < points.size(); i += 2) {
      bounds.min.x = std::min({bounds.min.x, points[i].x, points[i + 1].x});
      bounds.min.y = std::min({bounds.min.y, points[i].y, points[i + 1].y});
      bounds.max.x = std::max({bounds.max.x, points[i].x, points[i + 1].x});
      bounds.max.y = std::max({bounds.max.y, points[i].y, points[i + 1].y});
    }

    return bounds;
  }

  /**
   * @brief Moves the path cursor to the given point.
   *
   * @param p The point to move the cursor to.
   */
  inline virtual void move_to(const math::Vec2<T> p)
  {
    points.push_back(p);
  }

  /**
   * @brief Adds a line to the path.
   *
   * Lineare segments are treated as quadratic curves with p1 = p2.
   *
   * @param p The end point of the line.
   */
  inline void line_to(const math::Vec2<T> p)
  {
    GK_ASSERT(!points.empty(), "Cannot add a curve to an empty path.");

    points.push_back(p);
    points.push_back(p);
  }

  /**
   * @brief Adds a quadratic bezier curve to the path.
   *
   * @param p1 The first control point of the curve.
   * @param p2 The end point of the curve.
   */
  void quadratic_to(const math::Vec2<T> p1, const math::Vec2<T> p2);

  /**
   * @brief Adds a cubic bezier curve to the path.
   *
   * @param p1 The first control point of the curve.
   * @param p2 The second control point of the curve.
   * @param p3 The end point of the curve.
   */
  void cubic_to(const math::Vec2<T> p1,
                const math::Vec2<T> p2,
                const math::Vec2<T> p3,
                const T tolerance = T(2e-2));

  /**
   * @brief Adds an arc to the path.
   *
   * @param center The center of the arc.
   * @param to The end point of the arc.
   * @param clockwise Whether the arc is drawn clockwise or counter-clockwise.
   */
  void arc_to(const math::Vec2<T> center,
              const math::Vec2<T> to,
              const bool clockwise = true,
              const T tolerance = T(1e-2));

  /**
   * @brief Returns the winding number of a point with respect to the path.
   *
   * @param p The point to compute the winding number for.
   * @return The winding number of the point.
   */
  int winding_of(const math::Vec2<T> p) const;
};

template<typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
struct QuadraticMultipath : public QuadraticPath<T> {
  std::vector<size_t> starts;  // The starting indices of the paths.

  /**
   * @brief Returns the number of curves in the path.
   *
   * @return The number of curves in the path.
   */
  inline size_t size() const override
  {
    return this->empty() ? 0 : (this->points.size() - starts.size()) / 2;
  }

  /**
   * @brief Moves the path cursor to the given point.
   *
   * Adds a new path to the multipath (i.e. a new starting index).
   *
   * @param p The point to move the cursor to.
   */
  void move_to(const math::Vec2<T> p) override
  {
    starts.push_back(this->points.size());
    this->points.push_back(p);
  }
};

}  // namespace graphick::geom

/* -- Aliases -- */

namespace graphick::geom {

using quadratic_path = QuadraticPath<float>;
using dquadratic_path = QuadraticPath<double>;

using quadratic_multipath = QuadraticMultipath<float>;
using dquadratic_multipath = QuadraticMultipath<double>;

}  // namespace graphick::geom
