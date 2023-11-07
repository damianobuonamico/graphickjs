#include "contour.h"

#include "../../math/vector.h"

namespace Graphick::Renderer::Geometry {

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

}
