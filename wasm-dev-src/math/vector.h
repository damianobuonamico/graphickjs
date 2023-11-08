/**
 * @file vector.h
 * @brief Contains mathods for vector manipulation. Supports vec2, vec3, vec4, ivec2.
 */

#pragma once

#include "vec2.h"
#include "vec3.h"
#include "vec4.h"

#include "ivec2.h"

#include "../utils/defines.h"

#include <vector>

#define XY(v) Graphick::Math::vec2{ v.x, v.y }
#define RG(v) Graphick::Math::vec2{ v.r, v.g }
#define ST(v) Graphick::Math::vec2{ v.s, v.t }

#define XYZ(v) Graphick::Math::vec3{ v.x, v.y, v.z }
#define RGB(v) Graphick::Math::vec3{ v.r, v.g, v.b }
#define STP(v) Graphick::Math::vec3{ v.s, v.t, v.p }

#define IVEC2_TO_VEC2(v) Graphick::Math::vec2{ (float)v.x, (float)v.y }
#define VEC2_TO_IVEC2(v) Graphick::Math::ivec2{ (int)v.x, (int)v.y }

namespace Graphick::Math {

  /* -- min -- */

  /**
   * @brief Calculates the minimum of two vec2.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @return The minimum of the two vectors.
   */
  constexpr vec2 min(const vec2 v1, const vec2 v2) {
    return vec2{
      std::min(v1.x, v2.x),
      std::min(v1.y, v2.y)
    };
  }

  /**
   * @brief Calculates the minimum of two vec3.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @return The minimum of the two vectors.
   */
  constexpr vec3 min(const vec3& v1, const vec3& v2) {
    return vec3{
      std::min(v1.x, v2.x),
      std::min(v1.y, v2.y),
      std::min(v1.z, v2.z)
    };
  }

  /**
   * @brief Calculates the minimum of two vec4.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @return The minimum of the two vectors.
   */
  constexpr vec4 min(const vec4& v1, const vec4& v2) {
    return vec4{
      std::min(v1.x, v2.x),
      std::min(v1.y, v2.y),
      std::min(v1.z, v2.z),
      std::min(v1.w, v2.w),
    };
  }

  /**
   * @brief Calculates the minimum of two vec2.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @param out The output vector, can be the same as v1 or v2.
   * @return A reference to the output vector.
   */
  constexpr vec2& min(const vec2 v1, const vec2 v2, vec2& out) {
    out.x = std::min(v1.x, v2.x);
    out.y = std::min(v1.y, v2.y);

    return out;
  }

  /**
   * @brief Calculates the minimum of two vec3.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @param out The output vector, can be the same as v1 or v2.
   * @return A reference to the output vector.
   */
  constexpr vec3& min(const vec3& v1, const vec3& v2, vec3& out) {
    out.x = std::min(v1.x, v2.x);
    out.y = std::min(v1.y, v2.y);
    out.z = std::min(v1.z, v2.z);

    return out;
  }

  /**
   * @brief Calculates the minimum of two vec4.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @param out The output vector, can be the same as v1 or v2.
   * @return A reference to the output vector.
   */
  constexpr vec4& min(const vec4& v1, const vec4& v2, vec4& out) {
    out.x = std::min(v1.x, v2.x);
    out.y = std::min(v1.y, v2.y);
    out.z = std::min(v1.z, v2.z);
    out.w = std::min(v1.w, v2.w);

    return out;
  }

  /* -- max -- */

  /**
   * @brief Calculates the maximum of two vec2.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @return The maximum of the two vectors.
   */
  constexpr vec2 max(const vec2 v1, const vec2 v2) {
    return vec2{
      std::max(v1.x, v2.x),
      std::max(v1.y, v2.y)
    };
  }

  /**
   * @brief Calculates the maximum of two vec3.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @return The maximum of the two vectors.
   */
  constexpr vec3 max(const vec3& v1, const vec3& v2) {
    return vec3{
      std::max(v1.x, v2.x),
      std::max(v1.y, v2.y),
      std::max(v1.z, v2.z)
    };
  }

  /**
   * @brief Calculates the maximum of two vec4.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @return The maximum of the two vectors.
   */
  constexpr vec4 max(const vec4& v1, const vec4& v2) {
    return vec4{
      std::max(v1.x, v2.x),
      std::max(v1.y, v2.y),
      std::max(v1.z, v2.z),
      std::max(v1.w, v2.w)
    };
  }

  /**
   * @brief Calculates the maximum of two vec2.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @param out The output vector, can be the same as v1 or v2.
   * @return A reference to the output vector.
   */
  constexpr vec2 max(const vec2 v1, const vec2 v2, vec2& out) {
    out.x = std::max(v1.x, v2.x);
    out.y = std::max(v1.y, v2.y);

    return out;
  }

  /**
   * @brief Calculates the maximum of two vec3.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @param out The output vector, can be the same as v1 or v2.
   * @return A reference to the output vector.
   */
  constexpr vec3 max(const vec3& v1, const vec3& v2, vec3& out) {
    out.x = std::max(v1.x, v2.x);
    out.y = std::max(v1.y, v2.y);
    out.z = std::max(v1.z, v2.z);

    return out;
  }

  /**
   * @brief Calculates the maximum of two vec4.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @param out The output vector, can be the same as v1 or v2.
   * @return A reference to the output vector.
   */
  constexpr vec4 max(const vec4& v1, const vec4& v2, vec4& out) {
    out.x = std::max(v1.x, v2.x);
    out.y = std::max(v1.y, v2.y);
    out.z = std::max(v1.z, v2.z);
    out.w = std::max(v1.w, v2.w);

    return out;
  }

  /* -- length -- */

  /**
   * @brief Calculates the length or magnitude of a vec2.
   *
   * @param v The vector.
   * @return The length of the vector.
   */
  inline float length(const vec2 v) {
    return std::hypot(v.x, v.y);
  }

  /**
   * @brief Calculates the length or magnitude of a vec3.
   *
   * @param v The vector.
   * @return The length of the vector.
   */
  inline float length(const vec3& v) {
    return std::hypot(v.x, v.y, v.z);
  }

  /**
   * @brief Calculates the length or magnitude of a vec4.
   *
   * @param v The vector.
   * @return The length of the vector.
   */
  inline float length(const vec4& v) {
    return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
  }

  /* -- dot -- */

  /**
   * @brief Computes the dot product of two vec2s.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @return The dot product of the two vectors.
   */
  constexpr float dot(const vec2 v1, const vec2 v2) {
    return v1.x * v2.x + v1.y * v2.y;
  }

  /**
   * @brief Computes the dot product of two vec3s.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @return The dot product of the two vectors.
   */
  constexpr float dot(const vec3& v1, const vec3& v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
  }

  /**
   * @brief Computes the dot product of two vec4s.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @return The dot product of the two vectors.
   */
  constexpr float dot(const vec4& v1, const vec4& v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;
  }

  /* -- cross -- */

  /**
   * @brief Computes the cross product of two vec2s.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @return The cross product as a scalar value.
   */
  constexpr float cross(const vec2 v1, const vec2 v2) {
    return v1.x * v2.y - v2.x * v1.y;
  }

  /**
   * @brief Computes the cross product of two vec3s.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @return The cross product as a vec3.
   */
  constexpr vec3 cross(const vec3& v1, const vec3& v2) {
    return {
      v1.y * v2.z - v1.z * v2.y,
      -(v1.x * v2.z - v1.z * v2.x),
      v1.x * v2.y - v1.y * v2.x
    };
  }

  /* -- squared_length -- */

  /**
   * @brief Calculates the squared length or magnitude of a vec2.
   *
   * @param v The vector.
   * @return The squared length of the vector.
   */
  constexpr float squared_length(const vec2 v) {
    return v.x * v.x + v.y * v.y;
  }

  /**
   * @brief Calculates the squared length or magnitude of a vec3.
   *
   * @param v The vector.
   * @return The squared length of the vector.
   */
  constexpr float squared_length(const vec3& v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
  }

  /**
   * @brief Calculates the squared length or magnitude of a vec4.
   *
   * @param v The vector.
   * @return The squared length of the vector.
   */
  constexpr float squared_length(const vec4& v) {
    return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
  }

  /* -- distance -- */

  /**
   * @brief Calculates the distance between two vec2.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @return The distance between the two vectors.
   */
  inline float distance(const vec2 v1, const vec2 v2) {
    return std::hypot(v2.x - v1.x, v2.y - v1.y);
  }

  /**
   * @brief Calculates the distance between two vec3.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @return The distance between the two vectors.
   */
  inline float distance(const vec3& v1, const vec3& v2) {
    return std::hypot(v2.x - v1.x, v2.y - v1.y, v2.z - v1.z);
  }

  /**
   * @brief Calculates the distance between two vec4.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @return The distance between the two vectors.
   */
  inline float distance(const vec4& v1, const vec4& v2) {
    return length(v2 - v1);
  }

  /* -- squared_distance -- */

  /**
   * @brief Calculates the squared distance between two vec2.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @return The squared distance between the two vectors.
   */
  constexpr float squared_distance(const vec2 v1, const vec2 v2) {
    vec2 v = v2 - v1;
    return dot(v, v);
  }

  /**
   * @brief Calculates the squared distance between two vec3.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @return The squared distance between the two vectors.
   */
  constexpr float squared_distance(const vec3& v1, const vec3& v2) {
    vec3 v = v2 - v1;
    return dot(v, v);
  }

  /**
   * @brief Calculates the squared distance between two vec4.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @return The squared distance between the two vectors.
   */
  constexpr float squared_distance(const vec4& v1, const vec4& v2) {
    vec4 v = v2 - v1;
    return dot(v, v);
  }

  /* -- lerp -- */

  /**
   * @brief Linearly interpolates between two vec2.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @param t The interpolation factor.
   * @return The interpolated vector.
   */
  constexpr vec2 lerp(const vec2 v1, const vec2 v2, float t) {
    return vec2{
      v1.x + t * (v2.x - v1.x),
      v1.y + t * (v2.y - v1.y)
    };
  }

  /**
   * @brief Linearly interpolates between two vec3.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @param t The interpolation factor.
   * @return The interpolated vector.
   */
  constexpr vec3 lerp(const vec3& v1, const vec3& v2, float t) {
    return vec3{
      v1.x + t * (v2.x - v1.x),
      v1.y + t * (v2.y - v1.y),
      v1.z + t * (v2.z - v1.z)
    };
  }

  /**
   * @brief Linearly interpolates between two vec4.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @param t The interpolation factor.
   * @return The interpolated vector.
   */
  constexpr vec4 lerp(const vec4& v1, const vec4& v2, float t) {
    return vec4{
      v1.x + t * (v2.x - v1.x),
      v1.y + t * (v2.y - v1.y),
      v1.z + t * (v2.z - v1.z),
      v1.w + t * (v2.w - v1.w)
    };
  }

  /* -- quadratic -- */

  /**
   * @brief Quadratic bezier interpolation between three vec2.
   *
   * @param v1 p0.
   * @param v2 p1.
   * @param v3 p3.
   * @param t The interpolation factor.
   */
  constexpr vec2 quadratic(const vec2 v1, const vec2 v2, const vec2 v3, float t) {
    vec2 a = v1 - 2.0f * v2 + v3;
    vec2 b = 2.0f * (v2 - v1);
    vec2 c = v1;

    return a * t * t + b * t + c;
  }

  /**
   * @brief Calculates the derivative of a quadratic bezier curve.
   *
   * @param v1 p0.
   * @param v2 p1.
   * @param v3 p3.
   * @param t The interpolation factor.
  */
  constexpr vec2 quadratic_derivative(const vec2 v1, const vec2 v2, const vec2 v3, float t) {
    return 2.0f * (v1 - 2.0f * v2 + v3) * t + 2.0f * (v2 - v1);
  }

  /* -- bezier -- */

  /**
   * @brief Cubic bezier interpolation between four vec2.
   *
   * @param v1 p0.
   * @param v2 p1.
   * @param v3 p2.
   * @param v4 p3.
   * @param t The interpolation factor.
   */
  constexpr vec2 bezier(const vec2 v1, const vec2 v2, const vec2 v3, const vec2 v4, float t) {
    vec2 a = -v1 + 3.0f * v2 - 3.0f * v3 + v4;
    vec2 b = 3.0f * v1 - 6.0f * v2 + 3.0f * v3;
    vec2 c = -3.0f * v1 + 3.0f * v2;

    float t_sq = t * t;

    return a * t_sq * t + b * t_sq + c * t + v1;
  }

  /**
   * @brief Calculates the derivative of a cubic bezier curve.
   *
   * @param v1 p0.
   * @param v2 p1.
   * @param v3 p2.
   * @param v4 p3.
   * @param t The interpolation factor.
   */
  constexpr vec2 bezier_derivative(const vec2 v1, const vec2 v2, const vec2 v3, const vec2 v4, float t) {
    vec2 a = 3.0f * (-v1 + 3.0f * v2 - 3.0f * v3 + v4);
    vec2 b = 6.0f * (v1 - 2.0f * v2 + v3);
    vec2 c = -3.0f * (v1 - v2);

    return a * t * t + b * t + c;
  }

  /**
   * @brief Calculates the second derivative of a cubic bezier curve.
   *
   * @param v1 p0.
   * @param v2 p1.
   * @param v3 p2.
   * @param v4 p3.
   * @param t The interpolation factor.
  */
  constexpr vec2 bezier_second_derivative(const vec2 v1, const vec2 v2, const vec2 v3, const vec2 v4, float t) {
    vec2 a = 6.0f * (-v1 + 3.0f * v2 - 3.0f * v3 + v4);
    vec2 b = 6.0f * (v1 - 2.0f * v2 + v3);

    return a * t + b;
  }

  /**
   * @brief Calculates the extrema of a cubic bezier curve.
   *
   * @param v1 p0.
   * @param v2 p1.
   * @param v3 p2.
   * @param v4 p3.
   * @return The t values corresponding to the extrema of the curve.
   */
  std::vector<float> bezier_extrema(const vec2 v1, const vec2 v2, const vec2 v3, const vec2 v4);

  /* -- midpoint -- */

  /**
   * @brief Calculates the midpoint between two vec2s.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @return The midpoint between v1 and v2.
   */
  constexpr vec2 midpoint(const vec2 v1, const vec2 v2) {
    return vec2{
      (v1.x + v2.x) * 0.5f,
      (v1.y + v2.y) * 0.5f
    };
  }

  /**
   * @brief Calculates the midpoint between two vec3s.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @return The midpoint between v1 and v2.
   */
  constexpr vec3 midpoint(const vec3& v1, const vec3& v2) {
    return vec3{
      (v1.x + v2.x) * 0.5f,
      (v1.y + v2.y) * 0.5f,
      (v1.z + v2.z) * 0.5f
    };
  }

  /**
   * @brief Calculates the midpoint between two vec4s.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @return The midpoint between v1 and v2.
   */
  constexpr vec4 midpoint(const vec4& v1, const vec4& v2) {
    return vec4{
      (v1.x + v2.x) * 0.5f,
      (v1.y + v2.y) * 0.5f,
      (v1.z + v2.z) * 0.5f,
      (v1.w + v2.w) * 0.5f
    };
  }

  /* -- normalize -- */

  /**
   * @brief Normalizes a vec2 to have a length of 1.
   *
   * @param v The vector to normalize.
   * @return The normalized vec2.
   */
  constexpr vec2 normalize(const vec2 v) {
    float len = v.x * v.x + v.y * v.y;
    if (len > 0) len = 1 / std::sqrt(len);

    return vec2{
      v.x * len,
      v.y * len,
    };
  }

  /**
   * @brief Normalizes a vec3 to have a length of 1.
   *
   * @param v The vector to normalize.
   * @return The normalized vec3.
   */
  constexpr vec3 normalize(const vec3& v) {
    float len = v.x * v.x + v.y * v.y + v.z * v.z;
    if (len > 0) len = 1 / std::sqrt(len);

    return vec3{
      v.x * len,
      v.y * len,
      v.z * len,
    };
  }

  /**
   * @brief Normalizes a vec4 to have a length of 1.
   *
   * @param v The vector to normalize.
   * @return The normalized vec4.
   */
  constexpr vec4 normalize(const vec4& v) {
    float len = v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
    if (len > 0) len = 1 / std::sqrt(len);

    return vec4{
      v.x * len,
      v.y * len,
      v.z * len,
      v.w * len,
    };
  }

  /**
   * @brief Normalizes a vec2 to have a length of 1.
   *
   * @param v The vector to normalize.
   * @param out The normalized vector (output).
   * @return The normalized vec2.
   */
  constexpr vec2& normalize(const vec2 v, vec2& out) {
    float len = v.x * v.x + v.y * v.y;
    if (len > 0) len = 1 / std::sqrt(len);

    out.x = v.x * len;
    out.y = v.y * len;

    return out;
  }

  /**
   * @brief Normalizes a vec3 to have a length of 1.
   *
   * @param v The vector to normalize.
   * @param out The normalized vector (output).
   * @return The normalized vec3.
   */
  constexpr vec3& normalize(const vec3& v, vec3& out) {
    float len = v.x * v.x + v.y * v.y + v.z * v.z;
    if (len > 0) len = 1 / std::sqrt(len);

    out.x = v.x * len;
    out.y = v.y * len;
    out.z = v.z * len;

    return out;
  }

  /**
   * @brief Normalizes a vec4 to have a length of 1.
   *
   * @param v The vector to normalize.
   * @param out The normalized vector (output).
   * @return The normalized vec4.
   */
  constexpr vec4& normalize(const vec4& v, vec4& out) {
    float len = v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
    if (len > 0) len = 1 / std::sqrt(len);

    out.x = v.x * len;
    out.y = v.y * len;
    out.z = v.z * len;
    out.w = v.w * len;

    return out;
  }

  /* -- normalize_length -- */

  /**
   * @brief Normalizes a vec2 and scales it by a given length.
   *
   * @param v The vector to normalize and scale.
   * @param t The scaling factor.
   * @return The normalized and scaled vec2.
   */
  constexpr vec2 normalize_length(const vec2 v, float t) {
    float len = v.x * v.x + v.y * v.y;
    if (len > 0) len = 1 / std::sqrt(len);

    return vec2{
      v.x * len * t,
      v.y * len * t,
    };
  }

  /**
   * @brief Normalizes a vec3 and scales it by a given length.
   *
   * @param v The vector to normalize and scale.
   * @param t The scaling factor.
   * @return The normalized and scaled vec3.
   */
  constexpr vec3 normalize_length(const vec3& v, float t) {
    float len = v.x * v.x + v.y * v.y + v.z * v.z;
    if (len > 0) len = 1 / std::sqrt(len);

    return vec3{
      v.x * len * t,
      v.y * len * t,
      v.z * len * t,
    };
  }

  /**
   * @brief Normalizes a vec4 and scales it by a given length.
   *
   * @param v The vector to normalize and scale.
   * @param t The scaling factor.
   * @return The normalized and scaled vec4.
   */
  constexpr vec4 normalize_length(const vec4& v, float t) {
    float len = v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
    if (len > 0) len = 1 / std::sqrt(len);

    return vec4{
      v.x * len * t,
      v.y * len * t,
      v.z * len * t,
      v.w * len * t,
    };
  }

  /**
   * @brief Normalizes a vec2 and scales it by a given length.
   *
   * @param v The vector to normalize and scale.
   * @param t The scaling factor.
   * @param out The normalized and scaled vector (output).
   * @return The normalized and scaled vec2.
   */
  constexpr vec2& normalize_length(const vec2 v, float t, vec2& out) {
    float len = v.x * v.x + v.y * v.y;
    if (len > 0) len = 1 / std::sqrt(len);

    out.x = v.x * len * t;
    out.y = v.y * len * t;

    return out;
  }

  /**
   * @brief Normalizes a vec3 and scales it by a given length.
   *
   * @param v The vector to normalize and scale.
   * @param t The scaling factor.
   * @param out The normalized and scaled vector (output).
   * @return The normalized and scaled vec3.
   */
  constexpr vec3& normalize_length(const vec3& v, float t, vec3& out) {
    float len = v.x * v.x + v.y * v.y + v.z * v.z;
    if (len > 0) len = 1 / std::sqrt(len);

    out.x = v.x * len * t;
    out.y = v.y * len * t;
    out.z = v.z * len * t;

    return out;
  }

  /**
   * @brief Normalizes a vec4 and scales it by a given length.
   *
   * @param v The vector to normalize and scale.
   * @param t The scaling factor.
   * @param out The normalized and scaled vector (output).
   * @return The normalized and scaled vec4.
   */
  constexpr vec4& normalize_length(const vec4& v, float t, vec4& out) {
    float len = v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
    if (len > 0) len = 1 / std::sqrt(len);

    out.x = v.x * len * t;
    out.y = v.y * len * t;
    out.z = v.z * len * t;
    out.w = v.w * len * t;

    return out;
  }

  /* -- negate -- */

  /**
   * @brief Negates a vec2.
   *
   * @param v The vector to negate.
   * @return The negated vec2.
   */
  constexpr vec2 negate(const vec2 v) {
    return vec2{
      -v.x,
      -v.y
    };
  }

  /**
   * @brief Negates a vec3.
   *
   * @param v The vector to negate.
   * @return The negated vec3.
   */
  constexpr vec3 negate(const vec3& v) {
    return vec3{
      -v.x,
      -v.y,
      -v.z
    };
  }

  /**
   * @brief Negates a vec4.
   *
   * @param v The vector to negate.
   * @return The negated vec4.
   */
  constexpr vec4 negate(const vec4& v) {
    return vec4{
      -v.x,
      -v.y,
      -v.z,
      -v.w
    };
  }

  /**
   * @brief Negates a vec2.
   *
   * @param v The vector to negate.
   * @param out The negated vector (output).
   * @return The negated vec2.
   */
  constexpr vec2& negate(const vec2 v, vec2& out) {
    out.x = -v.x;
    out.y = -v.y;

    return out;
  }

  /**
   * @brief Negates a vec3.
   *
   * @param v The vector to negate.
   * @param out The negated vector (output).
   * @return The negated vec3.
   */
  constexpr vec3& negate(const vec3& v, vec3& out) {
    out.x = -v.x;
    out.y = -v.y;
    out.z = -v.z;

    return out;
  }

  /**
   * @brief Negates a vec4.
   *
   * @param v The vector to negate.
   * @param out The negated vector (output).
   * @return The negated vec4.
   */
  constexpr vec4& negate(const vec4& v, vec4& out) {
    out.x = -v.x;
    out.y = -v.y;
    out.z = -v.z;
    out.w = -v.w;

    return out;
  }

  /* -- abs -- */

  /**
   * @brief Calculates the absolute value of each component of a vec2.
   *
   * @param v The vector to calculate absolute values for.
   * @return The vec2 with absolute values of its components.
   */
  inline vec2 abs(const vec2 v) {
    return vec2{
      std::fabsf(v.x),
      std::fabsf(v.y)
    };
  }

  /**
   * @brief Calculates the absolute value of each component of a vec3.
   *
   * @param v The vector to calculate absolute values for.
   * @return The vec3 with absolute values of its components.
   */
  inline vec3 abs(const vec3& v) {
    return vec3{
      std::fabsf(v.x),
      std::fabsf(v.y),
      std::fabsf(v.z)
    };
  }

  /**
   * @brief Calculates the absolute value of each component of a vec4.
   *
   * @param v The vector to calculate absolute values for.
   * @return The vec4 with absolute values of its components.
   */
  inline vec4 abs(const vec4& v) {
    return vec4{
      std::fabsf(v.x),
      std::fabsf(v.y),
      std::fabsf(v.z),
      std::fabsf(v.w)
    };
  }

  /**
   * @brief Calculates the absolute value of each component of a vec2.
   *
   * @param v The vector to calculate absolute values for.
   * @param out The vector with absolute values of its components (output).
   * @return The vec2 with absolute values of its components.
   */
  inline vec2& abs(const vec2 v, vec2& out) {
    out.x = std::fabsf(v.x);
    out.y = std::fabsf(v.y);

    return out;
  }
  /**
   * @brief Calculates the absolute value of each component of a vec3.
   *
   * @param v The vector to calculate absolute values for.
   * @param out The vector with absolute values of its components (output).
   * @return The vec3 with absolute values of its components.
   */
  inline vec3& abs(const vec3& v, vec3& out) {
    out.x = std::fabsf(v.x);
    out.y = std::fabsf(v.y);
    out.z = std::fabsf(v.z);
    return out;
  }

  /**
   * @brief Calculates the absolute value of each component of a vec4.
   *
   * @param v The vector to calculate absolute values for.
   * @param out The vector with absolute values of its components (output).
   * @return The vec4 with absolute values of its components.
   */
  inline vec4& abs(const vec4& v, vec4& out) {
    out.x = std::fabsf(v.x);
    out.y = std::fabsf(v.y);
    out.z = std::fabsf(v.z);
    out.w = std::fabsf(v.w);
    return out;
  }

  /* -- zero -- */

  /**
   * @brief Sets all components of a vec2 to zero.
   *
   * @param v The vector to be zeroed.
   */
  constexpr void zero(vec2& v) {
    v.x = v.y = 0.0f;
  }

  /**
   * @brief Sets all components of a vec3 to zero.
   *
   * @param v The vector to be zeroed.
   */
  constexpr void zero(vec3& v) {
    v.x = v.y = v.z = 0.0f;
  }

  /**
   * @brief Sets all components of a vec4 to zero.
   *
   * @param v The vector to be zeroed.
   */
  constexpr void zero(vec4& v) {
    v.x = v.y = v.z = v.w = 0.0f;
  }

  /* -- is_zero -- */

  /**
   * @brief Checks if all components of a vec2 are zero.
   *
   * @param v The vector to be checked.
   * @return True if all components are zero, false otherwise.
   */
  constexpr bool is_zero(const vec2 v) {
    return v.x == 0 && v.y == 0;
  }

  /**
   * @brief Checks if all components of a vec3 are zero.
   *
   * @param v The vector to be checked.
   * @return True if all components are zero, false otherwise.
   */
  constexpr bool is_zero(const vec3& v) {
    return v.x == 0 && v.y == 0 && v.z == 0;
  }

  /**
   * @brief Checks if all components of a vec4 are zero.
   *
   * @param v The vector to be checked.
   * @return True if all components are zero, false otherwise.
   */
  constexpr bool is_zero(const vec4& v) {
    return v.x == 0 && v.y == 0 && v.z == 0 && v.w == 0;
  }

  /* -- not_zero -- */

  /**
   * @brief Checks if any component of a vec2 is non-zero.
   *
   * @param v The vector to be checked.
   * @return True if any component is non-zero, false if all are zero.
   */
  constexpr bool not_zero(const vec2 v) {
    return v.x != 0 || v.y != 0;
  }

  /**
   * @brief Checks if any component of a vec3 is non-zero.
   *
   * @param v The vector to be checked.
   * @return True if any component is non-zero, false if all are zero.
   */
  constexpr bool not_zero(const vec3& v) {
    return v.x != 0 || v.y != 0 || v.z != 0;
  }

  /**
   * @brief Checks if any component of a vec4 is non-zero.
   *
   * @param v The vector to be checked.
   * @return True if any component is non-zero, false if all are zero.
   */
  constexpr bool not_zero(const vec4& v) {
    return v.x != 0 || v.y != 0 || v.z != 0 || v.w != 0;
  }

  /* -- is_almost_zero -- */

  /**
   * @brief Checks if all components of a vec2 are nearly zero within a given epsilon.
   *
   * @param v The vector to be checked.
   * @param eps The epsilon value for comparison (default is GK_EPSILON).
   * @return True if all components are within epsilon of zero, false otherwise.
   */
  inline bool is_almost_zero(const vec2 v, const float eps = GK_EPSILON) {
    return !(std::abs(v.x) > eps || std::abs(v.y) > eps);
  }

  /**
   * @brief Checks if all components of a vec3 are nearly zero within a given epsilon.
   *
   * @param v The vector to be checked.
   * @param eps The epsilon value for comparison (default is GK_EPSILON).
   * @return True if all components are within epsilon of zero, false otherwise.
   */
  inline bool is_almost_zero(const vec3& v, const float eps = GK_EPSILON) {
    return !(std::abs(v.x) > eps || std::abs(v.y) > eps || std::abs(v.z) > eps);
  }

  /**
   * @brief Checks if all components of a vec4 are nearly zero within a given epsilon.
   *
   * @param v The vector to be checked.
   * @param eps The epsilon value for comparison (default is GK_EPSILON).
   * @return True if all components are within epsilon of zero, false otherwise.
   */
  inline bool is_almost_zero(const vec4& v, const float eps = GK_EPSILON) {
    return !(std::abs(v.x) > eps || std::abs(v.y) > eps || std::abs(v.z) > eps || std::abs(v.w) > eps);
  }

  /* -- is_almost_equal -- */

  /**
   * @brief Checks if all components of a vec2 are nearly equal to another vector within a given epsilon.
   *
   * @param v1 The first vector for comparison.
   * @param v2 The second vector for comparison.
   * @param eps The epsilon value for comparison (default is GK_EPSILON).
   * @return True if all components are within epsilon of each other, false otherwise.
   */
  inline bool is_almost_equal(const vec2 v1, const vec2 v2, const float eps = GK_EPSILON) {
    return !(std::abs(v1.x - v2.x) > eps || std::abs(v1.y - v2.y) > eps);
  }

  /**
   * @brief Checks if all components of a vec3 are nearly equal to another vector within a given epsilon.
   *
   * @param v1 The first vector for comparison.
   * @param v2 The second vector for comparison.
   * @param eps The epsilon value for comparison (default is GK_EPSILON).
   * @return True if all components are within epsilon of each other, false otherwise.
   */
  inline bool is_almost_equal(const vec3& v1, const vec3& v2, const float eps = GK_EPSILON) {
    return !(std::abs(v1.x - v2.x) > eps || std::abs(v1.y - v2.y) > eps || std::abs(v1.z - v2.z) > eps);
  }

  /**
   * @brief Checks if all components of a vec4 are nearly equal to another vector within a given epsilon.
   *
   * @param v1 The first vector for comparison.
   * @param v2 The second vector for comparison.
   * @param eps The epsilon value for comparison (default is GK_EPSILON).
   * @return True if all components are within epsilon of each other, false otherwise.
   */
  inline bool is_almost_equal(const vec4& v1, const vec4& v2, const float eps = GK_EPSILON) {
    return !(std::abs(v1.x - v2.x) > eps || std::abs(v1.y - v2.y) > eps || std::abs(v1.z - v2.z) > eps || std::abs(v1.w - v2.w) > eps);
  }

  /* -- floor -- */

  /**
   * @brief Rounds each component of a vec2 down to the nearest integer.
   *
   * @param v The vector to be rounded.
   * @return The vec2 with rounded components.
   */
  constexpr vec2 floor(const vec2 v) {
    return vec2{
      std::floor(v.x),
      std::floor(v.y),
    };
  }

  /**
   * @brief Rounds each component of a vec3 down to the nearest integer.
   *
   * @param v The vector to be rounded.
   * @return The vec3 with rounded components.
   */
  constexpr vec3 floor(const vec3& v) {
    return vec3{
     std::floor(v.x),
     std::floor(v.y),
     std::floor(v.z),
    };
  }

  /**
   * @brief Rounds each component of a vec4 down to the nearest integer.
   *
   * @param v The vector to be rounded.
   * @return The vec4 with rounded components.
   */
  constexpr vec4 floor(const vec4& v) {
    return vec4{
      std::floor(v.x),
      std::floor(v.y),
      std::floor(v.z),
      std::floor(v.w),
    };
  }

  /* -- ceil -- */

  /**
   * @brief Rounds each component of a vec2 up to the nearest integer.
   *
   * @param v The vector to be rounded.
   * @return The vec2 with rounded components.
   */
  constexpr vec2 ceil(const vec2 v) {
    return vec2{
      std::ceil(v.x),
      std::ceil(v.y),
    };
  }

  /**
   * @brief Rounds each component of a vec3 up to the nearest integer.
   *
   * @param v The vector to be rounded.
   * @return The vec3 with rounded components.
   */
  constexpr vec3 ceil(const vec3& v) {
    return vec3{
      std::ceil(v.x),
      std::ceil(v.y),
      std::ceil(v.z),
    };
  }

  /**
   * @brief Rounds each component of a vec4 up to the nearest integer.
   *
   * @param v The vector to be rounded.
   * @return The vec4 with rounded components.
   */
  constexpr vec4 ceil(const vec4& v) {
    return vec4{
      std::ceil(v.x),
      std::ceil(v.y),
      std::ceil(v.z),
      std::ceil(v.w),
    };
  }

  /* -- round -- */

  /**
   * @brief Rounds each component of a vec2 to the nearest integer.
   *
   * @param v The vector to be rounded.
   * @return The vec2 with rounded components.
   */
  constexpr vec2 round(const vec2 v) {
    return vec2{
      std::round(v.x),
      std::round(v.y)
    };
  }

  /**
   * @brief Rounds each component of a vec3 to the nearest integer.
   *
   * @param v The vector to be rounded.
   * @return The vec3 with rounded components.
   */
  constexpr vec3 round(const vec3& v) {
    return vec3{
      std::round(v.x),
      std::round(v.y),
      std::round(v.z)
    };
  }

  /**
   * @brief Rounds each component of a vec4 to the nearest integer.
   *
   * @param v The vector to be rounded.
   * @return The vec4 with rounded components.
   */
  constexpr vec4 round(const vec4& v) {
    return vec4{
      std::round(v.x),
      std::round(v.y),
      std::round(v.z),
      std::round(v.w)
    };
  }

  /* -- angle -- */

  /**
   * @brief Computes the angle in radians between two vec2s.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @return The angle in radians between the two vectors.
   */
  float angle(const vec2 v1, const vec2 v2);

  /**
   * @brief Computes the arctangent of the angle between two vec2s.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @return The arctangent of the angle between the two vectors.
   */
  inline float atan2(const vec2 v1, const vec2 v2) {
    return std::atan2(v2.y - v1.y, v2.x - v1.x);
  }

  /* -- rotate -- */

  /**
   * @brief Rotates a vec2 around a center point by a specified angle in radians.
   *
   * @param v The vector to be rotated.
   * @param c The center of rotation.
   * @param t The angle in radians by which to rotate the vector.
   * @return The rotated vec2.
   */
  inline vec2 rotate(const vec2 v, const vec2 c, float t) {
    float cx = v.x - c.x;
    float cy = v.y - c.y;
    float sin_t = std::sin(t);
    float cos_t = std::cos(t);

    return vec2{
      cx * cos_t - cy * sin_t + c.x,
      cx * sin_t + cy * cos_t + c.y
    };
  }

  /**
   * @brief Rotates a vec2 around a center point by a specified angle using precomputed sin and cos values.
   *
   * @param v The vector to be rotated.
   * @param c The center of rotation.
   * @param sin_t The sine of the rotation angle.
   * @param cos_t The cosine of the rotation angle.
   * @return The rotated vec2.
   */
  inline vec2 rotate(const vec2 v, const vec2 c, float sin_t, float cos_t) {
    float cx = v.x - c.x;
    float cy = v.y - c.y;

    return vec2{
      cx * cos_t - cy * sin_t + c.x,
      cx * sin_t + cy * cos_t + c.y
    };
  }

  /* -- scale -- */

  /**
   * @brief Scales a vec2 by a given factor.
   *
   * @param v The vector to be scaled.
   * @param c The center of scaling.
   * @param t The scaling factor.
   * @return The scaled vec2.
   */
  constexpr vec2 scale(const vec2 v, const vec2 c, const vec2 s) {
    return vec2{
      (v.x - c.x) * s.x + c.x,
      (v.y - c.y) * s.y + c.y
    };
  }

  /* -- orthogonal -- */

  /**
   * @brief Calculates the orthogonal vector of a vec2.
   *
   * @param v The vector to calculate the orthogonal vector for.
   * @return The orthogonal vector.
   */
  constexpr vec2 orthogonal(const vec2 v) {
    return vec2{ -v.y, v.x };
  }

  /**
   * @brief Calculates the orthogonal vector of a vec2.
   *
   * @param v The vector to calculate the orthogonal vector for.
   * @param out The orthogonal output vector.
   * @return A reference to the output vector.
   */
  constexpr vec2& orthogonal(const vec2 v, vec2& out) {
    float temp = v.x;

    out.x = -v.y;
    out.y = temp;

    return out;
  }

  /* -- normal -- */

  /**
   * @brief Calculates the normal vector of a vec2.
   *
   * @param v The vector to calculate the normal vector for.
   * @return The normal vector.
   */
  constexpr vec2 normal(const vec2 v1, const vec2 v2) {
    return normalize({ v2.y - v1.y, v1.x - v2.x });
  }

  /* -- swap_coordinates -- */

  /**
   * @brief Swaps the x and y coordinates of a vec2.
   *
   * @param v The vector to swap coordinates for.
   * @return The vector with swapped coordinates.
   */
  constexpr vec2 swap_coordinates(const vec2 v) {
    return {
      v.y,
      v.x
    };
  }

  /**
   * @brief Swaps the x and y coordinates of a vec2.
   *
   * @param v The vector to swap coordinates for.
   * @param out The output vector with swapped coordinates.
   * @return A reference to the output vector.
   */
  constexpr vec2& swap_coordinates(const vec2 v, vec2& out) {
    float temp = v.x;

    out.x = v.y;
    out.y = temp;

    return out;
  }

  /* -- collinear -- */

  /**
   * @brief Checks if three vec2s are collinear.
   *
   * @param v1 The first vector.
   * @param v2 The second vector.
   * @param v3 The third vector.
   * @param eps The epsilon value for comparison (default is GK_EPSILON).
   * @return True if the three vectors are collinear, false otherwise.
   */
  bool collinear(const vec2 v1, const vec2 v2, const vec2 v3, const float eps = GK_EPSILON);

  /* -- stringify -- */

  /**
   * @brief Converts a vec2 to a string.
   *
   * Returns a string in the format "(x, y)".
   *
   * @param v The vector to convert.
   * @return The string representation of the vector.
   */
  std::string stringify(const vec2 v);

  /**
   * @brief Converts a vec3 to a string.
   *
   * Returns a string in the format "(x, y, z)".
   *
   * @param v The vector to convert.
   * @return The string representation of the vector.
   */
  std::string stringify(const vec3& v);

  /**
   * @brief Converts a vec4 to a string.
   *
   * Returns a string in the format "(x, y, x, w)".
   *
   * @param v The vector to convert.
   * @return The string representation of the vector.
   */
  std::string stringify(const vec4& v);

}
