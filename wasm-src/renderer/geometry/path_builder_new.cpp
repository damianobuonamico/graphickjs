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

  /* -- Static -- */

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

  void PathBuilder::offset_quadratic(vec2 p0, vec2 p1, vec2 p2, const float width, const float tolerance, QuadraticPath& sink) const {
    sink.line_to(p0 + Math::normal(p1, p0) * width);

    while (true) {
      const vec2 a = 2.0f * (p0 - 2.0f * p1 + p2);
      const vec2 b = 2.0f * (p1 - p0);

      const float t = tolerance * (b.x * b.x + b.y * b.y) /
        (std::abs(a.x * b.y + a.y * b.x) - tolerance * (a.x * b.x + a.y * b.y));

      if (!Math::is_normalized(t, false)) {
        break;
      }

      const auto [p, q1, q2] = Math::split_quadratic(p0, p1, p2, t);

      const vec2 n0 = Math::normal(q1, p0);
      const vec2 n1 = Math::normal(p, q1);
      const vec2 n = Math::normalize(n0 + n1);

      sink.quadratic_to(q1 + 2.0f * n * width, p + n1 * width);

      p0 = p;
      p1 = q2;
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
