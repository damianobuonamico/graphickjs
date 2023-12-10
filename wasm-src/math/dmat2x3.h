/**
 * @file dmat2x3.h
 * @brief This file contains the definition of the dmat2x3 struct.
 */

#pragma once

#include "vec2.h"
#include "dvec3.h"

namespace Graphick::Math {

  /**
   * @brief A 2x3 matrix struct with 3 columns and 2 rows.
   *
   * This matrix is not mathematically correct, it is only used to reduce the memory footprint of 2D transforms.
   * The missing row is always interpreted as [0, 0, 1].
   * When multiplying a dvec2 with this matrix, the third component of the vector is always treated as 1.
   *
   * @struct dmat2x3
   */
  struct dmat2x3 {
    /* -- Component accesses -- */

    static constexpr uint8_t length() { return 2; }
    dvec3& operator[](uint8_t i);
    constexpr dvec3 const& operator[](uint8_t i) const;

    /* -- Constructors -- */

    constexpr dmat2x3();
    constexpr dmat2x3(const dmat2x3& m);
    constexpr explicit dmat2x3(double scalar);
    constexpr dmat2x3(const dvec3& v0, const dvec3& v1);
    constexpr dmat2x3(
      double x0, double y0, double z0,
      double x1, double y1, double z1
    );

    /* -- Assign operator -- */

    dmat2x3& operator=(const dmat2x3& m);

    /* -- Unary arithmetic operators -- */

    dmat2x3& operator+=(double scalar);
    dmat2x3& operator+=(const dmat2x3& m);
    dmat2x3& operator-=(double scalar);
    dmat2x3& operator-=(const dmat2x3& m);
    dmat2x3& operator*=(double scalar);
    dmat2x3& operator*=(const dmat2x3& m);
    dmat2x3& operator/=(double scalar);
    dmat2x3& operator/=(const dmat2x3& m);

    /* -- Increment/Decrement operators -- */

    dmat2x3& operator++();
    dmat2x3& operator--();
  private:
    dvec3 value[2];
  };

  /* -- Unary operators */

  dmat2x3 operator+(const dmat2x3& m);
  dmat2x3 operator-(const dmat2x3& m);

  /* -- Binary operators -- */

  dmat2x3 operator+(const dmat2x3& m, double scalar);
  dmat2x3 operator+(double scalar, const dmat2x3& m);
  dmat2x3 operator+(const dmat2x3& m1, const dmat2x3& m2);
  dmat2x3 operator-(const dmat2x3& m, double scalar);
  dmat2x3 operator-(double scalar, const dmat2x3& m);
  dmat2x3 operator-(const dmat2x3& m1, const dmat2x3& m2);
  dmat2x3 operator*(const dmat2x3& m, double scalar);
  dmat2x3 operator*(double scalar, const dmat2x3& m);
  dvec2 operator*(const dmat2x3& m, const dvec2& v);
  dmat2x3 operator*(const dmat2x3& m1, const dmat2x3& m2);
  dmat2x3 operator/(const dmat2x3& m, double scalar);
  dmat2x3 operator/(double scalar, const dmat2x3& m);
  dvec2 operator/(const dmat2x3& m, const dvec2& v);
  dmat2x3 operator/(const dmat2x3& m1, const dmat2x3& m2);

  /* -- Boolean operators -- */

  bool operator==(const dmat2x3& m1, const dmat2x3& m2);
  bool operator!=(const dmat2x3& m1, const dmat2x3& m2);

  /* -- Address operator -- */

  const double* operator&(const dmat2x3& m);

}

namespace Graphick {

  using dmat2x3 = Math::dmat2x3;

}

#include "dmat2x3.inl"
