/**
 * @file dvec2.h
 * @brief This file contains the definition of the dvec2 struct, a 2D double precision vector.
 */

#pragma once

#include <cstdint>

namespace Graphick::Math {

  /**
   * @brief A 2D vector struct with x and y components.
   *
   * @struct dvec2
   */
  struct dvec2 {
    union { double x, r, s; };   /* The 0 component of the vector. */
    union { double y, g, t; };   /* The 1 component of the vector. */

    /* -- Component accesses -- */

    static constexpr uint8_t length() { return 2; }
    constexpr double& operator[](uint8_t i);
    constexpr double const& operator[](uint8_t i) const;

    /* -- Constructors -- */

    dvec2() = default;
    constexpr dvec2(const dvec2& v) = default;
    constexpr explicit dvec2(double scalar);
    constexpr dvec2(double x, double y);

    /* -- Assign operator -- */

    constexpr dvec2& operator=(const dvec2 v);

    /* -- Unary arithmetic operators -- */

    constexpr dvec2& operator+=(double scalar);
    constexpr dvec2& operator+=(const dvec2 v);
    constexpr dvec2& operator-=(double scalar);
    constexpr dvec2& operator-=(const dvec2 v);
    constexpr dvec2& operator*=(double scalar);
    constexpr dvec2& operator*=(const dvec2 v);
    constexpr dvec2& operator/=(double scalar);
    constexpr dvec2& operator/=(const dvec2 v);

    /* -- Increment/Decrement operators -- */

    constexpr dvec2& operator++();
    constexpr dvec2& operator--();
  };

  /* -- Unary operators */

  constexpr dvec2 operator+(const dvec2 v);
  constexpr dvec2 operator-(const dvec2 v);

  /* -- Binary operators -- */

  constexpr dvec2 operator+(const dvec2 v, double scalar);
  constexpr dvec2 operator+(double scalar, const dvec2 v);
  constexpr dvec2 operator+(const dvec2 v1, const dvec2 v2);
  constexpr dvec2 operator-(const dvec2 v, double scalar);
  constexpr dvec2 operator-(double scalar, const dvec2 v);
  constexpr dvec2 operator-(const dvec2 v1, const dvec2 v2);
  constexpr dvec2 operator*(const dvec2 v, double scalar);
  constexpr dvec2 operator*(double scalar, const dvec2 v);
  constexpr dvec2 operator*(const dvec2 v1, const dvec2 v2);
  constexpr dvec2 operator/(const dvec2 v, double scalar);
  constexpr dvec2 operator/(double scalar, const dvec2 v);
  constexpr dvec2 operator/(const dvec2 v1, const dvec2 v2);
  constexpr dvec2 operator%(const dvec2 v, double scalar);
  constexpr dvec2 operator%(double scalar, const dvec2 v);
  constexpr dvec2 operator%(const dvec2 v1, const dvec2 v2);

  /* -- Boolean operators -- */

  constexpr bool operator==(const dvec2 v1, const dvec2 v2);
  constexpr bool operator!=(const dvec2 v1, const dvec2 v2);
  constexpr dvec2 operator&&(const dvec2 v1, const dvec2 v2);
  constexpr dvec2 operator||(const dvec2 v1, const dvec2 v2);

  /* -- Address operator -- */

  constexpr const double* operator&(const dvec2& v);

}

namespace Graphick {

  using dvec2 = Math::dvec2;

}

#include "dvec2.inl"
