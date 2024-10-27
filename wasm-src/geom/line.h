/**
 * @file geom/line.h
 * @brief This file contains the declaration of the Line class.
 */

#pragma once

#include "../math/rect.h"

namespace graphick::geom {

/**
 * @brief The Line class represents a line in 2D space.
 *
 * @struct Line
 */
template <typename T>
struct Line {
  math::Vec2<T> p0;  // The starting point of the line.
  math::Vec2<T> p1;  // The end point of the line.

  /* -- Component accesses -- */

  static constexpr uint8_t size() { return 2; }

  constexpr math::Vec2<T>& operator[](uint8_t i) {
    switch (i) {
    default:
    case 0:
      return p0;
    case 1:
      return p1;
    }
  }

  constexpr math::Vec2<T> const& operator[](uint8_t i) const {
    switch (i) {
    default:
    case 0:
      return p0;
    case 1:
      return p1;
    }
  }

  /* -- Constructors -- */

  Line() = default;

  constexpr Line(const Line<T>& l) = default;

  constexpr Line(const math::Vec2<T> p0, const math::Vec2<T> p1) : p0(p0), p1(p1) { }

  constexpr Line(const T x0, const T y0, const T x1, const T y1) : p0(x0, y0), p1(x1, y1) { }

  template <typename U>
  constexpr Line(const Line<U>& l) : p0(l.p0), p1(l.p1) { }

  /* -- Sample -- */

  constexpr math::Vec2<T> sample(T t) const { return p0 + t * (p1 - p0); }

  /* -- Bounding Rectangle -- */

  constexpr math::Rect<T> bounding_rect() { return math::Rect<T>::from_vectors(p0, p1); }

  /* -- Methods -- */

  constexpr math::Vec2<T> midpoint() const { return (p0 + p1) / T(2); }

  constexpr math::Vec2<T> raw_normal() const { return math::Vec2<T>(p1.y - p0.y, p0.x - p1.x); }

  constexpr T squared_length() const { return (p1.x - p0.x) * (p1.x - p0.x) + (p1.y - p0.y) * (p1.y - p0.y); }

  math::Vec2<T> direction() const;

  math::Vec2<T> normal() const;

  T angle() const;

  T length() const;
};

}  // namespace graphick::geom

/* -- Aliases -- */

namespace graphick::geom {

using line = Line<float>;
using dline = Line<double>;
using iline = Line<int32_t>;
using uline = Line<uint8_t>;

}  // namespace graphick::geom

namespace graphick {

using geom::dline;
using geom::iline;
using geom::line;
using geom::uline;

}  // namespace graphick
