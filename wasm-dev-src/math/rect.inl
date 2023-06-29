#pragma once

namespace Graphick::Math {

  /* -- Component accesses -- */

  constexpr vec2& rect::operator[](uint8_t i) {
    assert(i >= 0 && i < 2);
    switch (i) {
    default:
    case 0:
      return min;
    case 1:
      return max;
    }
  }

  constexpr vec2 const& rect::operator[](uint8_t i) const {
    assert(i >= 0 && i < 2);
    switch (i) {
    default:
    case 0:
      return min;
    case 1:
      return max;
    }
  }

  /* -- Constructors -- */

  constexpr rect::rect(vec2 v) : min(v), max(v) {}

  constexpr rect::rect(vec2 v1, vec2 v2) : min(v1), max(v2) {}

  constexpr vec2 rect::size() const {
    return max - min;
  };

}
