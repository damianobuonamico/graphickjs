#include "contour.h"

#include "../../math/vector.h"
#include "../../math/math.h"

#include "../../utils/console.h"

namespace Graphick::Renderer::Geometry {

#ifdef USE_F8x8
  static constexpr unsigned int curve_recursion_limit = 8;

  static constexpr double curve_angle_tolerance_epsilon = 0.01;
  static constexpr double curve_collinearity_epsilon = 1e-30;
  static constexpr double curve_distance_epsilon = 1e-30;
  static constexpr double tolerance = 0.25;

  void Contour::move_to(const f24x8x2 p0) {
    m_p0 = p0;
    points.push_back(p0);
  }

  void Contour::move_to(const dvec2 p0) {
    m_p0 = { Math::double_to_f24x8(p0.x), Math::double_to_f24x8(p0.y) };
    m_d_p0 = p0;

    points.push_back(m_p0);
  }

  void Contour::line_to(const f24x8x2 p3) {
    m_p0 = p3;
    points.push_back(p3);
  }

  void Contour::line_to(const dvec2 p3) {
    m_p0 = { Math::double_to_f24x8(p3.x), Math::double_to_f24x8(p3.y) };
    m_d_p0 = p3;

    points.push_back(m_p0);
  }

  void Contour::cubic_to(const f24x8x2 p1, const f24x8x2 p2, const f24x8x2 p3) {
    GK_TOTAL("non_recursive");

    // TODO: avoid unnecessary casts
    vec2 fp0 = { Math::f24x8_to_float(m_p0.x), Math::f24x8_to_float(m_p0.y) };
    vec2 fp1 = { Math::f24x8_to_float(p1.x), Math::f24x8_to_float(p1.y) };
    vec2 fp2 = { Math::f24x8_to_float(p2.x), Math::f24x8_to_float(p2.y) };
    vec2 fp3 = { Math::f24x8_to_float(p3.x), Math::f24x8_to_float(p3.y) };

    vec2 a = -fp0 + 3.0f * fp1 - 3.0f * fp2 + fp3;
    vec2 b = 3.0f * fp0 - 6.0f * fp1 + 3.0f * fp2;
    vec2 c = -3.0f * fp0 + 3.0f * fp1;
    vec2 p;

    float conc = std::max(Math::length(b), Math::length(a + b));
    float dt = std::sqrtf((std::sqrtf(8.0f) * tolerance) / conc);
    float t = dt;

    points.reserve(static_cast<int>(1.0 / dt) + 1);

    console::log("curve", static_cast<int>(1.0 / dt) + 1);

    while (t < 1.0f) {
      float t_sq = t * t;

      p = a * t_sq * t + b * t_sq + c * t + fp0;
      points.emplace_back(Math::float_to_f24x8(p.x), Math::float_to_f24x8(p.y));

      t += dt;
    }

    points.push_back(p3);

    m_p0 = p3;
  }

  void Contour::cubic_to(const dvec2 p1, const dvec2 p2, const dvec2 p3) {
    GK_TOTAL("non_recursive");

    dvec2 a = -m_d_p0 + 3.0f * p1 - 3.0f * p2 + p3;
    dvec2 b = 3.0f * m_d_p0 - 6.0f * p1 + 3.0f * p2;
    dvec2 c = -3.0f * m_d_p0 + 3.0f * p1;
    dvec2 p;

    double conc = std::max(std::hypot(b.x, b.y), std::hypot(a.x + b.x, a.y + b.y));
    double dt = std::sqrtf((std::sqrt(8.0) * tolerance) / conc);
    double t = dt;

    points.reserve(static_cast<int>(1.0f / dt) + 1);

    while (t < 1.0f) {
      double t_sq = t * t;

      p = a * t_sq * t + b * t_sq + c * t + m_d_p0;
      points.emplace_back(Math::float_to_f24x8(p.x), Math::float_to_f24x8(p.y));

      t += dt;
    }

    m_d_p0 = p3;
    m_p0 = { Math::float_to_f24x8(p3.x), Math::float_to_f24x8(p3.y) };

    points.push_back(m_p0);
  }

  void Contour::offset_cubic(const dvec2 p0, const dvec2 p1, const dvec2 p2, const dvec2 p3, const dvec2 end_normal, const double radius) {
    recursive_cubic(p0.x, p0.y, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, 0, curve_distance_epsilon, curve_angle_tolerance_epsilon, 0.0);
    line_to(p3 + end_normal * radius);
  }

  void Contour::add_cap(const dvec2 from, const dvec2 to, const dvec2 n, const double radius, const LineCap cap) {
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
  }

  void Contour::add_join(const dvec2 from, const dvec2 to, const dvec2 pivot, const dvec2 from_normal, const dvec2 to_normal, const double radius, const double inv_miter_limit, LineJoin join) {
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
  }

  // void Contour::offset_segment(const f24x8x2 p3, const f24x8 radius) {}

  // void Contour::offset_segment(const f24x8x2 p1, const f24x8x2 p2, const f24x8x2 p3, const f24x8 radius) {}

  void Contour::close() {
    if (!points.empty() && (points[0].x != points[points.size() - 1].x || points[0].y != points[points.size() - 1].y)) {
      points.push_back(points[0]);
    }
  }

  void Contour::reverse() {
    std::reverse(points.begin(), points.end());
  }

  void Contour::arc(const dvec2 center, const dvec2 from, const double radius, const dvec2 to) {
    const double ang1 = std::atan2(from.y - center.y, from.x - center.x);
    const double ang2 = std::atan2(to.y - center.y, to.x - center.x);
    const double dt = std::sqrtf(0.5 * tolerance / radius);

    double diff = std::abs(ang2 - ang1);
    if (diff > MATH_PI) diff = MATH_TWO_PI - diff;

    const int segments = std::max(static_cast<int>(std::ceil(diff / (MATH_PI * dt))), diff >= MATH_PI - 0.1 ? (diff >= MATH_TWO_PI - 0.1 ? 3 : 2) : 1);
    const double inc = diff / segments;

    for (int i = 0; i <= segments; ++i) {
      const double angle = ang1 + i * inc;
      const dvec2 point = center + dvec2(radius * std::cos(angle), radius * std::sin(angle));

      line_to(point);
    }

    console::log("segs", segments);
  }

  void Contour::recursive_cubic(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4, unsigned int level, const double distance_tolerance, const double angle_tolerance, const double cusp_limit) {
    if (level > curve_recursion_limit) return;

    /* Calculate all the mid-points of the line segments */

    double x12 = (x1 + x2) / 2.0;
    double y12 = (y1 + y2) / 2.0;
    double x23 = (x2 + x3) / 2.0;
    double y23 = (y2 + y3) / 2.0;
    double x34 = (x3 + x4) / 2.0;
    double y34 = (y3 + y4) / 2.0;
    double x123 = (x12 + x23) / 2.0;
    double y123 = (y12 + y23) / 2.0;
    double x234 = (x23 + x34) / 2.0;
    double y234 = (y23 + y34) / 2.0;
    double x1234 = (x123 + x234) / 2.0;
    double y1234 = (y123 + y234) / 2.0;

    /* Enforce subdivision first time */

    if (level > 0) {
      /* Try to approximate the full cubic curve by a single straight line */

      double dx = x4 - x1;
      double dy = y4 - y1;

      double d2 = std::fabs(((x2 - x4) * dy - (y2 - y4) * dx));
      double d3 = std::fabs(((x3 - x4) * dy - (y3 - y4) * dx));

      double da1, da2;

      if (d2 > curve_collinearity_epsilon && d3 > curve_collinearity_epsilon) {
        /* Regular care */

        if ((d2 + d3) * (d2 + d3) <= distance_tolerance * (dx * dx + dy * dy)) {
          /* If the curvature doesn't exceed the distance_tolerance value we tend to finish subdivisions. */

          if (angle_tolerance < curve_angle_tolerance_epsilon) {
            line_to(dvec2{ x1234, y1234 });
            // m_points.add(point_type(x1234, y1234));
            return;
          }

          /* Angle & Cusp Condition */

          double a23 = atan2(y3 - y2, x3 - x2);

          da1 = fabs(a23 - atan2(y2 - y1, x2 - x1));
          da2 = fabs(atan2(y4 - y3, x4 - x3) - a23);

          if (da1 >= MATH_PI) da1 = MATH_TWO_PI - da1;
          if (da2 >= MATH_PI) da2 = MATH_TWO_PI - da2;

          if (da1 + da2 < angle_tolerance) {
            /* Finally we can stop the recursion */

            line_to(dvec2{ x1234, y1234 });
            // m_points.add(point_type(x1234, y1234));
            return;
          }

          if (cusp_limit != 0.0) {
            if (da1 > cusp_limit) {
              line_to(dvec2{ x2, y2 });
              // m_points.add(point_type(x2, y2));
              return;
            }

            if (da2 > cusp_limit) {
              line_to(dvec2{ x3, y3 });
              // m_points.add(point_type(x3, y3));
              return;
            }
          }
        }
      } else if (d2 > curve_collinearity_epsilon) {
        /* p1,p3,p4 are collinear, p2 is considerable */

        if (d2 * d2 <= distance_tolerance * (dx * dx + dy * dy)) {
          if (angle_tolerance < curve_angle_tolerance_epsilon) {
            line_to(dvec2{ x1234, y1234 });
            // m_points.add(point_type(x1234, y1234));
            return;
          }

          /* Angle Condition */

          da1 = fabs(atan2(y3 - y2, x3 - x2) - atan2(y2 - y1, x2 - x1));
          if (da1 >= MATH_PI) da1 = MATH_TWO_PI - da1;

          if (da1 < angle_tolerance)
          {
            line_to(dvec2{ x2, y2 });
            line_to(dvec2{ x3, y3 });
            // m_points.add(point_type(x2, y2));
            // m_points.add(point_type(x3, y3));
            return;
          }

          if (cusp_limit != 0.0)
          {
            if (da1 > cusp_limit)
            {
              line_to(dvec2{ x2, y2 });
              // m_points.add(point_type(x2, y2));
              return;
            }
          }
        } else if (d3 > curve_collinearity_epsilon) {
          /* p1,p2,p4 are collinear, p3 is considerable */

          if (d3 * d3 <= distance_tolerance * (dx * dx + dy * dy)) {
            if (angle_tolerance < curve_angle_tolerance_epsilon) {
              line_to(dvec2{ x1234, y1234 });
              // m_points.add(point_type(x1234, y1234));
              return;
            }

            // Angle Condition
            //----------------------
            da1 = std::fabs(std::atan2(y4 - y3, x4 - x3) - std::atan2(y3 - y2, x3 - x2));
            if (da1 >= MATH_PI) da1 = MATH_TWO_PI - da1;

            if (da1 < angle_tolerance) {
              // m_points.add(point_type(x2, y2));
              // m_points.add(point_type(x3, y3));
              line_to(dvec2{ x2, y2 });
              line_to(dvec2{ x3, y3 });
              return;
            }

            if (cusp_limit != 0.0) {
              if (da1 > cusp_limit) {
                // m_points.add(point_type(x3, y3));
                line_to(dvec2{ x3, y3 });
                return;
              }
            }
          }
        } else {
          /* Collinear case */

          dx = x1234 - (x1 + x4) / 2;
          dy = y1234 - (y1 + y4) / 2;

          if (dx * dx + dy * dy <= distance_tolerance) {
            line_to(dvec2{ x1234, y1234 });
            // m_points.add(point_type(x1234, y1234));
            return;
          }
        }
      }
    }

    /* Continue subdivision */

    recursive_cubic(x1, y1, x12, y12, x123, y123, x1234, y1234, level + 1, distance_tolerance, angle_tolerance, cusp_limit);
    recursive_cubic(x1234, y1234, x234, y234, x34, y34, x4, y4, level + 1, distance_tolerance, angle_tolerance, cusp_limit);
  }

#else
  static constexpr float tolerance = 0.25f;

  void Contour::begin(const vec2 p0, const bool push) {
    if (push) points.push_back(p0);
    m_p0 = p0;
  }

  void Contour::push_segment(const vec2 p3) {
    points.push_back(p3);
    m_p0 = p3;
  }

  // TODO: polar parameterization vs error minimization performance
  void Contour::push_segment(const vec2 p1, const vec2 p2, const vec2 p3) {
    return push_segment(p3);

    vec2 a = -m_p0 + 3.0f * p1 - 3.0f * p2 + p3;
    vec2 b = 3.0f * m_p0 - 6.0f * p1 + 3.0f * p2;
    vec2 c = -3.0f * m_p0 + 3.0f * p1;
    vec2 p;

    float conc = std::max(Math::length(b), Math::length(a + b));
    float dt = std::sqrtf((std::sqrtf(8.0f) * tolerance) / conc);
    float t = dt;

    while (t < 1.0f) {
      float t_sq = t * t;

      p = a * t_sq * t + b * t_sq + c * t + m_p0;
      points.push_back(p);

      t += dt;
    }

    points.push_back(p3);

    m_p0 = p3;
  }

  void Contour::offset_segment(const vec2 p3, const float radius) {
    vec2 n = Math::normal(p3, m_p0) * radius;

    points.push_back(m_p0 + n);
    points.push_back(p3 + n);

    m_p0 = p3;
  }

  void Contour::offset_segment(const vec2 p1, const vec2 p2, const vec2 p3, const float radius) {
    return offset_segment(p3, radius);

    vec2 a = -m_p0 + 3.0f * p1 - 3.0f * p2 + p3;
    vec2 b = 3.0f * m_p0 - 6.0f * p1 + 3.0f * p2;
    vec2 c = -3.0f * m_p0 + 3.0f * p1;

    vec2 a_prime = 3.0f * a;
    vec2 b_prime = 2.0f * b;

    vec2 p = m_p0;
    vec2 tan = Math::is_almost_zero(c, GEOMETRY_CURVE_ERROR) ? Math::bezier_second_derivative(m_p0, p1, p2, p3, 0.0f) : c;
    vec2 n = Math::normalize_length(Math::orthogonal(tan), radius);

    points.push_back(m_p0 + n);

    for (int i = 1; i < 10; i++) {
      float t = (float)i / 10.0f;
      float t_sq = t * t;

      p = a * t_sq * t + b * t_sq + c * t + m_p0;
      tan = a_prime * t_sq + b_prime * t + c;
      n = Math::normalize_length(Math::orthogonal(tan), radius);

      points.push_back(p + n);
    }

    p = p3;
    tan = a_prime + b_prime + c;

    // TODO: inlined bezier_second_derivative
    if (Math::is_almost_zero(tan, GEOMETRY_CURVE_ERROR)) tan = -Math::bezier_second_derivative(m_p0, p1, p2, p3, 1.0f);

    n = Math::normalize_length(Math::orthogonal(tan), radius);

    points.push_back(p + n);

    m_p0 = p3;
  }

  void Contour::close() {
    if (points.empty() || points[0] == points[points.size() - 1]) return;
    points.push_back(points[0]);
}
#endif
}
