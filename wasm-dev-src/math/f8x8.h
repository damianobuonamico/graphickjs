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
  using f8x8x2 = uint32_t;

  struct f8x8x4 {
    f8x8x2 p0;
    f8x8x2 p1;
  };
}

namespace Graphick {

  using f8x8 = Math::f8x8;
  using f8x8x2 = Math::f8x8x2;
  using f8x8x4 = Math::f8x8x4;

}
