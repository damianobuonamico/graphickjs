/**
 * @file geom/cubic_bezier.h
 * @brief This file contains the declaration of the CubicBezier class.
 */

#pragma once

#include "../math/rect.h"

#include <array>

//TODO: implement quadratic(), quadratic_derivative(), cubic(), cubic_derivative(), cubic_second_derivative(), cubic_extrema()

namespace graphick::geom {

  /**
   * @brief The CubicBezier class represents a cubic bezier curve in 2D space.
   *
   * @struct CubicBezier
   */
  template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
  struct CubicBezier {
    math::Vec2<T> p0;    /* The starting point of the cubic bezier. */
    math::Vec2<T> p1;    /* The second control point of the cubic bezier. */
    math::Vec2<T> p2;    /* The third control point of the cubic bezier. */
    math::Vec2<T> p3;    /* The end point of the cubic bezier. */

    /* -- Component accesses -- */

    static constexpr uint8_t length() {
      return 4;
    }

    constexpr math::Vec2<T>& operator[](uint8_t i) {
      switch (i) {
      default:
      case 0:
        return p0;
      case 1:
        return p1;
      case 2:
        return p2;
      case 3:
        return p3;
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
      case 3:
        return p3;
      }
    }

    /* -- Constructors -- */

    CubicBezier() = default;

    constexpr CubicBezier(const CubicBezier<T>& c) :
      p0(c.p0),
      p1(c.p1),
      p2(c.p2),
      p3(c.p3) {}

    constexpr CubicBezier(const math::Vec2<T> p0, const math::Vec2<T> p3) :
      p0(p0),
      p1(p0),
      p2(p3),
      p3(p3) {}

    constexpr CubicBezier(const math::Vec2<T> p0, const math::Vec2<T> p2, const math::Vec2<T> p3) :
      p0(p0),
      p1(p0),
      p2(p2),
      p3(p3) {}

    constexpr CubicBezier(const math::Vec2<T> p0, const math::Vec2<T> p1, const math::Vec2<T> p2, const math::Vec2<T> p3) :
      p0(p0),
      p1(p1),
      p2(p2),
      p3(p3) {}

    template <typename U>
    constexpr CubicBezier(const CubicBezier<U>& c) :
      p0(c.p0),
      p1(c.p1),
      p2(c.p2),
      p3(c.p3) {}

    /* -- Coefficients -- */

    constexpr std::array<math::Vec2<T>, 4> coefficients() const {
      return {
        -p0 + T(3) * p1 - T(3) * p2 + p3,
        T(3) * (p0 - T(2) * p1 + p2),
        T(3) * (p1 - p0),
        p0
      };
    }

    constexpr std::array<math::Vec2<T>, 3> derivative_coefficients() const {
      return {
        T(3) * (T(3) * p1 - T(3) * p2 + p3 - p0),
        T(6) * (p0 - T(2) * p1 + p2),
        T(3) * (p1 - p0)
      }
    }

    constexpr std::array<math::Vec2<T>, 2> second_derivative_coefficients() const {
      return {
        T(6) * (T(3) * p1 - T(3) * p2 + p3 - p0),
        T(6) * (p0 - T(2) * p1 + p2)
      }
    }

    /* -- Sample -- */

    constexpr math::Vec2<T> sample(T t) const {
      const T t_sq = t * t;
      const T t_cb = t_sq * t;
      const T t_inv = T(1) - t;
      const T t_inv_sq = t_inv * t_inv;
      const T t_inv_cb = t_inv_sq * t_inv;

      return
        p0 * t_inv_cb +
        p1 * T(3) * t * t_inv_sq +
        p2 * T(3) * t_sq * t_inv +
        p3 * t_cb;
    }

    constexpr math::Vec2<T> derivative(T t) const {
      const auto [a, b, c] = derivative_coefficients();

      return
        a * t * t +
        b * t +
        c;
    }

    constexpr math::Vec2<T> second_derivative(T t) const {
      const auto [a, b] = second_derivative_coefficients();

      return
        a * t +
        b;
    }

    /* -- Bounding Rectangle -- */

    math::Rect<T> bounding_rect() const;

    math::Rect<T> approx_bounding_rect() const;
  };

}

/* -- Aliases -- */

namespace graphick::geom {

  using cubic_bezier = CubicBezier<float>;
  using dcubic_bezier = CubicBezier<double>;

}

namespace graphick {

  using geom::cubic_bezier;
  using geom::dcubic_bezier;

}
