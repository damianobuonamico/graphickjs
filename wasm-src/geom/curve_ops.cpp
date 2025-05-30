/**
 * @file geom/curve_ops.cpp
 * @brief This file contains the implementation of methods related to bezier curves.
 */

#include "curve_ops.h"
#include "geom.h"
#include "intersections.h"

#include "../math/vector.h"

#include "../utils/console.h"

#include <algorithm>

namespace graphick::geom {

/* -- Cubic Bezier -- */

template<typename T, typename _>
bool CubicBezier<T, _>::is_point(const T tolerance) const
{
  return (math::is_almost_equal(p0, p3, tolerance) && math::is_almost_equal(p0, p1, tolerance) &&
          math::is_almost_equal(p0, p2, tolerance));
}

template<typename T, typename _>
bool CubicBezier<T, _>::is_line(const T tolerance) const
{
  return (math::is_almost_equal(p1, p2, tolerance) && math::is_almost_equal(p1, p3, tolerance));
}

template<typename T, typename _>
math::Vec2<T> CubicBezier<T, _>::normal(const T t) const
{
  return math::normalize(raw_normal(t));
}

template<typename T, typename _>
Line<T> CubicBezier<T, _>::start_tangent() const
{
  if (math::is_almost_equal(p0, p1)) {
    if (math::is_almost_equal(p0, p2)) {
      return Line<T>(p0, p3);
    }

    return Line<T>(p0, p2);
  }

  return Line<T>(p0, p1);
}

template<typename T, typename _>
Line<T> CubicBezier<T, _>::end_tangent() const
{
  if (math::is_almost_equal(p2, p3)) {
    if (math::is_almost_equal(p1, p2)) {
      return Line<T>(p3, p0);
    }

    return Line<T>(p3, p1);
  }

  return Line<T>(p3, p2);
}

template<typename T, typename _>
math::Vec2<T> CubicBezier<T, _>::start_normal() const
{
  if (math::is_almost_equal(p0, p1)) {
    if (math::is_almost_equal(p0, p2)) {
      return math::normal(p0, p3);
    }

    return math::normal(p0, p2);
  }

  return math::normal(p0, p1);
}

template<typename T, typename _>
math::Vec2<T> CubicBezier<T, _>::end_normal() const
{
  if (math::is_almost_equal(p2, p3)) {
    if (math::is_almost_equal(p1, p2)) {
      return math::normal(p0, p3);
    }

    return math::normal(p1, p3);
  }

  return math::normal(p2, p3);
}

/* -- Line -- */

template<typename T>
math::Vec2<T> Line<T>::direction() const
{
  return math::normalize(p1 - p0);
}

template<typename T>
math::Vec2<T> Line<T>::normal() const
{
  return math::normalize(raw_normal());
}

template<typename T>
T Line<T>::angle() const
{
  const math::Vec2<T> delta = p1 - p0;

  const T theta = std::atan2(-delta.y, delta.x);
  const T theta_norm = theta < T(0) ? theta + math::two_pi<T> : theta;

  if (math::is_almost_equal(theta_norm, math::two_pi<T>)) {
    return 0;
  }

  if (math::is_almost_zero(theta_norm)) {
    // In case theta=-0, return positive zero.
    return 0;
  }

  return theta_norm;
}

template<typename T>
T Line<T>::length() const
{
  return math::length(p1 - p0);
}

/* -- Curvature -- */

template<typename T, typename _>
math::CubicSolutions<T> max_curvature(const CubicBezier<T>& cubic)
{
  const T axx = cubic.p1.x - cubic.p0.x;
  const T bxx = cubic.p2.x - T(2) * cubic.p1.x + cubic.p0.x;
  const T cxx = cubic.p3.x + T(3) * (cubic.p1.x - cubic.p2.x) - cubic.p0.x;

  const T cox0 = cxx * cxx;
  const T cox1 = T(3) * bxx * cxx;
  const T cox2 = T(2) * bxx * bxx + cxx * axx;
  const T cox3 = axx * bxx;

  const T ayy = cubic.p1.y - cubic.p0.y;
  const T byy = cubic.p2.y - T(2) * cubic.p1.y + cubic.p0.y;
  const T cyy = cubic.p3.y + T(3) * (cubic.p1.y - cubic.p2.y) - cubic.p0.y;

  const T coy0 = cyy * cyy;
  const T coy1 = T(3) * byy * cyy;
  const T coy2 = T(2) * byy * byy + cyy * ayy;
  const T coy3 = ayy * byy;

  const T coe0 = cox0 + coy0;
  const T coe1 = cox1 + coy1;
  const T coe2 = cox2 + coy2;
  const T coe3 = cox3 + coy3;

  return math::solve_cubic_normalized(coe0, coe1, coe2, coe3);
}

template<typename T, typename _>
math::QuadraticSolutions<T> inflections(const CubicBezier<T>& cubic)
{
  const T ax = cubic.p1.x - cubic.p0.x;
  const T ay = cubic.p1.y - cubic.p0.y;
  const T bx = cubic.p2.x - T(2) * cubic.p1.x + cubic.p0.x;
  const T by = cubic.p2.y - T(2) * cubic.p1.y + cubic.p0.y;
  const T cx = cubic.p3.x + T(3) * (cubic.p1.x - cubic.p2.x) - cubic.p0.x;
  const T cy = cubic.p3.y + T(3) * (cubic.p1.y - cubic.p2.y) - cubic.p0.y;

  return math::solve_quadratic_normalized(bx * cy - by * cx, ax * cy - ay * cx, ax * by - ay * bx);
}

/* -- Approximate Bounding Rectangle -- */

template<>
math::Rect<float> QuadraticBezier<float>::approx_bounding_rect() const
{
  return geom::approx_bounding_rect(*this);
}

template<>
math::Rect<double> QuadraticBezier<double>::approx_bounding_rect() const
{
  return geom::approx_bounding_rect(*this);
}

template<>
math::Rect<float> CubicBezier<float>::approx_bounding_rect() const
{
  return geom::approx_bounding_rect(*this);
}

template<>
math::Rect<double> CubicBezier<double>::approx_bounding_rect() const
{
  return geom::approx_bounding_rect(*this);
}

/* -- Bounding Rectangle -- */

template<typename T, typename _>
math::Rect<T> bounding_rect(const QuadraticBezier<T>& quad)
{
  math::Rect<T> bounds = math::Rect<T>::from_vectors({quad.p0, quad.p2});

  const auto [a, b, c] = quad.coefficients();

  for (int i = 0; i < 2; i++) {
    if (math::is_almost_zero(a[i]))
      continue;

    const T t = math::solve_linear(T(2) * a[i], b[i]);

    if (math::is_normalized(t, false)) {
      const math::Vec2<T> p = a * t * t + b * t + c;

      math::min(bounds.min, p, bounds.min);
      math::max(bounds.max, p, bounds.max);
    }
  }

  return bounds;
}

template<>
math::Rect<float> QuadraticBezier<float>::bounding_rect() const
{
  return geom::bounding_rect(*this);
}

template<>
math::Rect<double> QuadraticBezier<double>::bounding_rect() const
{
  return geom::bounding_rect(*this);
}

template<typename T, typename _>
math::Rect<T> bounding_rect(const CubicBezier<T>& cubic)
{
  math::Rect<T> bounds = math::Rect<T>::from_vectors({cubic.p0, cubic.p3});

  const auto [a, b, c, d] = cubic.coefficients();

  for (int i = 0; i < 2; i++) {
    if (math::is_almost_zero(a[i])) {
      if (math::is_almost_zero(b[i]))
        continue;

      const T t = math::solve_linear(T(2) * b[i], c[i]);

      if (math::is_normalized(t, false)) {
        const T t_sq = t * t;
        const math::Vec2<T> p = a * t_sq * t + b * t_sq + c * t + d;

        math::min(bounds.min, p, bounds.min);
        math::max(bounds.max, p, bounds.max);
      }

      continue;
    }

    const math::QuadraticSolutions t_values = math::solve_quadratic(
        T(3) * a[i], T(2) * b[i], c[i]);

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

template<>
math::Rect<float> CubicBezier<float>::bounding_rect() const
{
  return geom::bounding_rect(*this);
}

template<>
math::Rect<double> CubicBezier<double>::bounding_rect() const
{
  return geom::bounding_rect(*this);
}

template<typename T, typename _>
math::Rect<T> bounding_rect(const CubicPath<T>& path)
{
  math::Rect<T> bounds;

  for (size_t i = 0; i < path.size(); i++) {
    const math::Vec2<T> p0 = path[i * 3];
    const math::Vec2<T> p1 = path[i * 3 + 1];
    const math::Vec2<T> p2 = path[i * 3 + 2];
    const math::Vec2<T> p3 = path[i * 3 + 3];

    const CubicBezier<T> curve = CubicBezier<T>(p0, p1, p2, p3);

    bounds = math::Rect<T>::from_rects(bounds, geom::bounding_rect(curve));
  }

  return bounds;
}

template<>
math::Rect<float> CubicPath<float>::bounding_rect() const
{
  return geom::bounding_rect(*this);
}

template<>
math::Rect<double> CubicPath<double>::bounding_rect() const
{
  return geom::bounding_rect(*this);
}

/* -- Curve Splitting -- */

template<typename T, typename _>
inline std::array<QuadraticBezier<T>, 2> split(const QuadraticBezier<T>& quad, const T t)
{
  const math::Vec2<T> q = math::lerp(quad.p0, quad.p1, t);
  const math::Vec2<T> r = math::lerp(quad.p1, quad.p2, t);

  const math::Vec2<T> p = math::lerp(q, r, t);

  return {QuadraticBezier<T>(quad.p0, q, p), QuadraticBezier<T>(p, r, quad.p2)};
}

template<typename T, typename _>
std::array<QuadraticBezier<T>, 3> split(const QuadraticBezier<T>& quad, const T t1, const T t2)
{
  const math::Vec2<T> q1 = math::lerp(quad.p0, quad.p1, t1);
  const math::Vec2<T> q2 = math::lerp(quad.p0, quad.p1, t2);

  const math::Vec2<T> r1 = math::lerp(quad.p1, quad.p2, t1);
  const math::Vec2<T> r2 = math::lerp(quad.p1, quad.p2, t2);

  const math::Vec2<T> p1 = math::lerp(q1, r1, t1);
  const math::Vec2<T> p2 = math::lerp(q2, r2, t2);

  const math::Vec2<T> q = math::lerp(q1, r1, t2);

  return {QuadraticBezier<T>(quad.p0, q1, p1),
          QuadraticBezier<T>(p1, q, p2),
          QuadraticBezier<T>(p2, r2, quad.p2)};
}

template<typename T, typename _>
std::array<CubicBezier<T>, 2> split(const CubicBezier<T>& cubic, const T t)
{
  const math::Vec2<T> q = math::lerp(cubic.p0, cubic.p1, t);
  const math::Vec2<T> r = math::lerp(cubic.p1, cubic.p2, t);
  const math::Vec2<T> s = math::lerp(cubic.p2, cubic.p3, t);

  const math::Vec2<T> qr = math::lerp(q, r, t);
  const math::Vec2<T> rs = math::lerp(r, s, t);

  const math::Vec2<T> p = math::lerp(qr, rs, t);

  return {CubicBezier<T>(cubic.p0, q, qr, p), CubicBezier<T>(p, rs, s, cubic.p3)};
}

template<typename T, typename _>
std::array<CubicBezier<T>, 3> split(const CubicBezier<T>& cubic, const T t1, const T t2)
{
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

  return {CubicBezier<T>(cubic.p0, q1, qr1, p1),
          CubicBezier<T>(p1, q, r, p2),
          CubicBezier<T>(p2, rs2, s2, cubic.p3)};
}

/* -- Curve Extraction -- */

template<typename T, typename _>
QuadraticBezier<T> extract(const QuadraticBezier<T>& quad, const T t1, const T t2)
{
  const math::Vec2<T> q1 = math::lerp(quad.p0, quad.p1, t1);
  const math::Vec2<T> q2 = math::lerp(quad.p0, quad.p1, t2);

  const math::Vec2<T> r1 = math::lerp(quad.p1, quad.p2, t1);
  const math::Vec2<T> r2 = math::lerp(quad.p1, quad.p2, t2);

  const math::Vec2<T> p1 = math::lerp(q1, r1, t1);
  const math::Vec2<T> p2 = math::lerp(q2, r2, t2);

  const math::Vec2<T> q = math::lerp(q2, r2, t1);

  return QuadraticBezier<T>(p1, q, p2);
}

template<typename T, typename _>
CubicBezier<T> extract(const CubicBezier<T>& cubic, const T t1, const T t2)
{
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

template<typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
std::array<math::Vec2<T>, 3> taylor_expand(const std::array<math::Vec2<T>, 4>& coefficients,
                                           const T t0)
{
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
  return {f_second / T(2), f_prime - t0 * f_second, f - t0 * f_prime + t0_sq * f_second / T(2)};
}

template<typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
T taylor_expansion_error(const math::Vec2<T> a, const T t0, const T tolerance)
{
  const T t0_sq = t0 * t0;
  const T t0_cb = t0_sq * t0;

  const math::Vec2<T> b = -T(3) * a * t0;
  const math::Vec2<T> c = T(3) * a * t0_sq;
  const math::Vec2<T> d = -a * t0_cb;

  T t_e = T(2);

  for (uint8_t i = 0; i < 2; i++) {
    const math::CubicSolutions<T> t_errors_negative = math::solve_cubic(
        a[i], b[i], c[i], d[i] + tolerance);
    const math::CubicSolutions<T> t_errors_positive = math::solve_cubic(
        a[i], b[i], c[i], d[i] - tolerance);

    for (uint8_t i = 0; i < 3; i++) {
      const T t_negative = t_errors_negative.solutions[i];
      const T t_positive = t_errors_positive.solutions[i];

      if (t_negative > t0 && t_negative < t_e)
        t_e = t_negative;
      if (t_positive > t0 && t_positive < t_e)
        t_e = t_positive;
    }
  }

  return t_e;
}

template<typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
T next_taylor_center(const math::Vec2<T> a, const T t_e, const T tolerance)
{
  const T t_e_sq = t_e * t_e;
  const T t_e_cb = t_e_sq * t_e;

  const math::Vec2<T> b = T(3) * a * t_e;
  const math::Vec2<T> c = -T(3) * a * t_e_sq;
  const math::Vec2<T> d = a * t_e_cb;

  T t0_prime = T(2);

  for (uint8_t i = 0; i < 2; i++) {
    const math::CubicSolutions<T> t_centers_negative = math::solve_cubic(
        -a[i], b[i], c[i], d[i] + tolerance);
    const math::CubicSolutions<T> t_centers_positive = math::solve_cubic(
        -a[i], b[i], c[i], d[i] - tolerance);

    for (uint8_t i = 0; i < 3; i++) {
      const T t_negative = t_centers_negative.solutions[i];
      const T t_positive = t_centers_positive.solutions[i];

      if (t_negative > t_e && t_negative < t0_prime)
        t0_prime = t_negative;
      if (t_positive > t_e && t_positive < t0_prime)
        t0_prime = t_positive;
    }
  }

  return t0_prime;
}

template<typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
inline static T fast_cubic_first_solution(const T a, const T b, const T c, const T d)
{
  if (math::is_almost_zero(a)) {
    /* If a = 0, also b = 0. It is a linear equation. */

    return -d / c;
  }

  /* d is guaranteed to be non-zero. */
  /* Calculate coefficients of the depressed cubic equation: y^3 + py + q = 0. */
  const T p = (3.0 * a * c - b * b) / (3.0 * a * a);
  const T q = (2.0 * b * b * b - 9.0 * a * b * c + 27.0 * a * a * d) / (27.0 * a * a * a);

  /* Calculate discriminant, it is guaranteed to be positive. */
  const T discriminant = (q * q) / 4.0 + (p * p * p) / 27.0;

  const T u = std::cbrt(-q / 2.0 + std::sqrt(discriminant));

  /* One real root and two complex roots */
  const T v = std::cbrt(-q / 2.0 - std::sqrt(discriminant));
  const T real_root = u + v - b / (3.0 * a);

  return real_root;
}

template<typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
inline static math::Vec2<T> fast_cubic_first_solution(const math::Vec2<T> a,
                                                      const math::Vec2<T> b,
                                                      const math::Vec2<T> c,
                                                      const math::Vec2<T> d)
{
  /* d is guaranteed to be non-zero. */
  /* Calculate coefficients of the depressed cubic equation: y^3 + py + q = 0. */
  const math::Vec2<T> p = (T(3) * a * c - b * b) / (T(3) * a * a);
  const math::Vec2<T> q = (T(2) * b * b * b - T(9) * a * b * c + T(27) * a * a * d) /
                          (T(27) * a * a * a);

  /* Calculate discriminant, it is guaranteed to be positive. */
  const math::Vec2<T> discriminant = (q * q) / T(4) + (p * p * p) / T(27);

  math::Vec2<T> solutions;

  for (uint8_t i = 0; i < 2; i++) {
    if (math::is_almost_zero(a[i])) {
      /* If a = 0, also b = 0. It is a linear equation. */

      solutions[i] = -d[i] / c[i];
      continue;
    }

    /* One real root and two complex roots */
    const T u = std::cbrt(-q[i] / T(2) + std::sqrt(discriminant[i]));
    const T v = std::cbrt(-q[i] / T(2) - std::sqrt(discriminant[i]));

    solutions[i] = u + v - b[i] / (T(3) * a[i]);
  }

  return solutions;
}

template<typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
inline static math::Vec4<T> fast_cubic_first_solution_plus_minus(const math::Vec2<T> a,
                                                                 const math::Vec2<T> b,
                                                                 const math::Vec2<T> c,
                                                                 const math::Vec2<T> d,
                                                                 const T tolerance)
{
  const math::Vec2<T> d_plus = d + tolerance;
  const math::Vec2<T> d_minus = d - tolerance;

  /* d is guaranteed to be non-zero. */
  /* Calculate coefficients of the depressed cubic equation: y^3 + py + q = 0. */
  const math::Vec2<T> q1 = T(2) * b * b * b - T(9) * a * b * c;
  const math::Vec2<T> q2 = T(27) * a * a;
  const math::Vec2<T> q3 = q2 * a;

  const math::Vec2<T> p = (T(3) * a * c - b * b) / (T(3) * a * a);
  const math::Vec2<T> q_plus = (q1 + q2 * d_plus) / q3;
  const math::Vec2<T> q_minus = (q1 + q2 * d_minus) / q3;

  /* Calculate discriminant, it is guaranteed to be positive. */
  const math::Vec2<T> disc0 = (p * p * p) / T(27);
  const math::Vec2<T> discriminant_plus = (q_plus * q_plus) / T(4) + disc0;
  const math::Vec2<T> discriminant_minus = (q_minus * q_minus) / T(4) + disc0;

  math::Vec4<T> solutions;

  for (uint8_t i = 0; i < 2; i++) {
    if (math::is_almost_zero(a[i])) {
      /* If a = 0, also b = 0. It is a linear equation. */

      solutions[i * 2] = -d_plus[i] / c[i];
      solutions[i * 2 + 1] = -d_minus[i] / c[i];
      continue;
    }

    /* One real root and two complex roots */
    const T discriminant_plus_sqrt = std::sqrt(discriminant_plus[i]);
    const T discriminant_minus_sqrt = std::sqrt(discriminant_minus[i]);

    const T u_plus = std::cbrt(-q_plus[i] / T(2) + discriminant_plus_sqrt);
    const T v_plus = std::cbrt(-q_plus[i] / T(2) - discriminant_plus_sqrt);

    const T u_minus = std::cbrt(-q_minus[i] / T(2) + discriminant_minus_sqrt);
    const T v_minus = std::cbrt(-q_minus[i] / T(2) - discriminant_minus_sqrt);

    solutions[i * 2] = u_plus + v_plus - b[i] / (T(3) * a[i]);
    solutions[i * 2 + 1] = u_minus + v_minus - b[i] / (T(3) * a[i]);
  }

  return solutions;
}

template<typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
void monotonic_cubic_to_circular_quadratics(const CubicBezier<T>& cubic,
                                            const T tolerance,
                                            QuadraticPath<T>& sink)
{
  // Get the angle of the biarc of current Bezier arc.
  const math::Vec2<T> B = line_line_intersection_point_infinite(cubic.start_tangent(),
                                                                cubic.end_tangent())
                              .value_or(cubic.p0);
  const math::Vec2<T> AB = B - cubic.p0;
  const math::Vec2<T> BC = cubic.p3 - B;

  const T D = math::dot(AB, BC);
  const T sin = std::sqrt((T(1) - D) / T(2));

  const T angle = std::asin(sin);

  if (std::abs(angle) < math::pi<T> / T(2)) {
    const math::Vec2<T> O = B + math::length(AB) / sin * math::normalize(AB + BC);

    sink.arc_to(O, cubic.p3, angle);
  } else {
    const std::array<CubicBezier<T>, 2> cubics = split(cubic, T(0.5));

    monotonic_cubic_to_circular_quadratics(cubics[0], tolerance, sink);
    monotonic_cubic_to_circular_quadratics(cubics[1], tolerance, sink);
  }
}

template<typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
void cubic_to_circular_quadratics(const CubicBezier<T>& cubic,
                                  const T tolerance,
                                  QuadraticPath<T>& sink)
{
  const math::QuadraticSolutions<T> split_points = inflections(cubic);

  switch (split_points.count) {
    case 0:
      monotonic_cubic_to_circular_quadratics(cubic, tolerance, sink);
      break;
    case 1: {
      std::array<CubicBezier<T>, 2> cubics = split(cubic, split_points.solutions[0]);

      monotonic_cubic_to_circular_quadratics(cubics[0], tolerance, sink);
      monotonic_cubic_to_circular_quadratics(cubics[1], tolerance, sink);

      break;
    }
    case 2: {
      std::array<CubicBezier<T>, 3> cubics = split(
          cubic, split_points.solutions[0], split_points.solutions[1]);

      monotonic_cubic_to_circular_quadratics(cubics[0], tolerance, sink);
      monotonic_cubic_to_circular_quadratics(cubics[1], tolerance, sink);
      monotonic_cubic_to_circular_quadratics(cubics[2], tolerance, sink);

      break;
    }
  }
}

template<typename T, typename _>
void cubic_to_quadratics(const CubicBezier<T>& cubic, const T tolerance, QuadraticPath<T>& sink)
{
  const auto& [a, b, c, d] = cubic.coefficients();

  T t0 = T(0);
  T t_e = T(0);

  math::Vec2<T> p2 = cubic.p0;

  while (t0 < T(1)) {
    // Taylor expansion coefficients at t=t0.
    const T t0_sq = t0 * t0;
    const T t0_cb = t0_sq * t0;

    const math::Vec2<T> coeff0 = -T(3) * a * t0;
    const math::Vec2<T> coeff1 = T(3) * a * t0_sq;
    const math::Vec2<T> coeff2 = a * t0_cb;

    // Taylor coefficients at t=t0
    const math::Vec2<T> f = coeff2 + b * t0_sq + c * t0 + d;
    const math::Vec2<T> f_prime = coeff1 + T(2) * b * t0 + c;
    const math::Vec2<T> f_second = T(6) * a * t0 + T(2) * b;

    // Taylor series expansion at t=t0
    const math::Vec2<T> quad_a = f_second / T(2);
    const math::Vec2<T> quad_b = f_prime - t0 * f_second;
    const math::Vec2<T> quad_c = f - t0 * f_prime + t0_sq * f_second / T(2);

    // Value of t at which the Taylor approximation error equals the tolerance.
    T t_e_prime = T(2);

    const math::Vec4<T> t_error = fast_cubic_first_solution_plus_minus(
        a, coeff0, coeff1, -coeff2, tolerance);
    // const math::Vec2<T> t_error_negative = fast_cubic_first_solution(a, coeff0, coeff1, -coeff2
    // + tolerance); const math::Vec2<T> t_error_positive = fast_cubic_first_solution(a, coeff0,
    // coeff1, -coeff2 - tolerance);

    if (t_error.x > t0 && t_error.x < t_e_prime)
      t_e_prime = t_error.x;
    if (t_error.y > t0 && t_error.y < t_e_prime)
      t_e_prime = t_error.y;
    if (t_error.z > t0 && t_error.z < t_e_prime)
      t_e_prime = t_error.z;
    if (t_error.w > t0 && t_error.w < t_e_prime)
      t_e_prime = t_error.w;

    // Value of t at which the next Taylor approximation should start from
    // in order to mantain the max error at t=t_e.
    const T t_e_prime_sq = t_e_prime * t_e_prime;
    const T t_e_prime_cb = t_e_prime_sq * t_e_prime;

    const math::Vec2<T> e_prime_b = T(3) * a * t_e_prime;
    const math::Vec2<T> e_prime_c = -T(3) * a * t_e_prime_sq;
    const math::Vec2<T> e_prime_d = a * t_e_prime_cb;

    T t0_prime = T(2);

    const math::Vec4<T> t_centers = fast_cubic_first_solution_plus_minus(
        -a, e_prime_b, e_prime_c, e_prime_d, tolerance);
    // const math::Vec2<T> t_centers_negative = fast_cubic_first_solution(-a, e_prime_b, e_prime_c,
    // e_prime_d + tolerance); const math::Vec2<T> t_centers_positive =
    // fast_cubic_first_solution(-a, e_prime_b, e_prime_c, e_prime_d - tolerance);

    if (t_centers.x > t_e_prime && t_centers.x < t0_prime)
      t0_prime = t_centers.x;
    if (t_centers.y > t_e_prime && t_centers.y < t0_prime)
      t0_prime = t_centers.y;
    if (t_centers.z > t_e_prime && t_centers.z < t0_prime)
      t0_prime = t_centers.z;
    if (t_centers.w > t_e_prime && t_centers.w < t0_prime)
      t0_prime = t_centers.w;

    // Quadratic Bezier curve from t_e to t_e_prime.
    const QuadraticBezier<T> quad = QuadraticBezier<T>::from_coefficients(quad_a, quad_b, quad_c);
    const QuadraticBezier<T> extracted_quad = extract(quad, t_e, std::min(T(1), t_e_prime));

    sink.back() = math::midpoint(p2, extracted_quad.p0);
    sink.quadratic_to(extracted_quad.p1, extracted_quad.p2);

    t0 = t0_prime;
    t_e = t_e_prime;
    p2 = extracted_quad.p2;
  }

  // Close the approximation with one last quadratic curve if needed.
  if (t_e < T(1)) {
    const std::array<math::Vec2<T>, 3> quad_coefficients = taylor_expand({a, b, c, d}, t0);

    const QuadraticBezier<T> quad = QuadraticBezier<T>::from_coefficients(quad_coefficients);
    const QuadraticBezier<T> extracted_quad = extract(quad, t_e, T(1));

    sink.back() = math::midpoint(p2, extracted_quad.p0);

    sink.quadratic_to(extracted_quad.p1, cubic.p3);
  } else {
    sink.back() = cubic.p3;
  }
}

template<typename T, typename _>
std::vector<std::pair<QuadraticBezier<T>, math::Vec2<T>>> cubic_to_quadratics_with_intervals(
    const CubicBezier<T>& cubic)
{
  const std::array<math::Vec2<T>, 4> cubic_coefficients = cubic.coefficients();

  std::vector<std::pair<QuadraticBezier<T>, math::Vec2<T>>> quads;

  T t0 = T(0);
  T t_e = T(0);

  while (t0 < T(1)) {
    // Taylor expansion coefficients at t=t0.
    const std::array<math::Vec2<T>, 3> quad_coefficients = taylor_expand(cubic_coefficients, t0);

    // Value of t at which the Taylor approximation error equals the tolerance.
    const T t_e_prime = taylor_expansion_error(cubic_coefficients[0], t0, T(1e-2));

    // Value of t at which the next Taylor approximation should start from
    // in order to mantain the max error at t=t_e.
    const T t0_prime = next_taylor_center(cubic_coefficients[0], t_e_prime, T(1e-2));

    // Quadratic Bezier curve from t_e to t_e_prime
    const QuadraticBezier<T> quad = QuadraticBezier<T>::from_coefficients(quad_coefficients);
    quads.emplace_back(quad, math::Vec2<T>{t_e, std::min(T(1), t_e_prime)});

    t0 = t0_prime;
    t_e = t_e_prime;
  }

  // Close the approximation with one last quadratic curve if needed.
  if (t_e < T(1)) {
    const std::array<math::Vec2<T>, 3> quad_coefficients = taylor_expand(cubic_coefficients, t0);
    const QuadraticBezier<T> quad = QuadraticBezier<T>::from_coefficients(quad_coefficients);

    quads.emplace_back(quad, math::Vec2<T>{t_e, T(1)});
  }

  return quads;
}

template<typename T, typename _>
void QuadraticPath<T, _>::quadratic_to(const math::Vec2<T> p1, const math::Vec2<T> p2)
{
  GK_ASSERT(!points.empty(), "Cannot add a curve to an empty path.");

  const QuadraticBezier<T> quad = {back(), p1, p2};
  const auto& [a, b] = quad.derivative_coefficients();

  std::vector<float> split_points = {T(0), T(1)};

  for (uint8_t i = 0; i < 2; i++) {
    const T t = math::solve_linear(a[i], b[i]);

    if (math::is_normalized(t, false)) {
      split_points.push_back(t);
    }
  }

  std::sort(split_points.begin(), split_points.end());

  for (size_t i = 0; i < split_points.size() - 1; i++) {
    const T t0 = split_points[i];
    const T t1 = split_points[i + 1];

    const QuadraticBezier<T> sub = extract(quad, t0, t1);

    points.insert(points.end(), {sub.p1, sub.p2});
  }
}

template<typename T, typename _>
void QuadraticPath<T, _>::cubic_to(const math::Vec2<T> p1,
                                   const math::Vec2<T> p2,
                                   const math::Vec2<T> p3,
                                   const T tolerance)
{
  GK_ASSERT(!points.empty(), "Cannot add a curve to an empty path.");

  cubic_to_quadratics(CubicBezier<T>{back(), p1, p2, p3}, tolerance, *this);
}

template<typename T, typename _>
void QuadraticPath<T, _>::arc_to(const math::Vec2<T> center,
                                 const math::Vec2<T> to,
                                 const bool clockwise,
                                 const T tolerance)
{
  GK_ASSERT(!points.empty(), "Cannot add an arc to an empty path.");

  const math::Vec2<T> from = back();
  const T radius = math::distance(center, from);

  const T ang1 = std::atan2(from.y - center.y, from.x - center.x);
  const T ang2 = std::atan2(to.y - center.y, to.x - center.x);
  const T error = std::sqrt(tolerance * (2.0 + tolerance));
  const T dphi = 4.0 * std::acos(std::sqrt(2.0 + tolerance - error) / std::sqrt(2.0));

  T diff = std::abs(ang2 - ang1);

  if (diff > math::pi<T>)
    diff = math::two_pi<T> - diff;
  if (!clockwise)
    diff = -diff;

  const T diff_abs = std::abs(diff);

  const int segments = static_cast<int>(std::ceil(diff_abs / dphi));
  const T inc = diff / segments;
  const T b = (std::cos(inc) - 1.0) / std::sin(inc);

  for (int i = 0; i <= segments; i++) {
    const T angle = ang1 + i * inc;
    const T sin = std::sin(angle);
    const T cos = std::cos(angle);

    const math::Vec2<T> p1 = center + radius * math::Vec2<T>{cos - b * sin, sin + b * cos};
    const math::Vec2<T> p2 = center + radius * math::Vec2<T>{cos, sin};

    quadratic_to(p1, p2);
  }
}

template<typename T, typename _>
void CubicPath<T, _>::cubic_to(const math::Vec2<T> p1,
                               const math::Vec2<T> p2,
                               const math::Vec2<T> p3)
{
  GK_ASSERT(!points.empty(), "Cannot add a curve to an empty path.");

  const CubicBezier<T> cubic = {back(), p1, p2, p3};
  const auto& [a, b, c] = cubic.derivative_coefficients();
  const auto inflection_points = inflections(cubic);

  std::array<T, 8> split_points = {T(0), T(1), T(2), T(2), T(2), T(2), T(2), T(2)};
  uint8_t split_count = 2;

  for (uint8_t j = 0; j < inflection_points.count; j++) {
    const T t = inflection_points.solutions[j];

    if (t != T(0) && t != T(1)) {
      split_points[split_count++] = t;
    }
  }

  for (uint8_t i = 0; i < 2; i++) {
    const math::QuadraticSolutions<T> solutions = math::solve_quadratic(a[i], b[i], c[i]);

    for (uint8_t j = 0; j < solutions.count; j++) {
      const T t = solutions.solutions[j];

      if (math::is_almost_normalized(t)) {
        split_points[split_count++] = t;
      }
    }
  }

  std::sort(split_points.begin(), split_points.begin() + split_count);

  for (size_t i = 0; i < split_count - 1; i++) {
    const T t0 = split_points[i];
    const T t1 = split_points[i + 1];

    const CubicBezier<T> sub = extract(cubic, t0, t1);

    points.insert(points.end(), {sub.p1, sub.p2, sub.p3});
  }
}

template<typename T, typename _>
void CubicPath<T, _>::arc_to(const math::Vec2<T> center,
                             const math::Vec2<T> to,
                             const bool clockwise)
{
  GK_ASSERT(!points.empty(), "Cannot add an arc to an empty path.");

  const math::Vec2<T> from = back();
  const T radius = math::distance(center, from);

  if (math::is_almost_zero(radius)) {
    return;
  }

  T angle;

  const math::Vec2<T> center_from = from - center;
  const math::Vec2<T> center_to = to - center;
  const T cross = (center_from.x * center_to.x + center_from.y * center_to.y) /
                  (math::length(center_from) * math::length(center_to));

  if (cross >= -T(1) - math::geometric_epsilon<T> && cross <= T(1) + math::geometric_epsilon<T>) {
    angle = std::acos(std::clamp(cross, -T(1), T(1)));
  } else {
    angle = T(0);
  }

  if (math::is_almost_zero(angle)) {
    return;
  }

  T ang1 = std::atan2(center.y - from.y, from.x - center.x);

  ang1 = ang1 < 0 ? ang1 + math::two_pi<T> : ang1;

  if (math::is_almost_equal(ang1, math::two_pi<T>))
    ang1 = T(0);
  if (math::is_almost_zero(ang1))
    ang1 = T(0);

  const TriangleOrientation orientation = triangle_orientation(center, from, to);

  if (orientation != TriangleOrientation::Collinear) {
    const bool is_clockwise = orientation == TriangleOrientation::Clockwise;

    if (is_clockwise != clockwise) {
      angle = math::two_pi<T> - angle;
    }
  }

  const int segments = std::ceil(std::abs(angle) / (math::pi<T> / T(2)));
  const T step = angle / segments * (clockwise ? -T(1) : T(1));

  T s = -std::sin(ang1);
  T c = std::cos(ang1);

  for (int i = 1; i <= segments; i++) {
    const T a1 = ang1 + step * T(i);

    const T s1 = -std::sin(a1);
    const T c1 = std::cos(a1);

    const math::Vec2<T> a = math::Vec2<T>(c, s);
    const math::Vec2<T> b = math::Vec2<T>(c1, s1);

    const T q1 = math::squared_length(a);
    const T q2 = q1 + math::dot(a, b);
    const T k2 = T(4) / T(3) * (std::sqrt(T(2) * q1 * q2) - q2) / (a.x * b.y - a.y * b.x);

    const math::Vec2<T> p1 = a + k2 * math::Vec2<T>(-a.y, a.x);
    const math::Vec2<T> p2 = b + k2 * math::Vec2<T>(b.y, -b.x);

    if (i < segments) {
      cubic_to(center + p1 * radius, center + p2 * radius, center + b * radius);
    } else {
      cubic_to(center + p1 * radius, center + p2 * radius, to);
    }

    s = s1;
    c = c1;
  }
}

/* -- Winding Number -- */

// TODO: refactor quadratic winding
static inline int winding_of(const dquadratic_bezier& quad, const dvec2 p)
{
  if (std::max(std::max(quad.p0.x, quad.p1.x), quad.p2.x) < p.x) {
    // The curve is entirely on the left of the point.
    return 0;
  }

  if (std::min(std::min(quad.p0.y, quad.p1.y), quad.p2.y) > p.y) {
    // The curve is entirely below the point.
    return 0;
  }

  if (std::max(std::max(quad.p0.y, quad.p1.y), quad.p2.y) < p.y) {
    // The curve is entirely above the point.
    return 0;
  }

  const auto [a, b, c] = quad.coefficients();
  const math::QuadraticSolutions<double> solutions = math::solve_quadratic(a.y, b.y, c.y - p.y);

  if (solutions.count == 0) {
    return 0;
  }

  const int delta = quad.p0.y < quad.p2.y ? 1 : -1;

  for (uint8_t i = 0; i < solutions.count; i++) {
    const double t = solutions.solutions[i];

    if (!math::is_normalized(t, false)) {
      continue;
    }

    const double x = a.x * t * t + b.x * t + c.x;

    if (x > p.x) {
      return delta;
    }
  }

  return 0;
}

template<typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
static inline int winding_of(const CubicBezier<T>& c, const math::Vec2<T> p)
{
  if (std::max({c.p0.x, c.p1.x, c.p2.x, c.p3.x}) < p.x) {
    // The curve is entirely on the left of the point.
    return 0;
  }

  if (std::min({c.p0.y, c.p1.y, c.p2.y, c.p3.y}) > p.y) {
    // The curve is entirely below the point.
    return 0;
  }

  if (std::max({c.p0.y, c.p1.y, c.p2.y, c.p3.y}) < p.y) {
    // The curve is entirely above the point.
    return 0;
  }

  const bool b01 = std::abs(c.p1.x - c.p0.x) + std::abs(c.p1.y - c.p0.y) <
                   math::geometric_epsilon<T>;
  const bool b12 = std::abs(c.p2.x - c.p1.x) + std::abs(c.p2.y - c.p1.y) <
                   math::geometric_epsilon<T>;
  const bool b23 = std::abs(c.p3.x - c.p2.x) + std::abs(c.p3.y - c.p2.y) <
                   math::geometric_epsilon<T>;

  const bool linear = (b01 && (b23 || b12)) || (b23 && b12);
  const T t0 = (p.y - c.p0.y) / (c.p3.y - c.p0.y);
  const int delta = c.p0.y < c.p3.y ? 1 : -1;

  if (linear) {
    if (t0 >= -math::geometric_epsilon<T> && t0 <= T(1) + math::geometric_epsilon<T>) {
      const T x = c.p0.x + t0 * (c.p3.x - c.p0.x);

      if (x > p.x) {
        return delta;
      }
    }

    return 0;
  }

  const auto& [A, B, C, D] = c.coefficients();
  const T t = geom::cubic_line_intersect_approx(A.y, B.y, C.y, D.y, p.y, t0);

  if (t >= -math::geometric_epsilon<T> && t <= T(1) + math::geometric_epsilon<T>) {
    const T t_sq = t * t;
    const T x = A.x * t_sq * t + B.x * t_sq + C.x * t + D.x;

    if (x > p.x) {
      return delta;
    }
  }

  return 0;
}

template<>
int QuadraticPath<float>::winding_of(const vec2 p) const
{
  if (points.size() < 3) {
    return 0;
  }

  int winding = 0;

  for (size_t i = 0; i < size(); i++) {
    winding += geom::winding_of(dquadratic_bezier{dvec2(points[i * 2]),
                                                  dvec2(points[i * 2 + 1]),
                                                  dvec2(points[i * 2 + 2])},
                                dvec2(p));
  }

  return winding;
}

template<>
int QuadraticPath<double>::winding_of(const dvec2 p) const
{
  if (points.size() < 3) {
    return 0;
  }

  int winding = 0;

  for (size_t i = 0; i < size(); i++) {
    winding += geom::winding_of(
        dquadratic_bezier{points[i * 2], points[i * 2 + 1], points[i * 2 + 2]}, p);
  }

  return winding;
}

template<typename T, typename _>
int CubicPath<T, _>::winding_of(const math::Vec2<T> p) const
{
  if (points.size() < 4) {
    return 0;
  }

  int winding = 0;

  for (size_t i = 0; i < size(); i++) {
    winding += geom::winding_of(
        CubicBezier<T>{points[i * 3], points[i * 3 + 1], points[i * 3 + 2], points[i * 3 + 3]}, p);
  }

  return winding;
}

template<typename T, typename _>
int CubicMultipath<T, _>::winding_of(const math::Vec2<T> p) const
{
  if (starts.empty() || this->points.size() < 4) {
    return 0;
  }

  int winding = 0;

  for (size_t j = 0; j < starts.size(); j++) {
    const size_t end = starts.size() > (j + 1) ? starts[j + 1] : this->points.size();

    for (size_t i = starts[j]; i < end - 3; i += 3) {
      winding += geom::winding_of(
          CubicBezier<T>{
              this->points[i], this->points[i + 1], this->points[i + 2], this->points[i + 3]},
          p);
    }
  }

  return winding;
}

/* -- Template Instantiation -- */

template struct Line<float>;
template struct Line<double>;

template struct CubicBezier<float>;
template struct CubicBezier<double>;

template math::CubicSolutions<float> max_curvature(const CubicBezier<float>&);
template math::CubicSolutions<double> max_curvature(const CubicBezier<double>&);

template math::QuadraticSolutions<float> inflections(const CubicBezier<float>&);
template math::QuadraticSolutions<double> inflections(const CubicBezier<double>&);

template math::Rect<float> bounding_rect(const QuadraticBezier<float>&);
template math::Rect<double> bounding_rect(const QuadraticBezier<double>&);

template math::Rect<float> bounding_rect(const CubicBezier<float>&);
template math::Rect<double> bounding_rect(const CubicBezier<double>&);

template std::array<QuadraticBezier<float>, 2> split(const QuadraticBezier<float>&, const float);
template std::array<QuadraticBezier<double>, 2> split(const QuadraticBezier<double>&,
                                                      const double);

template std::array<QuadraticBezier<float>, 3> split(const QuadraticBezier<float>&,
                                                     const float,
                                                     const float);
template std::array<QuadraticBezier<double>, 3> split(const QuadraticBezier<double>&,
                                                      const double,
                                                      const double);

template std::array<CubicBezier<float>, 2> split(const CubicBezier<float>&, const float);
template std::array<CubicBezier<double>, 2> split(const CubicBezier<double>&, const double);

template std::array<CubicBezier<float>, 3> split(const CubicBezier<float>&,
                                                 const float,
                                                 const float);
template std::array<CubicBezier<double>, 3> split(const CubicBezier<double>&,
                                                  const double,
                                                  const double);

template QuadraticBezier<float> extract(const QuadraticBezier<float>&, const float, const float);
template QuadraticBezier<double> extract(const QuadraticBezier<double>&,
                                         const double,
                                         const double);

template CubicBezier<float> extract(const CubicBezier<float>&, const float, const float);
template CubicBezier<double> extract(const CubicBezier<double>&, const double, const double);

template void cubic_to_quadratics(const CubicBezier<float>&, const float, QuadraticPath<float>&);
template void cubic_to_quadratics(const CubicBezier<double>&,
                                  const double,
                                  QuadraticPath<double>&);

template std::vector<std::pair<QuadraticBezier<float>, math::Vec2<float>>>
cubic_to_quadratics_with_intervals(const CubicBezier<float>&);
template std::vector<std::pair<QuadraticBezier<double>, math::Vec2<double>>>
cubic_to_quadratics_with_intervals(const CubicBezier<double>&);

template struct QuadraticPath<float>;
template struct QuadraticPath<double>;
template struct QuadraticMultipath<float>;
template struct QuadraticMultipath<double>;

template struct CubicPath<float>;
template struct CubicPath<double>;
template struct CubicMultipath<float>;
template struct CubicMultipath<double>;

}  // namespace graphick::geom
