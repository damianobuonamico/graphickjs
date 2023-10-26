#pragma once

#include "vec2.h"
#include "vec3.h"
#include "vec4.h"

#include "../utils/defines.h"

#include <vector>

#define XY(v) Graphick::Math::vec2{ v.x, v.y }
#define RG(v) Graphick::Math::vec2{ v.r, v.g }
#define ST(v) Graphick::Math::vec2{ v.s, v.t }

#define XYZ(v) Graphick::Math::vec3{ v.x, v.y, v.z }
#define RGB(v) Graphick::Math::vec3{ v.r, v.g, v.b }
#define STP(v) Graphick::Math::vec3{ v.s, v.t, v.p }

#define IVEC2_TO_VEC2(v) Graphick::Math::vec2{ (float)v.x, (float)v.y }
#define VEC2_TO_IVEC2(v) Graphick::Math::vec2{ (int)v.x, (int)v.y }

namespace Graphick::Math {

  vec2 min(const vec2 v1, const vec2 v2);
  vec3 min(const vec3& v1, const vec3& v2);
  vec4 min(const vec4& v1, const vec4& v2);
  void min(const vec2 v1, const vec2 v2, vec2& out);
  void min(const vec3& v1, const vec3& v2, vec3& out);
  void min(const vec4& v1, const vec4& v2, vec4& out);

  vec2 max(const vec2 v1, const vec2 v2);
  vec3 max(const vec3& v1, const vec3& v2);
  vec4 max(const vec4& v1, const vec4& v2);
  void max(const vec2 v1, const vec2 v2, vec2& out);
  void max(const vec3& v1, const vec3& v2, vec3& out);
  void max(const vec4& v1, const vec4& v2, vec4& out);

  float length(const vec2 v);
  float length(const vec3& v);
  float length(const vec4& v);

  float squared_length(const vec2 v);
  float squared_length(const vec3& v);
  float squared_length(const vec4& v);

  float distance(const vec2 v1, const vec2 v2);
  float distance(const vec3& v1, const vec3& v2);
  float distance(const vec4& v1, const vec4& v2);

  float squared_distance(const vec2 v1, const vec2 v2);
  float squared_distance(const vec3& v1, const vec3& v2);
  float squared_distance(const vec4& v1, const vec4& v2);

  vec2 lerp(const vec2 v1, const vec2 v2, float t);
  vec3 lerp(const vec3& v1, const vec3& v2, float t);
  vec4 lerp(const vec4& v1, const vec4& v2, float t);

  vec2 quadratic(const vec2 v1, const vec2 v2, const vec2 v3, float t);
  vec2 quadratic_derivative(const vec2 v1, const vec2 v2, const vec2 v3, float t);

  vec2 bezier(const vec2 v1, const vec2 v2, const vec2 v3, const vec2 v4, float t);
  vec2 bezier_derivative(const vec2 v1, const vec2 v2, const vec2 v3, const vec2 v4, float t);
  vec2 bezier_second_derivative(const vec2 v1, const vec2 v2, const vec2 v3, const vec2 v4, float t);
  std::vector<float> bezier_extrema(const vec2 v1, const vec2 v2, const vec2 v3, const vec2 v4);

  vec2 midpoint(const vec2 v1, const vec2 v2);
  vec3 midpoint(const vec3& v1, const vec3& v2);
  vec4 midpoint(const vec4& v1, const vec4& v2);

  vec2 normalize(const vec2 v);
  vec3 normalize(const vec3& v);
  vec4 normalize(const vec4& v);
  vec2& normalize(const vec2 v, vec2& out);
  vec3& normalize(const vec3& v, vec3& out);
  vec4& normalize(const vec4& v, vec4& out);

  vec2 normalize_length(const vec2 v, float t);
  vec3 normalize_length(const vec3& v, float t);
  vec4 normalize_length(const vec4& v, float t);
  vec2& normalize_length(const vec2 v, float t, vec2& out);
  vec3& normalize_length(const vec3& v, float t, vec3& out);
  vec4& normalize_length(const vec4& v, float t, vec4& out);

  vec2 negate(const vec2 v);
  vec3 negate(const vec3& v);
  vec4 negate(const vec4& v);
  vec2& negate(const vec2 v, vec2& out);
  vec3& negate(const vec3& v, vec3& out);
  vec4& negate(const vec4& v, vec4& out);

  vec2 abs(const vec2 v);
  vec3 abs(const vec3& v);
  vec4 abs(const vec4& v);
  vec2& abs(const vec2 v, vec2& out);
  vec3& abs(const vec3& v, vec3& out);
  vec4& abs(const vec4& v, vec4& out);

  void zero(vec2& v);
  void zero(vec3& v);
  void zero(vec4& v);

  bool is_zero(const vec2 v);
  bool is_zero(const vec3& v);
  bool is_zero(const vec4& v);

  bool not_zero(const vec2 v);
  bool not_zero(const vec3& v);
  bool not_zero(const vec4& v);

  bool is_almost_zero(const vec2 v, const float eps = GK_EPSILON);
  bool is_almost_zero(const vec3& v, const float eps = GK_EPSILON);
  bool is_almost_zero(const vec4& v, const float eps = GK_EPSILON);

  bool is_almost_equal(const vec2 v1, const vec2 v2, const float eps = GK_EPSILON);
  bool is_almost_equal(const vec3& v1, const vec3& v2, const float eps = GK_EPSILON);
  bool is_almost_equal(const vec4& v1, const vec4& v2, const float eps = GK_EPSILON);

  vec2 floor(const vec2 v);
  vec3 floor(const vec3& v);
  vec4 floor(const vec4& v);

  vec2 ceil(const vec2 v);
  vec3 ceil(const vec3& v);
  vec4 ceil(const vec4& v);

  vec2 round(const vec2 v);
  vec3 round(const vec3& v);
  vec4 round(const vec4& v);

  float dot(const vec2 v1, const vec2 v2);
  float dot(const vec3& v1, const vec3& v2);
  float dot(const vec4& v1, const vec4& v2);

  float angle(const vec2 v1, const vec2 v2);
  float atan2(const vec2 v1, const vec2 v2);

  vec2 rotate(const vec2 v, const vec2 c, float t);
  vec2 rotate(const vec2 v, const vec2 c, float sin_t, float cos_t);

  vec2 scale(const vec2 v, const vec2 c, const vec2 s);

  vec2 orthogonal(const vec2 v);
  void orthogonal(const vec2 v, vec2& out);

  vec2 swap_coordinates(const vec2 v);
  vec2& swap_coordinates(const vec2 v, vec2& out);

  bool collinear(const vec2 v1, const vec2 v2, const vec2 v3, const float eps = GK_EPSILON);

  std::string stringify(const vec2 v);
  std::string stringify(const vec3& v);
  std::string stringify(const vec4& v);

}
