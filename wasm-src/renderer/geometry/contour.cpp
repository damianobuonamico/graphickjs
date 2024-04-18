/**
 * @file contour.cpp
 * @brief Contains the implementation of the Contour and OutlineContour classes.
 *
 * @todo avoid unnecessary casts (from Renderer::Tiler to these methods)
 */

#include "contour.h"

#include "../../math/vector.h"
#include "../../math/math.h"

#include "../../utils/console.h"

namespace graphick::renderer::geometry {

  static constexpr unsigned int curve_recursion_limit = 32;

  static constexpr double curve_angle_tolerance_epsilon = 0.01;
  static constexpr double curve_collinearity_epsilon = 1e-30;
  static constexpr double curve_cusp_limit_t = 1e-5;
  static constexpr double curve_cusp_limit = 1e-3;
  static constexpr double min_angle_tolerance = 0.1;

  /* -- Contour -- */

  void Contour::move_to(const f24x8x2 p0) {
    m_p0 = p0;
    points.push_back(p0);
  }

  void Contour::move_to(const dvec2 p0) {
#if 0
    m_p0 = { math::double_to_f24x8(p0.x), math::double_to_f24x8(p0.y) };
    m_d_p0 = p0;

    points.push_back(m_p0);
#endif
  }

  void Contour::line_to(const f24x8x2 p1) {
    m_p0 = p1;
    points.push_back(p1);
  }

  void Contour::line_to(const dvec2 p1) {
#if 0
    m_p0 = { math::double_to_f24x8(p1.x), math::double_to_f24x8(p1.y) };
    m_d_p0 = p1;

    points.push_back(m_p0);
#endif
  }

  void Contour::cubic_to(const f24x8x2 p1, const f24x8x2 p2, const f24x8x2 p3) {
#if 0
    GK_TOTAL("Contour::cubic_to");

    vec2 fp0 = { math::f24x8_to_float(m_p0.x), math::f24x8_to_float(m_p0.y) };
    vec2 fp1 = { math::f24x8_to_float(p1.x), math::f24x8_to_float(p1.y) };
    vec2 fp2 = { math::f24x8_to_float(p2.x), math::f24x8_to_float(p2.y) };
    vec2 fp3 = { math::f24x8_to_float(p3.x), math::f24x8_to_float(p3.y) };

    vec2 a = -fp0 + 3.0f * fp1 - 3.0f * fp2 + fp3;
    vec2 b = 3.0f * fp0 - 6.0f * fp1 + 3.0f * fp2;
    vec2 c = -3.0f * fp0 + 3.0f * fp1;
    vec2 p;

    float conc = std::max(math::length(b), math::length(a + b));
    float dt = std::sqrtf((std::sqrtf(8.0f) * m_tolerance) / conc);
    float t = dt;

    points.reserve(static_cast<int>(1.0 / dt) + 1);

    while (t < 1.0f) {
      float t_sq = t * t;

      p = a * t_sq * t + b * t_sq + c * t + fp0;
      points.emplace_back(math::float_to_f24x8(p.x), math::float_to_f24x8(p.y));

      t += dt;
    }

    points.push_back(p3);

    m_p0 = p3;
#endif
  }

  void Contour::cubic_to(const dvec2 p1, const dvec2 p2, const dvec2 p3) {
#if 0
    GK_TOTAL("Contour::cubic_to");

    dvec2 a = -m_d_p0 + 3.0 * p1 - 3.0 * p2 + p3;
    dvec2 b = 3.0 * m_d_p0 - 6.0 * p1 + 3.0 * p2;
    dvec2 c = -3.0 * m_d_p0 + 3.0 * p1;
    dvec2 p;

    double conc = std::max(std::hypot(b.x, b.y), std::hypot(a.x + b.x, a.y + b.y));
    double dt = std::sqrtf((std::sqrt(8.0) * m_tolerance) / conc);
    double t = dt;

    points.reserve(static_cast<int>(1.0f / dt) + 1);

    while (t < 1.0f) {
      double t_sq = t * t;

      p = a * t_sq * t + b * t_sq + c * t + m_d_p0;
      points.emplace_back(math::float_to_f24x8(p.x), math::float_to_f24x8(p.y));

      t += dt;
    }

    m_d_p0 = p3;
    m_p0 = { math::float_to_f24x8(p3.x), math::float_to_f24x8(p3.y) };

    points.push_back(m_p0);
#endif
  }

  std::unique_ptr<Contour::Parameterization> Contour::offset_cubic(const dvec2 p0, const dvec2 p1, const dvec2 p2, const dvec2 p3, const dvec2 end_normal, const double radius) {
#if 0
    GK_TOTAL("Contour::offset_cubic");

    std::unique_ptr<Parameterization> parameterization = std::make_unique<Parameterization>();

    const double angular_tolerance = radius > 0.5 ? std::max(2.0 * std::acos(std::max(radius - m_tolerance, 0.0) / radius), min_angle_tolerance) : 0.0;

    const dvec2 a = -p0 + 3.0 * p1 - 3.0 * p2 + p3;
    const dvec2 b = 3.0 * p0 - 6.0 * p1 + 3.0 * p2;
    const dvec2 c = -3.0 * p0 + 3.0 * p1;

    const double t_cusp = -0.5 * (a.y * c.x - a.x * c.y) / (a.y * b.x - a.x * b.y);
    const double radix = t_cusp * t_cusp - (b.y * c.x - b.x * c.y) / (a.y * b.x - a.x * b.y) / 3;

    if (t_cusp > curve_cusp_limit_t && t_cusp < 1.0 - curve_cusp_limit_t && std::abs(radix) <= curve_cusp_limit) {
      const dvec2 p01 = math::lerp(p0, p1, t_cusp - curve_cusp_limit_t);
      const dvec2 p12 = math::lerp(p1, p2, t_cusp);
      const dvec2 p23 = math::lerp(p2, p3, t_cusp + curve_cusp_limit_t);

      const dvec2 p012 = math::lerp(p01, p12, t_cusp - curve_cusp_limit_t);
      const dvec2 p123 = math::lerp(p12, p23, t_cusp + curve_cusp_limit_t);

      const dvec2 p0123a = math::lerp(p012, p123, t_cusp - curve_cusp_limit_t);
      const dvec2 p0123b = math::lerp(p012, p123, t_cusp + curve_cusp_limit_t);

      recursive_cubic_offset(p0, p01, p012, p0123a, 1, angular_tolerance, *parameterization);
      recursive_cubic_offset(p0123b, p123, p23, p3, 1, angular_tolerance, *parameterization);
    } else {
      recursive_cubic_offset(p0, p1, p2, p3, 0, angular_tolerance, *parameterization);
    }

    for (auto& [p, n] : *parameterization) {
      line_to(p + n * radius);
    }

    line_to(p3 + end_normal * radius);

    return std::move(parameterization);
#endif
    return {};
  }

  void Contour::offset_cubic(const Parameterization& parameterization, const dvec2 end_point, const double radius) {
    for (auto it = parameterization.rbegin(); it != parameterization.rend(); ++it) {
      line_to(it->first - it->second * radius);
    }

    line_to(end_point);
  }

  void Contour::add_cap(const dvec2 from, const dvec2 to, const dvec2 n, const double radius, const LineCap cap) {
#if 0
    switch (cap) {
    case LineCap::Butt: {
      line_to(to);
      break;
    }
    case LineCap::Round: {
      arc(from + (to - from) / 2.0, from, radius, to);
      break;
    }
    case LineCap::Square: {
      dvec2 dir = { -n.y * radius, n.x * radius };

      line_to(from + dir);
      line_to(to + dir);
      line_to(to);

      break;
    }
    }
#endif
  }

  void Contour::add_join(const dvec2 from, const dvec2 to, const dvec2 pivot, const dvec2 from_normal, const dvec2 to_normal, const double radius, const double inv_miter_limit, LineJoin join) {
#if 0
    const dvec2 center = from - from_normal * radius;
    const dvec2 a = from - center;
    const dvec2 b = to - center;

    const double dot = a.x * b.x + a.y * b.y;
    const double cross = a.x * b.y - a.y * b.x;

    double ang = std::atan2(cross, dot);

    if (ang < 0.0) ang += MATH_TWO_PI;
    if (ang >= MATH_PI) join = LineJoin::Bevel;

    switch (join) {
    case LineJoin::Bevel: {
      line_to(to);
      break;
    }
    case LineJoin::Round: {
      arc(center, from, radius, to);
      break;
    }
    case LineJoin::Miter: {
      double dot = from_normal.x * to_normal.x + from_normal.y * to_normal.y;
      double sin_half = std::sqrt((1.0 + dot) * 0.5);

      if (sin_half < inv_miter_limit) {
        line_to(to);
        break;
      } else {
        dvec2 mid = from_normal + to_normal;
        double l = radius / (sin_half * std::hypot(mid.x, mid.y));
        dvec2 p = pivot + mid * l;

        line_to(p);
        line_to(to);
        break;
      }
    }
    }
#endif
  }

  void Contour::close() {
    if (!points.empty() && (points[0].x != points[points.size() - 1].x || points[0].y != points[points.size() - 1].y)) {
      points.push_back(points[0]);
    }
  }

  void Contour::reverse() {
    std::reverse(points.begin(), points.end());
  }

  int Contour::winding_of(const f24x8x2 point) {
#if 0
    if (points.size() < 3) return 0;

    int winding = 0;

    f24x8x2 p0 = points[0];

    for (f24x8x2 p1 : points) {
      f24x8 min_y = std::min(p0.y, p1.y);
      f24x8 max_y = std::max(p0.y, p1.y);

      if (min_y > point.y || max_y <= point.y || std::min(p0.x, p1.x) > point.x || p0.y == p1.y) {
        p0 = p1;
        continue;
      }

      f24x8 d = p1.y - p0.y;
      f24x8 t = (point.y - p0.y) * FRACUNIT / d;
      f24x8 x = p0.x + t * (p1.x - p0.x) / FRACUNIT;

      if (x <= point.x) {
        winding += d > 0 ? 1 : -1;
      }

      p0 = p1;
    }

    return winding;
#endif
    return 0;
  }

  void Contour::arc(const dvec2 center, const dvec2 from, const double radius, const dvec2 to) {
#if 0
    const double ang1 = std::atan2(from.y - center.y, from.x - center.x);
    const double ang2 = std::atan2(to.y - center.y, to.x - center.x);
    const double dt = std::sqrtf(0.5 * m_tolerance / radius);

    double diff = std::abs(ang2 - ang1);
    if (diff > MATH_PI) diff = MATH_TWO_PI - diff;

    const int segments = std::max(static_cast<int>(std::ceil(diff / (MATH_PI * dt))), diff >= MATH_PI - 0.1 ? (diff >= MATH_TWO_PI - 0.1 ? 3 : 2) : 1);
    const double inc = diff / segments;

    for (int i = 0; i <= segments; ++i) {
      const double angle = ang1 + i * inc;
      const dvec2 point = center + dvec2(radius * std::cos(angle), radius * std::sin(angle));

      line_to(point);
    }
#endif
  }

  void Contour::recursive_cubic_offset(const dvec2 p0, const dvec2 p1, const dvec2 p2, const dvec2 p3, const unsigned int level, const double angular_tolerance, Parameterization& parameterization) {
#if 0
    if (level > curve_recursion_limit) return;

    /* Calculate all the mid-points of the line segments */

    dvec2 p01 = (p0 + p1) / 2.0;
    dvec2 p12 = (p1 + p2) / 2.0;
    dvec2 p23 = (p2 + p3) / 2.0;
    dvec2 p012 = (p01 + p12) / 2.0;
    dvec2 p123 = (p12 + p23) / 2.0;
    dvec2 p0123 = (p012 + p123) / 2.0;

    /* Try to approximate the full cubic curve by a single straight line */

    dvec2 d = p3 - p0;

    double tolerance_sq = m_tolerance * m_tolerance;
    double d2 = std::fabs(((p1.x - p3.x) * d.y - (p1.y - p3.y) * d.x));
    double d3 = std::fabs(((p2.x - p3.x) * d.y - (p2.y - p3.y) * d.x));
    double da1, da2, k;

    switch ((static_cast<int>(d2 > curve_collinearity_epsilon) << 1) + static_cast<int>(d3 > curve_collinearity_epsilon)) {
    case 0:
      /* All collinear or p0 == p3 */

      k = d.x * d.x + d.y * d.y;

      if (k == 0.0) {
        d2 = math::squared_distance(p0, p1);
        d3 = math::squared_distance(p2, p3);
      } else {
        k = 1 / k;
        da1 = p1.x - p0.x;
        da2 = p1.y - p0.y;
        d2 = k * (da1 * d.x + da1 * d.y);
        da1 = p2.x - p0.x;
        da2 = p2.y - p0.y;
        d3 = k * (da1 * d.x + da2 * d.y);

        if (d2 > 0 && d2 < 1 && d3 > 0 && d3 < 1) {
          /* Simple collinear case, 0---1---2---3, we can leave just two endpoints */
          return;
        }

        if (d2 <= 0) d2 = math::squared_distance(p0, p1);
        else if (d2 >= 1) d2 = math::squared_distance(p1, p3);
        else d2 = math::squared_distance(p1, p0 + d2 * d);

        if (d3 <= 0) d3 = math::squared_distance(p0, p2);
        else if (d3 >= 1) d3 = math::squared_distance(p0, p3);
        else d3 = math::squared_distance(p2, p0 + d3 * d);
      }

      if (d2 > d3) {
        if (d2 < tolerance_sq) {
          parameterization.push_back(std::make_pair(p1, math::normal(p0, p1)));
          return;
        }
      } else {
        if (d3 < tolerance_sq) {
          parameterization.push_back(std::make_pair(p2, math::normal(p2, p3)));
          return;
        }
      }

      break;
    case 1:
      /* p0, p1, p3 are collinear, p2 is significant */

      if (d3 * d3 <= tolerance_sq * (d.x * d.x + d.y * d.y)) {
        if (m_tolerance < curve_angle_tolerance_epsilon) {
          parameterization.push_back(std::make_pair(p12, math::normal(p1, p2)));
          return;
        }

        /* Angle Condition */

        da1 = std::fabs(std::atan2(p3.y - p2.y, p3.x - p2.x) - std::atan2(p2.y - p1.y, p2.x - p1.x));
        if (da1 >= MATH_PI) da1 = MATH_TWO_PI - da1;

        if (da1 < angular_tolerance) {
          parameterization.push_back(std::make_pair(p1, math::normal(p0, p2)));
          parameterization.push_back(std::make_pair(p2, math::normal(p2, p3)));
          return;
        }
      }

      break;
    case 2:
      /* p0, p2, p3 are collinear, p1 is significant */

      if (d2 * d2 <= tolerance_sq * (d.x * d.x + d.y * d.y)) {
        if (angular_tolerance < curve_angle_tolerance_epsilon) {
          parameterization.push_back(std::make_pair(p12, math::normal(p1, p2)));
          return;
        }

        /* Angle Condition */

        da1 = std::fabs(std::atan2(p2.y - p1.y, p2.x - p1.x) - std::atan2(p1.y - p0.y, p1.x - p0.x));
        if (da1 >= MATH_PI) da1 = MATH_TWO_PI - da1;

        if (da1 < angular_tolerance) {
          parameterization.push_back(std::make_pair(p1, math::normal(p0, p1)));
          parameterization.push_back(std::make_pair(p2, math::normal(p1, p2)));
          return;
        }
      }

      break;
    case 3:
      /* Regular case */

      if ((d2 + d3) * (d2 + d3) <= tolerance_sq * (d.x * d.x + d.y * d.y)) {
        /* If the curvature doesn't exceed the distance_tolerance value, we tend to finish subdivision */

        if (angular_tolerance < curve_angle_tolerance_epsilon) {
          parameterization.push_back(std::make_pair(p12, math::normal(p1, p2)));
          return;
        }

        /* Angle & Cusp Condition */

        k = std::atan2(p2.y - p1.y, p2.x - p1.x);
        da1 = std::fabs(k - std::atan2(p1.y - p0.y, p1.x - p0.x));
        da2 = std::fabs(std::atan2(p3.y - p2.y, p3.x - p2.x) - k);

        if (da1 >= MATH_PI) da1 = MATH_TWO_PI - da1;
        if (da2 >= MATH_PI) da2 = MATH_TWO_PI - da2;

        if (da1 + da2 < angular_tolerance) {
          /* Finally we can stop the recursion */

          parameterization.push_back(std::make_pair(p12, math::normal(p1, p2)));
          return;
        }
      }
      break;
    }

    recursive_cubic_offset(p0, p01, p012, p0123, level + 1, angular_tolerance, parameterization);
    recursive_cubic_offset(p0123, p123, p23, p3, level + 1, angular_tolerance, parameterization);
#endif
  }

  /* -- OutlineContour -- */

  void OutlineContour::move_to(const dvec2 p0) {
    m_p0 = p0;
    points.push_back(m_p0);
  }

  void OutlineContour::line_to(const dvec2 p1) {
    m_p0 = p1;
    points.push_back(m_p0);
  }

  void OutlineContour::cubic_to(const dvec2 p1, const dvec2 p2, const dvec2 p3) {
    GK_TOTAL("OutlineContour::cubic_to");

    dvec2 a = -m_p0 + 3.0 * p1 - 3.0 * p2 + p3;
    dvec2 b = 3.0 * m_p0 - 6.0 * p1 + 3.0 * p2;
    dvec2 c = -3.0 * m_p0 + 3.0 * p1;
    dvec2 p;

    double conc = std::max(std::hypot(b.x, b.y), std::hypot(a.x + b.x, a.y + b.y));
    double dt = std::sqrt((std::sqrt(8.0) * m_tolerance) / conc);
    double t = dt;

    points.reserve(static_cast<int>(1.0f / dt) + 1);

    while (t < 1.0f) {
      double t_sq = t * t;

      p = a * t_sq * t + b * t_sq + c * t + m_p0;
      points.emplace_back(p);

      t += dt;
    }

    m_p0 = p3;
    points.push_back(m_p0);
  }

  void OutlineContour::close() {
    if (!points.empty() && (points[0].x != points[points.size() - 1].x || points[0].y != points[points.size() - 1].y)) {
      points.push_back(points[0]);
    }
  }

}
