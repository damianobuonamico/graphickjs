#pragma once

#include <stdint.h>
#include <ostream>

struct vec2 {
  union { float x, r, s; };
  union { float y, g, t; };

  /* -- Component accesses -- */

  static constexpr uint8_t length() { return 2; }
  constexpr float& operator[](uint8_t i);
  constexpr float const& operator[](uint8_t i) const;

  /* -- Constructors -- */

  vec2() = default;
  constexpr vec2(const vec2& v) = default;
  constexpr explicit vec2(float scalar);
  constexpr vec2(float x, float y);

  /* -- Assign operator -- */

  constexpr vec2& operator=(const vec2& v);

  /* -- Unary arithmetic operators -- */

  constexpr vec2& operator+=(float scalar);
  constexpr vec2& operator+=(const vec2& v);
  constexpr vec2& operator-=(float scalar);
  constexpr vec2& operator-=(const vec2& v);
  constexpr vec2& operator*=(float scalar);
  constexpr vec2& operator*=(const vec2& v);
  constexpr vec2& operator/=(float scalar);
  constexpr vec2& operator/=(const vec2& v);

  /* -- Increment/Decrement operators -- */

  constexpr vec2& operator++();
  constexpr vec2& operator--();
};

/* -- Unary operators */

constexpr vec2 operator+(const vec2& v);
constexpr vec2 operator-(const vec2& v);

/* -- Binary operators -- */

constexpr vec2 operator+(const vec2& v, float scalar);
constexpr vec2 operator+(float scalar, const vec2& v);
constexpr vec2 operator+(const vec2& v1, const vec2& v2);
constexpr vec2 operator-(const vec2& v, float scalar);
constexpr vec2 operator-(float scalar, const vec2& v);
constexpr vec2 operator-(const vec2& v1, const vec2& v2);
constexpr vec2 operator*(const vec2& v, float scalar);
constexpr vec2 operator*(float scalar, const vec2& v);
constexpr vec2 operator*(const vec2& v1, const vec2& v2);
constexpr vec2 operator/(const vec2& v, float scalar);
constexpr vec2 operator/(float scalar, const vec2& v);
constexpr vec2 operator/(const vec2& v1, const vec2& v2);
constexpr vec2 operator%(const vec2& v, float scalar);
constexpr vec2 operator%(float scalar, const vec2& v);
constexpr vec2 operator%(const vec2& v1, const vec2& v2);

/* -- Boolean operators -- */

constexpr bool operator==(const vec2& v1, const vec2& v2);
constexpr bool operator!=(const vec2& v1, const vec2& v2);
constexpr vec2 operator&&(const vec2& v1, const vec2& v2);
constexpr vec2 operator||(const vec2& v1, const vec2& v2);

/* -- Stream operator -- */

std::ostream& operator<<(std::ostream& os, const vec2& v);

#include "vec2.inl"