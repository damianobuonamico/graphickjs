/**
 * @file utils/half.h
 * @brief The file contains the definition of the half float type.
 *
 * The following implementation is an adaptation of:
 * <https://github.com/acgessler/half_float/blob/master/umHalf.h>
 */

#pragma once

#include <cstdint>

namespace graphick::utils {

/**
 * @brief The half float type.
 *
 * This is a 16-bit floating point number.
 */
struct half {
  union {
    uint16_t bits;         // The 16-bit representation of the half float

    struct {
      uint16_t frac : 10;  // 10 bits for the mantissa
      uint16_t exp : 5;    // 5 bits for the exponent
      uint16_t sign : 1;   // 1 bit for the sign
    } IEEE;
  };

  union IEEESingle {
    float f;               // The 32-bit representation of the float

    struct {
      uint32_t frac : 23;  // 23 bits for the mantissa
      uint32_t exp : 8;    // 8 bits for the exponent
      uint32_t sign : 1;   // 1 bit for the sign
    } IEEE;
  };

  /**
   * @brief Default constructor.
   */
  half() = default;

  /**
   * @brief Constructs a half float from a float.
   *
   * @param other The float.
   */
  half(float other)
  {
    IEEESingle f;
    f.f = other;

    IEEE.sign = f.IEEE.sign;

    if (!f.IEEE.exp) {
      IEEE.frac = 0;
      IEEE.exp = 0;
    } else if (f.IEEE.exp == 0xFF) {
      // NaN or INF
      IEEE.frac = (f.IEEE.frac != 0) ? 1 : 0;
      IEEE.exp = 31;
    } else {
      // Regular number
      int new_exp = f.IEEE.exp - 127;

      if (new_exp < -24) {
        // this maps to 0
        IEEE.frac = 0;
        IEEE.exp = 0;
      }

      else if (new_exp < -14)
      {
        // This maps to a denorm
        IEEE.exp = 0;
        unsigned int exp_val = (unsigned int)(-14 - new_exp);  // 2^-exp_val

        switch (exp_val) {
          case 0:
            IEEE.frac = 0;
            break;
          case 1:
            IEEE.frac = 512 + (f.IEEE.frac >> 14);
            break;
          case 2:
            IEEE.frac = 256 + (f.IEEE.frac >> 15);
            break;
          case 3:
            IEEE.frac = 128 + (f.IEEE.frac >> 16);
            break;
          case 4:
            IEEE.frac = 64 + (f.IEEE.frac >> 17);
            break;
          case 5:
            IEEE.frac = 32 + (f.IEEE.frac >> 18);
            break;
          case 6:
            IEEE.frac = 16 + (f.IEEE.frac >> 19);
            break;
          case 7:
            IEEE.frac = 8 + (f.IEEE.frac >> 20);
            break;
          case 8:
            IEEE.frac = 4 + (f.IEEE.frac >> 21);
            break;
          case 9:
            IEEE.frac = 2 + (f.IEEE.frac >> 22);
            break;
          case 10:
            IEEE.frac = 1;
            break;
        }
      } else if (new_exp > 15) {
        // Map this value to infinity
        IEEE.frac = 0;
        IEEE.exp = 31;
      } else {
        IEEE.exp = new_exp + 15;
        IEEE.frac = (f.IEEE.frac >> 13);
      }
    }
  }
};

}  // namespace graphick::utils

namespace graphick {
using half = utils::half;
}
