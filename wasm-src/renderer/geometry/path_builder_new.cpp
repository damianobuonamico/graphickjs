/**
 * @file path_builder.cpp
 * @brief Contains the PathBuilder class implementation.
 *
 * @todo fix flickering at super high zoom levels
 */

#include "path_builder_new.h"

#include "../../geom/path.h"
#include "../../geom/curve_ops.h"
#include "../../geom/intersections.h"

#include "../properties.h"

#include "../../math/matrix.h"
#include "../../math/vector.h"
#include "../../math/math.h"

#include "../../utils/defines.h"
#include "../../utils/console.h"

namespace graphick::renderer::geometry {

  /* -- Static -- */

  /**
   * @brief Approximates a circular arc with a series of quadratic bezier curves.
   *
   * @param center The center of the arc.
   * @param from The start point of the arc.
   * @param to The end point of the arc.
   * @param radius The radius of the arc.
   * @param tolerance The error tolerance.
   * @param sink The output path.
   * @param reverse Whether to fill the arc angle or not
   */
  static void quadratic_arc(const dvec2 center, const dvec2 from, const dvec2 to, const double radius, const double tolerance, geom::QuadraticPath& sink, const bool reverse = false) {
    const double ang1 = std::atan2(from.y - center.y, from.x - center.x);
    const double ang2 = std::atan2(to.y - center.y, to.x - center.x);
    const double dphi = 4 * std::acos(std::sqrt(2 + tolerance - std::sqrt(tolerance * (2.0f + tolerance))) / std::sqrt(2.0f));

    double diff = std::abs(ang2 - ang1);

    if (diff > math::pi<double>) diff = math::two_pi<double> -diff;
    if (reverse) diff = -diff;

    const double diff_abs = std::abs(diff);

    const int segments = static_cast<int>(std::ceil(diff_abs / dphi));
    const double inc = diff / segments;
    const double b = (std::cos(inc) - 1.0f) / std::sin(inc);

    for (int i = 0; i <= segments; i++) {
      const double angle = ang1 + i * inc;
      const double sin = std::sin(angle);
      const double cos = std::cos(angle);

      const dvec2 p1 = center + radius * dvec2{ cos - b * sin, sin + b * cos };
      const dvec2 p2 = center + radius * dvec2{ cos, sin };

      sink.quadratic_to(vec2(p1), vec2(p2));
    }
  }

  /**
   * @brief Adds the given cap to the contour.
   *
   * @param from The start point of the cap.
   * @param to The end point of the cap.
   * @param n The normal of the cap.
   * @param radius The radius of the cap.
   * @param cap The cap type.
   * @param sink The contour to add the cap to.
   */
  void add_cap(const dvec2 from, const dvec2 to, const dvec2 n, const double radius, const LineCap cap, geom::QuadraticPath& sink) {
    switch (cap) {
    case LineCap::Round: {
      // TODO: angular tolerance
      quadratic_arc(from + (to - from) / 2.0, from, to, radius, 0.01, sink);
      break;
    }
    case LineCap::Square: {
      dvec2 dir = { -n.y * radius, n.x * radius };

      sink.line_to(vec2(from + dir));
      sink.line_to(vec2(to + dir));
      sink.line_to(vec2(to));

      break;
    }
    default:
    case LineCap::Butt: {
      sink.line_to(vec2(to));
      break;
    }
    }
  }

  /**
   * @brief Adds the given join to the contour.
   *
   * @param from The start point of the join.
   * @param to The end point of the join.
   * @param pivot The pivot point of the join.
   * @param from_normal The normal of the start point.
   * @param to_normal The normal of the end point.
   * @param radius The radius of the join.
   * @param inv_miter_limit The inverse miter limit.
   * @param join The join type.
   * @param sink The contour to add the join to.
   * @param reverse Whether to consider the complementary angle.
   */
  static void add_join(const dvec2 from, const dvec2 to, const dvec2 pivot, const dvec2 from_normal, const dvec2 to_normal, const double radius, const double inv_miter_limit, LineJoin join, geom::QuadraticPath& sink, const bool reverse = false) {
    if (math::is_almost_equal(from, to, math::geometric_epsilon<double>)) {
      return;
    }

    const dvec2 a = from - pivot;
    const dvec2 b = to - pivot;

    double dot = a.x * b.x + a.y * b.y;
    double cross = a.x * b.y - a.y * b.x;

    if (reverse) cross = -cross;

    double ang = std::atan2(cross, dot);

    if (ang < 0.0) ang += math::two_pi<double>;
    if (ang >= math::pi<double>) join = LineJoin::Bevel;

    if (math::is_almost_zero(ang)) {
      return;
    }

    switch (join) {
    case LineJoin::Round: {
      // TODO: tolerance
      quadratic_arc(pivot, from, to, radius, 0.01, sink, reverse);
      break;
    }
    case LineJoin::Miter: {
      double dot = from_normal.x * to_normal.x + from_normal.y * to_normal.y;
      double sin_half = std::sqrt((1.0 + dot) * 0.5);

      if (sin_half < inv_miter_limit) {
        sink.line_to(vec2(to));
        break;
      } else {
        dvec2 mid = from_normal + to_normal;
        double l = radius / (sin_half * std::hypot(mid.x, mid.y));
        dvec2 p = pivot + mid * l;

        sink.line_to(vec2(p));
        sink.line_to(vec2(to));
        break;
      }
    }
    default:
    case LineJoin::Bevel: {
      sink.line_to(vec2(to));
      break;
    }
    }
  }

  /**
   * @brief Offsets a line segment.
   *
   * @param p0 The start point of the line.
   * @param p1 The end point of the line.
   * @param radius The radius to offset the line by.
   * @param sink The output path.
   * @return The normal of the end point.
   */
  static dvec2 offset_line(const dvec2 p0, const dvec2 p1, const double radius, PathBuilder::StrokeOutline& sink) {
    const dvec2 n = math::normal(p0, p1);
    const dvec2 nr = n * radius;

    sink.inner.line_to(vec2(p1 - nr));
    sink.outer.line_to(vec2(p1 + nr));

    return n;
  }

  /**
   * @brief Offsets a monotonic quadratic bezier curve.
   *
   * @param p0 The start point of the curve.
   * @param p1 The control point of the curve.
   * @param p2 The end point of the curve.
   * @param radius The radius to offset the curve by.
   * @param tolerance The offset error tolerance.
   * @param sink The output path.
   * @return The normal of the end point.
   */
  static dvec2 offset_monotonic_quadratic(dvec2 p0, dvec2 p1, dvec2 p2, const double radius, const double tolerance, PathBuilder::StrokeOutline& sink) {
    dvec2 start_n = math::normal(p0, p1);

    while (true) {
      const dvec2 a = 2.0 * (p0 - 2.0 * p1 + p2);
      const dvec2 b = 2.0 * (p1 - p0);

      const double aob = math::dot(a, b);
      const double axb = math::cross(a, b);

      if (aob == 0.0) {
        break;
      }

      double t = tolerance * math::squared_length(b) / (std::abs(axb) - tolerance * aob);

      if (!(t > math::geometric_epsilon<double> && t < 1.0 - math::geometric_epsilon<double>)) {
        t = 1.0;
      }

#if 0
      const auto [p, q1, q2] = math::split_quadratic(p0, p1, p2, t);

      const dvec2 end_n = math::normal(q1, p);
      const dvec2 n = start_n + end_n;

      const dvec2 nr1 = n * (2.0 * radius / math::squared_length(n));
      const dvec2 nr2 = end_n * radius;

      sink.inner.quadratic_to(vec2(q1 - nr1), vec2(p - nr2));
      sink.outer.quadratic_to(vec2(q1 + nr1), vec2(p + nr2));

      start_n = end_n;

      if (t == 1.0) {
        break;
      }

      p0 = p;
      p1 = q2;
#endif
    }

    return start_n;
  }

  /**
   * @brief Offsets a quadratic bezier curve.
   *
   * https://github.com/blend2d/blend2d/blob/master/src/blend2d/pathstroke.cpp
   *
   * @param p0 The start point of the curve.
   * @param p1 The control point of the curve.
   * @param p2 The end point of the curve.
   * @param radius The radius to offset the curve by.
   * @param tolerance The offset error tolerance.
   * @param sink The output path.
   * @return The normal of the end point.
   */
  static dvec2 offset_quadratic(dvec2 p0, dvec2 p1, dvec2 p2, const double radius, const double tolerance, PathBuilder::StrokeOutline& sink) {
    const dvec2 v1 = p1 - p0;
    const dvec2 v2 = p2 - p1;

    const double cross = math::cross(v2, v1);

    if (math::is_almost_zero(cross, 3.0)) {
      const double dot = math::dot(-v1, v2);

      /* Check if the control point lies outside of the start/end points. */
      if (dot > 0.0) {
        /* Rotate all points to x-axis. */
        const double r1 = math::squared_length(v1);
        const double r2 = math::dot(p2 - p0, v1);

        /* Parameter of the cusp if it's within (0, 1). */
        const double t = r1 / (2.0 * r1 - r2);

        if (math::is_normalized(t, false)) {
          const dvec2 p = geom::quadratic({ p0, p1, p2 }, t);
          const dvec2 n = math::normal(p, p2);

          offset_line(p0, p, radius, sink);
          add_cap(dvec2(sink.outer.points.back()), p + n * radius, n, radius, LineCap::Round, sink.outer);
          add_cap(dvec2(sink.inner.points.back()), p - n * radius, -n, radius, LineCap::Round, sink.inner);

          return offset_line(p, p2, radius, sink);
        }
      }

      return offset_line(p0, p2, radius, sink);
    }

    const dvec2 a = 2.0 * (v2 - v1);
    const dvec2 b = 2.0 * (p1 - p0);

    const double bxa = math::cross(b, a);
    const double boa = math::dot(b, a);

    if (bxa == 0) {
      return dvec2{ 0.0, 0.0 };
    }

    const double alen2 = math::squared_length(a);
    const double blen2 = math::squared_length(b);

    const double fac = -1.0 / alen2;
    const double sqrt_ = std::sqrt(boa * boa - alen2 * (blen2 - std::cbrt(radius * radius * bxa * bxa)));

    const double t1 = fac * (boa + sqrt_);
    const double t2 = fac * (boa - sqrt_);

    const uint8_t code = static_cast<uint8_t>(math::is_normalized(t1, false)) |
      (static_cast<uint8_t>(math::is_normalized(t2, false)) << 1);

#if 0
    switch (code) {
    case 1: {
      const auto [p, q1, q2] = math::split_quadratic(p0, p1, p2, t1);
      offset_monotonic_quadratic(p0, q1, p, radius, tolerance, sink);
      return offset_monotonic_quadratic(p, q2, p2, radius, tolerance, sink);
    }
    case 2: {
      const auto [p, q1, q2] = math::split_quadratic(p0, p1, p2, t2);
      offset_monotonic_quadratic(p0, q1, p, radius, tolerance, sink);
      return offset_monotonic_quadratic(p, q2, p2, radius, tolerance, sink);
    }
    case 3: {
      const auto [q1, p01, q, p02, q2] = math::split_quadratic(p0, p1, p2, t1, t2);
      offset_monotonic_quadratic(p0, q1, p01, radius, tolerance, sink);
      offset_monotonic_quadratic(p01, q, p02, radius, tolerance, sink);
      return offset_monotonic_quadratic(p02, q2, p2, radius, tolerance, sink);
    }
    case 0:
    default: {
      return offset_monotonic_quadratic(p0, p1, p2, radius, tolerance, sink);
    }
    }
#endif

    return dvec2::zero();
  }

  static void fast_flatten(const vec2 p0, const vec2 p1, const vec2 p2, const float tolerance, std::vector<vec4>& sink) {
    const vec2 a = p0 - 2.0f * p1 + p2;
    const vec2 b = 2.0f * (p1 - p0);
    const vec2 c = p0;

    const float dt = std::sqrtf((2.0 * tolerance) / math::length(p0 - 2.0 * p1 + p2));

    vec2 last = p0;
    float t = dt;

    while (t < 1.0f) {
      const float t_sq = t * t;
      const vec2 p = a * t_sq + b * t + c;

      sink.emplace_back(last.x, last.y, p.x, p.y);

      last = p;
      t += dt;
    }

    sink.emplace_back(last.x, last.y, p2.x, p2.y);
  }

  static void recursive_flatten(const vec2 p0, const vec2 p1, const vec2 p2, const rect& clip, const float tolerance, std::vector<vec4>& sink, uint8_t depth = 0) {
    if (depth > GK_MAX_RECURSION) {
      sink.emplace_back(p0.x, p0.y, p2.x, p2.y);
      return;
    }

    // const bool p0_in = math::is_point_in_rect(p0, clip);
    // const bool p1_in = math::is_point_in_rect(p1, clip);
    // const bool p2_in = math::is_point_in_rect(p2, clip);

    // TODO: implement math::rect_from_points
    rect bounds = { p0, p0 };

    math::min(bounds.min, p1, bounds.min);
    math::min(bounds.min, p2, bounds.min);
    math::max(bounds.max, p1, bounds.max);
    math::max(bounds.max, p2, bounds.max);

    if (!geom::does_rect_intersect_rect(bounds, clip)) {
      return;
    }

    // if (!p0_in && !p1_in && !p2_in) {
    //   sink.emplace_back(p0.x, p0.y, p2.x, p2.y);
    //   return;
    // } else if (p0_in && p1_in && p2_in) {
    //   fast_flatten(p0, p1, p2, tolerance, sink);
    //   return;
    // }

    depth += 1;

    const vec2 p01 = (p0 + p1) * 0.5f;
    const vec2 p12 = (p1 + p2) * 0.5f;
    const vec2 p012 = (p01 + p12) * 0.5f;

    const float num = std::abs((p2.x - p0.x) * (p0.y - p012.y) - (p0.x - p012.x) * (p2.y - p0.y));
    const float den = math::squared_distance(p0, p2);
    const float sq_error = num * num / den;

    if (sq_error < tolerance * tolerance) {
      sink.emplace_back(p0.x, p0.y, p2.x, p2.y);
      return;
    }

    recursive_flatten(p0, p01, p012, clip, tolerance, sink, depth);
    recursive_flatten(p012, p12, p2, clip, tolerance, sink, depth);
  }

  /* -- PathBuilder -- */

  PathBuilder::PathBuilder(const geom::QuadraticPath& path, const mat2x3& transform, const rect* bounding_rect) :
    m_path(path), m_transform(transform), m_bounding_rect(transform* (bounding_rect ? *bounding_rect : path.approx_bounding_rect())) {}

  PathBuilder::StrokeOutline PathBuilder::stroke(const Stroke& stroke, const float tolerance) const {
    if (m_path.empty()) {
      return {};
    }

    // TODO: transform if necessary
    dvec2 p0 = dvec2(m_path[0]);
    dvec2 p1 = dvec2(m_path[1]);
    dvec2 p2 = dvec2(m_path[2]);

    const double radius = static_cast<double>(stroke.width) * 0.5;
    const double inv_miter_limit = 1.0 / static_cast<double>(stroke.miter_limit);

    PathBuilder::StrokeOutline outline;

    if (m_path.size() == 1 && (p0 == p1 && p1 == p2)) {
      if (stroke.cap != LineCap::Butt) {
        return {};
      }

      dvec2 n = { 0.0, 1.0 };
      dvec2 nr = n * radius;
      dvec2 start = p0 + nr;
      dvec2 rstart = p0 - nr;

      outline.outer.move_to(vec2(start));

      add_cap(start, rstart, n, radius, stroke.cap, outline.outer);
      add_cap(rstart, start, -n, radius, stroke.cap, outline.outer);

      return outline;
    }

    dvec2 pivot = p0;
    dvec2 last_p1 = p1;
    dvec2 last_n = math::normal(p0, p1);

    if (m_path.closed()) {
      outline.inner.move_to(vec2(p0 - last_n * radius));
      outline.outer.move_to(vec2(p0 + last_n * radius));
    } else {
      const dvec2 start = p0 - last_n * radius;

      outline.inner.move_to(vec2(start));
      outline.outer.move_to(vec2(start));

      add_cap(start, p0 + last_n * radius, -last_n, radius, stroke.cap, outline.outer);
    }

    for (size_t i = 0; i < m_path.size(); i++) {
      p0 = dvec2(m_path[i * 2]);
      p1 = dvec2(m_path[i * 2 + 1]);
      p2 = dvec2(m_path[i * 2 + 2]);

      const dvec2 start_n = math::normal(p0, p1);
      const dvec2 start_nr = start_n * radius;

      const dvec2 a = p1 - p0;
      const dvec2 b = last_p1 - p0;

      double cos = math::dot(a, b) / (math::length(a) * math::length(b));

      if (cos > 0 || std::abs(cos) < 1.0 - math::geometric_epsilon<double>) {
        const dvec2 inner_start = p0 - start_nr;
        const dvec2 outer_start = p0 + start_nr;

        add_join(dvec2(outline.inner.points.back()), inner_start, pivot, -last_n, -start_n, radius, inv_miter_limit, stroke.join, outline.inner, true);
        add_join(dvec2(outline.outer.points.back()), outer_start, pivot, last_n, start_n, radius, inv_miter_limit, stroke.join, outline.outer);
      }

      if (p1 == p2) {
        /* Linear segment. */

        outline.inner.line_to(vec2(p2 - start_nr));
        outline.outer.line_to(vec2(p2 + start_nr));

        last_n = start_n;
        last_p1 = p0;
      } else {
        /* Quadratic segment. */

        last_n = offset_quadratic(p0, p1, p2, radius, tolerance, outline);
        last_p1 = p1;
      }

      pivot = p2;
    }

    if (m_path.closed()) {
      const dvec2 start_n = math::normal(dvec2(m_path[0]), dvec2(m_path[1]));

      add_join(dvec2(outline.inner.points.back()), dvec2(outline.inner.points.front()), pivot, -last_n, -start_n, radius, inv_miter_limit, stroke.join, outline.inner, true);
      add_join(dvec2(outline.outer.points.back()), dvec2(outline.outer.points.front()), pivot, last_n, start_n, radius, inv_miter_limit, stroke.join, outline.outer);
    } else {
      add_cap(dvec2(outline.outer.points.back()), dvec2(outline.inner.points.back()), last_n, radius, stroke.cap, outline.outer);

      outline.outer.points.insert(outline.outer.points.end(), outline.inner.points.rbegin() + 1, outline.inner.points.rend());
      outline.inner.points.clear();
    }

    return outline;
  }

  void PathBuilder::flatten(const rect& clip, const float tolerance, std::vector<vec4>& sink) const {
    GK_TOTAL("PathBuilder::flatten");

    if (m_path.empty()) {
      return;
    }

    const float coverage = geom::rect_rect_intersection_area(m_bounding_rect, clip) / m_bounding_rect.area();

    if (coverage <= 0.0f) {
      return;
    } else if (coverage <= 0.5f) {
      flatten_clipped(clip, tolerance, sink);
    } else {
      flatten_unclipped(tolerance, sink);
    }
  }

  void PathBuilder::flatten_clipped(const rect& clip, const float tolerance, std::vector<vec4>& sink) const {
    vec2 p0 = m_transform * m_path[0];

    for (size_t i = 0; i < m_path.size(); i++) {
      const vec2 p1 = m_transform * m_path[i * 2 + 1];
      const vec2 p2 = m_transform * m_path[i * 2 + 2];

      if (p1 == p2) {
        sink.emplace_back(p0.x, p0.y, p2.x, p2.y);
      } else {
        recursive_flatten(p0, p1, p2, clip, tolerance, sink);
      }

      p0 = p2;
    }
  }

  void PathBuilder::flatten_unclipped(const float tolerance, std::vector<vec4>& sink) const {
    vec2 p0 = m_transform * m_path[0];

    for (size_t i = 0; i < m_path.size(); i++) {
      const vec2 p1 = m_transform * m_path[i * 2 + 1];
      const vec2 p2 = m_transform * m_path[i * 2 + 2];

      if (p1 == p2) {
        sink.emplace_back(p0.x, p0.y, p2.x, p2.y);
      } else {
        fast_flatten(p0, p1, p2, tolerance, sink);
      }

      p0 = p2;
    }
  }

}
