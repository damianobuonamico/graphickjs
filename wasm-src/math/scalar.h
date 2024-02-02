/**
 * @file scalar.h
 * @brief This file contains scalar math functions.
 */

#pragma once

#include "../utils/defines.h"

#include <cstdint>
#include <cfloat>
#include <cmath>

/* -- Defines -- */

#define MATH_F_PI 3.14159265358979323846f
#define MATH_F_TWO_PI (MATH_F_PI * 2.0f)
#define MATH_PI 3.14159265358979323846
#define MATH_TWO_PI MATH_PI * 2.0

namespace Graphick::Math {

  /**
   * @brief Rounds a float to a certain number of decimals.
   *
   * @param t The float to round.
   * @param precision The smallest value to round to.
   * @return The rounded float.
   */
  inline float round(float t, float precision) noexcept {
    if (precision >= 1.0f) {
      return std::round(t / precision) * precision;
    }

    float integer_part = std::floor(t);
    float decimal_part = t - integer_part;

    return integer_part + std::round(decimal_part / precision) * precision;
  }

  /**
   * @brief Maps a value from one range to another.
   *
   * @param t The value to map.
   * @param old_min The minimum value of the old range.
   * @param old_max The maximum value of the old range.
   * @param new_min The minimum value of the new range.
   * @param new_max The maximum value of the new range.
   * @return The mapped value.
   */
  inline float map(float t, float old_min, float old_max, float new_min, float new_max) {
    return (
      ((t - old_min) * (new_max - new_min)) /
      (old_max - old_min) + new_min
    );
  }

  /**
   * @brief Clamps a value between a minimum and a maximum.
   *
   * @param t The value to clamp.
   * @param min The minimum value.
   * @param max The maximum value.
   * @return The clamped value.
   */
  inline float clamp(float t, float min, float max) {
    if (t < min) return min;
    if (t > max) return max;
    return t;
  }

  /**
   * @brief Linearly interpolates between two values.
   *
   * @param a The first value.
   * @param b The second value.
   * @param t The interpolation value.
   * @return The interpolated value.
   */
  inline float lerp(float a, float b, float t) {
    return a + (b - a) * t;
  }

  /**
   * @brief Wraps a value between a minimum and a maximum.
   *
   * @param t The value to wrap.
   * @param min The minimum value.
   * @param max The maximum value.
   * @return The wrapped value.
   */
  inline int wrap(int t, int min, int max) {
    int range_size = max - min + 1;

    if (t < min) {
      t += range_size * ((min - t) / range_size + 1);
    }

    return min + (t - min) % range_size;
  }

  /**
   * @brief Checks if a float is almost zero.
   *
   * @param t The float to check.
   * @param eps The precision to check with.
   * @return Whether the float is almost zero.
   */
  inline bool is_almost_zero(const float t, const float eps = GK_EPSILON) {
    return std::fabsf(t) <= eps;
  }

  /**
   * @brief Checks if a double is almost zero.
   *
   * @param t The double to check.
   * @param eps The precision to check with.
   * @return Whether the double is almost zero.
   */
  inline bool is_almost_zero(const double t, const double eps = GK_EPSILON) {
    return std::abs(t) <= eps;
  }

  /**
   * @brief Checks if two floats are almost equal.
   *
   * @param t1 The first float.
   * @param t2 The second float.
   * @param eps The precision to check with.
   * @return Whether the floats are almost equal.
   */
  inline bool is_almost_equal(const float t1, const float t2, const float eps = GK_EPSILON) {
    return std::abs(t1 - t2) <= eps;
  }

  /**
   * @brief Checks if two doubles are almost equal.
   *
   * @param t1 The first double.
   * @param t2 The second double.
   * @param eps The precision to check with.
   * @return Whether the doubles are almost equal.
   */
  inline bool is_almost_equal(const double t1, const double t2, const double eps = GK_EPSILON) {
    return std::abs(t1 - t2) <= eps;
  }

  /**
   * @brief Checks if a float is normalized.
   *
   * @param t The float to check.
   * @param include_ends Whether to include the ends of the range, defaults to true.
   * @return Whether the float is normalized.
   */
  inline bool is_normalized(const float t, bool include_ends = true) {
    if (include_ends) {
      return t >= 0.0f && t <= 1.0f;
    }

    return t > 0.0f && t < 1.0f;
  }

  /**
   * @brief Checks if a float is in a range.
   *
   * @param t The float to check.
   * @param min The minimum value of the range.
   * @param max The maximum value of the range.
   * @param include_ends Whether to include the ends of the range, defaults to true.
   * @return Whether the float is in the range.
   */
  inline bool is_in_range(const float t, const float min, const float max, bool include_ends = true) {
    if (include_ends) {
      return t >= min && t <= max;
    }

    return t > min && t < max;
  }

  /**
   * @brief Converts a float from degrees to radians.
   *
   * @param a The angle in degrees.
   * @return The angle in radians.
   */
  inline float degrees_to_radians(float a) {
    return a * MATH_F_PI / 180.0f;
  }

  /**
   * @brief Converts a double from degrees to radians.
   *
   * @param a The angle in degrees.
   * @return The angle in radians.
   */
  inline double degrees_to_radians(double a) {
    return a * MATH_PI / 180.0;
  }

  /**
   * @brief Converts a float from radians to degrees.
   *
   * @param a The angle in radians.
   * @return The angle in degrees.
   */
  inline float radians_to_degrees(float a) {
    return a * 180.0f / MATH_F_PI;
  }

  /**
   * @brief Converts a double from radians to degrees.
   *
   * @param a The angle in radians.
   * @return The angle in degrees.
   */
  inline double radians_to_degrees(double a) {
    return a * 180.0 / MATH_PI;
  }

  /**
   * @brief Calculates the next power of two of a number.
   *
   * @param n The number to calculate the next power of two of.
   * @return The next power of two.
   */
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

  /**
   * @brief Calculates the sign of a number.
   *
   * @param val The number to calculate the sign of.
   * @return The sign of the number.
   */
  template <typename T>
  inline T sign(T val) {
    return static_cast<T>((T(0) < val) - (val < T(0)));
  }

}
