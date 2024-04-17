/**
 * @file vector.h
 * @brief Contains methods for vector manipulation.
 */

#pragma once

#include "scalar.h"

#include "vec2.h"
#include "vec3.h"
#include "vec4.h"

#include <vector>

#define XY(v) graphick::math::Vec2{ v.x, v.y }
#define RG(v) graphick::math::Vec2{ v.r, v.g }
#define ST(v) graphick::math::Vec2{ v.s, v.t }

#define XYZ(v) graphick::math::Vec3{ v.x, v.y, v.z }
#define RGB(v) graphick::math::Vec3{ v.r, v.g, v.b }
#define STP(v) graphick::math::Vec3{ v.s, v.t, v.p }

namespace graphick::math {

  /* -- min -- */

  /**
   * @brief Calculates the minimum of two Vec2s.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @return The minimum of the two vectors.
   */
  template <typename T>
  constexpr Vec2<T> min(const Vec2<T> v1, const Vec2<T> v2) {
    return Vec2<T>(
      std::min(v1.x, v2.x),
      std::min(v1.y, v2.y)
    );
  }

  /**
   * @brief Calculates the minimum of two Vec3s.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @return The minimum of the two vectors.
   */
  template <typename T>
  constexpr Vec3<T> min(const Vec3<T>& v1, const Vec3<T>& v2) {
    return Vec3<T>(
      std::min(v1.x, v2.x),
      std::min(v1.y, v2.y),
      std::min(v1.z, v2.z)
    );
  }

  /**
   * @brief Calculates the minimum of two Vec4s.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @return The minimum of the two vectors.
   */
  template <typename T>
  constexpr Vec4<T> min(const Vec4<T>& v1, const Vec4<T>& v2) {
    return Vec4<T>(
      std::min(v1.x, v2.x),
      std::min(v1.y, v2.y),
      std::min(v1.z, v2.z),
      std::min(v1.w, v2.w),
      );
  }

  /**
   * @brief Calculates the minimum of two Vec2s.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @param out The output vector, can be the same as v1 or v2.
   * @return A reference to the output vector.
   */
  template <typename T>
  constexpr Vec2<T>& min(const Vec2<T> v1, const Vec2<T> v2, Vec2<T>& out) {
    out.x = std::min(v1.x, v2.x);
    out.y = std::min(v1.y, v2.y);

    return out;
  }

  /**
   * @brief Calculates the minimum of two Vec3s.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @param out The output vector, can be the same as v1 or v2.
   * @return A reference to the output vector.
   */
  template <typename T>
  constexpr Vec3<T>& min(const Vec3<T>& v1, const Vec3<T>& v2, Vec3<T>& out) {
    out.x = std::min(v1.x, v2.x);
    out.y = std::min(v1.y, v2.y);
    out.z = std::min(v1.z, v2.z);

    return out;
  }

  /**
   * @brief Calculates the minimum of two Vec4s.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @param out The output vector, can be the same as v1 or v2.
   * @return A reference to the output vector.
   */
  template <typename T>
  constexpr Vec4<T>& min(const Vec4<T>& v1, const Vec4<T>& v2, Vec4<T>& out) {
    out.x = std::min(v1.x, v2.x);
    out.y = std::min(v1.y, v2.y);
    out.z = std::min(v1.z, v2.z);
    out.w = std::min(v1.w, v2.w);

    return out;
  }

  /* -- max -- */

  /**
   * @brief Calculates the maximum of two Vec2s.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @return The maximum of the two vectors.
   */
  template <typename T>
  constexpr Vec2<T> max(const Vec2<T> v1, const Vec2<T> v2) {
    return Vec2<T>(
      std::max(v1.x, v2.x),
      std::max(v1.y, v2.y)
    );
  }

  /**
   * @brief Calculates the maximum of two Vec3s.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @return The maximum of the two vectors.
   */
  template <typename T>
  constexpr Vec3<T> max(const Vec3<T>& v1, const Vec3<T>& v2) {
    return Vec3<T>(
      std::max(v1.x, v2.x),
      std::max(v1.y, v2.y),
      std::max(v1.z, v2.z)
    );
  }

  /**
   * @brief Calculates the maximum of two Vec4s.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @return The maximum of the two vectors.
   */
  template <typename T>
  constexpr Vec4<T> max(const Vec4<T>& v1, const Vec4<T>& v2) {
    return Vec4<T>(
      std::max(v1.x, v2.x),
      std::max(v1.y, v2.y),
      std::max(v1.z, v2.z),
      std::max(v1.w, v2.w)
    );
  }

  /**
   * @brief Calculates the maximum of two Vec2s.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @param out The output vector, can be the same as v1 or v2.
   * @return A reference to the output vector.
   */
  template <typename T>
  constexpr Vec2<T> max(const Vec2<T> v1, const Vec2<T> v2, Vec2<T>& out) {
    out.x = std::max(v1.x, v2.x);
    out.y = std::max(v1.y, v2.y);

    return out;
  }

  /**
   * @brief Calculates the maximum of two Vec3s.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @param out The output vector, can be the same as v1 or v2.
   * @return A reference to the output vector.
   */
  template <typename T>
  constexpr Vec3<T> max(const Vec3<T>& v1, const Vec3<T>& v2, Vec3<T>& out) {
    out.x = std::max(v1.x, v2.x);
    out.y = std::max(v1.y, v2.y);
    out.z = std::max(v1.z, v2.z);

    return out;
  }

  /**
   * @brief Calculates the maximum of two Vec4s.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @param out The output vector, can be the same as v1 or v2.
   * @return A reference to the output vector.
   */
  template <typename T>
  constexpr Vec4<T> max(const Vec4<T>& v1, const Vec4<T>& v2, Vec4<T>& out) {
    out.x = std::max(v1.x, v2.x);
    out.y = std::max(v1.y, v2.y);
    out.z = std::max(v1.z, v2.z);
    out.w = std::max(v1.w, v2.w);

    return out;
  }

  /* -- length -- */

  /**
   * @brief Calculates the length or magnitude of a Vec2.
   *
   * @param v The vector.
   * @return The length of the vector.
   */
  template <typename T>
  inline T length(const Vec2<T> v) {
    return std::hypot(v.x, v.y);
  }

  /**
   * @brief Calculates the length or magnitude of a Vec3.
   *
   * @param v The vector.
   * @return The length of the vector.
   */
  template <typename T>
  inline T length(const Vec3<T>& v) {
    return std::hypot(v.x, v.y, v.z);
  }

  /**
   * @brief Calculates the length or magnitude of a Vec4.
   *
   * @param v The vector.
   * @return The length of the vector.
   */
  template <typename T>
  inline T length(const Vec4<T>& v) {
    return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
  }

  /* -- dot -- */

  /**
   * @brief Computes the dot product of two Vec2s.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @return The dot product of the two vectors.
   */
  template <typename T>
  constexpr T dot(const Vec2<T> v1, const Vec2<T> v2) {
    return v1.x * v2.x + v1.y * v2.y;
  }

  /**
   * @brief Computes the dot product of two Vec3s.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @return The dot product of the two vectors.
   */
  template <typename T>
  constexpr T dot(const Vec3<T>& v1, const Vec3<T>& v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
  }

  /**
   * @brief Computes the dot product of two Vec4s.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @return The dot product of the two vectors.
   */
  template <typename T>
  constexpr T dot(const Vec4<T>& v1, const Vec4<T>& v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;
  }

  /* -- cross -- */

  /**
   * @brief Computes the cross product of two Vec2s.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @return The cross product as a scalar value.
   */
  template <typename T>
  constexpr T cross(const Vec2<T> v1, const Vec2<T> v2) {
    return v1.x * v2.y - v2.x * v1.y;
  }

  /**
   * @brief Computes the cross product of two Vec3s.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @return The cross product as a Vec3.
   */
  template <typename T>
  constexpr Vec3<T> cross(const Vec3<T>& v1, const Vec3<T>& v2) {
    return Vec3<T>(
      v1.y * v2.z - v1.z * v2.y,
      -(v1.x * v2.z - v1.z * v2.x),
      v1.x * v2.y - v1.y * v2.x
    );
  }

  /* -- squared_length -- */

  /**
   * @brief Calculates the squared length or magnitude of a Vec2.
   *
   * @param v The vector.
   * @return The squared length of the vector.
   */
  template <typename T>
  constexpr T squared_length(const Vec2<T> v) {
    return v.x * v.x + v.y * v.y;
  }

  /**
   * @brief Calculates the squared length or magnitude of a Vec3.
   *
   * @param v The vector.
   * @return The squared length of the vector.
   */
  template <typename T>
  constexpr T squared_length(const Vec3<T>& v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
  }

  /**
   * @brief Calculates the squared length or magnitude of a Vec4.
   *
   * @param v The vector.
   * @return The squared length of the vector.
   */
  template <typename T>
  constexpr T squared_length(const Vec4<T>& v) {
    return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
  }

  /* -- distance -- */

  /**
   * @brief Calculates the distance between two Vec2s.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @return The distance between the two vectors.
   */
  template <typename T>
  inline T distance(const Vec2<T> v1, const Vec2<T> v2) {
    return std::hypot(v2.x - v1.x, v2.y - v1.y);
  }

  /**
   * @brief Calculates the distance between two Vec3s.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @return The distance between the two vectors.
   */
  template <typename T>
  inline T distance(const Vec3<T>& v1, const Vec3<T>& v2) {
    return std::hypot(v2.x - v1.x, v2.y - v1.y, v2.z - v1.z);
  }

  /**
   * @brief Calculates the distance between two Vec4s.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @return The distance between the two vectors.
   */
  template <typename T>
  inline T distance(const Vec4<T>& v1, const Vec4<T>& v2) {
    return length(v2 - v1);
  }

  /* -- squared_distance -- */

  /**
   * @brief Calculates the squared distance between two Vec2s.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @return The squared distance between the two vectors.
   */
  template <typename T>
  constexpr T squared_distance(const Vec2<T> v1, const Vec2<T> v2) {
    Vec2<T> v = v2 - v1;
    return dot(v, v);
  }

  /**
   * @brief Calculates the squared distance between two Vec3s.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @return The squared distance between the two vectors.
   */
  template <typename T>
  constexpr T squared_distance(const Vec3<T>& v1, const Vec3<T>& v2) {
    Vec3<T> v = v2 - v1;
    return dot(v, v);
  }

  /**
   * @brief Calculates the squared distance between two Vec4s.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @return The squared distance between the two vectors.
   */
  template <typename T>
  constexpr T squared_distance(const Vec4<T>& v1, const Vec4<T>& v2) {
    Vec4<T> v = v2 - v1;
    return dot(v, v);
  }

  /* -- lerp -- */

  /**
   * @brief Linearly interpolates between two Vec2s.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @param t The interpolation factor.
   * @return The interpolated vector.
   */
  template <typename T>
  constexpr Vec2<T> lerp(const Vec2<T> v1, const Vec2<T> v2, T t) {
    return Vec2<T>(
      v1.x + t * (v2.x - v1.x),
      v1.y + t * (v2.y - v1.y)
    );
  }

  /**
   * @brief Linearly interpolates between two Vec3s.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @param t The interpolation factor.
   * @return The interpolated vector.
   */
  template <typename T>
  constexpr Vec3<T> lerp(const Vec3<T>& v1, const Vec3<T>& v2, T t) {
    return Vec3<T>(
      v1.x + t * (v2.x - v1.x),
      v1.y + t * (v2.y - v1.y),
      v1.z + t * (v2.z - v1.z)
    );
  }

  /**
   * @brief Linearly interpolates between two Vec4s.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @param t The interpolation factor.
   * @return The interpolated vector.
   */
  template <typename T>
  constexpr Vec4<T> lerp(const Vec4<T>& v1, const Vec4<T>& v2, T t) {
    return Vec4<T>(
      v1.x + t * (v2.x - v1.x),
      v1.y + t * (v2.y - v1.y),
      v1.z + t * (v2.z - v1.z),
      v1.w + t * (v2.w - v1.w)
    );
  }

  //TODO: implement quadratic(), quadratic_derivative(), cubic(), cubic_derivative(), cubic_second_derivative(), cubic_extrema()

  /* -- midpoint -- */

  /**
   * @brief Calculates the midpoint between two Vec2s.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @return The midpoint between v1 and v2.
   */
  template <typename T>
  constexpr Vec2<T> midpoint(const Vec2<T> v1, const Vec2<T> v2) {
    return Vec2<T>(
      (v1.x + v2.x) / T(2),
      (v1.y + v2.y) / T(2)
    );
  }

  /**
   * @brief Calculates the midpoint between two Vec3s.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @return The midpoint between v1 and v2.
   */
  template <typename T>
  constexpr Vec3<T> midpoint(const Vec3<T>& v1, const Vec3<T>& v2) {
    return Vec3<T>(
      (v1.x + v2.x) / T(2),
      (v1.y + v2.y) / T(2),
      (v1.z + v2.z) / T(2)
    );
  }

  /**
   * @brief Calculates the midpoint between two Vec4s.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @return The midpoint between v1 and v2.
   */
  template <typename T>
  constexpr Vec4<T> midpoint(const Vec4<T>& v1, const Vec4<T>& v2) {
    return Vec4<T>(
      (v1.x + v2.x) / T(2),
      (v1.y + v2.y) / T(2),
      (v1.z + v2.z) / T(2),
      (v1.w + v2.w) / T(2)
    );
  }

  /* -- normalize -- */

  /**
   * @brief Normalizes a Vec2 to have a length of 1.
   *
   * @param v The vector to normalize.
   * @return The normalized Vec2.
   */
  template <typename T>
  constexpr Vec2<T> normalize(const Vec2<T> v) {
    T len = v.x * v.x + v.y * v.y;
    if (len > 0) len = 1 / std::sqrt(len);

    return Vec2<T>(
      v.x * len,
      v.y * len
    );
  }

  /**
   * @brief Normalizes a Vec3<T> to have a length of 1.
   *
   * @param v The vector to normalize.
   * @return The normalized Vec3.
   */
  template <typename T>
  constexpr Vec3<T> normalize(const Vec3<T>& v) {
    T len = v.x * v.x + v.y * v.y + v.z * v.z;
    if (len > 0) len = 1 / std::sqrt(len);

    return Vec3<T>(
      v.x * len,
      v.y * len,
      v.z * len
    );
  }

  /**
   * @brief Normalizes a Vec4<T> to have a length of 1.
   *
   * @param v The vector to normalize.
   * @return The normalized Vec4.
   */
  template <typename T>
  constexpr Vec4<T> normalize(const Vec4<T>& v) {
    T len = v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
    if (len > 0) len = 1 / std::sqrt(len);

    return Vec4<T>(
      v.x * len,
      v.y * len,
      v.z * len,
      v.w * len
    );
  }

  /**
   * @brief Normalizes a Vec2 to have a length of 1.
   *
   * @param v The vector to normalize.
   * @param out The normalized vector (output).
   * @return The normalized Vec2.
   */
  template <typename T>
  constexpr Vec2<T>& normalize(const Vec2<T> v, Vec2<T>& out) {
    T len = v.x * v.x + v.y * v.y;
    if (len > 0) len = 1 / std::sqrt(len);

    out.x = v.x * len;
    out.y = v.y * len;

    return out;
  }

  /**
   * @brief Normalizes a Vec3<T> to have a length of 1.
   *
   * @param v The vector to normalize.
   * @param out The normalized vector (output).
   * @return The normalized Vec3.
   */
  template <typename T>
  constexpr Vec3<T>& normalize(const Vec3<T>& v, Vec3<T>& out) {
    T len = v.x * v.x + v.y * v.y + v.z * v.z;
    if (len > 0) len = 1 / std::sqrt(len);

    out.x = v.x * len;
    out.y = v.y * len;
    out.z = v.z * len;

    return out;
  }

  /**
   * @brief Normalizes a Vec4<T> to have a length of 1.
   *
   * @param v The vector to normalize.
   * @param out The normalized vector (output).
   * @return The normalized Vec4.
   */
  template <typename T>
  constexpr Vec4<T>& normalize(const Vec4<T>& v, Vec4<T>& out) {
    T len = v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
    if (len > 0) len = 1 / std::sqrt(len);

    out.x = v.x * len;
    out.y = v.y * len;
    out.z = v.z * len;
    out.w = v.w * len;

    return out;
  }

  /* -- normalize_length -- */

  /**
   * @brief Normalizes a Vec2 and scales it by a given length.
   *
   * @param v The vector to normalize and scale.
   * @param t The scaling factor.
   * @return The normalized and scaled Vec2.
   */
  template <typename T>
  constexpr Vec2<T> normalize_length(const Vec2<T> v, T t) {
    T len = v.x * v.x + v.y * v.y;
    if (len > 0) len = 1 / std::sqrt(len);

    return Vec2<T>(
      v.x * len * t,
      v.y * len * t
    );
  }

  /**
   * @brief Normalizes a Vec3<T> and scales it by a given length.
   *
   * @param v The vector to normalize and scale.
   * @param t The scaling factor.
   * @return The normalized and scaled Vec3.
   */
  template <typename T>
  constexpr Vec3<T> normalize_length(const Vec3<T>& v, T t) {
    T len = v.x * v.x + v.y * v.y + v.z * v.z;
    if (len > 0) len = 1 / std::sqrt(len);

    return Vec3<T>(
      v.x * len * t,
      v.y * len * t,
      v.z * len * t
    );
  }

  /**
   * @brief Normalizes a Vec4<T> and scales it by a given length.
   *
   * @param v The vector to normalize and scale.
   * @param t The scaling factor.
   * @return The normalized and scaled Vec4.
   */
  template <typename T>
  constexpr Vec4<T> normalize_length(const Vec4<T>& v, T t) {
    T len = v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
    if (len > 0) len = 1 / std::sqrt(len);

    return Vec4<T>(
      v.x * len * t,
      v.y * len * t,
      v.z * len * t,
      v.w * len * t
    );
  }

  /**
   * @brief Normalizes a Vec2 and scales it by a given length.
   *
   * @param v The vector to normalize and scale.
   * @param t The scaling factor.
   * @param out The normalized and scaled vector (output).
   * @return The normalized and scaled Vec2.
   */
  template <typename T>
  constexpr Vec2<T>& normalize_length(const Vec2<T> v, T t, Vec2<T>& out) {
    T len = v.x * v.x + v.y * v.y;
    if (len > 0) len = 1 / std::sqrt(len);

    out.x = v.x * len * t;
    out.y = v.y * len * t;

    return out;
  }

  /**
   * @brief Normalizes a Vec3<T> and scales it by a given length.
   *
   * @param v The vector to normalize and scale.
   * @param t The scaling factor.
   * @param out The normalized and scaled vector (output).
   * @return The normalized and scaled Vec3.
   */
  template <typename T>
  constexpr Vec3<T>& normalize_length(const Vec3<T>& v, T t, Vec3<T>& out) {
    T len = v.x * v.x + v.y * v.y + v.z * v.z;
    if (len > 0) len = 1 / std::sqrt(len);

    out.x = v.x * len * t;
    out.y = v.y * len * t;
    out.z = v.z * len * t;

    return out;
  }

  /**
   * @brief Normalizes a Vec4<T> and scales it by a given length.
   *
   * @param v The vector to normalize and scale.
   * @param t The scaling factor.
   * @param out The normalized and scaled vector (output).
   * @return The normalized and scaled Vec4.
   */
  template <typename T>
  constexpr Vec4<T>& normalize_length(const Vec4<T>& v, T t, Vec4<T>& out) {
    T len = v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
    if (len > 0) len = 1 / std::sqrt(len);

    out.x = v.x * len * t;
    out.y = v.y * len * t;
    out.z = v.z * len * t;
    out.w = v.w * len * t;

    return out;
  }

  /* -- negate -- */

  /**
   * @brief Negates a Vec2.
   *
   * @param v The vector to negate.
   * @return The negated Vec2.
   */
  template <typename T>
  constexpr Vec2<T> negate(const Vec2<T> v) {
    return -v;
  }

  /**
   * @brief Negates a Vec3.
   *
   * @param v The vector to negate.
   * @return The negated Vec3.
   */
  template <typename T>
  constexpr Vec3<T> negate(const Vec3<T>& v) {
    return -v;
  }

  /**
   * @brief Negates a Vec4.
   *
   * @param v The vector to negate.
   * @return The negated Vec4.
   */
  template <typename T>
  constexpr Vec4<T> negate(const Vec4<T>& v) {
    return -v;
  }

  /**
   * @brief Negates a Vec2.
   *
   * @param v The vector to negate.
   * @param out The negated vector (output).
   * @return The negated Vec2.
   */
  template <typename T>
  constexpr Vec2<T>& negate(const Vec2<T> v, Vec2<T>& out) {
    out.x = -v.x;
    out.y = -v.y;

    return out;
  }

  /**
   * @brief Negates a Vec3.
   *
   * @param v The vector to negate.
   * @param out The negated vector (output).
   * @return The negated Vec3.
   */
  template <typename T>
  constexpr Vec3<T>& negate(const Vec3<T>& v, Vec3<T>& out) {
    out.x = -v.x;
    out.y = -v.y;
    out.z = -v.z;

    return out;
  }

  /**
   * @brief Negates a Vec4.
   *
   * @param v The vector to negate.
   * @param out The negated vector (output).
   * @return The negated Vec4.
   */
  template <typename T>
  constexpr Vec4<T>& negate(const Vec4<T>& v, Vec4<T>& out) {
    out.x = -v.x;
    out.y = -v.y;
    out.z = -v.z;
    out.w = -v.w;

    return out;
  }

  /* -- abs -- */

  /**
   * @brief Calculates the absolute value of each component of a Vec2.
   *
   * @param v The vector to calculate absolute values for.
   * @return The Vec2<T> with absolute values of its components.
   */
  template <typename T>
  inline Vec2<T> abs(const Vec2<T> v) {
    return Vec2<T>(
      std::abs(v.x),
      std::abs(v.y)
    );
  }

  /**
   * @brief Calculates the absolute value of each component of a Vec3.
   *
   * @param v The vector to calculate absolute values for.
   * @return The Vec3<T> with absolute values of its components.
   */
  template <typename T>
  inline Vec3<T> abs(const Vec3<T>& v) {
    return Vec3<T>(
      std::abs(v.x),
      std::abs(v.y),
      std::abs(v.z)
    );
  }

  /**
   * @brief Calculates the absolute value of each component of a Vec4.
   *
   * @param v The vector to calculate absolute values for.
   * @return The Vec4<T> with absolute values of its components.
   */
  template <typename T>
  inline Vec4<T> abs(const Vec4<T>& v) {
    return Vec4<T>(
      std::abs(v.x),
      std::abs(v.y),
      std::abs(v.z),
      std::abs(v.w)
    );
  }

  /**
   * @brief Calculates the absolute value of each component of a Vec2.
   *
   * @param v The vector to calculate absolute values for.
   * @param out The vector with absolute values of its components (output).
   * @return The Vec2<T> with absolute values of its components.
   */
  template <typename T>
  inline Vec2<T>& abs(const Vec2<T> v, Vec2<T>& out) {
    out.x = std::abs(v.x);
    out.y = std::abs(v.y);

    return out;
  }
  /**
   * @brief Calculates the absolute value of each component of a Vec3.
   *
   * @param v The vector to calculate absolute values for.
   * @param out The vector with absolute values of its components (output).
   * @return The Vec3<T> with absolute values of its components.
   */
  template <typename T>
  inline Vec3<T>& abs(const Vec3<T>& v, Vec3<T>& out) {
    out.x = std::abs(v.x);
    out.y = std::abs(v.y);
    out.z = std::abs(v.z);
    return out;
  }

  /**
   * @brief Calculates the absolute value of each component of a Vec4.
   *
   * @param v The vector to calculate absolute values for.
   * @param out The vector with absolute values of its components (output).
   * @return The Vec4<T> with absolute values of its components.
   */
  template <typename T>
  inline Vec4<T>& abs(const Vec4<T>& v, Vec4<T>& out) {
    out.x = std::abs(v.x);
    out.y = std::abs(v.y);
    out.z = std::abs(v.z);
    out.w = std::abs(v.w);
    return out;
  }

  /* -- zero -- */

  /**
   * @brief Sets all components of a Vec2 to zero.
   *
   * @param v The vector to be zeroed.
   */
  template <typename T>
  constexpr void zero(Vec2<T>& v) {
    v.x = v.y = T(0);
  }

  /**
   * @brief Sets all components of a Vec3<T> to zero.
   *
   * @param v The vector to be zeroed.
   */
  template <typename T>
  constexpr void zero(Vec3<T>& v) {
    v.x = v.y = v.z = T(0);
  }

  /**
   * @brief Sets all components of a Vec4<T> to zero.
   *
   * @param v The vector to be zeroed.
   */
  template <typename T>
  constexpr void zero(Vec4<T>& v) {
    v.x = v.y = v.z = v.w = T(0);
  }

  /* -- is_zero -- */

  /**
   * @brief Checks if all components of a Vec2 are zero.
   *
   * @param v The vector to be checked.
   * @return True if all components are zero, false otherwise.
   */
  template <typename T>
  constexpr bool is_zero(const Vec2<T> v) {
    return v.x == T(0) && v.y == T(0);
  }

  /**
   * @brief Checks if all components of a Vec3<T> are zero.
   *
   * @param v The vector to be checked.
   * @return True if all components are zero, false otherwise.
   */
  template <typename T>
  constexpr bool is_zero(const Vec3<T>& v) {
    return v.x == T(0) && v.y == T(0) && v.z == T(0);
  }

  /**
   * @brief Checks if all components of a Vec4<T> are zero.
   *
   * @param v The vector to be checked.
   * @return True if all components are zero, false otherwise.
   */
  template <typename T>
  constexpr bool is_zero(const Vec4<T>& v) {
    return v.x == T(0) && v.y == T(0) && v.z == T(0) && v.w == T(0);
  }

  /* -- not_zero -- */

  /**
   * @brief Checks if any component of a Vec2 is non-zero.
   *
   * @param v The vector to be checked.
   * @return True if any component is non-zero, false if all are zero.
   */
  template <typename T>
  constexpr bool not_zero(const Vec2<T> v) {
    return v.x != T(0) || v.y != T(0);
  }

  /**
   * @brief Checks if any component of a Vec3<T> is non-zero.
   *
   * @param v The vector to be checked.
   * @return True if any component is non-zero, false if all are zero.
   */
  template <typename T>
  constexpr bool not_zero(const Vec3<T>& v) {
    return v.x != T(0) || v.y != T(0) || v.z != T(0);
  }

  /**
   * @brief Checks if any component of a Vec4<T> is non-zero.
   *
   * @param v The vector to be checked.
   * @return True if any component is non-zero, false if all are zero.
   */
  template <typename T>
  constexpr bool not_zero(const Vec4<T>& v) {
    return v.x != T(0) || v.y != T(0) || v.z != T(0) || v.w != T(0);
  }

  /* -- is_almost_zero -- */

  /**
   * @brief Checks if all components of a Vec2 are nearly zero within a given epsilon.
   *
   * @param v The vector to be checked.
   * @param eps The epsilon value for comparison (default is math::epsilon).
   * @return True if all components are within epsilon of zero, false otherwise.
   */
  template <typename T>
  inline bool is_almost_zero(const Vec2<T> v, const T eps = epsilon<T>) {
    return !(std::abs(v.x) > eps || std::abs(v.y) > eps);
  }

  /**
   * @brief Checks if all components of a Vec3<T> are nearly zero within a given epsilon.
   *
   * @param v The vector to be checked.
   * @param eps The epsilon value for comparison (default is math::epsilon).
   * @return True if all components are within epsilon of zero, false otherwise.
   */
  template <typename T>
  inline bool is_almost_zero(const Vec3<T>& v, const T eps = epsilon<T>) {
    return !(std::abs(v.x) > eps || std::abs(v.y) > eps || std::abs(v.z) > eps);
  }

  /**
   * @brief Checks if all components of a Vec4<T> are nearly zero within a given epsilon.
   *
   * @param v The vector to be checked.
   * @param eps The epsilon value for comparison (default is math::epsilon).
   * @return True if all components are within epsilon of zero, false otherwise.
   */
  template <typename T>
  inline bool is_almost_zero(const Vec4<T>& v, const T eps = epsilon<T>) {
    return !(std::abs(v.x) > eps || std::abs(v.y) > eps || std::abs(v.z) > eps || std::abs(v.w) > eps);
  }

  /* -- is_almost_equal -- */

  /**
   * @brief Checks if all components of a Vec2 are nearly equal to another vector within a given epsilon.
   *
   * @param v1 The first vector for comparison.
   * @param v2 The second vector for comparison.
   * @param eps The epsilon value for comparison (default is math::epsilon).
   * @return True if all components are within epsilon of each other, false otherwise.
   */
  template <typename T>
  inline bool is_almost_equal(const Vec2<T> v1, const Vec2<T> v2, const T eps = epsilon<T>) {
    return !(std::abs(v1.x - v2.x) > eps || std::abs(v1.y - v2.y) > eps);
  }

  /**
   * @brief Checks if all components of a Vec3<T> are nearly equal to another vector within a given epsilon.
   *
   * @param v1 The first vector for comparison.
   * @param v2 The second vector for comparison.
   * @param eps The epsilon value for comparison (default is math::epsilon).
   * @return True if all components are within epsilon of each other, false otherwise.
   */
  template <typename T>
  inline bool is_almost_equal(const Vec3<T>& v1, const Vec3<T>& v2, const T eps = epsilon<T>) {
    return !(std::abs(v1.x - v2.x) > eps || std::abs(v1.y - v2.y) > eps || std::abs(v1.z - v2.z) > eps);
  }

  /**
   * @brief Checks if all components of a Vec4<T> are nearly equal to another vector within a given epsilon.
   *
   * @param v1 The first vector for comparison.
   * @param v2 The second vector for comparison.
   * @param eps The epsilon value for comparison (default is math::epsilon).
   * @return True if all components are within epsilon of each other, false otherwise.
   */
  template <typename T>
  inline bool is_almost_equal(const Vec4<T>& v1, const Vec4<T>& v2, const T eps = epsilon<T>) {
    return !(std::abs(v1.x - v2.x) > eps || std::abs(v1.y - v2.y) > eps || std::abs(v1.z - v2.z) > eps || std::abs(v1.w - v2.w) > eps);
  }

  /* -- floor -- */

  /**
   * @brief Rounds each component of a Vec2 down to the nearest integer.
   *
   * @param v The vector to be rounded.
   * @return The Vec2<T> with rounded components.
   */
  template <typename T>
  constexpr Vec2<T> floor(const Vec2<T> v) {
    return Vec2<T>(
      std::floor(v.x),
      std::floor(v.y)
    );
  }

  /**
   * @brief Rounds each component of a Vec3<T> down to the nearest integer.
   *
   * @param v The vector to be rounded.
   * @return The Vec3<T> with rounded components.
   */
  template <typename T>
  constexpr Vec3<T> floor(const Vec3<T>& v) {
    return Vec3<T>(
      std::floor(v.x),
      std::floor(v.y),
      std::floor(v.z)
    );
  }

  /**
   * @brief Rounds each component of a Vec4<T> down to the nearest integer.
   *
   * @param v The vector to be rounded.
   * @return The Vec4<T> with rounded components.
   */
  template <typename T>
  constexpr Vec4<T> floor(const Vec4<T>& v) {
    return Vec4<T>(
      std::floor(v.x),
      std::floor(v.y),
      std::floor(v.z),
      std::floor(v.w)
    );
  }

  /* -- ceil -- */

  /**
   * @brief Rounds each component of a Vec2 up to the nearest integer.
   *
   * @param v The vector to be rounded.
   * @return The Vec2<T> with rounded components.
   */
  template <typename T>
  constexpr Vec2<T> ceil(const Vec2<T> v) {
    return Vec2<T>(
      std::ceil(v.x),
      std::ceil(v.y)
    );
  }

  /**
   * @brief Rounds each component of a Vec3<T> up to the nearest integer.
   *
   * @param v The vector to be rounded.
   * @return The Vec3<T> with rounded components.
   */
  template <typename T>
  constexpr Vec3<T> ceil(const Vec3<T>& v) {
    return Vec3<T>(
      std::ceil(v.x),
      std::ceil(v.y),
      std::ceil(v.z)
    );
  }

  /**
   * @brief Rounds each component of a Vec4<T> up to the nearest integer.
   *
   * @param v The vector to be rounded.
   * @return The Vec4<T> with rounded components.
   */
  template <typename T>
  constexpr Vec4<T> ceil(const Vec4<T>& v) {
    return Vec4<T>(
      std::ceil(v.x),
      std::ceil(v.y),
      std::ceil(v.z),
      std::ceil(v.w)
    );
  }

  /* -- round -- */

  /**
   * @brief Rounds each component of a Vec2 to the nearest integer.
   *
   * @param v The vector to be rounded.
   * @return The Vec2<T> with rounded components.
   */
  template <typename T>
  constexpr Vec2<T> round(const Vec2<T> v) {
    return Vec2<T>(
      std::round(v.x),
      std::round(v.y)
    );
  }

  /**
   * @brief Rounds each component of a Vec3<T> to the nearest integer.
   *
   * @param v The vector to be rounded.
   * @return The Vec3<T> with rounded components.
   */
  template <typename T>
  constexpr Vec3<T> round(const Vec3<T>& v) {
    return Vec3<T>(
      std::round(v.x),
      std::round(v.y),
      std::round(v.z)
    );
  }

  /**
   * @brief Rounds each component of a Vec4<T> to the nearest integer.
   *
   * @param v The vector to be rounded.
   * @return The Vec4<T> with rounded components.
   */
  template <typename T>
  constexpr Vec4<T> round(const Vec4<T>& v) {
    return Vec4<T>(
      std::round(v.x),
      std::round(v.y),
      std::round(v.z),
      std::round(v.w)
    );
  }

  /* -- angle -- */

  // TODO: move here with template std::enable_if<std::is_floating_point<T>::value, T>::type

  /**
   * @brief Computes the angle in radians between two Vec2s.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @return The angle in radians between the two vectors.
   */
  template <typename T, typename = std::enable_if_t<std::is_floating_point<T>::value>>
  inline T angle(const Vec2<T> v1, const Vec2<T> v2) {
    return sign(cross(v1, v2)) * std::acos(dot(v1, v2) / std::sqrt(squared_length(v1) * squared_length(v2)))
  }

  /**
   * @brief Computes the arctangent of the angle between two Vec2s.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @return The arctangent of the angle between the two vectors.
   */
  template <typename T>
  inline T atan2(const Vec2<T> v1, const Vec2<T> v2) {
    return std::atan2(v2.y - v1.y, v2.x - v1.x);
  }

  /* -- rotate -- */

  /**
   * @brief Rotates a Vec2 around a center point by a specified angle in radians.
   *
   * @param v The vector to be rotated.
   * @param c The center of rotation.
   * @param t The angle in radians by which to rotate the vector.
   * @return The rotated Vec2.
   */
  template <typename T>
  inline Vec2<T> rotate(const Vec2<T> v, const Vec2<T> c, T t) {
    const T cx = v.x - c.x;
    const T cy = v.y - c.y;
    const T sin_t = std::sin(t);
    const T cos_t = std::cos(t);

    return Vec2<T>(
      cx * cos_t - cy * sin_t + c.x,
      cx * sin_t + cy * cos_t + c.y
    );
  }

  /**
   * @brief Rotates a Vec2 around a center point by a specified angle using precomputed sin and cos values.
   *
   * @param v The vector to be rotated.
   * @param c The center of rotation.
   * @param sin_t The sine of the rotation angle.
   * @param cos_t The cosine of the rotation angle.
   * @return The rotated Vec2.
   */
  template <typename T>
  inline Vec2<T> rotate(const Vec2<T> v, const Vec2<T> c, T sin_t, T cos_t) {
    const T cx = v.x - c.x;
    const T cy = v.y - c.y;

    return Vec2<T>(
      cx * cos_t - cy * sin_t + c.x,
      cx * sin_t + cy * cos_t + c.y
    );
  }

  /* -- scale -- */

  /**
   * @brief Scales a Vec2 by a given factor.
   *
   * @param v The vector to be scaled.
   * @param c The center of scaling.
   * @param t The scaling factor.
   * @return The scaled Vec2.
   */
  template <typename T>
  constexpr Vec2<T> scale(const Vec2<T> v, const Vec2<T> c, const Vec2<T> s) {
    return Vec2<T>(
      (v.x - c.x) * s.x + c.x,
      (v.y - c.y) * s.y + c.y
    );
  }

  /* -- orthogonal -- */

  /**
   * @brief Calculates the orthogonal vector of a Vec2.
   *
   * @param v The vector to calculate the orthogonal vector for.
   * @return The orthogonal vector.
   */
  template <typename T>
  constexpr Vec2<T> orthogonal(const Vec2<T> v) {
    return Vec2<T>(-v.y, v.x);
  }

  /**
   * @brief Calculates the orthogonal vector of a Vec2.
   *
   * @param v The vector to calculate the orthogonal vector for.
   * @param out The orthogonal output vector.
   * @return A reference to the output vector.
   */
  template <typename T>
  constexpr Vec2<T>& orthogonal(const Vec2<T> v, Vec2<T>& out) {
    T temp = v.x;

    out.x = -v.y;
    out.y = temp;

    return out;
  }

  /* -- normal -- */

  /**
   * @brief Calculates the normal vector of a Vec2.
   *
   * @param v The vector to calculate the normal vector for.
   * @return The normal vector.
   */
  template <typename T>
  constexpr Vec2<T> normal(const Vec2<T> v1, const Vec2<T> v2) {
    return normalize(Vec2<T>(v2.y - v1.y, v1.x - v2.x));
  }

  /* -- swap_coordinates -- */

  /**
   * @brief Swaps the x and y coordinates of a Vec2.
   *
   * @param v The vector to swap coordinates for.
   * @return The vector with swapped coordinates.
   */
  template <typename T>
  constexpr Vec2<T> swap_coordinates(const Vec2<T> v) {
    return (
      v.y,
      v.x
    );
  }

  /**
   * @brief Swaps the x and y coordinates of a Vec2.
   *
   * @param v The vector to swap coordinates for.
   * @param out The output vector with swapped coordinates.
   * @return A reference to the output vector.
   */
  template <typename T>
  constexpr Vec2<T>& swap_coordinates(const Vec2<T> v, Vec2<T>& out) {
    T temp = v.x;

    out.x = v.y;
    out.y = temp;

    return out;
  }

  /* -- collinear -- */

  /**
   * @brief Checks if three Vec2s are collinear.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @param v3 The third vector.
   * @param eps The epsilon value for comparison (default is math::epsilon).
   * @return True if the three vectors are collinear, false otherwise.
   */
  template <typename T>
  inline bool collinear(const Vec2<T> v1, const Vec2<T> v2, const Vec2<T> v3, const T eps = epsilon<T>) {
    const T t = v1.x * (v2.y - v3.y) + v2.x * (v3.y - v1.y) + v3.x * (v1.y - v2.y);
    return std::abs(t) <= eps;
  }

}
