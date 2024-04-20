/**
 * @file geom/curve_ops.cpp
 * @brief This file contains the implementation of methods related to bezier curves.
 */

#include "curve_ops.h"

#include "../math/vector.h"
#include "../math/math.h"

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

    const math::Vec2<T> r1 = math::lerp(q1, q2, t1);
    const math::Vec2<T> r2 = math::lerp(q1, q2, t2);

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

    const math::Vec2<T> r1 = math::lerp(q1, q2, t1);
    const math::Vec2<T> r2 = math::lerp(q1, q2, t2);

    const math::Vec2<T> p1 = math::lerp(q1, r1, t1);
    const math::Vec2<T> p2 = math::lerp(r1, r2, t2);

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
  std::pair<math::Vec3<T>, math::Vec2<T>> taylor_expansion_at_t(const std::array<T, 4>& cubic_coefficients, const T t0, const T tolerance) {
    const T a = cubic_coefficients[0];
    const T b = cubic_coefficients[1];
    const T c = cubic_coefficients[2];
    const T d = cubic_coefficients[3];

    const T t0_sq = t0 * t0;
    const T t0_cb = t0_sq * t0;

    const T f = a * t0_cb + b * t0_sq + c * t0 + d;
    const T f_prime = T(3) * a * t0_sq + T(2) * b * t0 + c;
    const T f_second = T(6) * a * t0 + T(2) * b;

    // Taylor series expansion
    const math::Vec3<T> quadratic_coefficients = {
      f_second / T(2),
      f_prime - t0 * f_second,
      f - t0 * f_prime + t0_sq * f_second / T(2)
    };

    // Taylor series error
    const math::CubicSolutions<T> t_errors_negative = math::solve_cubic(
      T(1),
      -T(3) * t0,
      T(3) * t0_sq,
      -T(6) * tolerance / a - t0_cb
    );

    const math::CubicSolutions<T> t_errors_positive = math::solve_cubic(
      T(1),
      -T(3) * t0,
      T(3) * t0_sq,
      T(6) * tolerance / a - t0_cb
    );

    T t_error_negative = T(-1);
    T t_error_positive = T(2);

    for (uint8_t i = 0; i < t_errors_negative.count; i++) {
      const T t_error = t_errors_negative.solutions[i];
      const T t_delta = t_error - t0;

      if (t_delta < T(0) && (t_error_negative - t0) < t_delta) {
        t_error_negative = t_error;
      } else if (t_delta > T(0) && (t_error_positive - t0) > t_delta) {
        t_error_positive = t_error;
      }
    }

    for (uint8_t i = 0; i < t_errors_positive.count; i++) {
      const T t_error = t_errors_positive.solutions[i];
      const T t_delta = t_error - t0;

      if (t_delta < T(0) && (t_error_negative - t0) < t_delta) {
        t_error_negative = t_error;
      } else if (t_delta > T(0) && (t_error_positive - t0) > t_delta) {
        t_error_positive = t_error;
      }
    }

    return {
      quadratic_coefficients,
      { t_error_negative, t_error_positive}
    };
  }

  // TODO: handle cusp case
  template <typename T, typename _>
  std::vector<QuadraticBezier<T>> cubic_to_quadratics(const CubicBezier<T>& cubic) {
    const auto [a, b, c, d] = cubic.coefficients();

    const std::array<T, 4> x_coefficients = { a[0], b[0], c[0], d[0] };
    const std::array<T, 4> y_coefficients = { a[1], b[1], c[1], d[1] };

    // Find local min/max points
    const math::QuadraticSolutions<T> extrema = math::solve_quadratic(T(3) * a[0], T(2) * b[0], c[0]);

    std::vector<QuadraticBezier<T>> quads;
    std::vector<T> t_values = { T(0) };

    for (uint8_t i = 0; i < extrema.count; i++) {
      const T t = extrema.solutions[i];

      if (math::is_normalized(t, false)) {
        t_values.push_back(t);
      }
    }

    t_values.push_back(T(1));

    for (T t0 : t_values) {
      const auto [quad_coefficients, t_error] = taylor_expansion_at_t(x_coefficients, t0, T(1));
      const QuadraticBezier<T> quad = QuadraticBezier<T>::from_coefficients(math::Vec2<T>(quad_coefficients[0]), math::Vec2<T>(quad_coefficients[1]), math::Vec2<T>(quad_coefficients[2]));

      quads.push_back(extract(quad, std::max(T(0), t_error[0]), std::min(T(1), t_error[1])));
      // math::Vec2<T> A_quad, B_quad, C_quad;

      // for (int i = 0; i < 2; i++) {
      //   const T a = A[i];
      //   const T b = B[i];
      //   const T c = C[i];
      //   const T d = D[i];

      //   const T t0_sq = t0 * t0;
      //   const T t0_cb = t0_sq * t0;

      //   const T f = a * t0_cb + b * t0_sq + c * t0 + d;
      //   const T f_prime = T(3) * a * t0_sq + T(2) * b * t0 + c;
      //   const T f_second = T(6) * a * t0 + T(2) * b;

      //   // Taylor series expansion
      //   A_quad[i] = f_second / T(2);
      //   B_quad[i] = f_prime - t0 * f_second;
      //   C_quad[i] = f - t0 * f_prime + t0_sq * f_second / T(2);
      // }

      // quads.push_back(QuadraticBezier<T>::from_coefficients(A_quad, B_quad, C_quad));
    }

    return quads;
  }

  template <typename T, typename _>
  std::vector<std::pair<QuadraticBezier<T>, math::Vec2<T>>> cubic_to_quadratics_with_intervals(const CubicBezier<T>& cubic) {
    const auto [a, b, c, d] = cubic.coefficients();

    const std::array<T, 4> x_coefficients = { a[0], b[0], c[0], d[0] };
    const std::array<T, 4> y_coefficients = { a[1], b[1], c[1], d[1] };

    // Find local min/max points
    const math::QuadraticSolutions<T> extrema = math::solve_quadratic(T(3) * a[0], T(2) * b[0], c[0]);

    std::vector<std::pair<QuadraticBezier<T>, math::Vec2<T>>> quads;
    std::vector<T> t_values = { T(0) };

    for (uint8_t i = 0; i < extrema.count; i++) {
      const T t = extrema.solutions[i];

      if (math::is_normalized(t, false)) {
        t_values.push_back(t);
      }
    }

    t_values.push_back(T(1));

    for (T t0 : t_values) {
      const auto [quad_coefficients, t_error] = taylor_expansion_at_t(x_coefficients, t0, T(1e-1));
      const QuadraticBezier<T> quad = QuadraticBezier<T>::from_coefficients(math::Vec2<T>(quad_coefficients[0]), math::Vec2<T>(quad_coefficients[1]), math::Vec2<T>(quad_coefficients[2]));

      const T min_t = std::max(T(0), t_error[0]);
      const T max_t = std::min(T(1), t_error[1]);

      quads.emplace_back(quad, math::Vec2<T>{ min_t, max_t });
      // quads.emplace_back(extract(quad, min_t, max_t), math::Vec2<T>{ min_t, max_t });
    }

    return quads;
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

  template std::vector<QuadraticBezier<float>> cubic_to_quadratics(const CubicBezier<float>& cubic);
  template std::vector<QuadraticBezier<double>> cubic_to_quadratics(const CubicBezier<double>& cubic);

  template std::vector<std::pair<QuadraticBezier<float>, math::Vec2<float>>> cubic_to_quadratics_with_intervals(const CubicBezier<float>& cubic);
  template std::vector<std::pair<QuadraticBezier<double>, math::Vec2<double>>> cubic_to_quadratics_with_intervals(const CubicBezier<double>& cubic);

}
