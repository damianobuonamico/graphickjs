#include "contour.h"

#include "../../math/vector.h"
#include "../../math/math.h"

#include "../../utils/console.h"

namespace Graphick::Renderer::Geometry {

#ifdef USE_F8x8
  static constexpr float tolerance = 0.25f;

  static double vec_angle(const double ux, const double uy, const double vx, const double vy) {
    double dot = ux * vx + uy * vy;

    if (dot > 1.0) dot = 1.0;
    if (dot < -1.0) dot = -1.0;

    const double sign = (ux * vy - uy * vx) < 0.0 ? -1.0 : 1.0;

    return sign * std::acos(dot);
  }

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

  void Contour::add_cap(const dvec2 from, const dvec2 to, const dvec2 n, const double radius, const LineCap cap) {
    switch (cap) {
    case LineCap::Butt: {
      line_to(to);
      break;
    }
    case LineCap::Square: {
      dvec2 dir = { -n.y * radius, n.x * radius };

      line_to(from + dir);
      line_to(to + dir);
      line_to(to);

      break;
    }
    case LineCap::Round: {
      arc(from, radius, to);
      break;
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

  void Contour::arc(const dvec2 from, const double radius, const dvec2 to) {
#if 1
    const dvec2 c = from + (to - from) / 2.0;

    const double ang1 = std::atan2(from.y - c.y, from.x - c.x);
    const double ang2 = std::atan2(to.y - c.y, to.x - c.x);
    const double diff = std::abs(ang2 - ang1);
    const double dt = std::sqrtf(0.5 * tolerance / radius);

    const int segments = std::max(static_cast<int>(std::ceil(diff / MATH_PI / dt)), diff >= MATH_PI - 0.1 ? (diff >= MATH_TWO_PI - 0.1 ? 3 : 2) : 1);
    const double inc = diff / segments;

    for (int i = 0; i <= segments; ++i) {
      const double angle = ang1 + i * inc;
      const dvec2 point = c + dvec2(radius * std::cos(angle), radius * std::sin(angle));

      line_to(point);
    }

    console::log("segs", segments);
#else
    const double px = from.x;
    const double py = from.y;

    const double pxp = (px - to.x) / 2.0;
    const double pyp = (py - to.y) / 2.0;

    if (pxp == 0.0 && pyp == 0.0) return;

    double r = radius;

    const double lambda = (pxp * pxp + pyp * pyp) / (r * r);

    if (lambda > 1.0) {
      const double s = std::sqrt(lambda);
      r *= s;
    }

    /* Arc size is always small and arc sweep is always positive */

    const double rsq = r * r;
    const double pxpsq = pxp * pxp;
    const double pypsq = pyp * pyp;

    double radicant = rsq - pypsq - pxpsq;

    if (radicant < 0.0) radicant = 0.0;

    radicant /= pxpsq + pypsq;
    radicant = std::sqrt(radicant);

    const double cxp = radicant * pyp;
    const double cyp = -radicant * pxp;
    const double cx = cxp + (px + to.x) / 2.0;
    const double cy = cyp + (py + to.y) / 2.0;
    const double vx1 = (pxp - cxp) / r;
    const double vy1 = (pyp - cyp) / r;
    const double vx2 = (-pxp - cxp) / r;
    const double vy2 = (-pyp - cyp) / r;

    double ang1 = vec_angle(1.0, 0.0, vx1, vy1);
    double ang2 = vec_angle(vx1, vy1, vx2, vy2);

    if (ang2 < 0.0) ang2 += 2.0 * MATH_PI;

    double ratio = 2.0 * std::abs(ang2) / MATH_PI;

    if (std::abs(1.0 - ratio) < 1e-7) ratio = 1.0;

    const int segments = std::max(static_cast<int>(std::ceil(ratio)), 1);

    ang2 /= segments;

    double a;

    if (ang2 == 1.5707963267948966) a = 0.551915024494;
    else if (ang2 == -1.5707963267948966)a = -0.551915024494;
    else a = 4.0 / 3.0 * std::tan(ang2 / 4.0);

    for (int i = 0; i < segments; i++) {
      const double x1 = std::cos(ang1);
      const double y1 = std::sin(ang1);
      const double x2 = std::cos(ang1 + ang2);
      const double y2 = std::sin(ang1 + ang2);

      const dvec2 c1 = { cx + (x1 - y1 * a) * r, cy + (y1 + x1 * a) * r };
      const dvec2 c2 = { cx + (x2 + y2 * a) * r, cy + (y2 - x2 * a) * r };
      const dvec2 p = { x2 * r, y2 * r };

      cubic_to(c1, c2, p);

      ang1 += ang2;
    }
#endif
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
