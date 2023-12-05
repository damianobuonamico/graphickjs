#pragma once

namespace Graphick::Math {

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

  inline float lerp(float a, float b, float t) {
    return a + (b - a) * t;
  }

  inline int wrap(int t, int min, int max) {
    int range_size = max - min + 1;

    if (t < min) {
      t += range_size * ((min - t) / range_size + 1);
    }

    return min + (t - min) % range_size;
  }

  inline bool is_almost_zero(const float t, const float eps = GK_EPSILON) {
    return std::fabsf(t) <= eps;
  }

  inline bool is_almost_zero(const double t, const double eps = GK_EPSILON) {
    return std::abs(t) <= eps;
  }

  inline bool is_almost_equal(const float t1, const float t2, const float eps = GK_EPSILON) {
    return std::abs(t1 - t2) <= eps;
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

  inline float degrees_to_radians(float a) {
    return a * MATH_PI / 180.0f;
  }

  inline float radians_to_degrees(float a) {
    return a * 180.0f / MATH_PI;
  }

  inline size_t next_power_of_two(size_t n) {
    n--;

    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
#ifndef EMSCRIPTEN
    n |= n >> 32;
#endif

    return n++;
  }

}
