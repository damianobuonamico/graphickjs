#pragma once

#include "vec4.h"

struct mat4 {
  /* -- Component accesses -- */

  static constexpr uint8_t length() { return 4; }
  vec4& operator[](uint8_t i);
  constexpr vec4 const& operator[](uint8_t i) const;

  /* -- Constructors -- */

  constexpr mat4();
  constexpr mat4(const mat4& m);
  constexpr explicit mat4(float scalar);
  constexpr mat4(const vec4& v0, const vec4& v1, const vec4& v2, const vec4& v3);
  constexpr mat4(
    float x0, float y0, float z0, float w0,
    float x1, float y1, float z1, float w1,
    float x2, float y2, float z2, float w2,
    float x3, float y3, float z3, float w3
  );

  /* -- Assign operator -- */

  mat4& operator=(const mat4& m);

  /* -- Unary arithmetic operators -- */

  mat4& operator+=(float scalar);
  mat4& operator+=(const mat4& m);
  mat4& operator-=(float scalar);
  mat4& operator-=(const mat4& m);
  mat4& operator*=(float scalar);
  mat4& operator*=(const mat4& m);
  mat4& operator/=(float scalar);
  mat4& operator/=(const mat4& m);

  /* -- Increment/Decrement operators -- */

  mat4& operator++();
  mat4& operator--();
private:
  vec4 value[4];
};

/* -- Unary operators */

mat4 operator+(const mat4& m);
mat4 operator-(const mat4& m);

/* -- Binary operators -- */

mat4 operator+(const mat4& m, float scalar);
mat4 operator+(float scalar, const mat4& m);
mat4 operator+(const mat4& m1, const mat4& m2);
mat4 operator-(const mat4& m, float scalar);
mat4 operator-(float scalar, const mat4& m);
mat4 operator-(const mat4& m1, const mat4& m2);
mat4 operator*(const mat4& m, float scalar);
mat4 operator*(float scalar, const mat4& m);
vec4 operator*(const mat4& m, const vec4& v);
mat4 operator*(const mat4& m1, const mat4& m2);
mat4 operator/(const mat4& m, float scalar);
mat4 operator/(float scalar, const mat4& m);
vec4 operator/(const mat4& m, const vec4& v);
mat4 operator/(const mat4& m1, const mat4& m2);

/* -- Boolean operators -- */

bool operator==(const mat4& m1, const mat4& m2);
bool operator!=(const mat4& m1, const mat4& m2);

/* -- Address operator -- */

const float* operator&(const mat4& m);

/* -- Stream operator -- */

std::ostream& operator<<(std::ostream& os, const mat4& m);

#include "mat4.inl"
