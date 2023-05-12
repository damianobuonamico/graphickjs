#pragma once

#include "vec2.h"

#include <stdint.h>
#include <ostream>

struct ivec2 {
  union { int x, r, s; };
  union { int y, g, t; };

  /* -- Component accesses -- */

  static constexpr uint8_t length() { return 2; }
  constexpr int& operator[](uint8_t i);
  constexpr int const& operator[](uint8_t i) const;

  /* -- Constructors -- */

  ivec2() = default;
  constexpr ivec2(const ivec2& v) = default;
  constexpr explicit ivec2(int scalar);
  constexpr ivec2(int x, int y);

  /* -- Assign operator -- */

  constexpr ivec2& operator=(const ivec2& v);

  /* -- Unary arithmetic operators -- */

  constexpr ivec2& operator+=(int scalar);
  constexpr ivec2& operator+=(const ivec2& v);
  constexpr ivec2& operator-=(int scalar);
  constexpr ivec2& operator-=(const ivec2& v);
  constexpr ivec2& operator*=(int scalar);
  constexpr ivec2& operator*=(const ivec2& v);
  constexpr ivec2& operator/=(int scalar);
  constexpr ivec2& operator/=(const ivec2& v);

  /* -- Increment/Decrement operators -- */

  constexpr ivec2& operator++();
  constexpr ivec2& operator--();
};

/* -- Unary operators */

constexpr ivec2 operator+(const ivec2& v);
constexpr ivec2 operator-(const ivec2& v);

/* -- Binary operators -- */

constexpr ivec2 operator+(const ivec2& v, int scalar);
constexpr ivec2 operator+(int scalar, const ivec2& v);
constexpr ivec2 operator+(const ivec2& v1, const ivec2& v2);
constexpr ivec2 operator-(const ivec2& v, int scalar);
constexpr ivec2 operator-(int scalar, const ivec2& v);
constexpr ivec2 operator-(const ivec2& v1, const ivec2& v2);
constexpr ivec2 operator*(const ivec2& v, int scalar);
constexpr ivec2 operator*(int scalar, const ivec2& v);
constexpr ivec2 operator*(const ivec2& v1, const ivec2& v2);
constexpr ivec2 operator/(const ivec2& v, int scalar);
constexpr ivec2 operator/(int scalar, const ivec2& v);
constexpr ivec2 operator/(const ivec2& v1, const ivec2& v2);
constexpr ivec2 operator%(const ivec2& v, int scalar);
constexpr ivec2 operator%(int scalar, const ivec2& v);
constexpr ivec2 operator%(const ivec2& v1, const ivec2& v2);

/* -- Boolean operators -- */

constexpr bool operator==(const ivec2& v1, const ivec2& v2);
constexpr bool operator!=(const ivec2& v1, const ivec2& v2);
constexpr ivec2 operator&&(const ivec2& v1, const ivec2& v2);
constexpr ivec2 operator||(const ivec2& v1, const ivec2& v2);

/* -- Address operator -- */

constexpr const int* operator&(const ivec2& v);

#include "ivec2.inl"
