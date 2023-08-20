#pragma once

#include "vec2.h"

namespace Graphick::Math {

  struct rect {
    vec2 min;
    vec2 max;

    /* -- Component accesses -- */

    constexpr vec2& operator[](uint8_t i);
    constexpr vec2 const& operator[](uint8_t i) const;

    /* -- Constructors -- */

    rect() : min(std::numeric_limits<vec2>::max()), max(std::numeric_limits<vec2>::lowest()) {}
    constexpr rect(const rect& r) = default;
    constexpr rect(vec2 v);
    constexpr rect(vec2 v1, vec2 v2);

    /* -- Dimensions -- */

    constexpr vec2 size() const;
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

  /* -- Binary operators -- */

  constexpr rect operator+(const rect r, float scalar);
  constexpr rect operator+(const rect r, const vec2 v);
  constexpr rect operator-(const rect r, float scalar);
  constexpr rect operator-(const rect r, const vec2 v);
  constexpr rect operator*(const rect r, float scalar);
  constexpr rect operator*(const rect r, const vec2 v);
  constexpr rect operator/(const rect r, float scalar);
  constexpr rect operator/(const rect r, const vec2 v);
  constexpr rect operator%(const rect r, float scalar);
  constexpr rect operator%(const rect r, const vec2 v);

}

namespace Graphick {

  using rect = Math::rect;

}

#include "rect.inl"
