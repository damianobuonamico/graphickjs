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
    const math::Vec2<T> p2 = math::lerp(r1, r2, t1);

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
    const math::Vec2<T> qr2 = math::lerp(q2, r2, t1);

    const math::Vec2<T> rs1 = math::lerp(r1, s1, t1);
    const math::Vec2<T> rs2 = math::lerp(r2, s2, t1);

    const math::Vec2<T> p1 = math::lerp(qr1, rs1, t1);
    const math::Vec2<T> p2 = math::lerp(qr2, rs2, t1);

    const math::Vec2<T> q = math::lerp(qr1, rs1, t2);
    const math::Vec2<T> r = math::lerp(qr2, rs2, t2);

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
    const math::Vec2<T> p2 = math::lerp(r1, r2, t1);

    const math::Vec2<T> q = math::lerp(q1, r1, t2);

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
    const math::Vec2<T> qr2 = math::lerp(q2, r2, t1);

    const math::Vec2<T> rs1 = math::lerp(r1, s1, t1);
    const math::Vec2<T> rs2 = math::lerp(r2, s2, t1);

    const math::Vec2<T> p1 = math::lerp(qr1, rs1, t1);
    const math::Vec2<T> p2 = math::lerp(qr2, rs2, t1);

    const math::Vec2<T> q = math::lerp(qr1, rs1, t2);
    const math::Vec2<T> r = math::lerp(qr2, rs2, t2);

    return CubicBezier<T>(p1, q, r, p2);
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

}
