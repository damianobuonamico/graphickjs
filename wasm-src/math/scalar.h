/**
 * @file scalar.h
 * @brief This file contains scalar math functions.
 */

#pragma once

#include <cstdint>
#include <limits>
#include <cfloat>
#include <cmath>

/* -- Defines -- */

template <typename T, typename = std::enable_if<std::is_floating_point<T>::value>>
constexpr T pi = T(3.14159265358979323846);

template <typename T, typename = std::enable_if<std::is_floating_point<T>::value>>
constexpr T two_pi = T(2) * pi<T>;

template <typename T, typename = std::enable_if<std::is_floating_point<T>::value>>
constexpr T epsilon = std::numeric_limits<T>::epsilon();

template <typename T, typename = std::enable_if<std::is_integral<T>::value>>
constexpr T epsilon = T(0);

namespace graphick::math {

  /**
   * @brief Rounds a scalar to a certain number of decimals.
   *
   * @param t The scalar to round.
   * @param precision The smallest value to round to.
   * @return The rounded scalar.
   */
  template <typename T>
  inline T round(const T t, const T precision) noexcept {
    if (precision >= T(1)) {
      return std::round(t / precision) * precision;
    }

    const T integer_part = std::floor(t);
    const T decimal_part = t - integer_part;

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
  template <typename T>
  inline T map(const T t, const T old_min, const T old_max, const T new_min, const T new_max) {
    return (
      ((t - old_min) * (new_max - new_min)) /
      (old_max - old_min) + new_min
    );
  }

  /**
   * @brief Linearly interpolates between two values.
   *
   * @param a The first value.
   * @param b The second value.
   * @param t The interpolation value.
   * @return The interpolated value.
   */
  template <typename T>
  inline T lerp(const T a, const T b, const T t) {
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
  template <typename T, typename = std::enable_if<std::is_integral<T>::value>>
  inline T wrap(T t, const T min, const T max) {

    const T range_size = max - min + 1;

    if (t < min) {
      t += range_size * ((min - t) / range_size + 1);
    }

    return min + (t - min) % range_size;
  }

  /**
   * @brief Checks if a scalar is almost zero.
   *
   * @param t The scalar to check.
   * @param eps The precision to check with.
   * @return Whether the scalar is almost zero.
   */
  template <typename T, typename = std::enable_if<std::is_floating_point<T>::value>>
  inline bool is_almost_zero(const T t, const T eps = epsilon<T>) {
    return std::abs(t) <= eps;
  }

  /**
   * @brief Checks if two scalars are almost equal.
   *
   * @param t1 The first scalar.
   * @param t2 The second scalar.
   * @param eps The precision to check with.
   * @return Whether the scalars are almost equal.
   */
  template <typename T, typename = std::enable_if<std::is_floating_point<T>::value>>
  inline bool is_almost_equal(const T t1, const T t2, const T eps = epsilon<T>) {
    return std::abs(t1 - t2) <= eps;
  }

  /**
   * @brief Checks if a scalar is normalized.
   *
   * @param t The scalar to check.
   * @param include_ends Whether to include the ends of the range, defaults to true.
   * @return Whether the scalar is normalized.
   */
  template <typename T, typename = std::enable_if<std::is_floating_point<T>::value>>
  inline bool is_normalized(const T t, const bool include_ends = true) {
    return include_ends ? (t >= T(0) && t <= T(1)) : (t > T(0) && t < T(1));
  }

  /**
   * @brief Checks if a scalar is in a range.
   *
   * @param t The scalar to check.
   * @param min The minimum value of the range.
   * @param max The maximum value of the range.
   * @param include_ends Whether to include the ends of the range, defaults to true.
   * @return Whether the scalar is in the range.
   */
  template <typename T>
  inline bool is_in_range(const T t, const T min, const T max, const bool include_ends = true) {
    return include_ends ? (t >= min && t <= max) : (t > min && t < max);
  }

  /**
   * @brief Converts a scalar from degrees to radians.
   *
   * @param a The angle in degrees.
   * @return The angle in radians.
   */
  template <typename T, typename = std::enable_if<std::is_floating_point<T>::value>>
  inline T degrees_to_radians(const T a) {
    return a * pi<T> / T(180);
  }

  /**
   * @brief Converts a scalar from radians to degrees.
   *
   * @param a The angle in radians.
   * @return The angle in degrees.
   */
  template <typename T, typename = std::enable_if<std::is_floating_point<T>::value>>
  inline T radians_to_degrees(const T a) {
    return a * T(180) / pi<T>;
  }

  /**
   * @brief Calculates the next power of two of a scalar.
   *
   * @param n The scalar to calculate the next power of two of.
   * @return The next power of two.
   */
  template <typename T, typename = std::enable_if<std::is_integral<T>::value>>
  inline T next_power_of_two(T n) {
    n--;

    uint8_t power = (sizeof(T) < 8 ? 8 : sizeof(T)) - 2;

    for (uint8_t i = 0; i < power; i++) {
      n |= n >> uint8_t(std::pow(2, i));
    }

    return n + 1;
  }

  /**
   * @brief Calculates the sign of a scalar.
   *
   * @param val The scalar to calculate the sign of.
   * @return The sign of the scalar.
   */
  template <typename T>
  inline T sign(T val) {
    return static_cast<T>((T(0) < val) - (val < T(0)));
  }

}
