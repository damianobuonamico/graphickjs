/**
 * @file geom/quadratic_bezier.h
 * @brief This file contains the declaration of the QuadraticBezier class.
 */

#pragma once

#include "../math/rect.h"

#include <array>

namespace graphick::geom {

/**
 * @brief The QuadraticBezier class represents a quadratic bezier curve in 2D space.
 *
 * @struct QuadraticBezier
 */
template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
struct QuadraticBezier {
  math::Vec2<T> p0;  // The starting point of the quadratic bezier.
  math::Vec2<T> p1;  // The second control point of the quadratic bezier.
  math::Vec2<T> p2;  // The end point of the quadratic bezier.

  /* -- Component accesses -- */

  static constexpr uint8_t length() { return 3; }

  constexpr math::Vec2<T>& operator[](uint8_t i) {
    switch (i) {
    default:
    case 0:
      return p0;
    case 1:
      return p1;
    case 2:
      return p2;
    }
  }

  constexpr math::Vec2<T> const& operator[](uint8_t i) const {
    switch (i) {
    default:
    case 0:
      return p0;
    case 1:
      return p1;
    case 2:
      return p2;
    }
  }

  /* -- Constructors -- */

  QuadraticBezier() = default;

  constexpr QuadraticBezier(const QuadraticBezier<T>& q) : p0(q.p0), p1(q.p1), p2(q.p2) { }

  constexpr QuadraticBezier(const math::Vec2<T> p0, const math::Vec2<T> p2) : p0(p0), p1(p2), p2(p2) { }

  constexpr QuadraticBezier(const math::Vec2<T> p0, const math::Vec2<T> p1, const math::Vec2<T> p2) : p0(p0), p1(p1), p2(p2) { }

  template <typename U>
  constexpr QuadraticBezier(const QuadraticBezier<U>& q) : p0(q.p0), p1(q.p1), p2(q.p2) { }

  /* -- Static Constructors -- */

  static constexpr QuadraticBezier<T> from_coefficients(const math::Vec2<T> a, const math::Vec2<T> b, const math::Vec2<T> c) {
    return QuadraticBezier<T>(c, c + b / T(2), c + b + a);
  }

  static constexpr QuadraticBezier<T> from_coefficients(const std::array<math::Vec2<T>, 3>& coefficients) {
    return QuadraticBezier<T>(
      coefficients[2],
      coefficients[2] + coefficients[1] / T(2),
      coefficients[2] + coefficients[1] + coefficients[0]
    );
  }

  /* -- Coefficients -- */

  constexpr std::array<math::Vec2<T>, 3> coefficients() const { return {p0 - T(2) * p1 + p2, T(2) * (p1 - p0), p0}; }

  constexpr std::array<math::Vec2<T>, 2> derivative_coefficients() const {
    return {T(2) * (p0 - T(2) * p1 + p2), T(2) * (p1 - p0)};
  }

  /* -- Sample -- */

  constexpr math::Vec2<T> sample(T t) const {
    const T t_sq = t * t;
    const T t_inv = T(1) - t;
    const T t_inv_sq = t_inv * t_inv;

    return t_inv_sq * p0 + T(2) * t * t_inv * p1 + t_sq * p2;
  }

  constexpr math::Vec2<T> derivative(T t) const {
    const auto [a, b] = derivative_coefficients();

    return a * t + b;
  }

  /* -- Bounding Rectangle -- */

  math::Rect<T> bounding_rect() const;

  math::Rect<T> approx_bounding_rect() const;
};

}  // namespace graphick::geom

/* -- Aliases -- */

namespace graphick::geom {

using quadratic_bezier = QuadraticBezier<float>;
using dquadratic_bezier = QuadraticBezier<double>;

}  // namespace graphick::geom

namespace graphick {

using geom::dquadratic_bezier;
using geom::quadratic_bezier;

}  // namespace graphick
