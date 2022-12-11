#pragma once

#include "vec2.h"

#include <assert.h>

/* -- Component accesses -- */

constexpr float& vec2::operator[](uint8_t i) {
  assert(i >= 0 && i < this->length());
  switch (i) {
  default:
  case 0:
    return x;
  case 1:
    return y;
  }
}

constexpr float const& vec2::operator[](uint8_t i) const {
  assert(i >= 0 && i < this->length());
  switch (i) {
  default:
  case 0:
    return x;
  case 1:
    return y;
  }
}

/* -- Constructors -- */

constexpr vec2::vec2(float scalar): x(scalar), y(scalar) {}

constexpr vec2::vec2(float x, float y) : x(x), y(y) {}

/* -- Assign operator -- */

constexpr vec2& vec2::operator=(const vec2& v) {
  this->x = v.x;
  this->y = v.y;
  return *this;
}

/* -- Unary arithmetic operators -- */

constexpr vec2& vec2::operator+=(float scalar) {
  this->x += scalar;
  this->y += scalar;
  return *this;
}

constexpr vec2& vec2::operator+=(const vec2& v) {
  this->x += v.x;
  this->y += v.y;
  return *this;
}

constexpr vec2& vec2::operator-=(float scalar) {
  this->x -= scalar;
  this->y -= scalar;
  return *this;
}

constexpr vec2& vec2::operator-=(const vec2& v) {
  this->x -= v.x;
  this->y -= v.y;
  return *this;
}

constexpr vec2& vec2::operator*=(float scalar) {
  this->x *= scalar;
  this->y *= scalar;
  return *this;
}

constexpr vec2& vec2::operator*=(const vec2& v) {
  this->x *= v.x;
  this->y *= v.y;
  return *this;
}

constexpr vec2& vec2::operator/=(float scalar) {
  this->x /= scalar;
  this->y /= scalar;
  return *this;
}

constexpr vec2& vec2::operator/=(const vec2& v) {
  this->x /= v.x;
  this->y /= v.y;
  return *this;
}


/* -- Increment/Decrement operators -- */

constexpr vec2& vec2::operator++() {
  ++this->x;
  ++this->y;
  return *this;
}

constexpr vec2& vec2::operator--() {
  --this->x;
  --this->y;
  return *this;
}

/* -- Unary operators */

constexpr vec2 operator+(const vec2& v) {
  return v;
}

constexpr vec2 operator-(const vec2& v) {
  return vec2(-v.x, -v.y);
}

/* -- Binary operators -- */

constexpr vec2 operator+(const vec2& v, float scalar) {
  return vec2(v.x + scalar, v.y + scalar);
}

constexpr vec2 operator+(float scalar, const vec2& v) {
  return vec2(scalar + v.x, scalar + v.y);
}

constexpr vec2 operator+(const vec2& v1, const vec2& v2) {
  return vec2(v1.x + v2.x, v1.y + v2.y);
}

constexpr vec2 operator-(const vec2& v, float scalar) {
  return vec2(v.x - scalar, v.y - scalar);
}

constexpr vec2 operator-(float scalar, const vec2& v) {
  return vec2(scalar - v.x, scalar - v.y);
}

constexpr vec2 operator-(const vec2& v1, const vec2& v2) {
  return vec2(v1.x - v2.x, v1.y - v2.y);
}

constexpr vec2 operator*(const vec2& v, float scalar) {
  return vec2(v.x * scalar, v.y * scalar);
}

constexpr vec2 operator*(float scalar, const vec2& v) {
  return vec2(scalar * v.x, scalar * v.y);
}

constexpr vec2 operator*(const vec2& v1, const vec2& v2) {
  return vec2(v1.x * v2.x, v1.y * v2.y);
}

constexpr vec2 operator/(const vec2& v, float scalar) {
  return vec2(v.x / scalar, v.y / scalar);
}

constexpr vec2 operator/(float scalar, const vec2& v) {
  return vec2(scalar / v.x, scalar / v.y);
}

constexpr vec2 operator/(const vec2& v1, const vec2& v2) {
  return vec2(v1.x / v2.x, v1.y / v2.y);
}

constexpr vec2 operator%(const vec2& v, float scalar) {
  return vec2((int)v.x % (int)scalar, (int)v.y % (int)scalar);
}

constexpr vec2 operator%(float scalar, const vec2& v) {
  return vec2((int)scalar % (int)v.x, (int)scalar % (int)v.y);
}

constexpr vec2 operator%(const vec2& v1, const vec2& v2) {
  return vec2((int)v1.x % (int)v2.x, (int)v1.y % (int)v2.y);
}

/* -- Boolean operators -- */

constexpr bool operator==(const vec2& v1, const vec2& v2) {
  return v1.x == v2.x && v1.y == v2.y;
}

constexpr bool operator!=(const vec2& v1, const vec2& v2) {
  return !(v1 == v2);
}

constexpr vec2 operator&&(const vec2& v1, const vec2& v2) {
  return vec2(v1.x && v2.x, v1.y && v2.y);
}

constexpr vec2 operator||(const vec2& v1, const vec2& v2) {
  return vec2(v1.x || v2.x, v1.y || v2.y);
}

/* -- Stream operator -- */

std::ostream& operator<<(std::ostream& os, const vec2& v) {
  os << "(" << v.x << ", " << v.y << ")";
  return os;
}