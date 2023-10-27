#pragma once

namespace Graphick::Math {

  /* -- Component accesses -- */

  constexpr vec2& rect::operator[](uint8_t i) {
    // assert(i >= 0 && i < 2);
    switch (i) {
    default:
    case 0:
      return min;
    case 1:
      return max;
    }
  }

  constexpr vec2 const& rect::operator[](uint8_t i) const {
    // assert(i >= 0 && i < 2);
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

  /* -- Dimensions -- */

  constexpr vec2 rect::size() const {
    return max - min;
  };

  constexpr vec2 rect::center() const {
    return (min + max) / 2.0f;
  }

  constexpr float rect::area() const {
    vec2 size = this->size();
    return size.x * size.y;
  }

  /* -- Unary arithmetic operators -- */

  constexpr rect& rect::operator+=(float scalar) {
    this->min += scalar;
    this->max += scalar;
    return *this;
  }

  constexpr rect& rect::operator+=(const vec2 v) {
    this->min += v;
    this->max += v;
    return *this;
  }

  constexpr rect& rect::operator-=(float scalar) {
    this->min -= scalar;
    this->max -= scalar;
    return *this;
  }

  constexpr rect& rect::operator-=(const vec2 v) {
    this->min -= v;
    this->max -= v;
    return *this;
  }

  constexpr rect& rect::operator*=(float scalar) {
    this->min *= scalar;
    this->max *= scalar;
    return *this;
  }

  constexpr rect& rect::operator*=(const vec2 v) {
    this->min *= v;
    this->max *= v;
    return *this;
  }

  constexpr rect& rect::operator/=(float scalar) {
    this->min /= scalar;
    this->max /= scalar;
    return *this;
  }

  constexpr rect& rect::operator/=(const vec2 v) {
    this->min /= v;
    this->max /= v;
    return *this;
  }

  /* -- Binary operators -- */

  constexpr rect operator+(const rect r, float scalar) {
    return rect(r.min + scalar, r.max + scalar);
  }

  constexpr rect operator+(const rect r, const vec2 v) {
    return rect(r.min + v, r.max + v);
  }

  constexpr rect operator-(const rect r, float scalar) {
    return rect(r.min - scalar, r.max - scalar);
  }

  constexpr rect operator-(const rect r, const vec2 v) {
    return rect(r.min - v, r.max - v);
  }

  constexpr rect operator*(const rect r, float scalar) {
    return rect(r.min * scalar, r.max * scalar);
  }

  constexpr rect operator*(const rect r, const vec2 v) {
    return rect(r.min * v, r.max * v);
  }

  constexpr rect operator/(const rect r, float scalar) {
    return rect(r.min / scalar, r.max / scalar);
  }

  constexpr rect operator/(const rect r, const vec2 v) {
    return rect(r.min / v, r.max / v);
  }

  constexpr rect operator%(const rect r, float scalar) {
    return rect(r.min % scalar, r.max % scalar);
  }

  constexpr rect operator%(const rect r, const vec2 v) {
    return rect(r.min % v, r.max % v);
  }

}
