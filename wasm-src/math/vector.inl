#include <algorithm>

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
