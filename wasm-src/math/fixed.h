/**
 * @file math/fixed.h
 * @brief This file contains templated fixed point number types.
 *
 * The f8x8 type is a 16 bit fixed point number with 8 bits for the integer part and 8 bits for the fractional part.
 * The f8x8x2 type is a 32 bit 2D vector of f8x8 numbers.
 * The f8x8x4 type is a 64 bit 4D vector of f8x8 numbers.
 *
 * The f24x8 type is a 32 bit fixed point number with 24 bits for the integer part and 8 bits for the fractional part.
 * The f24x8x2 type is a 64 bit 2D vector of f24x8 numbers.
 * The f24x8x4 type is a 128 bit 4D vector of f24x8 numbers.
 */

#pragma once

#include "vec2.h"
#include "vec3.h"
#include "vec4.h"

namespace graphick::math {

  /**
   * @brief A I.D fixed point number.
   *
   * @tparam Base The base type of the fixed point number.
   * @tparam Intermediate The intermediate type of the fixed point number calculations.
   * @tparam Fractional The number of fractional bits.
   *
   * @struct Fixed
   */
  template <typename Base, typename Intermediate, uint8_t Fractional>
  struct Fixed {
    Base x;    /* The underlying value of the fixed point number. */

    static constexpr uint8_t FRACBITS = Fractional;
    static constexpr uint8_t INTBITS = sizeof(Base) * 8 - FRACBITS;
    static constexpr Base FRACUNIT = (1 << FRACBITS);

    /* -- Component accesses -- */

    constexpr Fixed int_bits() const {
      return (x >> FRACBITS) << FRACBITS;
    }

    constexpr Fixed frac_bits() const {
      return (x << INTBITS) >> INTBITS;
    }

    /* -- Constructors -- */

    Fixed() = default;

    constexpr Fixed(const Fixed& x) = default;

    template <typename U, typename = std::enable_if<std::is_base_of<Fixed, U>::value>>
    constexpr explicit Fixed(U x) :
      x(static_cast<Base>(x.x)) {}

    template <typename U>
    constexpr explicit Fixed(U x) :
      x(static_cast<Base>(x* FRACUNIT)) {}

    /* -- Static constructors -- */

    static constexpr Fixed<Base, Intermediate, Fractional> from_raw(Base x) {
      Fixed temp;
      temp.x = x;
      return temp;
    }

    /* -- Unary arithmetic operators -- */

    template <typename U>
    constexpr Fixed<Base, Intermediate, Fractional>& operator+=(U scalar) {
      this->x += Fixed<Base, Intermediate, Fractional>(scalar);
      return *this;
    }

    constexpr Fixed<Base, Intermediate, Fractional>& operator+=(const Fixed<Base, Intermediate, Fractional> x) {
      this->x += x.x;
      return *this;
    }

    template <typename U>
    constexpr Fixed<Base, Intermediate, Fractional>& operator-=(U scalar) {
      this->x -= Fixed<Base, Intermediate, Fractional>(scalar);
      return *this;
    }

    constexpr Fixed<Base, Intermediate, Fractional>& operator-=(const Fixed<Base, Intermediate, Fractional> x) {
      this->x -= x.x;
      return *this;
    }

    template <typename U>
    constexpr Fixed<Base, Intermediate, Fractional>& operator*=(U scalar) {
      this->x *= scalar;
      return *this;
    }

    constexpr Fixed<Base, Intermediate, Fractional>& operator*=(const Fixed<Base, Intermediate, Fractional> x) {
      this->x = static_cast<Base>((static_cast<Intermediate>(this->x) * static_cast<Intermediate>(x.x)) >> FRACBITS);
      return *this;
    }

    template <typename U>
    constexpr Fixed<Base, Intermediate, Fractional>& operator/=(U scalar) {
      this->x /= scalar;
      return *this;
    }

    constexpr Fixed<Base, Intermediate, Fractional>& operator/=(const Fixed<Base, Intermediate, Fractional> v) {
      this->x = static_cast<Base>((static_cast<Intermediate>(this->x) << FRACBITS) / static_cast<Intermediate>(v.x));
      return *this;
    }

    /* -- Increment/Decrement operators -- */

    constexpr Fixed<Base, Intermediate, Fractional>& operator++() {
      this->x += FRACUNIT;
      return *this;
    }

    constexpr Fixed<Base, Intermediate, Fractional>& operator--() {
      this->x -= FRACUNIT;
      return *this;
    }

    /* -- Type conversion operators -- */

    template <typename U, typename = std::enable_if<std::is_floating_point_v<U>>>
    constexpr explicit operator U() const {
      return static_cast<U>(this->x) / FRACUNIT;
    }

    template <typename U, typename = std::enable_if<std::is_integral_v<U>>>
    constexpr explicit operator U() const {
      return static_cast<U>(this->x >> FRACBITS);
    }

    constexpr Base raw() const {
      return this->x;
    }

    constexpr float to_float() const {
      return static_cast<float>(*this);
    }

    constexpr double to_double() const {
      return static_cast<double>(*this);
    }
  };

  /* -- Unary operators */

  template <typename T, typename I, uint8_t F>
  constexpr Fixed<T, I, F> operator+(const Fixed<T, I, F> x) {
    return x;
  }

  template <typename T, typename I, uint8_t F>
  constexpr Fixed<T, I, F> operator-(const Fixed<T, I, F> x) {
    return Fixed<T, I, F>::from_raw(-x.x);
  }

  /* -- Binary operators -- */

  template <typename T, typename I, uint8_t F, typename U>
  constexpr Fixed<T, I, F> operator+(const Fixed<T, I, F> x, U scalar) {
    return x + Fixed<T, I, F>(scalar);
  }

  template <typename T, typename I, uint8_t F, typename U>
  constexpr Fixed<T, I, F> operator+(U scalar, const Fixed<T, I, F> x) {
    return Fixed<T, I, F>(scalar) + x;
  }

  template <typename T, typename I, uint8_t F>
  constexpr Fixed<T, I, F> operator+(const Fixed<T, I, F> x, const Fixed<T, I, F> y) {
    return Fixed<T, I, F>::from_raw(x.x + y.x);
  }

  template <typename T, typename I, uint8_t F, typename U>
  constexpr Fixed<T, I, F> operator-(const Fixed<T, I, F> x, U scalar) {
    return x - Fixed<T, I, F>(scalar);
  }

  template <typename T, typename I, uint8_t F, typename U>
  constexpr Fixed<T, I, F> operator-(U scalar, const Fixed<T, I, F> x) {
    return Fixed<T, I, F>(scalar) - x;
  }

  template <typename T, typename I, uint8_t F>
  constexpr Fixed<T, I, F> operator-(const Fixed<T, I, F> x, const Fixed<T, I, F> y) {
    return Fixed<T, I, F>::from_raw(x.x - y.x);
  }

  template <typename T, typename I, uint8_t F, typename U>
  constexpr Fixed<T, I, F> operator*(const Fixed<T, I, F> x, U scalar) {
    return Fixed<T, I, F>::from_raw(x.x * scalar);
  }

  template <typename T, typename I, uint8_t F, typename U>
  constexpr Fixed<T, I, F> operator*(U scalar, const Fixed<T, I, F> x) {
    return Fixed<T, I, F>::from_raw(scalar * x.x);
  }

  template <typename T, typename I, uint8_t F>
  constexpr Fixed<T, I, F> operator*(const Fixed<T, I, F> x, const Fixed<T, I, F> y) {
    return Fixed<T, I, F>::from_raw((static_cast<I>(x.x) * static_cast<I>(y.x)) >> F);
  }

  template <typename T, typename I, uint8_t F, typename U>
  constexpr Fixed<T, I, F> operator/(const Fixed<T, I, F> x, U scalar) {
    return Fixed<T, I, F>(x.x / scalar);
  }

  template <typename T, typename I, uint8_t F, typename U>
  constexpr Fixed<T, I, F> operator/(U scalar, const Fixed<T, I, F> x) {
    return Fixed<T, I, F>(scalar / x.x);
  }

  template <typename T, typename I, uint8_t F>
  constexpr Fixed<T, I, F> operator/(const Fixed<T, I, F> x, const Fixed<T, I, F> y) {
    return Fixed<T, I, F>::from_raw((static_cast<I>(x.x) << F) / y.x);
  }

  template <typename T, typename I, uint8_t F, typename U>
  constexpr Fixed<T, I, F> operator%(const Fixed<T, I, F> x, U scalar) {
    return Fixed<T, I, F>(((x.x >> x.FRACBITS) % static_cast<T>(scalar)) << x.FRACBITS + x.frac_bits());
  }

  template <typename T, typename I, uint8_t F, typename U>
  constexpr Fixed<T, I, F> operator%(U scalar, const Fixed<T, I, F> x) {
    return Fixed<T, I, F>((static_cast<T>(scalar) % (x.x >> x.FRACBITS)) << x.FRACBITS);
  }

  template <typename T, typename I, uint8_t F>
  constexpr Fixed<T, I, F> operator%(const Fixed<T, I, F> x, const Fixed<T, I, F> y) {
    return Fixed<T, I, F>((x.x >> x.FRACBITS) % (y.x >> y.FRACBITS) << x.FRACBITS);
  }

  /* -- Boolean operators -- */

  template <typename T, typename I, uint8_t F>
  constexpr bool operator==(const Fixed<T, I, F> x, const Fixed<T, I, F> y) {
    return x.x == y.x;
  }

  template <typename T, typename I, uint8_t F>
  constexpr bool operator!=(const Fixed<T, I, F> x, const Fixed<T, I, F> y) {
    return x.x != y.x;
  }
}

namespace std {

  /* -- numeric_limits -- */

  template <typename T, typename I, uint8_t F>
  class numeric_limits<graphick::math::Fixed<T, I, F>> {
  public:
    static inline graphick::math::Fixed<T, I, F> min() {
      return graphick::math::Fixed{ numeric_limits<T>::min() };
    }

    static inline graphick::math::Fixed<T, I, F> max() {
      return graphick::math::Fixed{ numeric_limits<T>::max() };
    }

    static inline graphick::math::Fixed<T, I, F> lowest() {
      return graphick::math::Fixed{ numeric_limits<T>::lowest() };
    }
  };

}

/* -- Aliases -- */

namespace graphick::math {

  using f8x8 = math::Fixed<int16_t, int32_t, 8>;
  using f8x8x2 = math::Vec2<f8x8>;
  using f8x8x4 = math::Vec4<f8x8>;

  using f24x8 = math::Fixed<int32_t, int64_t, 8>;
  using f24x8x2 = math::Vec2<f24x8>;
  using f24x8x4 = math::Vec4<f24x8>;

}

namespace graphick {

  using math::f8x8;
  using math::f8x8x2;
  using math::f8x8x4;

  using math::f24x8;
  using math::f24x8x2;
  using math::f24x8x4;

}
