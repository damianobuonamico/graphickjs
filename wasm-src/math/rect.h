/**
 * @file rect.h
 * @brief This file contains the Rect and RRect structs, templated 2D rects.
 */

#pragma once

#include "vec2.h"

namespace Graphick::Math {

  /**
   * @brief A 2D rectangle struct with min and max components.
   *
   * @struct Rect
   */
  template<typename T>
  struct Rect {
    Vec2<T> min;   /* The minimum point of the rectangle. */
    Vec2<T> max;   /* The maximum point of the rectangle. */

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

    Rect() :
      min(std::numeric_limits<Vec2<T>>::max()),
      max(std::numeric_limits<Vec2<T>>::lowest()) {}

    constexpr Rect(const Rect<T>& r) = default;

    constexpr Rect(Vec2<T> v) :
      min(v),
      max(v) {}

    constexpr Rect(Vec2<T> v1, Vec2<T> v2) :
      min(v1),
      max(v2) {}

    template<typename U>
    constexpr Rect(const Rect<U>& r) :
      min(r.min),
      max(r.max) {}

    /* -- Dimensions -- */

    constexpr T width() const {
      return max.x - min.x;
    }

    constexpr T height() const {
      return max.y - min.y;
    }

    constexpr Vec2<T> size() const {
      return max - min;
    }

    constexpr Vec2<T> center() const {
      return (min + max) / 2;
    }

    constexpr T area() const {
      Vec2<T> size = this->size();
      return size.x * size.y;
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
  template<typename T>
  struct RRect : public Rect<T> {
    T angle;    /* The angle of rotation of the rectangle. */

    /* -- Constructors -- */

    RRect() :
      Rect(),
      angle(0) {}

    constexpr RRect(const RRect& r) :
      Rect(r.min, r.max),
      angle(r.angle) {}

    constexpr RRect(const Rect<T>& r, T t = 0) :
      Rect(r),
      angle(t) {}

    constexpr RRect(Vec2<T> v, T t = 0) :
      Rect(v),
      angle(t) {}

    constexpr RRect(Vec2<T> v1, Vec2<T> v2, T t = 0) :
      Rect(v1, v2),
      angle(t) {}

    template<typename U>
    constexpr RRect(const RRect<U>& r) :
      Rect(r.min, r.max),
      angle(static_cast<T>(r.angle)) {}
  };

  /* -- Binary operators -- */

  template<typename T>
  constexpr Rect<T> operator+(const Rect<T>& r1, const Rect<T>& r2) {
    return Rect<T>(
      r1.min + r2.min,
      r1.max + r2.max
    );
  }

  template<typename T>
  constexpr Rect<T> operator+(const Rect<T>& r, const Vec2<T> v) {
    return Rect<T>(
      r.min + v,
      r.max + v
    );
  }

  template<typename T, typename U>
  constexpr Rect<T> operator+(const Rect<T>& r, U scalar) {
    return Rect<T>(
      r.min + scalar,
      r.max + scalar
    );
  }

  template<typename T>
  constexpr Rect<T> operator-(const Rect<T>& r1, const Rect<T>& r2) {
    return Rect<T>(
      r1.min - r2.min,
      r1.max - r2.max
    );
  }

  template<typename T>
  constexpr Rect<T> operator-(const Rect<T>& r, const Vec2<T> v) {
    return Rect<T>(
      r.min - v,
      r.max - v
    );
  }

  template<typename T, typename U>
  constexpr Rect<T> operator-(const Rect<T>& r, U scalar) {
    return Rect<T>(
      r.min - scalar,
      r.max - scalar
    );
  }

  template<typename T>
  constexpr Rect<T> operator*(const Rect<T>& r, const Vec2<T> v) {
    return Rect<T>(
      r.min * v,
      r.max * v
    );
  }

  template<typename T, typename U>
  constexpr Rect<T> operator*(const Rect<T>& r, U scalar) {
    return Rect<T>(
      r.min * scalar,
      r.max * scalar
    );
  }

  template<typename T>
  constexpr Rect<T> operator/(const Rect<T>& r, const Vec2<T> v) {
    return Rect<T>(
      r.min / v,
      r.max / v
    );
  }

  template<typename T, typename U>
  constexpr Rect<T> operator/(const Rect<T>& r, U scalar) {
    return Rect<T>(
      r.min / scalar,
      r.max / scalar
    );
  }

  template<typename T>
  constexpr Rect<T> operator%(const Rect<T>& r, const Vec2<T> v) {
    return Rect<T>(
      r.min % v,
      r.max % v
    );
  }

  template<typename T, typename U>
  constexpr Rect<T> operator%(const Rect<T>& r, U scalar) {
    return Rect<T>(
      r.min % scalar,
      r.max % scalar
    );
  }

}

namespace Graphick::Math {

  using rect = Math::Rect<float>;
  using drect = Math::Rect<double>;
  using irect = Math::Rect<int32_t>;
  using urect = Math::Rect<uint32_t>;

  using rrect = Math::RRect<float>;
  using drrect = Math::RRect<double>;
  using irrect = Math::RRect<int32_t>;
  using urrect = Math::RRect<uint32_t>;

}

namespace Graphick {

  using rect = Math::rect;
  using drect = Math::drect;
  using irect = Math::irect;
  using urect = Math::urect;

  using rrect = Math::rrect;
  using drrect = Math::drrect;
  using irrect = Math::irrect;
  using urrect = Math::urrect;

}
