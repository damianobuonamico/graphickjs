#include <algorithm>

#include "math.h"

/* -- min -- */

inline vec2 min(const vec2& v1, const vec2& v2) {
  return vec2{
    std::min(v1.x, v2.x),
    std::min(v1.y, v2.y)
  };
}

inline vec3 min(const vec3& v1, const vec3& v2) {
  return vec3{
    std::min(v1.x, v2.x),
    std::min(v1.y, v2.y),
    std::min(v1.z, v2.z)
  };
}

inline vec4 min(const vec4& v1, const vec4& v2) {
  return vec4{
    std::min(v1.x, v2.x),
    std::min(v1.y, v2.y),
    std::min(v1.z, v2.z),
    std::min(v1.w, v2.w),
  };
}

/* -- max -- */

inline vec2 max(const vec2& v1, const vec2& v2) {
  return vec2{
    std::max(v1.x, v2.x),
    std::max(v1.y, v2.y)
  };
}

inline vec3 max(const vec3& v1, const vec3& v2) {
  return vec3{
    std::max(v1.x, v2.x),
    std::max(v1.y, v2.y),
    std::max(v1.z, v2.z)
  };
}

inline vec4 max(const vec4& v1, const vec4& v2) {
  return vec4{
    std::max(v1.x, v2.x),
    std::max(v1.y, v2.y),
    std::max(v1.z, v2.z),
    std::max(v1.w, v2.w),
  };
}

/* -- length -- */

inline float length(const vec2& v) {
  return std::hypot(v.x, v.y);
}

inline float length(const vec3& v) {
  return std::hypot(v.x, v.y, v.z);
}

inline float length(const vec4& v) {
  return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
}

/* -- squared_length -- */

inline float squared_length(const vec2& v) {
  return v.x * v.x + v.y * v.y;
}

inline float squared_length(const vec3& v) {
  return v.x * v.x + v.y * v.y + v.z * v.z;
}

inline float squared_length(const vec4& v) {
  return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
}

/* -- distance -- */

inline float distance(const vec2& v1, const vec2& v2) {
  return std::hypot(v2.x - v1.x, v2.y - v1.y);
}

inline float distance(const vec3& v1, const vec3& v2) {
  return std::hypot(v2.x - v1.x, v2.y - v1.y, v2.z - v1.z);
}

inline float distance(const vec4& v1, const vec4& v2) {
  return length(v2 - v1);
}

/* -- squared_distance -- */

inline float squared_distance(const vec2& v1, const vec2& v2) {
  vec2 v = v2 - v1;
  return dot(v, v);
}

inline float squared_distance(const vec3& v1, const vec3& v2) {
  vec3 v = v2 - v1;
  return dot(v, v);
}

inline float squared_distance(const vec4& v1, const vec4& v2) {
  vec4 v = v2 - v1;
  return dot(v, v);
}

/* -- lerp -- */

inline vec2 lerp(const vec2& v1, const vec2& v2, float t) {
  return vec2{
    v1.x + t * (v2.x - v1.x),
    v1.y + t * (v2.y - v1.y),
  };
}

inline vec3 lerp(const vec3& v1, const vec3& v2, float t) {
  return vec3{
    v1.x + t * (v2.x - v1.x),
    v1.y + t * (v2.y - v1.y),
    v1.z + t * (v2.z - v1.z),
  };
}

inline vec4 lerp(const vec4& v1, const vec4& v2, float t) {
  return vec4{
    v1.x + t * (v2.x - v1.x),
    v1.y + t * (v2.y - v1.y),
    v1.z + t * (v2.z - v1.z),
    v1.w + t * (v2.w - v1.w),
  };
}

/* -- midpoint -- */
inline vec2 midpoint(const vec2& v1, const vec2& v2) {
  return vec2{
    (v1.x + v2.x) * 0.5f,
    (v1.y + v2.y) * 0.5f,
  };
}

inline vec3 midpoint(const vec3& v1, const vec3& v2) {
  return vec3{
    (v1.x + v2.x) * 0.5f,
    (v1.y + v2.y) * 0.5f,
    (v1.z + v2.z) * 0.5f,
  };
}

inline vec4 midpoint(const vec4& v1, const vec4& v2) {
  return vec4{
    (v1.x + v2.x) * 0.5f,
    (v1.y + v2.y) * 0.5f,
    (v1.z + v2.z) * 0.5f,
    (v1.w + v2.w) * 0.5f,
  };
}

/* -- normalize -- */

inline vec2 normalize(const vec2& v) {
  float len = v.x * v.x + v.y * v.y;
  if (len > 0) len = 1 / std::sqrt(len);

  return vec2{
    v.x * len,
    v.y * len,
  };
}

inline vec3 normalize(const vec3& v) {
  float len = v.x * v.x + v.y * v.y + v.z * v.z;
  if (len > 0) len = 1 / std::sqrt(len);

  return vec3{
    v.x * len,
    v.y * len,
    v.z * len,
  };
}

inline vec4 normalize(const vec4& v) {
  float len = v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
  if (len > 0) len = 1 / std::sqrt(len);

  return vec4{
    v.x * len,
    v.y * len,
    v.z * len,
    v.w * len,
  };
}

inline vec2& normalize(const vec2& v, vec2& out) {
  float len = v.x * v.x + v.y * v.y;
  if (len > 0) len = 1 / std::sqrt(len);

  out.x = v.x * len;
  out.y = v.y * len;

  return out;
}

inline vec3& normalize(const vec3& v, vec3& out) {
  float len = v.x * v.x + v.y * v.y + v.z * v.z;
  if (len > 0) len = 1 / std::sqrt(len);

  out.x = v.x * len;
  out.y = v.y * len;
  out.z = v.z * len;

  return out;
}

inline vec4& normalize(const vec4& v, vec4& out) {
  float len = v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
  if (len > 0) len = 1 / std::sqrt(len);

  out.x = v.x * len;
  out.y = v.y * len;
  out.z = v.z * len;
  out.w = v.w * len;

  return out;
}

/* -- normalize_length -- */

inline vec2 normalize_length(const vec2& v, float t) {
  float len = v.x * v.x + v.y * v.y;
  if (len > 0) len = 1 / std::sqrt(len);

  return vec2{
    v.x * len * t,
    v.y * len * t,
  };
}

inline vec3 normalize_length(const vec3& v, float t) {
  float len = v.x * v.x + v.y * v.y + v.z * v.z;
  if (len > 0) len = 1 / std::sqrt(len);

  return vec3{
    v.x * len * t,
    v.y * len * t,
    v.z * len * t,
  };
}

inline vec4 normalize_length(const vec4& v, float t) {
  float len = v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
  if (len > 0) len = 1 / std::sqrt(len);

  return vec4{
    v.x * len * t,
    v.y * len * t,
    v.z * len * t,
    v.w * len * t,
  };
}

inline vec2& normalize_length(const vec2& v, float t, vec2& out) {
  float len = v.x * v.x + v.y * v.y;
  if (len > 0) len = 1 / std::sqrt(len);

  out.x = v.x * len * t;
  out.y = v.y * len * t;

  return out;
}

inline vec3& normalize_length(const vec3& v, float t, vec3& out) {
  float len = v.x * v.x + v.y * v.y + v.z * v.z;
  if (len > 0) len = 1 / std::sqrt(len);

  out.x = v.x * len * t;
  out.y = v.y * len * t;
  out.z = v.z * len * t;

  return out;
}

inline vec4& normalize_length(const vec4& v, float t, vec4& out) {
  float len = v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
  if (len > 0) len = 1 / std::sqrt(len);

  out.x = v.x * len * t;
  out.y = v.y * len * t;
  out.z = v.z * len * t;
  out.w = v.w * len * t;

  return out;
}

/* -- negate -- */

inline vec2 negate(const vec2& v) {
  return vec2{
    -v.x,
    -v.y,
  };
}

inline vec3 negate(const vec3& v) {
  return vec3{
    -v.x,
    -v.y,
    -v.z,
  };
}

inline vec4 negate(const vec4& v) {
  return vec4{
    -v.x,
    -v.y,
    -v.z,
    -v.w,
  };
}

inline vec2& negate(const vec2& v, vec2& out) {
  out.x = -v.x;
  out.y = -v.y;

  return out;
}

inline vec3& negate(const vec3& v, vec3& out) {
  out.x = -v.x;
  out.y = -v.y;
  out.z = -v.z;

  return out;
}

inline vec4& negate(const vec4& v, vec4& out) {
  out.x = -v.x;
  out.y = -v.y;
  out.z = -v.z;
  out.w = -v.w;

  return out;
}

/* -- zero -- */

inline void zero(vec2& v) {
  v.x = v.y = 0.0f;
}

inline void zero(vec3& v) {
  v.x = v.y = v.z = 0.0f;
}

inline void zero(vec4& v) {
  v.x = v.y = v.z = v.w = 0.0f;
}

/* -- is_zero -- */

inline bool is_zero(const vec2& v) {
  return v.x == 0 && v.y == 0;
}

inline bool is_zero(const vec3& v) {
  return v.x == 0 && v.y == 0 && v.z == 0;
}

inline bool is_zero(const vec4& v) {
  return v.x == 0 && v.y == 0 && v.z == 0 && v.w == 0;
}

/* -- not_zero -- */

inline bool not_zero(const vec2& v) {
  return v.x != 0 || v.y != 0;
}

inline bool not_zero(const vec3& v) {
  return v.x != 0 || v.y != 0 || v.z != 0;
}

inline bool not_zero(const vec4& v) {
  return v.x != 0 || v.y != 0 || v.z != 0 || v.w != 0;
}

/* -- is_almost_zero -- */

inline bool is_almost_zero(const vec2& v, const float eps = FLT_EPSILON) {
  return abs(v.x) <= eps && abs(v.y) <= eps;
}

inline bool is_almost_zero(const vec3& v, const float eps = FLT_EPSILON) {
  return abs(v.x) <= eps && abs(v.y) <= eps && abs(v.z) <= eps;
}

inline bool is_almost_zero(const vec4& v, const float eps = FLT_EPSILON) {
  return abs(v.x) <= eps && abs(v.y) <= eps && abs(v.z) <= eps && abs(v.w) <= eps;
}

/* -- dot -- */

inline float dot(const vec2& v1, const vec2& v2) {
  return v1.x * v2.x + v1.y * v2.y;
}

inline float dot(const vec3& v1, const vec3& v2) {
  return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

inline float dot(const vec4& v1, const vec4& v2) {
  return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;
}

/* -- angle -- */

inline float angle(const vec2& v1, const vec2& v2) {
  return sign(v1.x * v2.y - v1.y * v2.x) * std::acos(dot(v1, v2) / (length(v1) * length(v2)));
}

inline float atan2(const vec2& v1, const vec2& v2) {
  return std::atan2(v2.y - v1.y, v2.x - v1.x);
}

/* -- rotate -- */

inline vec2 rotate(const vec2& v, const vec2& c, float t) {
  float cx = v.x - c.x;
  float cy = v.y - c.y;
  float sin = std::sin(t);
  float cos = std::cos(t);

  return vec2{
    cx * cos - cy * sin + c.x,
    cx * sin + cy * cos + c.y,
  };
}

/* -- swap -- */
inline vec2& swap_coordinates(const vec2& v, vec2& out) {
  float temp = v.x;

  out.x = v.y;
  out.y = temp;

  return out;
}
