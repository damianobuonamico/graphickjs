/**
 * @file math/scalar.h
 * @brief This file contains scalar math functions.
 */

#pragma once

#include <cfloat>
#include <cmath>
#include <cstdint>
#include <limits>

namespace graphick::math {

/* -- Defines -- */

template<typename T>
inline constexpr std::enable_if_t<std::is_floating_point_v<T>, T> pi = T(3.14159265358979323846);

template<typename T>
inline constexpr std::enable_if_t<std::is_floating_point_v<T>, T> two_pi = T(2) * pi<T>;

template<typename T>
inline constexpr T epsilon = T(0);
template<>
inline constexpr float epsilon<float> = std::numeric_limits<float>::epsilon();
template<>
inline constexpr double epsilon<double> = std::numeric_limits<double>::epsilon();

template<typename T>
inline constexpr T math_epsilon = T(0);
template<>
inline constexpr float math_epsilon<float> = 1e-9f;
template<>
inline constexpr double math_epsilon<double> = 1e-9;

template<typename T>
inline constexpr T geometric_epsilon = T(0);
template<>
inline constexpr float geometric_epsilon<float> = 1e-3f;
template<>
inline constexpr double geometric_epsilon<double> = 1e-3;

template<typename T>
inline constexpr T newton_raphson_iterations = T(5);

template<typename T>
inline constexpr T max_recursion_depth = T(16);

template<typename T>
inline constexpr std::enable_if_t<std::is_floating_point_v<T>, T> bezier_circle_ratio = T(
    0.55228474983079339840);

/**
 * @brief Rounds a scalar to a certain number of decimals.
 *
 * @param t The scalar to round.
 * @param precision The smallest value to round to.
 * @return The rounded scalar.
 */
template<typename T>
inline T round(const T t, const T precision) noexcept
{
  if (precision >= T(1)) {
    return std::round(t / precision) * precision;
  }

  const T integer_part = std::floor(t);
  const T decimal_part = t - integer_part;

  return integer_part + std::round(decimal_part / precision) * precision;
}

/**
 * @brief Returns the minimum of two scalars.
 *
 * @param t1 The first scalar.
 * @param t2 The second scalar.
 * @return The minimum scalar.
 */
template<typename T>
inline T min(const T t1, const T t2)
{
  return t1 < t2 ? t1 : t2;
}

/**
 * @brief Returns the minimum of three scalars.
 *
 * @param t1 The first scalar.
 * @param t2 The second scalar.
 * @param t3 The third scalar.
 * @return The minimum scalar.
 */
template<typename T>
inline T min(const T t1, const T t2, const T t3)
{
  return min(min(t1, t2), t3);
}

/**
 * @brief Returns the maximum of two scalars.
 *
 * @param t1 The first scalar.
 * @param t2 The second scalar.
 * @return The maximum scalar.
 */
template<typename T>
inline T max(const T t1, const T t2)
{
  return t1 > t2 ? t1 : t2;
}

/**
 * @brief Returns the maximum of two scalars.
 *
 * @param t1 The first scalar.
 * @param t2 The second scalar.
 * @return The maximum scalar.
 */
template<typename T>
inline T max(const T t1, const T t2, const T t3)
{
  return max(max(t1, t2), t3);
}

/**
 * @brief Clamps a scalar between a minimum and a maximum.
 *
 * @param t The scalar to clamp.
 * @param min The minimum value.
 * @param max The maximum value.
 * @return The clamped scalar.
 */
template<typename T>
inline T clamp(const T t, const T min, const T max)
{
  return t < min ? min : (t > max ? max : t);
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
template<typename T>
inline T map(const T t, const T old_min, const T old_max, const T new_min, const T new_max)
{
  return (((t - old_min) * (new_max - new_min)) / (old_max - old_min) + new_min);
}

/**
 * @brief Linearly interpolates between two values.
 *
 * @param a The first value.
 * @param b The second value.
 * @param t The interpolation value.
 * @return The interpolated value.
 */
template<typename T>
inline T lerp(const T a, const T b, const T t)
{
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
template<typename T, typename = std::enable_if<std::is_integral_v<T>>>
inline T wrap(T t, const T min, const T max)
{

  const T range_size = max - min + 1;

  if (t < min) {
    t += range_size * ((min - t) / range_size + 1);
  }

  return min + (t - min) % range_size;
}

/**
 * @brief Checks if two scalars are almost equal.
 *
 * @param t1 The first scalar.
 * @param t2 The second scalar.
 * @param eps The precision to check with.
 * @return Whether the scalars are almost equal.
 */
template<typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
inline bool is_almost_equal(const T t1, const T t2, const T eps = math_epsilon<T>)
{
  return std::abs(t1 - t2) <= eps;
}

/**
 * @brief Checks if a scalar is almost zero.
 *
 * @param t The scalar to check.
 * @param eps The precision to check with.
 * @return Whether the scalar is almost zero.
 */
template<typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
inline bool is_almost_zero(const T t, const T eps = math_epsilon<T>)
{
  return std::abs(t) <= eps;
}

/**
 * @brief Checks if a scalar is almost zero or one.
 *
 * @param t The scalar to check.
 * @param eps The precision to check with.
 * @return Whether the scalar is almost zero or one.
 */
template<typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
inline bool is_almost_zero_or_one(const T t, const T eps = math_epsilon<T>)
{
  return std::abs(0.5 - std::abs(t - 0.5)) <= eps;
}

/**
 * @brief Checks if a scalar is normalized.
 *
 * @param t The scalar to check.
 * @param include_ends Whether to include the ends of the range, defaults to true.
 * @return Whether the scalar is normalized.
 */
template<typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
inline bool is_normalized(const T t, const bool include_ends = true)
{
  return include_ends ? (t >= T(0) && t <= T(1)) : (t > T(0) && t < T(1));
}

/**
 * @brief Checks if a scalar is almost normalized.
 *
 * Ends are never included when an epsilon is provided.
 *
 * @param t The scalar to check.
 * @param eps The precision to check with.
 * @return Whether the scalar is almost normalized.
 */
template<typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
inline bool is_almost_normalized(const T t, const T eps = math_epsilon<T>)
{
  return t > T(0) + eps && t < T(1) - eps;
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
template<typename T>
inline bool is_in_range(const T t, const T min, const T max, const bool include_ends = true)
{
  return include_ends ? (t >= min && t <= max) : (t > min && t < max);
}

/**
 * @brief Converts a scalar from degrees to radians.
 *
 * @param a The angle in degrees.
 * @return The angle in radians.
 */
template<typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
inline T degrees_to_radians(const T a)
{
  return a * pi<T> / T(180);
}

/**
 * @brief Converts a scalar from radians to degrees.
 *
 * @param a The angle in radians.
 * @return The angle in degrees.
 */
template<typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
inline T radians_to_degrees(const T a)
{
  return a * T(180) / pi<T>;
}

/**
 * @brief Calculates the next power of two of a scalar.
 *
 * @param n The scalar to calculate the next power of two of.
 * @return The next power of two.
 */
template<typename T, typename = std::enable_if<std::is_integral_v<T>>>
inline T next_power_of_two(T n)
{
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
template<typename T>
inline T sign(T val)
{
  return static_cast<T>((T(0) < val) - (val < T(0)));
}

}  // namespace graphick::math
