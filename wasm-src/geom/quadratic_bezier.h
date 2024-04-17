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
    math::Vec2<T> p0;    /* The starting point of the quadratic bezier. */
    math::Vec2<T> p1;    /* The second control point of the quadratic bezier. */
    math::Vec2<T> p2;    /* The end point of the quadratic bezier. */

    /* -- Component accesses -- */

    static constexpr uint8_t length() {
      return 3;
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

    constexpr QuadraticBezier(const QuadraticBezier<T>& l) = default;

    constexpr QuadraticBezier(const math::Vec2<T> p0, const math::Vec2<T> p2) :
      p0(p0),
      p1(p2),
      p2(p2) {}

    constexpr QuadraticBezier(const math::Vec2<T> p0, const math::Vec2<T> p1, const math::Vec2<T> p2) :
      p0(p0),
      p1(p1),
      p2(p2) {}

    template <typename U>
    constexpr QuadraticBezier(const QuadraticBezier<U>& q) :
      p0(q.p0),
      p1(q.p1),
      p2(q.p2) {}

    /* -- Coefficients -- */

    std::array<math::Vec2<T>, 3> coefficients() const {
      return {
        p0 - T(2) * p1 + p2,
        T(2) * (p1 - p0),
        p0
      };
    }

    /* -- Methods -- */

    math::Rect<T> bounding_rect() const;

    math::Rect<T> approx_bounding_rect() const;
  };

}

/* -- Aliases -- */

namespace graphick::geom {

  using quadratic_bezier = QuadraticBezier<float>;
  using dquadratic_bezier = QuadraticBezier<double>;

}

namespace graphick {

  using geom::quadratic_bezier;
  using geom::dquadratic_bezier;

}
