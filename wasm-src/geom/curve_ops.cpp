/**
 * @file geom/curve_ops.cpp
 * @brief This file contains the implementation of methods related to bezier curves.
 */

#include "curve_ops.h"

#include "../math/vector.h"
#include "../math/math.h"

#include "../utils/console.h"

namespace graphick::geom {

  /* -- Approximate Bounding Rectangle -- */

  template <>
  math::Rect<float> QuadraticBezier<float>::approx_bounding_rect() const {
    return geom::approx_bounding_rect(*this);
  }

  template <>
  math::Rect<double> QuadraticBezier<double>::approx_bounding_rect() const {
    return geom::approx_bounding_rect(*this);
  }

  template <>
  math::Rect<float> CubicBezier<float>::approx_bounding_rect() const {
    return geom::approx_bounding_rect(*this);
  }

  template <>
  math::Rect<double> CubicBezier<double>::approx_bounding_rect() const {
    return geom::approx_bounding_rect(*this);
  }

  /* -- Bounding Rectangle -- */

  template <typename T, typename _>
  math::Rect<T> bounding_rect(const QuadraticBezier<T>& quad) {
    math::Rect<T> bounds = math::Rect<T>::from_vectors({ quad.p0, quad.p2 });

    const auto [a, b, c] = quad.coefficients();

    for (int i = 0; i < 2; i++) {
      if (math::is_almost_zero(a[i])) continue;

      const T t = math::solve_linear(T(2) * a[i], b[i]);

      if (math::is_normalized(t, false)) {
        const math::Vec2<T> p = a * t * t + b * t + c;

        math::min(bounds.min, p, bounds.min);
        math::max(bounds.max, p, bounds.max);
      }
    }

    return bounds;
  }

  template <>
  math::Rect<float> QuadraticBezier<float>::bounding_rect() const {
    return geom::bounding_rect(*this);
  }

  template <>
  math::Rect<double> QuadraticBezier<double>::bounding_rect() const {
    return geom::bounding_rect(*this);
  }

  template <typename T, typename _>
  math::Rect<T> bounding_rect(const CubicBezier<T>& cubic) {
    math::Rect<T> bounds = math::Rect<T>::from_vectors({ cubic.p0, cubic.p3 });

    const auto [a, b, c, d] = cubic.coefficients();

    for (int i = 0; i < 2; i++) {
      if (math::is_almost_zero(a[i])) {
        if (math::is_almost_zero(b[i])) continue;

        const T t = math::solve_linear(T(2) * b[i], c[i]);

        if (math::is_normalized(t, false)) {
          const T t_sq = t * t;
          const math::Vec2<T> p = a * t_sq * t + b * t_sq + c * t + d;

          math::min(bounds.min, p, bounds.min);
          math::max(bounds.max, p, bounds.max);
        }

        continue;
      }

      const math::QuadraticSolutions t_values = math::solve_quadratic(T(3) * a[i], T(2) * b[i], c[i]);

      for (uint8_t j = 0; j < t_values.count; j++) {
        const T t = t_values.solutions[j];

        if (math::is_normalized(t, false)) {
          const T t_sq = t * t;
          const math::Vec2<T> p = a * t_sq * t + b * t_sq + c * t + d;

          math::min(bounds.min, p, bounds.min);
          math::max(bounds.max, p, bounds.max);
        }
      }
    }

    return bounds;
  }

  template <>
  math::Rect<float> CubicBezier<float>::bounding_rect() const {
    return geom::bounding_rect(*this);
  }

  template <>
  math::Rect<double> CubicBezier<double>::bounding_rect() const {
    return geom::bounding_rect(*this);
  }

  /* -- Curve Splitting -- */

  template <typename T, typename _>
  inline std::array<QuadraticBezier<T>, 2> split(const QuadraticBezier<T>& quad, const T t) {
    const math::Vec2<T> q = math::lerp(quad.p0, quad.p1, t);
    const math::Vec2<T> r = math::lerp(quad.p1, quad.p2, t);

    const math::Vec2<T> p = math::lerp(q, r, t);

    return {
      QuadraticBezier<T>(quad.p0, q, p),
      QuadraticBezier<T>(p, r, quad.p2)
    };
  }

  template <typename T, typename _>
  std::array<QuadraticBezier<T>, 3> split(const QuadraticBezier<T>& quad, const T t1, const T t2) {
    const math::Vec2<T> q1 = math::lerp(quad.p0, quad.p1, t1);
    const math::Vec2<T> q2 = math::lerp(quad.p0, quad.p1, t2);

    const math::Vec2<T> r1 = math::lerp(quad.p1, quad.p2, t1);
    const math::Vec2<T> r2 = math::lerp(quad.p1, quad.p2, t2);

    const math::Vec2<T> p1 = math::lerp(q1, r1, t1);
    const math::Vec2<T> p2 = math::lerp(q2, r2, t2);

    const math::Vec2<T> q = math::lerp(q1, r1, t2);

    return {
      QuadraticBezier<T>(quad.p0, q1, p1),
      QuadraticBezier<T>(p1, q, p2),
      QuadraticBezier<T>(p2, r2, quad.p2)
    };
  }

  template <typename T, typename _>
  std::array<CubicBezier<T>, 2> split(const CubicBezier<T>& cubic, const T t) {
    const math::Vec2<T> q = math::lerp(cubic.p0, cubic.p1, t);
    const math::Vec2<T> r = math::lerp(cubic.p1, cubic.p2, t);
    const math::Vec2<T> s = math::lerp(cubic.p2, cubic.p3, t);

    const math::Vec2<T> qr = math::lerp(q, r, t);
    const math::Vec2<T> rs = math::lerp(r, s, t);

    const math::Vec2<T> p = math::lerp(qr, rs, t);

    return {
      CubicBezier<T>(cubic.p0, q, qr, p),
      CubicBezier<T>(p, rs, s, cubic.p3)
    };
  }

  template <typename T, typename _>
  std::array<CubicBezier<T>, 3> split(const CubicBezier<T>& cubic, const T t1, const T t2) {
    const math::Vec2<T> q1 = math::lerp(cubic.p0, cubic.p1, t1);
    const math::Vec2<T> q2 = math::lerp(cubic.p0, cubic.p1, t2);

    const math::Vec2<T> r1 = math::lerp(cubic.p1, cubic.p2, t1);
    const math::Vec2<T> r2 = math::lerp(cubic.p1, cubic.p2, t2);

    const math::Vec2<T> s1 = math::lerp(cubic.p2, cubic.p3, t1);
    const math::Vec2<T> s2 = math::lerp(cubic.p2, cubic.p3, t2);

    const math::Vec2<T> qr1 = math::lerp(q1, r1, t1);
    const math::Vec2<T> qr2 = math::lerp(q2, r2, t2);

    const math::Vec2<T> rs1 = math::lerp(r1, s1, t1);
    const math::Vec2<T> rs2 = math::lerp(r2, s2, t2);

    const math::Vec2<T> p1 = math::lerp(qr1, rs1, t1);
    const math::Vec2<T> p2 = math::lerp(qr2, rs2, t2);

    const math::Vec2<T> q = math::lerp(qr1, rs1, t2);
    const math::Vec2<T> r = math::lerp(qr2, rs2, t1);

    return {
      CubicBezier<T>(cubic.p0, q1, qr1, p1),
      CubicBezier<T>(p1, q, r, p2),
      CubicBezier<T>(p2, rs2, s2, cubic.p3)
    };
  }

  /* -- Curve Extraction -- */

  template <typename T, typename _>
  QuadraticBezier<T> extract(const QuadraticBezier<T>& quad, const T t1, const T t2) {
    const math::Vec2<T> q1 = math::lerp(quad.p0, quad.p1, t1);
    const math::Vec2<T> q2 = math::lerp(quad.p0, quad.p1, t2);

    const math::Vec2<T> r1 = math::lerp(quad.p1, quad.p2, t1);
    const math::Vec2<T> r2 = math::lerp(quad.p1, quad.p2, t2);

    const math::Vec2<T> p1 = math::lerp(q1, r1, t1);
    const math::Vec2<T> p2 = math::lerp(q2, r2, t2);

    const math::Vec2<T> q = math::lerp(q2, r2, t1);

    return QuadraticBezier<T>(p1, q, p2);
  }

  template <typename T, typename _>
  CubicBezier<T> extract(const CubicBezier<T>& cubic, const T t1, const T t2) {
    const math::Vec2<T> q1 = math::lerp(cubic.p0, cubic.p1, t1);
    const math::Vec2<T> q2 = math::lerp(cubic.p0, cubic.p1, t2);

    const math::Vec2<T> r1 = math::lerp(cubic.p1, cubic.p2, t1);
    const math::Vec2<T> r2 = math::lerp(cubic.p1, cubic.p2, t2);

    const math::Vec2<T> s1 = math::lerp(cubic.p2, cubic.p3, t1);
    const math::Vec2<T> s2 = math::lerp(cubic.p2, cubic.p3, t2);

    const math::Vec2<T> qr1 = math::lerp(q1, r1, t1);
    const math::Vec2<T> qr2 = math::lerp(q2, r2, t2);

    const math::Vec2<T> rs1 = math::lerp(r1, s1, t1);
    const math::Vec2<T> rs2 = math::lerp(r2, s2, t2);

    const math::Vec2<T> p1 = math::lerp(qr1, rs1, t1);
    const math::Vec2<T> p2 = math::lerp(qr2, rs2, t2);

    const math::Vec2<T> q = math::lerp(qr1, rs1, t2);
    const math::Vec2<T> r = math::lerp(qr2, rs2, t1);

    return CubicBezier<T>(p1, q, r, p2);
  }

  /* -- Conversion -- */

  template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
  std::array<math::Vec2<T>, 3> taylor_expand(const std::array<math::Vec2<T>, 4>& coefficients, const T t0) {
    const math::Vec2<T> a = coefficients[0];
    const math::Vec2<T> b = coefficients[1];
    const math::Vec2<T> c = coefficients[2];
    const math::Vec2<T> d = coefficients[3];

    const T t0_sq = t0 * t0;
    const T t0_cb = t0_sq * t0;

    // Taylor coefficients at t=t0
    const math::Vec2<T> f = a * t0_cb + b * t0_sq + c * t0 + d;
    const math::Vec2<T> f_prime = T(3) * a * t0_sq + T(2) * b * t0 + c;
    const math::Vec2<T> f_second = T(6) * a * t0 + T(2) * b;

    // Taylor series expansion at t=t0
    return {
      f_second / T(2),
      f_prime - t0 * f_second,
      f - t0 * f_prime + t0_sq * f_second / T(2)
    };
  }

  template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
  T taylor_expansion_error(const math::Vec2<T> a, const T t0, const T tolerance) {
    const T t0_sq = t0 * t0;
    const T t0_cb = t0_sq * t0;

    const math::Vec2<T> b = -T(3) * a * t0;
    const math::Vec2<T> c = T(3) * a * t0_sq;
    const math::Vec2<T> d = -a * t0_cb;

    T t_e = T(2);

    for (uint8_t i = 0; i < 2; i++) {
      const math::CubicSolutions<T> t_errors_negative = math::solve_cubic(a[i], b[i], c[i], d[i] + tolerance);
      const math::CubicSolutions<T> t_errors_positive = math::solve_cubic(a[i], b[i], c[i], d[i] - tolerance);

      for (uint8_t i = 0; i < 3; i++) {
        const T t_negative = t_errors_negative.solutions[i];
        const T t_positive = t_errors_positive.solutions[i];

        if (t_negative > t0 && t_negative < t_e) t_e = t_negative;
        if (t_positive > t0 && t_positive < t_e) t_e = t_positive;
      }
    }

    return t_e;
  }

  template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
  T next_taylor_center(const math::Vec2<T> a, const T t_e, const T tolerance) {
    const T t_e_sq = t_e * t_e;
    const T t_e_cb = t_e_sq * t_e;

    const math::Vec2<T> b = T(3) * a * t_e;
    const math::Vec2<T> c = -T(3) * a * t_e_sq;
    const math::Vec2<T> d = a * t_e_cb;

    T t0_prime = T(2);

    for (uint8_t i = 0; i < 2; i++) {
      const math::CubicSolutions<T> t_centers_negative = math::solve_cubic(-a[i], b[i], c[i], d[i] + tolerance);
      const math::CubicSolutions<T> t_centers_positive = math::solve_cubic(-a[i], b[i], c[i], d[i] - tolerance);

      for (uint8_t i = 0; i < 3; i++) {
        const T t_negative = t_centers_negative.solutions[i];
        const T t_positive = t_centers_positive.solutions[i];

        if (t_negative > t_e && t_negative < t0_prime) t0_prime = t_negative;
        if (t_positive > t_e && t_positive < t0_prime) t0_prime = t_positive;
      }
    }

    return t0_prime;
  }

  template <typename T, typename _>
  void cubic_to_quadratics(const CubicBezier<T>& cubic, const T tolerance, QuadraticPath<T>& sink) {
    GK_TOTAL("geom::cubic_to_quadratics");

    const std::array<math::Vec2<T>, 4> cubic_coefficients = cubic.coefficients();

    T t0 = T(0);
    T t_e = T(0);

    math::Vec2<T> p2 = cubic.p0;

    while (t0 < T(1)) {
      // Taylor expansion coefficients at t=t0
      const std::array<math::Vec2<T>, 3> quad_coefficients = taylor_expand(cubic_coefficients, t0);

      // Value of t at which the Taylor approximation error equals the tolerance
      const T t_e_prime = taylor_expansion_error(cubic_coefficients[0], t0, tolerance);

      // Value of t at which the next Taylor approximation should start from
      // in order to mantain the max error at t=t_e
      const T t0_prime = next_taylor_center(cubic_coefficients[0], t_e_prime, tolerance);

      // Quadratic Bezier curve from t_e to t_e_prime
      const QuadraticBezier<T> quad = QuadraticBezier<T>::from_coefficients(quad_coefficients);
      const QuadraticBezier<T> extracted_quad = extract(quad, t_e, std::min(T(1), t_e_prime));

      if (t0 != T(0)) {
        sink.back() = math::midpoint(p2, extracted_quad.p0);
      }

      sink.quadratic_to(extracted_quad.p1, extracted_quad.p2);

      t0 = t0_prime;
      t_e = t_e_prime;
      p2 = extracted_quad.p2;
    }

    // Close the approximation with one last quadratic curve if needed
    if (t_e < T(1)) {
      const std::array<math::Vec2<T>, 3> quad_coefficients = taylor_expand(cubic_coefficients, t0);

      const QuadraticBezier<T> quad = QuadraticBezier<T>::from_coefficients(quad_coefficients);
      const QuadraticBezier<T> extracted_quad = extract(quad, t_e, T(1));

      sink.back() = math::midpoint(p2, extracted_quad.p0);

      sink.quadratic_to(extracted_quad.p1, extracted_quad.p2);
    }
  }

  template <typename T, typename _>
  std::vector<std::pair<QuadraticBezier<T>, math::Vec2<T>>> cubic_to_quadratics_with_intervals(const CubicBezier<T>& cubic) {
    const std::array<math::Vec2<T>, 4> cubic_coefficients = cubic.coefficients();

    std::vector<std::pair<QuadraticBezier<T>, math::Vec2<T>>> quads;

    T t0 = T(0);
    T t_e = T(0);

    while (t0 < T(1)) {
      // Taylor expansion coefficients at t=t0
      const std::array<math::Vec2<T>, 3> quad_coefficients = taylor_expand(cubic_coefficients, t0);

      // Value of t at which the Taylor approximation error equals the tolerance
      const T t_e_prime = taylor_expansion_error(cubic_coefficients[0], t0, T(1e-2));

      // Value of t at which the next Taylor approximation should start from
      // in order to mantain the max error at t=t_e
      const T t0_prime = next_taylor_center(cubic_coefficients[0], t_e_prime, T(1e-2));

      // Quadratic Bezier curve from t_e to t_e_prime
      const QuadraticBezier<T> quad = QuadraticBezier<T>::from_coefficients(quad_coefficients);
      quads.emplace_back(quad, math::Vec2<T>{ t_e, std::min(T(1), t_e_prime) });

      t0 = t0_prime;
      t_e = t_e_prime;
    }

    // Close the approximation with one last quadratic curve if needed
    if (t_e < T(1)) {
      const std::array<math::Vec2<T>, 3> quad_coefficients = taylor_expand(cubic_coefficients, t0);
      const QuadraticBezier<T> quad = QuadraticBezier<T>::from_coefficients(quad_coefficients);

      quads.emplace_back(quad, math::Vec2<T>{ t_e, T(1) });
    }

    return quads;
  }

  /* -- Winding Number -- */

  template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
  static inline int winding_of(const QuadraticBezier<T>& quad, const math::Vec2<T> p) {
    // TODO: implement the rendering algorithm

    return 0;
  }

  template <>
  int QuadraticPath<float>::winding_of(const vec2 p) const {
    if (points.size() < 3) {
      return 0;
    }

    int winding = 0;

    for (size_t i = 0; i < size(); i++) {
      winding += geom::winding_of(
        QuadraticBezier<float>{ points[i * 2], points[i * 2 + 1], points[i * 2 + 2] }, p
      );
    }

    return winding;
  }

  template <>
  int QuadraticPath<double>::winding_of(const dvec2 p) const {
    if (points.size() < 3) {
      return 0;
    }

    int winding = 0;

    for (size_t i = 0; i < size(); i++) {
      winding += geom::winding_of(
        QuadraticBezier<double>{ points[i * 2], points[i * 2 + 1], points[i * 2 + 2] }, p
      );
    }

    return winding;
  }

  /* -- Template Instantiation -- */

  template math::Rect<float> bounding_rect(const QuadraticBezier<float>& quad);
  template math::Rect<double> bounding_rect(const QuadraticBezier<double>& quad);

  template math::Rect<float> bounding_rect(const CubicBezier<float>& cubic);
  template math::Rect<double> bounding_rect(const CubicBezier<double>& cubic);

  template std::array<QuadraticBezier<float>, 2> split(const QuadraticBezier<float>& quad, const float t);
  template std::array<QuadraticBezier<double>, 2> split(const QuadraticBezier<double>& quad, const double t);

  template std::array<QuadraticBezier<float>, 3> split(const QuadraticBezier<float>& quad, const float t1, const float t2);
  template std::array<QuadraticBezier<double>, 3> split(const QuadraticBezier<double>& quad, const double t1, const double t2);

  template std::array<CubicBezier<float>, 2> split(const CubicBezier<float>& cubic, const float t);
  template std::array<CubicBezier<double>, 2> split(const CubicBezier<double>& cubic, const double t);

  template std::array<CubicBezier<float>, 3> split(const CubicBezier<float>& cubic, const float t1, const float t2);
  template std::array<CubicBezier<double>, 3> split(const CubicBezier<double>& cubic, const double t1, const double t2);

  template QuadraticBezier<float> extract(const QuadraticBezier<float>& cubic, const float t1, const float t2);
  template QuadraticBezier<double> extract(const QuadraticBezier<double>& cubic, const double t1, const double t2);

  template CubicBezier<float> extract(const CubicBezier<float>& cubic, const float t1, const float t2);
  template CubicBezier<double> extract(const CubicBezier<double>& cubic, const double t1, const double t2);

  template void cubic_to_quadratics(const CubicBezier<float>& cubic, const float tolerance, QuadraticPath<float>& sink);
  template void cubic_to_quadratics(const CubicBezier<double>& cubic, const double tolerance, QuadraticPath<double>& sink);

  template std::vector<std::pair<QuadraticBezier<float>, math::Vec2<float>>> cubic_to_quadratics_with_intervals(const CubicBezier<float>& cubic);
  template std::vector<std::pair<QuadraticBezier<double>, math::Vec2<double>>> cubic_to_quadratics_with_intervals(const CubicBezier<double>& cubic);

}
