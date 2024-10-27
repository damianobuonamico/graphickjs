/**
 * @file math/rect.h
 * @brief This file contains the Rect and RRect structs, templated 2D rects.
 */

#pragma once

#include "vec2.h"

#include <initializer_list>

namespace graphick::math {

/**
 * @brief A 2D rectangle struct with min and max components.
 *
 * @struct Rect
 */
template <typename T>
struct Rect {
  Vec2<T> min;  // The minimum point of the rectangle.
  Vec2<T> max;  // The maximum point of the rectangle.

  /* -- Component accesses -- */

  constexpr Vec2<T>& operator[](uint8_t i) {
    switch (i) {
    default:
    case 0:
      return min;
    case 1:
      return max;
    }
  }

  constexpr Vec2<T> const& operator[](uint8_t i) const {
    switch (i) {
    default:
    case 0:
      return min;
    case 1:
      return max;
    }
  }

  /* -- Constructors -- */

  Rect() : min(std::numeric_limits<Vec2<T>>::max()), max(std::numeric_limits<Vec2<T>>::lowest()) { }

  constexpr Rect(const Rect<T>& r) = default;

  constexpr Rect(Vec2<T> v) : min(v), max(v) { }

  constexpr Rect(Vec2<T> v1, Vec2<T> v2) : min(v1), max(v2) { }

  constexpr Rect(T a, T b, T c, T d) : min(a, b), max(c, d) { }

  template <typename U>
  constexpr Rect(const Rect<U>& r) : min(r.min), max(r.max) { }

  /* -- Static constructors -- */

  static constexpr Rect<T> from_size(const Vec2<T> size) { return Rect<T>(Vec2<T>::zero(), size); }

  static constexpr Rect<T> from_center(const Vec2<T> center, const Vec2<T> size) {
    return Rect<T>(center - size / 2, center + size / 2);
  }

  static constexpr Rect<T> from_vectors(const Vec2<T> v1, const Vec2<T> v2) {
    return Rect<T>(
      {v1.x < v2.x ? v1.x : v2.x, v1.y < v2.y ? v1.y : v2.y},
      {v1.x > v2.x ? v1.x : v2.x, v1.y > v2.y ? v1.y : v2.y}
    );
  }

  static constexpr Rect<T> from_vectors(const std::initializer_list<Vec2<T>> vectors) {
    Vec2<T> min = std::numeric_limits<Vec2<T>>::max();
    Vec2<T> max = std::numeric_limits<Vec2<T>>::lowest();

    for (Vec2<T> v : vectors) {
      min.x = min.x > v.x ? v.x : min.x;
      min.y = min.y > v.y ? v.y : min.y;
      max.x = max.x < v.x ? v.x : max.x;
      max.y = max.y < v.y ? v.y : max.y;
    }

    return Rect<T>(min, max);
  }

  static constexpr Rect<T> from_rects(const Rect<T> r1, const Rect<T> r2) {
    Vec2<T> min = r1.min;
    Vec2<T> max = r1.max;

    for (uint8_t i = 0; i < 2; i++) {
      min.x = min.x > r2[i].x ? r2[i].x : min.x;
      min.y = min.y > r2[i].y ? r2[i].y : min.y;
      max.x = max.x < r2[i].x ? r2[i].x : max.x;
      max.y = max.y < r2[i].y ? r2[i].y : max.y;
    }

    return Rect<T>(min, max);
  }

  /* -- Dimensions -- */

  constexpr T width() const { return max.x - min.x; }

  constexpr T height() const { return max.y - min.y; }

  constexpr Vec2<T> size() const { return max - min; }

  constexpr Vec2<T> center() const { return (min + max) / T(2); }

  constexpr T area() const {
    Vec2<T> size = this->size();
    return size.x * size.y;
  }

  /* -- Methods -- */

  constexpr Rect<T>& include(const Vec2<T> v) {
    min.x = min.x > v.x ? v.x : min.x;
    min.y = min.y > v.y ? v.y : min.y;
    max.x = max.x < v.x ? v.x : max.x;
    max.y = max.y < v.y ? v.y : max.y;
    return *this;
  }

  /* -- Unary arithmetic operators -- */

  constexpr Rect<T>& operator+=(T scalar) {
    min += scalar;
    max += scalar;
    return *this;
  }

  constexpr Rect<T>& operator+=(const Vec2<T> v) {
    min += v;
    max += v;
    return *this;
  }

  constexpr Rect<T>& operator-=(T scalar) {
    min -= scalar;
    max -= scalar;
    return *this;
  }

  constexpr Rect<T>& operator-=(const Vec2<T> v) {
    min -= v;
    max -= v;
    return *this;
  }

  constexpr Rect<T>& operator*=(T scalar) {
    min *= scalar;
    max *= scalar;
    return *this;
  }

  constexpr Rect<T>& operator*=(const Vec2<T> v) {
    min *= v;
    max *= v;
    return *this;
  }

  constexpr Rect<T>& operator/=(T scalar) {
    min /= scalar;
    max /= scalar;
    return *this;
  }

  constexpr Rect<T>& operator/=(const Vec2<T> v) {
    min /= v;
    max /= v;
    return *this;
  }
};

/**
 * @brief A 2D rotated rectangle struct with min and max components and an angle of rotation.
 *
 * @struct rrect
 */
template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
struct RRect : public Rect<T> {
  T angle;  // The angle of rotation of the rectangle.

  /* -- Constructors -- */

  RRect() : Rect<T>(), angle(0) { }

  constexpr RRect(const RRect& r) : Rect<T>(r.min, r.max), angle(r.angle) { }

  constexpr RRect(const Rect<T>& r, T t = 0) : Rect<T>(r), angle(t) { }

  constexpr RRect(Vec2<T> v, T t = 0) : Rect<T>(v), angle(t) { }

  constexpr RRect(Vec2<T> v1, Vec2<T> v2, T t = 0) : Rect<T>(v1, v2), angle(t) { }

  template <typename U>
  constexpr RRect(const RRect<U>& r) : Rect<T>(r.min, r.max), angle(static_cast<T>(r.angle)) { }
};

/* -- Binary operators -- */

template <typename T>
constexpr Rect<T> operator+(const Rect<T>& r1, const Rect<T>& r2) {
  return Rect<T>(r1.min + r2.min, r1.max + r2.max);
}

template <typename T>
constexpr Rect<T> operator+(const Rect<T>& r, const Vec2<T> v) {
  return Rect<T>(r.min + v, r.max + v);
}

template <typename T, typename U>
constexpr Rect<T> operator+(const Rect<T>& r, U scalar) {
  return Rect<T>(r.min + scalar, r.max + scalar);
}

template <typename T>
constexpr Rect<T> operator-(const Rect<T>& r1, const Rect<T>& r2) {
  return Rect<T>(r1.min - r2.min, r1.max - r2.max);
}

template <typename T>
constexpr Rect<T> operator-(const Rect<T>& r, const Vec2<T> v) {
  return Rect<T>(r.min - v, r.max - v);
}

template <typename T, typename U>
constexpr Rect<T> operator-(const Rect<T>& r, U scalar) {
  return Rect<T>(r.min - scalar, r.max - scalar);
}

template <typename T>
constexpr Rect<T> operator*(const Rect<T>& r, const Vec2<T> v) {
  return Rect<T>(r.min * v, r.max * v);
}

template <typename T, typename U>
constexpr Rect<T> operator*(const Rect<T>& r, U scalar) {
  return Rect<T>(r.min * scalar, r.max * scalar);
}

template <typename T>
constexpr Rect<T> operator/(const Rect<T>& r, const Vec2<T> v) {
  return Rect<T>(r.min / v, r.max / v);
}

template <typename T, typename U>
constexpr Rect<T> operator/(const Rect<T>& r, U scalar) {
  return Rect<T>(r.min / scalar, r.max / scalar);
}

template <typename T>
constexpr Rect<T> operator%(const Rect<T>& r, const Vec2<T> v) {
  return Rect<T>(r.min % v, r.max % v);
}

template <typename T, typename U>
constexpr Rect<T> operator%(const Rect<T>& r, U scalar) {
  return Rect<T>(r.min % scalar, r.max % scalar);
}

/* -- Boolean operators -- */

template <typename T>
constexpr bool operator==(const Rect<T>& r1, const Rect<T>& r2) {
  return r1.min == r2.min && r1.max == r2.max;
}

template <typename T>
constexpr bool operator!=(const Rect<T>& r1, const Rect<T>& r2) {
  return !(r1 == r2);
}
}  // namespace graphick::math

/* -- Aliases -- */

namespace graphick::math {

using rect = math::Rect<float>;
using drect = math::Rect<double>;
using irect = math::Rect<int32_t>;
using urect = math::Rect<uint8_t>;

using rrect = math::RRect<float>;
using drrect = math::RRect<double>;
using irrect = math::RRect<int32_t>;
using urrect = math::RRect<uint8_t>;

}  // namespace graphick::math

namespace graphick {

using math::drect;
using math::irect;
using math::rect;
using math::urect;

using math::drrect;
using math::irrect;
using math::rrect;
using math::urrect;

}  // namespace graphick
