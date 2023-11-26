/**
 * @file math/rect.h
 * @brief This file contains the definition of the rect and rrect struct.
 */

#pragma once

#include "vec2.h"

namespace Graphick::Math {

  /**
   * @brief A 2D rectangle struct with min and max components.
   *
   * @struct rect
   */
  struct rect {
    vec2 min;   /* The minimum point of the rectangle. */
    vec2 max;   /* The maximum point of the rectangle. */

    /* -- Component accesses -- */

    constexpr vec2& operator[](uint8_t i);
    constexpr vec2 const& operator[](uint8_t i) const;

    /* -- Constructors -- */

    rect() : min(std::numeric_limits<vec2>::max()), max(std::numeric_limits<vec2>::lowest()) {}
    constexpr rect(const rect& r) = default;
    constexpr rect(vec2 v);
    constexpr rect(vec2 v1, vec2 v2);

    /* -- Dimensions -- */

    constexpr float width() const;
    constexpr float height() const;
    constexpr vec2 size() const;
    constexpr vec2 center() const;
    constexpr float area() const;

    /* -- Unary arithmetic operators -- */

    constexpr rect& operator+=(float scalar);
    constexpr rect& operator+=(const vec2 v);
    constexpr rect& operator-=(float scalar);
    constexpr rect& operator-=(const vec2 v);
    constexpr rect& operator*=(float scalar);
    constexpr rect& operator*=(const vec2 v);
    constexpr rect& operator/=(float scalar);
    constexpr rect& operator/=(const vec2 v);
  };

  /**
   * @brief A 2D rotated rectangle struct with min and max components and an angle of rotation.
   *
   * @struct rrect
   */
  struct rrect : public rect {
    float angle;    /* The angle of rotation of the rectangle. */

    /* -- Constructors -- */

    rrect() : rect(), angle(0.0f) {}
    constexpr rrect(const rrect& r) : rect(r.min, r.max), angle(r.angle) {}
    constexpr rrect(const rect& r, float t = 0.0f) : rect(r), angle(t) {}
    constexpr rrect(vec2 v, float t = 0.0f) : rect(v), angle(t) {}
    constexpr rrect(vec2 v1, vec2 v2, float t = 0.0f) : rect(v1, v2), angle(t) {}
  };

  /* -- Binary operators -- */

  constexpr rect operator+(const rect& r1, const rect& r2);
  constexpr rect operator+(const rect& r, float scalar);
  constexpr rect operator+(const rect& r, const vec2 v);
  constexpr rect operator-(const rect& r1, const rect& r2);
  constexpr rect operator-(const rect& r, float scalar);
  constexpr rect operator-(const rect& r, const vec2 v);
  constexpr rect operator*(const rect& r, float scalar);
  constexpr rect operator*(const rect& r, const vec2 v);
  constexpr rect operator/(const rect& r, float scalar);
  constexpr rect operator/(const rect& r, const vec2 v);
  constexpr rect operator%(const rect& r, float scalar);
  constexpr rect operator%(const rect& r, const vec2 v);

}

namespace Graphick {

  using rect = Math::rect;
  using rrect = Math::rrect;
}

#include "rect.inl"
