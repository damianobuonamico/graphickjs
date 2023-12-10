/**
 * @file dvec3.h
 * @brief This file contains the definition of the dvec3 struct.
 */

#pragma once

#include <cstdint>

namespace Graphick::Math {

  /**
   * @brief A 3D vector struct with x, y and z components.
   *
   * @struct dvec3
   */
  struct dvec3 {
    union { double x, r, s; };
    union { double y, g, t; };
    union { double z, b, p; };

    /* -- Component accesses -- */

    static constexpr uint8_t length() { return 3; }
    constexpr double& operator[](uint8_t i);
    constexpr const double& operator[](uint8_t i) const;

    /* -- Constructors -- */

    dvec3() = default;
    constexpr dvec3(const dvec3& v) = default;
    constexpr explicit dvec3(double scalar);
    constexpr dvec3(double x, double y, double z);

    /* -- Assign operator -- */

    constexpr dvec3& operator=(const dvec3& v);

    /* -- Unary arithmetic operators -- */

    constexpr dvec3& operator+=(double scalar);
    constexpr dvec3& operator+=(const dvec3& v);
    constexpr dvec3& operator-=(double scalar);
    constexpr dvec3& operator-=(const dvec3& v);
    constexpr dvec3& operator*=(double scalar);
    constexpr dvec3& operator*=(const dvec3& v);
    constexpr dvec3& operator/=(double scalar);
    constexpr dvec3& operator/=(const dvec3& v);

    /* -- Increment/Decrement operators -- */

    constexpr dvec3& operator++();
    constexpr dvec3& operator--();
  };

  /* -- Unary operators */

  constexpr dvec3 operator+(const dvec3& v);
  constexpr dvec3 operator-(const dvec3& v);

  /* -- Binary operators -- */

  constexpr dvec3 operator+(const dvec3& v, double scalar);
  constexpr dvec3 operator+(double scalar, const dvec3& v);
  constexpr dvec3 operator+(const dvec3& v1, const dvec3& v2);
  constexpr dvec3 operator-(const dvec3& v, double scalar);
  constexpr dvec3 operator-(double scalar, const dvec3& v);
  constexpr dvec3 operator-(const dvec3& v1, const dvec3& v2);
  constexpr dvec3 operator*(const dvec3& v, double scalar);
  constexpr dvec3 operator*(double scalar, const dvec3& v);
  constexpr dvec3 operator*(const dvec3& v1, const dvec3& v2);
  constexpr dvec3 operator/(const dvec3& v, double scalar);
  constexpr dvec3 operator/(double scalar, const dvec3& v);
  constexpr dvec3 operator/(const dvec3& v1, const dvec3& v2);
  constexpr dvec3 operator%(const dvec3& v, double scalar);
  constexpr dvec3 operator%(double scalar, const dvec3& v);
  constexpr dvec3 operator%(const dvec3& v1, const dvec3& v2);

  /* -- Boolean operators -- */

  constexpr bool operator==(const dvec3& v1, const dvec3& v2);
  constexpr bool operator!=(const dvec3& v1, const dvec3& v2);
  constexpr dvec3 operator&&(const dvec3& v1, const dvec3& v2);
  constexpr dvec3 operator||(const dvec3& v1, const dvec3& v2);

  /* -- Address operator -- */

  constexpr const double* operator&(const dvec3& v);

}

namespace Graphick {

  using dvec3 = Math::dvec3;

}

#include "dvec3.inl"
