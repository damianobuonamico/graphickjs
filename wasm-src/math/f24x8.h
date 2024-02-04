/**
 * @file f24x8.h
 * @brief This file contains the definition of the f24.8 type, a 24.8 fixed point number.
 *
 * The f24x8 type is a 32 bit fixed point number with 24 bits for the integer part and 8 bits for the fractional part.
 * The f24x8x2 type is a 64 bit 2D vector of f24x8 numbers.
 * The f24x8x4 type is a 128 bit 4D vector of f24x8 numbers.
 *
 * @todo finish documenting functions
 */

#pragma once

#include "f8x8.h"

#define INTBITS24 (sizeof(f24x8) * 8 - FRACBITS)

namespace Graphick::Math {

  using f24x8 = int32_t;

  /**
   * @brief A 2D vector of f24.8 fixed point numbers.
   *
   * @struct f24x8x2
   */
  struct f24x8x2 {
    f24x8 x;
    f24x8 y;

    f24x8x2(const f24x8 x, const f24x8 y) :
      x(x),
      y(y) {}

    inline bool operator==(const f24x8x2 other) const {
      return x == other.x && y == other.y;
    }

    inline bool operator!=(const f24x8x2 other) const {
      return x != other.x || y != other.y;
    }
  };

  /**
   * @brief A 4D vector of f24.8 fixed point numbers.
   *
   * @struct f24x8x4
   */
  struct f24x8x4 {
    f24x8 x0;
    f24x8 y0;
    f24x8 x1;
    f24x8 y1;

    f24x8x4() = default;
    f24x8x4(const f24x8 x0, const f24x8 y0, const f24x8 x1, const f24x8 y1) : x0(x0), y0(y0), x1(x1), y1(y1) {}
  };

  /**
    * @brief Converts a float to a f24.8 fixed point number.
    *
    * @param x The float to convert.
    * @return The converted f24.8 fixed point number.
    */
  inline f24x8 float_to_f24x8(const float x) {
    return static_cast<f24x8>(x * FRACUNIT);
  }

  /**
   * @brief Converts a double to a f24.8 fixed point number.
   *
   * @param x The double to convert.
   * @return The converted f24.8 fixed point number.
   */
  inline f24x8 double_to_f24x8(const double x) {
    return static_cast<f24x8>(x * FRACUNIT);
  }

  /**
   * @brief Converts two doubles to a f24.8x2 fixed point number.
   *
   * @param x The first double to convert.
   * @param y The second double to convert.
   * @return The converted f24.8x2 fixed point number.
   */
  inline f24x8x2 double_to_f24x8x2(const double x, const double y) {
    return { double_to_f24x8(x), double_to_f24x8(y) };
  }

  /**
   * @brief Converts a f24.8 fixed point number to a float.
   *
   * @param x The f24.8 fixed point number to convert.
   * @return The converted float.
   */
  inline float f24x8_to_float(const f24x8 x) {
    return static_cast<float>(x) / FRACUNIT;
  }

  /**
   * @brief Returns the integer part of a f24.8 fixed point number.
   *
   * @param x The f24.8 fixed point number.
   * @return The integer part of the f24.8 fixed point number.
   */
  inline f24x8 int_bits(const f24x8 x) {
    return (x >> FRACBITS) << FRACBITS;
  }

  /**
   * @brief Returns the fractional part of a f24.8 fixed point number.
   *
   * @param x The f24.8 fixed point number.
   * @return The fractional part of the f24.8 fixed point number.
   */
  inline f24x8 frac_bits(const f24x8 x) {
    return (x << INTBITS24) >> INTBITS24;
  }

}

namespace Graphick {

  using f24x8 = Math::f24x8;
  using f24x8x2 = Math::f24x8x2;
  using f24x8x4 = Math::f24x8x4;

}
