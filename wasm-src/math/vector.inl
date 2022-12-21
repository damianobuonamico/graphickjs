#include <algorithm>

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
