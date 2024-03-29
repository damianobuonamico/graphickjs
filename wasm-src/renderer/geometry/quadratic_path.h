/**
 * @file quadratic_path.h
 * @brief Contains the definition of the QuadraticPath struct.
 *
 * @todo when new renderer is implemented, remove old one and unify namespaces with lower case convention.
 */

#pragma once

#include "../../math/vec2.h"
#include "../../math/rect.h"

#include <vector>

namespace Graphick::renderer::geometry {

  /**
   * @brief A quadratic path is a series of control points that are connected by quadratic curves.
   *
   * The last control point of a curve is the first control point of the next curve.
   * Linear segments are treated as quadratic curves with p1 = p2.
   *
   * @struct QuadraticPath
   */
  struct QuadraticPath {
    std::vector<vec2> points;    /* The control points of the path. */

    /**
     * @brief Returns whether the path is empty.
     *
     * A path is considered empty if it has less than 3 control points (i.e. less than 1 curve).
     *
     * @return true if the path is empty, false otherwise.
     */
    inline bool empty() const {
      return points.size() < 3;
    }

    /**
     * @brief Returns the number of curves in the path.
     *
     * @return The number of curves in the path.
    */
    inline size_t size() const {
      return empty() ? 0 : (points.size() - 1) / 2;
    }

    /**
     * @brief Returns whether the path is closed.
     *
     * A path is considered closed if the first and last control points are the same.
     *
     * @return true if the path is closed, false otherwise.
     */
    inline bool closed() const {
      return !empty() && points.front() == points.back();
    }

    /**
     * @brief Returns the i-th control point of the path.
     *
     * @param i The index of the control point.
     * @return The i-th control point.
     */
    inline vec2 operator[](const size_t i) const {
      return points[i];
    }

    inline rect approx_bounding_rect() const {
      if (empty()) {
        return rect{};
      }

      rect bounds{ points[0], points[0] };

      for (size_t i = 1; i < points.size(); i += 2) {
        bounds.min.x = std::min({ bounds.min.x, points[i].x, points[i + 1].x });
        bounds.min.y = std::min({ bounds.min.y, points[i].y, points[i + 1].y });
        bounds.max.x = std::max({ bounds.max.x, points[i].x, points[i + 1].x });
        bounds.max.y = std::max({ bounds.max.y, points[i].y, points[i + 1].y });
      }

      return bounds;
    }

    /**
     * @brief Moves the path cursor to the given point.
     *
     * @param p The point to move the cursor to.
     */
    inline void move_to(const vec2 p) {
      points.push_back(p);
    }

    /**
     * @brief Adds a line to the path.
     *
     * Lineare segments are treated as quadratic curves with p1 = p2.
     *
     * @param p The end point of the line.
     */
    inline void line_to(const vec2 p) {
      points.push_back(p);
      points.push_back(p);
    }

    /**
     * @brief Adds a quadratic bezier curve to the path.
     *
     * @param p1 The first control point of the curve.
     * @param p2 The end point of the curve.
     */
    inline void quadratic_to(const vec2 p1, const vec2 p2) {
      points.push_back(p1);
      points.push_back(p2);
    }
  };

}
