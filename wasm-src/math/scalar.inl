#include "scalar.h"

inline float round(float t, float precision) noexcept {
  if (precision >= 1.0f) {
    return std::round(t / precision) * precision;
  }

  float integer_part = std::floor(t);
  float decimal_part = t - integer_part;

  return integer_part + std::round(decimal_part / precision) * precision;
}

inline float map(float t, float old_min, float old_max, float new_min, float new_max) {
  return ((t - old_min) * (new_max - new_min)) / (old_max - old_min) + new_min;
}

inline float clamp(float t, float min, float max) {
  if (t < min) return min;
  if (t > max) return max;
  return t;
}

inline float sign(float t) {
  return (float)((t > 0) - (t < 0));
}

inline float lerp(float a, float b, float t) {
  return a + (b - a) * t;
}

inline bool is_almost_zero(const float t, const float eps = FLT_EPSILON) {
  return abs(t) <= eps;
}

inline bool is_almost_equal(const float t1, const float t2, const float eps = FLT_EPSILON) {
  return abs(t1 - t2) <= eps;
}

inline bool is_normalized(const float t, bool include_ends = true) {
  if (include_ends) {
    return t >= 0.0f && t <= 1.0f;
  }

  return t > 0.0f && t < 1.0f;
}

inline bool is_in_range(const float t, const float min, const float max, bool include_ends = true) {
  if (include_ends) {
    return t >= min && t <= max;
  }

  return t > min && t < max;
}
