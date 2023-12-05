/**
 * @file f8x8.h
 * @brief This file contains the definition of the f8.8 type, a 8.8 fixed point number.
 *
 * The f8x8 type is a 16 bit fixed point number with 8 bits for the integer part and 8 bits for the fractional part.
 * The f8x8x2 type is a 32 bit 2D vector of f8x8 numbers.
 */

#pragma once

#include <cstdint>

#define USE_F8x8 1
#define FRACBITS 8
#define FRACUNIT (1 << FRACBITS)

namespace Graphick::Math {

  using f8x8 = int16_t;

  struct f8x8x2 {
    f8x8 x;
    f8x8 y;

    f8x8x2(const f8x8 x, const f8x8 y) : x(x), y(y) {}
  };

  struct f8x8x4 {
    f8x8 x0;
    f8x8 y0;
    f8x8 x1;
    f8x8 y1;

    f8x8x4() : x0(0), y0(0), x1(0), y1(0) {}
    f8x8x4(const f8x8 x0, const f8x8 y0, const f8x8 x1, const f8x8 y1) : x0(x0), y0(y0), x1(x1), y1(y1) {}
  };

  /**
    * @brief Converts a float to a f8.8 fixed point number.
    *
    * @param x The float to convert.
    * @return The converted f8.8 fixed point number.
    */
  inline f8x8 float_to_f8x8(const float x) {
    return static_cast<f8x8>(x * FRACUNIT);
  }

  /**
   * @brief Converts a f8.8 fixed point number to a float.
   *
   * @param x The f8.8 fixed point number to convert.
   * @return The converted float.
   */
  inline float f8x8_to_float(const f8x8 x) {
    return static_cast<float>(x) / FRACUNIT;
  }

  inline f8x8 int_bits(const f8x8 x) {
    return (x >> FRACBITS) << FRACBITS;
  }

  inline f8x8 frac_bits(const f8x8 x) {
    return (x << FRACBITS) >> FRACBITS;
  }
}

namespace Graphick {

  using f8x8 = Math::f8x8;
  using f8x8x2 = Math::f8x8x2;
  using f8x8x4 = Math::f8x8x4;

}
