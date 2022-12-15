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
