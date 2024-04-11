/**
 * @file path_builder.cpp
 * @brief Contains the PathBuilder class implementation.
 *
 * @todo fix flickering at super high zoom levels
 */

#include "path_builder_new.h"

#include "path.h"

#include "../properties.h"

#include "../../math/matrix.h"
#include "../../math/vector.h"
#include "../../math/math.h"

#include "../../utils/console.h"

namespace Graphick::renderer::geometry {

  using LineCap = Graphick::Renderer::LineCap;
  using LineJoin = Graphick::Renderer::LineJoin;

  struct StrokeOutline {
    QuadraticPath inner;
    QuadraticPath outer;
  };

  /* -- Static -- */

  /**
   * @brief Offsets a monotonic quadratic bezier curve.
   *
   * @param p0 The start point of the curve.
   * @param p1 The control point of the curve.
   * @param p2 The end point of the curve.
   * @param radius The radius to offset the curve by.
   * @param tolerance The offset error tolerance.
   * @param sink The output path.
   */
  static vec2 offset_monotonic_quadratic(vec2 p0, vec2 p1, vec2 p2, const float radius, const float tolerance, StrokeOutline& sink) {
    vec2 start_n = Math::normal(p0, p1);

    while (true) {
      const vec2 a = 2.0f * (p0 - 2.0f * p1 + p2);
      const vec2 b = 2.0f * (p1 - p0);

      const float aob = Math::dot(a, b);
      const float axb = Math::cross(a, b);

      if (aob == 0.0f) {
        break;
      }

      float t = tolerance * Math::squared_length(b) / (std::abs(axb) - tolerance * aob);

      if (!(t > GK_QUADRATIC_EPSILON && t < 1.0f - GK_QUADRATIC_EPSILON)) {
        t = 1.0;
      }

      const auto [p, q1, q2] = Math::split_quadratic(p0, p1, p2, t);

      const vec2 end_n = Math::normal(q1, p);
      const vec2 n = start_n + end_n;

      const vec2 nr1 = n * (2.0f * radius / Math::squared_length(n));
      const vec2 nr2 = end_n * radius;

      sink.inner.quadratic_to(q1 - nr1, p - nr2);
      sink.outer.quadratic_to(q1 + nr1, p + nr2);

      if (t == 1.0f) {
        break;
      }

      p0 = p;
      p1 = q2;
      start_n = end_n;
    }

    return start_n;
  }

  static void offset_monotonic_quadratic(vec2 p0, vec2 p1, vec2 p2, const float radius, const float tolerance, QuadraticPath& sink) {
    // TODO: don't recalcuate start normal

    while (true) {
      const vec2 a = 2.0f * (p0 - 2.0f * p1 + p2);
      const vec2 b = 2.0f * (p1 - p0);

      const float aob = Math::dot(a, b);
      const float axb = Math::cross(a, b);

      if (aob == 0.0f) {
        break;
      }

      float t = tolerance * Math::squared_length(b) / (std::abs(axb) - tolerance * aob);

      if (!(t > GK_QUADRATIC_EPSILON && t < 1.0f - GK_QUADRATIC_EPSILON)) {
        t = 1.0;
      }

      const auto [p, q1, q2] = Math::split_quadratic(p0, p1, p2, t);

      const vec2 start_n = Math::normal(p0, q1);
      const vec2 end_n = Math::normal(q1, p);
      const vec2 n = start_n + end_n;

      const vec2 k1 = n * (2.0f * radius / Math::squared_length(n));
      const vec2 k2 = end_n * radius;

      sink.quadratic_to(q1 + k1, p + k2);

      if (t == 1.0f) {
        break;
      }

      p0 = p;
      p1 = q2;
    }
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
   */
  static vec2 offset_quadratic(vec2 p0, vec2 p1, vec2 p2, const float radius, const float tolerance, StrokeOutline& sink) {
    const vec2 v1 = p1 - p0;
    const vec2 v2 = p2 - p1;

    const float cross = Math::cross(v2, v1);

    if (Math::is_almost_zero(cross, 3.0f)) {
      // TODO: implement
      const float dot = Math::dot(-v1, v2);

      /* Check if the control point lies outside of the start/end points. */
      if (dot > 0.0f) {
        /* Rotate all points to x-axis. */
        const float r1 = Math::squared_length(v1);
        const float r2 = Math::dot(p2 - p0, v1);

        /* Parameter of the cusp if it's within (0, 1). */
        const float t = r2 / r1;

        if (Math::is_normalized(t, false)) {
          // Offset line
          return vec2{ 0.0f, 0.0f };
        }
      }

      // Offset line
      // sink.line_to(p2);

      return vec2{ 0.0f, 0.0f };
    }

    const vec2 a = 2.0f * (v2 - v1);
    const vec2 b = 2.0f * (p1 - p0);

    const float bxa = Math::cross(b, a);
    const float boa = Math::dot(b, a);

    if (bxa == 0) {
      return vec2{ 0.0f, 0.0f };
    }

    const double alen2 = Math::squared_length(a);
    const double blen2 = Math::squared_length(b);

    const float fac = -1.0f / alen2;
    const float sqrt_ = std::sqrtf(boa * boa - alen2 * (blen2 - std::cbrtf(radius * radius * bxa * bxa)));

    const float t1 = fac * (boa + sqrt_);
    const float t2 = fac * (boa - sqrt_);

    const uint8_t code = static_cast<uint8_t>(Math::is_normalized(t1, false)) |
      (static_cast<uint8_t>(Math::is_normalized(t2, false)) << 1);

    switch (code) {
    case 1: {
      const auto [p, q1, q2] = Math::split_quadratic(p0, p1, p2, t1);
      offset_monotonic_quadratic(p0, q1, p, radius, tolerance, sink);
      return offset_monotonic_quadratic(p, q2, p2, radius, tolerance, sink);
    }
    case 2: {
      const auto [p, q1, q2] = Math::split_quadratic(p0, p1, p2, t2);
      offset_monotonic_quadratic(p0, q1, p, radius, tolerance, sink);
      return offset_monotonic_quadratic(p, q2, p2, radius, tolerance, sink);
    }
    case 3: {
      const auto [q1, p01, q, p02, q2] = Math::split_quadratic(p0, p1, p2, t1, t2);
      offset_monotonic_quadratic(p0, q1, p01, radius, tolerance, sink);
      offset_monotonic_quadratic(p01, q, p02, radius, tolerance, sink);
      return offset_monotonic_quadratic(p02, q2, p2, radius, tolerance, sink);
    }
    case 0:
    default: {
      return offset_monotonic_quadratic(p0, p1, p2, radius, tolerance, sink);
    }
    }
  }

  static void offset_quadratic(vec2 p0, vec2 p1, vec2 p2, const float radius, const float tolerance, QuadraticPath& sink) {
    const vec2 v1 = p1 - p0;
    const vec2 v2 = p2 - p1;

    const float cross = Math::cross(v2, v1);

    if (Math::is_almost_zero(cross, 3.0f)) {
      const float dot = Math::dot(-v1, v2);

      /* Check if the control point lies outside of the start/end points. */
      if (dot > 0.0f) {
        /* Rotate all points to x-axis. */
        const float r1 = Math::squared_length(v1);
        const float r2 = Math::dot(p2 - p0, v1);

        /* Parameter of the cusp if it's within (0, 1). */
        const float t = r2 / r1;

        if (Math::is_normalized(t, false)) {
          // Offset line
          return;
        }
      }

      // Offset line
      sink.line_to(p2);

      return;
    }

    const vec2 a = 2.0 * (p0 - 2.0 * p1 + p2);
    const vec2 b = 2.0 * (p1 - p0);

    const float bxa = Math::cross(b, a);
    const float boa = Math::dot(b, a);

    if (bxa == 0) {
      return;
    }

    const double alen2 = Math::squared_length(a);
    const double blen2 = Math::squared_length(b);

    const float fac = -1.0f / alen2;
    const float sqrt_ = std::sqrtf(boa * boa - alen2 * (blen2 - std::cbrtf(radius * radius * bxa * bxa)));

    const float t1 = fac * (boa + sqrt_);
    const float t2 = fac * (boa - sqrt_);

    // const float den = a.x * a.x + a.y * a.y;
    // const float dot = a.x * b.x + a.y * b.y;
    // const float radix = std::sqrtf(
    //   dot * dot - den *
    //   (b.x * b.x + b.y * b.y - std::cbrtf(radius * radius * (b.x * a.y - a.x * b.y) * (b.x * a.y - a.x * b.y)))
    // );

    // const float t1 = (-dot - radix) / den;
    // const float t2 = (-dot + radix) / den;

    const uint8_t code = static_cast<uint8_t>(Math::is_normalized(t1, false)) |
      (static_cast<uint8_t>(Math::is_normalized(t2, false)) << 1);

    switch (code) {
    case 1: {
      const auto [p, q1, q2] = Math::split_quadratic(p0, p1, p2, t1);
      offset_monotonic_quadratic(p0, q1, p, radius, tolerance, sink);
      offset_monotonic_quadratic(p, q2, p2, radius, tolerance, sink);
      break;
    }
    case 2: {
      const auto [p, q1, q2] = Math::split_quadratic(p0, p1, p2, t2);
      offset_monotonic_quadratic(p0, q1, p, radius, tolerance, sink);
      offset_monotonic_quadratic(p, q2, p2, radius, tolerance, sink);
      break;
    }
    case 3: {
      const auto [q1, p01, q, p02, q2] = Math::split_quadratic(p0, p1, p2, t1, t2);
      offset_monotonic_quadratic(p0, q1, p01, radius, tolerance, sink);
      offset_monotonic_quadratic(p01, q, p02, radius, tolerance, sink);
      offset_monotonic_quadratic(p02, q2, p2, radius, tolerance, sink);
      break;
    }
    case 0:
    default: {
      offset_monotonic_quadratic(p0, p1, p2, radius, tolerance, sink);
      break;
    }
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
  void add_cap(const vec2 from, const vec2 to, const vec2 n, const float radius, const LineCap cap, QuadraticPath& sink) {
    switch (cap) {
    case LineCap::Round: {
      // TODO: implement as static function here
      // sink.arc(from + (to - from) / 2.0, from, radius, to);
      // break;
    }
    case LineCap::Square: {
      vec2 dir = { -n.y * radius, n.x * radius };

      sink.line_to(from + dir);
      sink.line_to(to + dir);
      sink.line_to(to);

      break;
    }
    default:
    case LineCap::Butt: {
      sink.line_to(to);
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
  static void add_join(const vec2 from, const vec2 to, const vec2 pivot, const vec2 from_normal, const vec2 to_normal, const float radius, const float inv_miter_limit, LineJoin join, QuadraticPath& sink, const bool reverse = false) {
    if (Math::is_almost_equal(from, to, GK_POINT_EPSILON)) {
      return;
    }

    //const vec2 center = from - from_normal * radius;
    const vec2 a = from - pivot;
    const vec2 b = to - pivot;

    float dot = a.x * b.x + a.y * b.y;
    float cross = a.x * b.y - a.y * b.x;

    if (reverse) cross = -cross;

    float ang = std::atan2(cross, dot);

    if (ang < 0.0) ang += MATH_TWO_PI;
    if (ang >= MATH_PI) join = LineJoin::Bevel;

    if (Math::is_almost_zero(ang)) {
      return;
    }

    switch (join) {
    case LineJoin::Round: {
      // TODO: implement
      // sink.arc(center, from, radius, to);
      // break;
    }
    case LineJoin::Miter: {
      float dot = from_normal.x * to_normal.x + from_normal.y * to_normal.y;
      float sin_half = std::sqrt((1.0 + dot) * 0.5);

      if (sin_half < inv_miter_limit) {
        sink.line_to(to);
        break;
      } else {
        vec2 mid = from_normal + to_normal;
        float l = radius / (sin_half * std::hypot(mid.x, mid.y));
        vec2 p = pivot + mid * l;

        sink.line_to(p);
        sink.line_to(to);
        break;
      }
    }
    default:
    case LineJoin::Bevel: {
      sink.line_to(to);
      break;
    }
    }
  }

  static void fast_flatten(const vec2 p0, const vec2 p1, const vec2 p2, const float tolerance, std::vector<vec4>& sink) {
    const vec2 a = p0 - 2.0f * p1 + p2;
    const vec2 b = 2.0f * (p1 - p0);
    const vec2 c = p0;

    const float dt = std::sqrtf((2.0 * tolerance) / Math::length(p0 - 2.0 * p1 + p2));

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

    const bool p0_in = Math::is_point_in_rect(p0, clip);
    const bool p1_in = Math::is_point_in_rect(p1, clip);
    const bool p2_in = Math::is_point_in_rect(p2, clip);

    if (!p0_in && !p1_in && !p2_in) {
      sink.emplace_back(p0.x, p0.y, p2.x, p2.y);
      return;
    } else if (p0_in && p1_in && p2_in) {
      fast_flatten(p0, p1, p2, tolerance, sink);
      return;
    }

    depth += 1;

    const vec2 p01 = (p0 + p1) * 0.5f;
    const vec2 p12 = (p1 + p2) * 0.5f;
    const vec2 p012 = (p01 + p12) * 0.5f;

    const float num = std::abs((p2.x - p0.x) * (p0.y - p012.y) - (p0.x - p012.x) * (p2.y - p0.y));
    const float den = Math::squared_distance(p0, p2);
    const float sq_error = num * num / den;

    if (sq_error < tolerance * tolerance) {
      sink.emplace_back(p0.x, p0.y, p2.x, p2.y);
      return;
    }

    recursive_flatten(p0, p01, p012, clip, tolerance, sink, depth);
    recursive_flatten(p012, p12, p2, clip, tolerance, sink, depth);
  }

  /* -- PathBuilder -- */

  PathBuilder::PathBuilder(const QuadraticPath& path, const mat2x3& transform, const rect* bounding_rect) :
    m_path(path), m_transform(transform), m_bounding_rect(transform* (bounding_rect ? *bounding_rect : path.approx_bounding_rect())) {}

#if 0
  QuadraticPath PathBuilder::stroke(const Graphick::Renderer::Stroke& stroke, const float tolerance) const {
    if (m_path.empty()) {
      return {};
    }

    QuadraticPath fill;

    fill.move_to(m_path[0] + Math::normal(m_path[1], m_path[0]) * stroke.width);

    for (size_t i = 0; i < m_path.size(); i++) {
      const vec2 p0 = m_path[i * 2];
      const vec2 p1 = m_path[i * 2 + 1];
      const vec2 p2 = m_path[i * 2 + 2];

      if (p1 == p2) {
        // TODO: linear offset
        continue;
      }

      const vec2 a = 2.0 * (p0 - 2.0 * p1 + p2);
      const vec2 b = 2.0 * (p1 - p0);

      const float den = a.x * a.x + a.y * a.y;
      const float dot = a.x * b.x + a.y * b.y;
      const float radix = std::sqrtf(
        dot * dot - den *
        (b.x * b.x + b.y * b.y - std::cbrtf(stroke.width * stroke.width * (b.x * a.y - a.x * b.y) * (b.x * a.y - a.x * b.y)))
      );

      const float t1 = (-dot - radix) / den;
      const float t2 = (-dot + radix) / den;

      const uint8_t code = static_cast<uint8_t>(Math::is_normalized(t1, false)) |
        (static_cast<uint8_t>(Math::is_normalized(t2, false)) << 1);

      switch (code) {
      case 1: {
        const auto [p, q1, q2] = Math::split_quadratic(p0, p1, p2, t1);
        offset_quadratic(p0, q1, p, stroke.width, tolerance, fill);
        offset_quadratic(p, q2, p2, stroke.width, tolerance, fill);
        break;
      }
      case 2: {
        const auto [p, q1, q2] = Math::split_quadratic(p0, p1, p2, t2);
        offset_quadratic(p0, q1, p, stroke.width, tolerance, fill);
        offset_quadratic(p, q2, p2, stroke.width, tolerance, fill);
        break;
      }
      case 3: {
        const auto [q1, p01, q, p02, q2] = Math::split_quadratic(p0, p1, p2, t1, t2);
        offset_quadratic(p0, q1, p01, stroke.width, tolerance, fill);
        offset_quadratic(p01, q, p02, stroke.width, tolerance, fill);
        offset_quadratic(p02, q2, p2, stroke.width, tolerance, fill);
        break;
      }
      case 0:
      default: {
        offset_quadratic(p0, p1, p2, stroke.width, tolerance, fill);
        break;
      }
      }
    }

    for (int64_t i = m_path.size() - 1; i >= 0; --i) {
      const vec2 p0 = m_path[i * 2 + 2];
      const vec2 p1 = m_path[i * 2 + 1];
      const vec2 p2 = m_path[i * 2];

      if (p1 == p2) {
        // TODO: linear offset
        continue;
      }

      const vec2 a = 2.0 * (p0 - 2.0 * p1 + p2);
      const vec2 b = 2.0 * (p1 - p0);

      const float den = a.x * a.x + a.y * a.y;
      const float dot = a.x * b.x + a.y * b.y;
      const float radix = std::sqrtf(
        dot * dot - den *
        (b.x * b.x + b.y * b.y - std::cbrtf(stroke.width * stroke.width * (b.x * a.y - a.x * b.y) * (b.x * a.y - a.x * b.y)))
      );

      const float t1 = (-dot - radix) / den;
      const float t2 = (-dot + radix) / den;

      const uint8_t code = static_cast<uint8_t>(Math::is_normalized(t1, false)) |
        (static_cast<uint8_t>(Math::is_normalized(t2, false)) << 1);

      switch (code) {
      case 1: {
        const auto [p, q1, q2] = Math::split_quadratic(p0, p1, p2, t1);
        offset_quadratic(p0, q1, p, stroke.width, tolerance, fill);
        offset_quadratic(p, q2, p2, stroke.width, tolerance, fill);
        break;
      }
      case 2: {
        const auto [p, q1, q2] = Math::split_quadratic(p0, p1, p2, t2);
        offset_quadratic(p0, q1, p, stroke.width, tolerance, fill);
        offset_quadratic(p, q2, p2, stroke.width, tolerance, fill);
        break;
      }
      case 3: {
        const auto [q1, p01, q, p02, q2] = Math::split_quadratic(p0, p1, p2, t1, t2);
        offset_quadratic(p0, q1, p01, stroke.width, tolerance, fill);
        offset_quadratic(p01, q, p02, stroke.width, tolerance, fill);
        offset_quadratic(p02, q2, p2, stroke.width, tolerance, fill);
        break;
      }
      case 0:
      default: {
        offset_quadratic(p0, p1, p2, stroke.width, tolerance, fill);
        break;
      }
      }
    }

    return fill;
  }
#else

  QuadraticPath PathBuilder::stroke(const Graphick::Renderer::Stroke& stroke, const float tolerance) const {
    if (m_path.empty()) {
      return {};
    }

    vec2 p0 = m_path[0];
    vec2 p1 = m_path[1];
    vec2 p2 = m_path[2];

    const float radius = stroke.width * 0.5f * 20.0f;
    const float inv_miter_limit = 1.0f / stroke.miter_limit;

    StrokeOutline outline;

    if (m_path.size() == 1 && (p0 == p1 && p1 == p2)) {
      if (stroke.cap != LineCap::Butt) {
        return {};
      }

      vec2 n = { 0.0, 1.0 };
      vec2 nr = n * radius;
      vec2 start = p0 + nr;
      vec2 rstart = p0 - nr;

      outline.outer.move_to(start);

      add_cap(start, rstart, n, radius, stroke.cap, outline.outer);
      add_cap(rstart, start, -n, radius, stroke.cap, outline.outer);

      return outline.outer;
    }

    p0 = m_path[0];
    p1 = m_path[1];
    p2 = m_path[2];

    vec2 pivot = p0;
    vec2 last_p1 = p1;
    vec2 last_n = Math::normal(p0, p1);

    outline.inner.move_to(p0 - last_n * radius);
    outline.outer.move_to(p0 + last_n * radius);

    for (size_t i = 0; i < m_path.size(); i++) {
      p0 = m_path[i * 2];
      p1 = m_path[i * 2 + 1];
      p2 = m_path[i * 2 + 2];

      const vec2 start_n = Math::normal(p0, p1);
      const vec2 start_nr = start_n * radius;

      const dvec2 a = dvec2(p1) - dvec2(p0);
      const dvec2 b = dvec2(last_p1) - dvec2(p0);

      double cos = Math::dot(a, b) / (Math::length(a) * Math::length(b));
      // double cross = Math::cross(a, b);
      // cos *= cos / (Math::squared_length(a) * Math::squared_length(b));
      // console::log("cossssssssssssssssssssssssssssssssssssssssssssssssssssss", cos > 0.0);

      if (cos > 0 || std::abs(cos) < 1.0 - GK_POINT_EPSILON) {
        const vec2 inner_start = p0 - start_nr;
        const vec2 outer_start = p0 + start_nr;

        add_join(outline.inner.points.back(), inner_start, pivot, -last_n, -start_n, radius, inv_miter_limit, stroke.join, outline.inner, true);
        add_join(outline.outer.points.back(), outer_start, pivot, last_n, start_n, radius, inv_miter_limit, stroke.join, outline.outer);
      }

      if (p1 == p2) {
        /* Linear segment. */

        outline.inner.line_to(p2 - start_nr);
        outline.outer.line_to(p2 + start_nr);

        last_n = start_n;
        last_p1 = p0;
      } else {
        /* Quadratic segment. */

        last_n = offset_quadratic(p0, p1, p2, radius, tolerance, outline);
        last_p1 = p1;
      }

      pivot = p2;
    }

    QuadraticPath stroke_outline = std::move(outline.outer);

    // TODO: handle closed
    add_cap(stroke_outline.points.back(), outline.inner.points.back(), last_n, radius, stroke.cap, stroke_outline);

    stroke_outline.points.insert(stroke_outline.points.end(), outline.inner.points.rbegin() + 1, outline.inner.points.rend());

    return stroke_outline;
#if 0
    vec2 last_dir = { 0.0f, 0.0f };
    vec2 first_point = { 0.0f, 0.0f };
    vec2 last_point = { 0.0f, 0.0f };
    vec2 pivot = { 0.0f, 0.0f };

    bool is_closed = m_path.closed();
    bool is_first = !is_closed;

    /* Forward for the outer stroke. */

    contour = &fill.emplace_back();

    p0 = m_path[0];
    p1 = m_path[1];

    const vec2 front_n = Math::normal(p0, p1);
    const vec2 front_start = p0 + radius * front_n;

    if (is_closed) {
      p0 = m_path[(m_path.size() - 1) * 2];
      p1 = m_path[(m_path.size() - 1) * 2 + 1];
      p2 = m_path[0];

      if (p1 == p2) {
        const vec2 n = Math::normal(p0, p1);

        pivot = p1;
        last_dir = n;
        first_point = last_point = p1 + n * radius;
      } else {
        const vec2 n = Math::normal(p1, p2);

        pivot = p2;
        last_dir = n;
        first_point = last_point = p2 + n * radius;
      }

      contour->move_to(last_point);
      add_join(last_point, front_start, pivot, last_dir, front_n, radius, inv_miter_limit, stroke.join, *contour);
    } else {
      pivot = m_path[0];
      contour->move_to(front_start);
    }

#endif
#if 0
    for (size_t i = 0; i < m_path.size(); i++) {
      p0 = m_path[i * 2];
      p1 = m_path[i * 2 + 1];
      p2 = m_path[i * 2 + 2];

      if (p1 == p2) {
        if (Math::is_almost_equal(p0, p1, GK_POINT_EPSILON)) continue;

        const vec2 n = Math::normal(p0, p1);
        const vec2 nr = n * radius;

        const vec2 start = p0 + nr;

        if (i != 0) {
          add_join(last_point, start, pivot, last_dir, n, radius, inv_miter_limit, stroke.join, *contour);
        }

        last_dir = n;
        pivot = p1;
        last_point = p1 + nr;

        contour->line_to(last_point);
      } else {
        const vec2 start_n = Math::normal(p0, p1);
        const vec2 end_n = Math::normal(p1, p2);

        const vec2 start = p0 + start_n * radius;

        if (i != 0) {
          add_join(last_point, start, pivot, last_dir, start_n, radius, inv_miter_limit, stroke.join, *contour);
        }

        last_dir = end_n;
        pivot = p2;
        last_point = p2 + end_n * radius;

        offset_quadratic(p0, p1, p2, radius, tolerance, *contour);
      }
    }

    /* Backward for the inner stroke. */

    p0 = m_path[m_path.size() * 2];
    p1 = m_path[m_path.size() * 2 - 1];

    if (p0 == p1) {
      p1 = m_path[m_path.size() * 2 - 2];
    }

    const vec2 back_n = Math::normal(p0, p1);
    const vec2 back_start = p0 + back_n * radius;

    if (is_closed) {
      p0 = m_path[2];
      p1 = m_path[1];
      p2 = m_path[0];

      vec2 n;

      if (p0 == p1) {
        n = Math::normal(p0, p2);
      } else {
        n = Math::normal(p1, p2);
      }

      last_dir = n;
      pivot = p2;
      last_point = p2 + n * radius;

      contour->line_to(last_point);
      add_join(last_point, back_start, pivot, last_dir, back_n, radius, inv_miter_limit, stroke.join, *contour);
    } else {
      const vec2 p0 = m_path[m_path.size() * 2 - 1];
      const vec2 p1 = m_path[m_path.size() * 2 - 2];
      const vec2 n = Math::normal(p0, p1);

      add_cap(last_point, back_start, last_dir, radius, stroke.cap, *contour);
    }

    for (int64_t i = m_path.size() - 1; i >= 0; i--) {
      p0 = m_path[i * 2 + 2];
      p1 = m_path[i * 2 + 1];
      p2 = m_path[i * 2];

      if (p0 == p1) {
        if (Math::is_almost_equal(p0, p2, GK_POINT_EPSILON)) continue;

        const vec2 n = Math::normal(p0, p2);
        const vec2 nr = n * radius;

        if (i < m_path.size() - 1) {
          add_join(last_point, p0 + nr, pivot, last_dir, n, radius, inv_miter_limit, stroke.join, *contour);
        }

        last_dir = n;
        pivot = p2;
        last_point = p2 + nr;

        contour->line_to(last_point);
      } else {
        const vec2 start_n = Math::normal(p0, p1);
        const vec2 end_n = Math::normal(p1, p2);

        if (i < m_path.size() - 1) {
          add_join(last_point, p0 + start_n * radius, pivot, last_dir, start_n, radius, inv_miter_limit, stroke.join, *contour);
        }

        last_dir = end_n;
        pivot = p2;
        last_point = p2 + end_n * radius;

        offset_quadratic(p0, p1, p2, radius, tolerance, *contour);
      }
    }

    if (!is_closed) {
      const vec2 p0 = m_path[0];
      const vec2 p1 = m_path[1];
      const vec2 n = Math::normal(p0, p1);

      add_cap(last_point, front_start, last_dir, radius, stroke.cap, *contour);
    }

    return fill;
#endif
    }
#endif

  void PathBuilder::flatten(const rect& clip, const float tolerance, std::vector<vec4>& sink) const {
    GK_TOTAL("PathBuilder::flatten");

    if (m_path.empty()) {
      return;
    }

    const float coverage = Math::rect_rect_intersection_area(m_bounding_rect, clip) / m_bounding_rect.area();

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
