#include "math.h"

#include <cmath>

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

inline bool is_almost_zero(const float t, const float eps = FLT_EPSILON) {
  return abs(t) <= eps;
}
