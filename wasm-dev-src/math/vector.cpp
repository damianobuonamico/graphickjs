#include "vector.h"

#include "scalar.h"

#include <algorithm>
#include <sstream>

namespace Graphick::Math {

  /* -- min -- */

  vec2 min(const vec2 v1, const vec2 v2) {
    return vec2{
      std::min(v1.x, v2.x),
      std::min(v1.y, v2.y)
    };
  }

  vec3 min(const vec3& v1, const vec3& v2) {
    return vec3{
      std::min(v1.x, v2.x),
      std::min(v1.y, v2.y),
      std::min(v1.z, v2.z)
    };
  }

  vec4 min(const vec4& v1, const vec4& v2) {
    return vec4{
      std::min(v1.x, v2.x),
      std::min(v1.y, v2.y),
      std::min(v1.z, v2.z),
      std::min(v1.w, v2.w),
    };
  }

  void min(const vec2 v1, const vec2 v2, vec2& out) {
    out.x = std::min(v1.x, v2.x);
    out.y = std::min(v1.y, v2.y);
  }

  void min(const vec3& v1, const vec3& v2, vec3& out) {
    out.x = std::min(v1.x, v2.x);
    out.y = std::min(v1.y, v2.y);
    out.z = std::min(v1.z, v2.z);
  }

  void min(const vec4& v1, const vec4& v2, vec4& out) {
    out.x = std::min(v1.x, v2.x);
    out.y = std::min(v1.y, v2.y);
    out.z = std::min(v1.z, v2.z);
    out.w = std::min(v1.w, v2.w);
  }

  /* -- max -- */

  vec2 max(const vec2 v1, const vec2 v2) {
    return vec2{
      std::max(v1.x, v2.x),
      std::max(v1.y, v2.y)
    };
  }

  vec3 max(const vec3& v1, const vec3& v2) {
    return vec3{
      std::max(v1.x, v2.x),
      std::max(v1.y, v2.y),
      std::max(v1.z, v2.z)
    };
  }

  vec4 max(const vec4& v1, const vec4& v2) {
    return vec4{
      std::max(v1.x, v2.x),
      std::max(v1.y, v2.y),
      std::max(v1.z, v2.z),
      std::max(v1.w, v2.w),
    };
  }

  void max(const vec2 v1, const vec2 v2, vec2& out) {
    out.x = std::max(v1.x, v2.x);
    out.y = std::max(v1.y, v2.y);
  }

  void max(const vec3& v1, const vec3& v2, vec3& out) {
    out.x = std::max(v1.x, v2.x);
    out.y = std::max(v1.y, v2.y);
    out.z = std::max(v1.z, v2.z);
  }

  void max(const vec4& v1, const vec4& v2, vec4& out) {
    out.x = std::max(v1.x, v2.x);
    out.y = std::max(v1.y, v2.y);
    out.z = std::max(v1.z, v2.z);
    out.w = std::max(v1.w, v2.w);
  }

  /* -- length -- */

  float length(const vec2 v) {
    return std::hypot(v.x, v.y);
  }

  float length(const vec3& v) {
    return std::hypot(v.x, v.y, v.z);
  }

  float length(const vec4& v) {
    return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
  }

  /* -- squared_length -- */

  float squared_length(const vec2 v) {
    return v.x * v.x + v.y * v.y;
  }

  float squared_length(const vec3& v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
  }

  float squared_length(const vec4& v) {
    return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
  }

  /* -- distance -- */

  float distance(const vec2 v1, const vec2 v2) {
    return std::hypot(v2.x - v1.x, v2.y - v1.y);
  }

  float distance(const vec3& v1, const vec3& v2) {
    return std::hypot(v2.x - v1.x, v2.y - v1.y, v2.z - v1.z);
  }

  float distance(const vec4& v1, const vec4& v2) {
    return length(v2 - v1);
  }

  /* -- squared_distance -- */

  float squared_distance(const vec2 v1, const vec2 v2) {
    vec2 v = v2 - v1;
    return dot(v, v);
  }

  float squared_distance(const vec3& v1, const vec3& v2) {
    vec3 v = v2 - v1;
    return dot(v, v);
  }

  float squared_distance(const vec4& v1, const vec4& v2) {
    vec4 v = v2 - v1;
    return dot(v, v);
  }

  /* -- lerp -- */

  vec2 lerp(const vec2 v1, const vec2 v2, float t) {
    return vec2{
      v1.x + t * (v2.x - v1.x),
      v1.y + t * (v2.y - v1.y),
    };
  }

  vec3 lerp(const vec3& v1, const vec3& v2, float t) {
    return vec3{
      v1.x + t * (v2.x - v1.x),
      v1.y + t * (v2.y - v1.y),
      v1.z + t * (v2.z - v1.z),
    };
  }

  vec4 lerp(const vec4& v1, const vec4& v2, float t) {
    return vec4{
      v1.x + t * (v2.x - v1.x),
      v1.y + t * (v2.y - v1.y),
      v1.z + t * (v2.z - v1.z),
      v1.w + t * (v2.w - v1.w),
    };
  }

  /* -- quadratic -- */

  vec2 quadratic(const vec2 v1, const vec2 v2, const vec2 v3, float t) {
    vec2 a = v1 - 2.0f * v2 + v3;
    vec2 b = 2.0f * (v2 - v1);
    vec2 c = v1;

    return a * t * t + b * t + c;
  }

  vec2 quadratic_derivative(const vec2 v1, const vec2 v2, const vec2 v3, float t) {
    return 2.0f * (v1 - 2.0f * v2 + v3) * t + 2.0f * (v2 - v1);
  }

  /* -- bezier -- */

  vec2 bezier(const vec2 v1, const vec2 v2, const vec2 v3, const vec2 v4, float t) {
    vec2 a = -v1 + 3.0f * v2 - 3.0f * v3 + v4;
    vec2 b = 3.0f * v1 - 6.0f * v2 + 3.0f * v3;
    vec2 c = -3.0f * v1 + 3.0f * v2;

    float t_sq = t * t;

    return a * t_sq * t + b * t_sq + c * t + v1;
  }

  vec2 bezier_derivative(const vec2 v1, const vec2 v2, const vec2 v3, const vec2 v4, float t) {
    vec2 a = 3.0f * (-v1 + 3.0f * v2 - 3.0f * v3 + v4);
    vec2 b = 6.0f * (v1 - 2.0f * v2 + v3);
    vec2 c = -3.0f * (v1 - v2);

    return a * t * t + b * t + c;
  }

  vec2 bezier_second_derivative(const vec2 v1, const vec2 v2, const vec2 v3, const vec2 v4, float t) {
    vec2 a = 6.0f * (-v1 + 3.0f * v2 - 3.0f * v3 + v4);
    vec2 b = 6.0f * (v1 - 2.0f * v2 + v3);

    return a * t + b;
  }

  std::vector<float> bezier_extrema(const vec2 v1, const vec2 v2, const vec2 v3, const vec2 v4) {
    const vec2 a = 3.0f * (-v1 + 3.0f * v2 - 3.0f * v3 + v4);
    const vec2 b = 6.0f * (v1 - 2.0f * v2 + v3);
    const vec2 c = 3.0f * (v2 - v1);

    std::vector<float> roots = { 0.0f, 1.0f };

    for (int i = 0; i < 2; i++) {
      if (Math::is_almost_zero(a[i])) {
        if (Math::is_almost_zero(b[i])) continue;

        float t = -c[i] / b[i];
        if (t > 0.0f && t < 1.0f) {
          roots.push_back(t);
        }

        continue;
      }

      float delta = b[i] * b[i] - 4.0f * a[i] * c[i];

      if (Math::is_almost_zero(delta)) {
        roots.push_back(-b[i] / (2.0f * a[i]));
      } else if (delta < 0.0f) {
        continue;
      } else {
        float sqrt_delta = std::sqrtf(delta);

        float t1 = (-b[i] + sqrt_delta) / (2.0f * a[i]);
        float t2 = (-b[i] - sqrt_delta) / (2.0f * a[i]);

        if (t1 > 0.0f && t1 < 1.0f) {
          roots.push_back(t1);
        }
        if (t2 > 0.0f && t2 < 1.0f) {
          roots.push_back(t2);
        }
      }
    }

    return roots;
  }

  /* -- midpoint -- */

  vec2 midpoint(const vec2 v1, const vec2 v2) {
    return vec2{
      (v1.x + v2.x) * 0.5f,
      (v1.y + v2.y) * 0.5f,
    };
  }

  vec3 midpoint(const vec3& v1, const vec3& v2) {
    return vec3{
      (v1.x + v2.x) * 0.5f,
      (v1.y + v2.y) * 0.5f,
      (v1.z + v2.z) * 0.5f,
    };
  }

  vec4 midpoint(const vec4& v1, const vec4& v2) {
    return vec4{
      (v1.x + v2.x) * 0.5f,
      (v1.y + v2.y) * 0.5f,
      (v1.z + v2.z) * 0.5f,
      (v1.w + v2.w) * 0.5f,
    };
  }

  /* -- normalize -- */

  vec2 normalize(const vec2 v) {
    float len = v.x * v.x + v.y * v.y;
    if (len > 0) len = 1 / std::sqrt(len);

    return vec2{
      v.x * len,
      v.y * len,
    };
  }

  vec3 normalize(const vec3& v) {
    float len = v.x * v.x + v.y * v.y + v.z * v.z;
    if (len > 0) len = 1 / std::sqrt(len);

    return vec3{
      v.x * len,
      v.y * len,
      v.z * len,
    };
  }

  vec4 normalize(const vec4& v) {
    float len = v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
    if (len > 0) len = 1 / std::sqrt(len);

    return vec4{
      v.x * len,
      v.y * len,
      v.z * len,
      v.w * len,
    };
  }

  vec2& normalize(const vec2 v, vec2& out) {
    float len = v.x * v.x + v.y * v.y;
    if (len > 0) len = 1 / std::sqrt(len);

    out.x = v.x * len;
    out.y = v.y * len;

    return out;
  }

  vec3& normalize(const vec3& v, vec3& out) {
    float len = v.x * v.x + v.y * v.y + v.z * v.z;
    if (len > 0) len = 1 / std::sqrt(len);

    out.x = v.x * len;
    out.y = v.y * len;
    out.z = v.z * len;

    return out;
  }

  vec4& normalize(const vec4& v, vec4& out) {
    float len = v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
    if (len > 0) len = 1 / std::sqrt(len);

    out.x = v.x * len;
    out.y = v.y * len;
    out.z = v.z * len;
    out.w = v.w * len;

    return out;
  }

  /* -- normalize_length -- */

  vec2 normalize_length(const vec2 v, float t) {
    float len = v.x * v.x + v.y * v.y;
    if (len > 0) len = 1 / std::sqrt(len);

    return vec2{
      v.x * len * t,
      v.y * len * t,
    };
  }

  vec3 normalize_length(const vec3& v, float t) {
    float len = v.x * v.x + v.y * v.y + v.z * v.z;
    if (len > 0) len = 1 / std::sqrt(len);

    return vec3{
      v.x * len * t,
      v.y * len * t,
      v.z * len * t,
    };
  }

  vec4 normalize_length(const vec4& v, float t) {
    float len = v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
    if (len > 0) len = 1 / std::sqrt(len);

    return vec4{
      v.x * len * t,
      v.y * len * t,
      v.z * len * t,
      v.w * len * t,
    };
  }

  vec2& normalize_length(const vec2 v, float t, vec2& out) {
    float len = v.x * v.x + v.y * v.y;
    if (len > 0) len = 1 / std::sqrt(len);

    out.x = v.x * len * t;
    out.y = v.y * len * t;

    return out;
  }

  vec3& normalize_length(const vec3& v, float t, vec3& out) {
    float len = v.x * v.x + v.y * v.y + v.z * v.z;
    if (len > 0) len = 1 / std::sqrt(len);

    out.x = v.x * len * t;
    out.y = v.y * len * t;
    out.z = v.z * len * t;

    return out;
  }

  vec4& normalize_length(const vec4& v, float t, vec4& out) {
    float len = v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
    if (len > 0) len = 1 / std::sqrt(len);

    out.x = v.x * len * t;
    out.y = v.y * len * t;
    out.z = v.z * len * t;
    out.w = v.w * len * t;

    return out;
  }

  /* -- negate -- */

  vec2 negate(const vec2 v) {
    return vec2{
      -v.x,
      -v.y,
    };
  }

  vec3 negate(const vec3& v) {
    return vec3{
      -v.x,
      -v.y,
      -v.z,
    };
  }

  vec4 negate(const vec4& v) {
    return vec4{
      -v.x,
      -v.y,
      -v.z,
      -v.w,
    };
  }

  vec2& negate(const vec2 v, vec2& out) {
    out.x = -v.x;
    out.y = -v.y;

    return out;
  }

  vec3& negate(const vec3& v, vec3& out) {
    out.x = -v.x;
    out.y = -v.y;
    out.z = -v.z;

    return out;
  }

  vec4& negate(const vec4& v, vec4& out) {
    out.x = -v.x;
    out.y = -v.y;
    out.z = -v.z;
    out.w = -v.w;

    return out;
  }

  /* -- abs -- */

  vec2 abs(const vec2 v) {
    return vec2{
      std::fabsf(v.x),
      std::fabsf(v.y),
    };
  }

  vec3 abs(const vec3& v) {
    return vec3{
      std::fabsf(v.x),
      std::fabsf(v.y),
      std::fabsf(v.z),
    };
  }

  vec4 abs(const vec4& v) {
    return vec4{
      std::fabsf(v.x),
      std::fabsf(v.y),
      std::fabsf(v.z),
      std::fabsf(v.w),
    };
  }

  vec2& abs(const vec2 v, vec2& out) {
    out.x = std::fabs(v.x);
    out.y = std::fabs(v.y);

    return out;
  }

  vec3& abs(const vec3& v, vec3& out) {
    out.x = std::fabs(v.x);
    out.y = std::fabs(v.y);
    out.z = std::fabs(v.z);

    return out;
  }

  vec4& abs(const vec4& v, vec4& out) {
    out.x = std::fabs(v.x);
    out.y = std::fabs(v.y);
    out.z = std::fabs(v.z);
    out.w = std::fabs(v.w);

    return out;
  }

  /* -- zero -- */

  void zero(vec2& v) {
    v.x = v.y = 0.0f;
  }

  void zero(vec3& v) {
    v.x = v.y = v.z = 0.0f;
  }

  void zero(vec4& v) {
    v.x = v.y = v.z = v.w = 0.0f;
  }

  /* -- is_zero -- */

  bool is_zero(const vec2 v) {
    return v.x == 0 && v.y == 0;
  }

  bool is_zero(const vec3& v) {
    return v.x == 0 && v.y == 0 && v.z == 0;
  }

  bool is_zero(const vec4& v) {
    return v.x == 0 && v.y == 0 && v.z == 0 && v.w == 0;
  }

  /* -- not_zero -- */

  bool not_zero(const vec2 v) {
    return v.x != 0 || v.y != 0;
  }

  bool not_zero(const vec3& v) {
    return v.x != 0 || v.y != 0 || v.z != 0;
  }

  bool not_zero(const vec4& v) {
    return v.x != 0 || v.y != 0 || v.z != 0 || v.w != 0;
  }

  /* -- is_almost_zero -- */

  bool is_almost_zero(const vec2 v, const float eps) {
    return !(std::abs(v.x) > eps || std::abs(v.y) > eps);
  }

  bool is_almost_zero(const vec3& v, const float eps) {
    return !(std::abs(v.x) > eps || std::abs(v.y) > eps || std::abs(v.z) > eps);
  }

  bool is_almost_zero(const vec4& v, const float eps) {
    return !(std::abs(v.x) > eps || std::abs(v.y) > eps || std::abs(v.z) > eps || std::abs(v.w) > eps);
  }

  /* -- is_almost_equal -- */

  bool is_almost_equal(const vec2 v1, const vec2 v2, const float eps) {
    return !(std::abs(v1.x - v2.x) > eps || std::abs(v1.y - v2.y) > eps);
  }

  bool is_almost_equal(const vec3& v1, const vec3& v2, const float eps) {
    return !(std::abs(v1.x - v2.x) > eps || std::abs(v1.y - v2.y) > eps || std::abs(v1.z - v2.z) > eps);
  }

  bool is_almost_equal(const vec4& v1, const vec4& v2, const float eps) {
    return !(std::abs(v1.x - v2.x) > eps || std::abs(v1.y - v2.y) > eps || std::abs(v1.z - v2.z) > eps || std::abs(v1.w - v2.w) > eps);
  }

  /* -- floor -- */

  vec2 floor(const vec2 v) {
    return vec2{
      std::floor(v.x),
      std::floor(v.y),
    };
  }

  vec3 floor(const vec3& v) {
    return vec3{
      std::floor(v.x),
      std::floor(v.y),
      std::floor(v.z),
    };
  }

  vec4 floor(const vec4& v) {
    return vec4{
      std::floor(v.x),
      std::floor(v.y),
      std::floor(v.z),
      std::floor(v.w),
    };
  }

  /* -- ceil -- */

  vec2 ceil(const vec2 v) {
    return vec2{
      std::ceil(v.x),
      std::ceil(v.y),
    };
  }

  vec3 ceil(const vec3& v) {
    return vec3{
      std::ceil(v.x),
      std::ceil(v.y),
      std::ceil(v.z),
    };
  }

  vec4 ceil(const vec4& v) {
    return vec4{
      std::ceil(v.x),
      std::ceil(v.y),
      std::ceil(v.z),
      std::ceil(v.w),
    };
  }

  /* -- round -- */

  vec2 round(const vec2 v) {
    return vec2{
      std::round(v.x),
      std::round(v.y),
    };
  }

  vec3 round(const vec3& v) {
    return vec3{
      std::round(v.x),
      std::round(v.y),
      std::round(v.z),
    };
  }

  vec4 round(const vec4& v) {
    return vec4{
      std::round(v.x),
      std::round(v.y),
      std::round(v.z),
      std::round(v.w),
    };
  }

  /* -- dot -- */

  float dot(const vec2 v1, const vec2 v2) {
    return v1.x * v2.x + v1.y * v2.y;
  }

  float dot(const vec3& v1, const vec3& v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
  }

  float dot(const vec4& v1, const vec4& v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;
  }

  /* -- cross -- */

  float cross(const vec2 v1, const vec2 v2) {
    return v1.x * v2.y - v2.x * v1.y;
  }

  vec3 cross(const vec3& v1, const vec3& v2) {
    return {
      v1.y * v2.z - v1.z * v2.y,
      -(v1.x * v2.z - v1.z * v2.x),
      v1.x * v2.y - v1.y * v2.x
    };
  }

  /* -- angle -- */

  float angle(const vec2 v1, const vec2 v2) {
    return sign(v1.x * v2.y - v1.y * v2.x) * std::acos(dot(v1, v2) / (length(v1) * length(v2)));
  }

  float atan2(const vec2 v1, const vec2 v2) {
    return std::atan2(v2.y - v1.y, v2.x - v1.x);
  }

  /* -- rotate -- */

  vec2 rotate(const vec2 v, const vec2 c, float t) {
    float cx = v.x - c.x;
    float cy = v.y - c.y;
    float sin = std::sin(t);
    float cos = std::cos(t);

    return vec2{
      cx * cos - cy * sin + c.x,
      cx * sin + cy * cos + c.y,
    };
  }

  vec2 rotate(const vec2 v, const vec2 c, float sin_t, float cos_t) {
    float cx = v.x - c.x;
    float cy = v.y - c.y;

    return vec2{
      cx * cos_t - cy * sin_t + c.x,
      cx * sin_t + cy * cos_t + c.y,
    };
  }

  /* -- scale -- */

  vec2 scale(const vec2 v, const vec2 c, const vec2 s) {
    return vec2{
      (v.x - c.x) * s.x + c.x,
      (v.y - c.y) * s.y + c.y
    };
  }

  /* -- orthogonal -- */

  vec2 orthogonal(const vec2 v) {
    return vec2{ -v.y, v.x };
  }

  void orthogonal(const vec2 v, vec2& out) {
    float temp = v.x;

    out.x = -v.y;
    out.y = temp;
  }

  /* -- swap_coordinates -- */

  vec2 swap_coordinates(const vec2 v) {
    return {
      v.y,
      v.x
    };
  }

  vec2& swap_coordinates(const vec2 v, vec2& out) {
    float temp = v.x;

    out.x = v.y;
    out.y = temp;

    return out;
  }

  /* -- collinear -- */

  bool collinear(const vec2 v1, const vec2 v2, const vec2 v3, const float eps) {
    float t = v1.x * (v2.y - v3.y) + v2.x * (v3.y - v1.y) + v3.x * (v1.y - v2.y);
    return is_almost_zero(t, eps);
  }

  /* -- stringify -- */

  std::string stringify(const vec2 v) {
    std::stringstream ss;
    ss << "[" << v.x << ',' << v.y << "]";
    return ss.str();
  }

  std::string stringify(const vec3& v) {
    std::stringstream ss;
    ss << "[" << v.x << ',' << v.y << ',' << v.z << "]";
    return ss.str();
  }

  std::string stringify(const vec4& v) {
    std::stringstream ss;
    ss << "[" << v.x << ',' << v.y << ',' << v.z << ',' << v.w << "]";
    return ss.str();
  }

}
