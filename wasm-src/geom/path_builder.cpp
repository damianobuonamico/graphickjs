/**
 * @file geom/path_builder.h
 * @brief Containes the PathBuilder class definition and its options.
 */

#include "path_builder.h"

#include "quadratic_bezier.h"
#include "quadratic_path.h"
#include "intersections.h"

#include "../math/matrix.h"

namespace graphick::geom {

  /**
   * @brief Flattens a quadratic bezier curve and outputs the line segments to a sink vector.
   *
   * This method uses a fast flattening algorithm that will create lots of extra lines if the curve is too large.
   *
   * @tparam T The type of the output coordinates.
   * @param quad The quadratic bezier curve to flatten.
   * @param tolerance The tolerance to use when flattening the curve.
   * @param sink_callback The callback to output the lines to.
   */
  template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
  static void fast_flatten(
    const dquadratic_bezier& quad,
    const double tolerance,
    std::function<void(const math::Vec2<T>, const math::Vec2<T>)> sink_callback
  ) {
    const auto& [a, b, c] = quad.coefficients();
    const double dt = std::sqrt((2.0 * tolerance) / math::length(quad.p0 - 2.0 * quad.p1 + quad.p2));

    dvec2 last = quad.p0;
    double t = dt;

    while (t < 1.0) {
      const double t_sq = t * t;
      const dvec2 p = a * t_sq + b * t + c;

      sink_callback(math::Vec2<T>(last), math::Vec2<T>(p));

      last = p;
      t += dt;
    }

    sink_callback(math::Vec2<T>(last), math::Vec2<T>(quad.p2));
  }

  /**
   * @brief Recursively flattens a quadratic bezier curve and outputs the line segments to a sink vector.
   *
   * This method uses a recursive flattening algorithm that will create less extra lines than the fast flattening algorithm.
   *
   * @tparam T The type of the output coordinates.
   * @param quad The quadratic bezier curve to flatten.
   * @param clip The rectangle to clip the curve to.
   * @param tolerance_sq The squared tolerance to use when flattening the curve.
   * @param sink_callback The callback to output the lines to.
   * @param depth The current recursion depth.
   */
  template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
  static void recursive_flatten(
    const dquadratic_bezier& quad,
    const drect& clip,
    const double tolerance_sq,
    std::function<void(const math::Vec2<T>, const math::Vec2<T>)> sink_callback,
    uint8_t depth = 0
  ) {
    if (depth > math::max_recursion_depth<uint8_t>) {
      sink_callback(math::Vec2<T>(quad.p0), math::Vec2<T>(quad.p2));
      return;
    }

    const drect bounds = quad.approx_bounding_rect();

    if (!does_rect_intersect_rect(bounds, clip)) {
      return;
    }

    depth += 1;

    const dvec2 p01 = (quad.p0 + quad.p1) * 0.5;
    const dvec2 p12 = (quad.p1 + quad.p2) * 0.5;
    const dvec2 p012 = (p01 + p12) * 0.5;

    const double den = math::squared_distance(quad.p0, quad.p2);
    const double num = std::abs(
      (quad.p2.x - quad.p0.x) * (quad.p0.y - p012.y) -
      (quad.p0.x - p012.x) * (quad.p2.y - quad.p0.y)
    );

    const double sq_error = num * num / den;

    if (sq_error < tolerance_sq) {
      sink_callback(math::Vec2<T>(quad.p0), math::Vec2<T>(quad.p2));
      return;
    }

    recursive_flatten(dquadratic_bezier{ quad.p0, p01, p012 }, clip, tolerance_sq, sink_callback, depth);
    recursive_flatten(dquadratic_bezier{ p012, p12, quad.p2 }, clip, tolerance_sq, sink_callback, depth);
  }

  template <typename T, typename _>
  PathBuilder<T, _>::PathBuilder(
    const QuadraticPath<T, _>& path,
    const math::Mat2x3<T>& transform,
    const math::Rect<T>* bounding_rect
  ) :
    m_path(path),
    m_transform(dmat2x3(transform))
  {
    drect bounds = bounding_rect ? drect(*bounding_rect) : drect(path.approx_bounding_rect());
    m_bounding_rect = m_transform * bounds;
  }

  template <typename T, typename _>
  void PathBuilder<T, _>::flatten(
    const math::Rect<T>& clip,
    const T tolerance,
    std::function<void(const math::Vec2<T>, const math::Vec2<T>)> sink_callback
  ) const {
    if (m_path.empty()) {
      return;
    }

    const drect clipping_rect = drect(clip);
    const double coverage = rect_rect_intersection_area(m_bounding_rect, clipping_rect) / m_bounding_rect.area();

    if (coverage <= 0.0) {
      return;
    } else if (coverage <= 0.5) {
      flatten_clipped(drect(clip), static_cast<double>(tolerance) * static_cast<double>(tolerance), sink_callback);
    } else {
      flatten_unclipped(static_cast<double>(tolerance), sink_callback);
    }
  }

  template <typename T, typename _>
  void PathBuilder<T, _>::flatten_clipped(
    const drect& clip,
    const double tolerance_sq,
    std::function<void(const math::Vec2<T>, const math::Vec2<T>)> sink_callback
  ) const {
    dvec2 p0 = m_transform * dvec2(m_path[0]);

    for (size_t i = 0; i < m_path.size(); i++) {
      const dvec2 p1 = m_transform * dvec2(m_path[i * 2 + 1]);
      const dvec2 p2 = m_transform * dvec2(m_path[i * 2 + 2]);

      if (math::is_almost_equal(p1, p2)) {
        sink_callback(math::Vec2<T>(p0), math::Vec2<T>(p2));
      } else {
        recursive_flatten(dquadratic_bezier{ p0, p1, p2 }, clip, tolerance_sq, sink_callback);
      }

      p0 = p2;
    }
  }

  template <typename T, typename _>
  void PathBuilder<T, _>::flatten_unclipped(
    const double tolerance,
    std::function<void(const math::Vec2<T>, const math::Vec2<T>)> sink_callback
  ) const {
    dvec2 p0 = m_transform * dvec2(m_path[0]);

    for (size_t i = 0; i < m_path.size(); i++) {
      const dvec2 p1 = m_transform * dvec2(m_path[i * 2 + 1]);
      const dvec2 p2 = m_transform * dvec2(m_path[i * 2 + 2]);

      if (p1 == p2) {
        sink_callback(math::Vec2<T>(p0), math::Vec2<T>(p2));
      } else {
        fast_flatten(dquadratic_bezier{ p0, p1, p2 }, tolerance, sink_callback);
      }

      p0 = p2;
    }
  }

  /* -- Template instantiations -- */

  template class PathBuilder<float>;
  template class PathBuilder<double>;

}
