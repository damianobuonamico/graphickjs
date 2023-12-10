/**
 * @file dmat2x3.inl
 * @brief This file contains the inline implementation of the dmat2x3 struct.
 */

#pragma once

namespace Graphick::Math {

  /* -- Forward declarations -- */

  dmat2x3 inverse(const dmat2x3& m);

  /* -- Component accesses -- */

  inline dvec3& dmat2x3::operator[](uint8_t i) {
    return this->value[i];
  }

  constexpr const dvec3& dmat2x3::operator[](uint8_t i) const {
    return this->value[i];
  }

  /* -- Constructors -- */

  constexpr dmat2x3::dmat2x3()
    : value{ dvec3(1, 0, 0), dvec3(0, 1, 0) } {}

  constexpr dmat2x3::dmat2x3(const dmat2x3& m)
    : value{ dvec3(m[0]), dvec3(m[1]) } {}

  constexpr dmat2x3::dmat2x3(double scalar)
    : value{ dvec3(scalar, 0, 0), dvec3(0, scalar, 0) } {}

  constexpr dmat2x3::dmat2x3(const dvec3& v0, const dvec3& v1)
    : value{ dvec3(v0), dvec3(v1) } {}

  constexpr dmat2x3::dmat2x3(
    double x0, double y0, double z0,
    double x1, double y1, double z1
  ) : value{ dvec3(x0, y0, z0), dvec3(x1, y1, z1) } {}

  /* -- Assign operator -- */

  inline dmat2x3& dmat2x3::operator=(const dmat2x3& m) {
    this->value[0] = m[0];
    this->value[1] = m[1];
    return *this;
  }

  /* -- Unary arithmetic operators -- */

  inline dmat2x3& dmat2x3::operator+=(double scalar) {
    this->value[0] += scalar;
    this->value[1] += scalar;
    return *this;
  }

  inline dmat2x3& dmat2x3::operator+=(const dmat2x3& m) {
    this->value[0] += m[0];
    this->value[1] += m[1];
    return *this;
  }

  inline dmat2x3& dmat2x3::operator-=(double scalar) {
    this->value[0] -= scalar;
    this->value[1] -= scalar;
    return *this;
  }

  inline dmat2x3& dmat2x3::operator-=(const dmat2x3& m) {
    this->value[0] -= m[0];
    this->value[1] -= m[1];
    return *this;
  }

  inline dmat2x3& dmat2x3::operator*=(double scalar) {
    this->value[0] *= scalar;
    this->value[1] *= scalar;
    return *this;
  }

  inline dmat2x3& dmat2x3::operator*=(const dmat2x3& m) {
    return (*this = *this * m);
  }

  inline dmat2x3& dmat2x3::operator/=(double scalar) {
    this->value[0] /= scalar;
    this->value[1] /= scalar;
    return *this;
  }

  inline dmat2x3& dmat2x3::operator/=(const dmat2x3& m) {
    return *this *= inverse(m);
  }

  /* -- Increment/Decrement operators -- */

  inline dmat2x3& dmat2x3::operator++() {
    ++this->value[0];
    ++this->value[1];
    return *this;
  }

  inline dmat2x3& dmat2x3::operator--() {
    --this->value[0];
    --this->value[1];
    return *this;
  }

  /* -- Unary operators */

  inline dmat2x3 operator+(const dmat2x3& m) {
    return m;
  }

  inline dmat2x3 operator-(const dmat2x3& m) {
    return dmat2x3(-m[0], -m[1]);
  }

  /* -- Binary operators -- */

  inline dmat2x3 operator+(const dmat2x3& m, double scalar) {
    return dmat2x3(
      m[0] + scalar,
      m[1] + scalar
    );
  }

  inline dmat2x3 operator+(double scalar, const dmat2x3& m) {
    return dmat2x3(
      scalar + m[0],
      scalar + m[1]
    );
  }

  inline dmat2x3 operator+(const dmat2x3& m1, const dmat2x3& m2) {
    return dmat2x3(
      m1[0] + m2[0],
      m1[1] + m2[1]
    );
  }

  inline dmat2x3 operator-(const dmat2x3& m, double scalar) {
    return dmat2x3(
      m[0] - scalar,
      m[1] - scalar
    );
  }

  inline dmat2x3 operator-(double scalar, const dmat2x3& m) {
    return dmat2x3(
      scalar - m[0],
      scalar - m[1]
    );
  }

  inline dmat2x3 operator-(const dmat2x3& m1, const dmat2x3& m2) {
    return dmat2x3(
      m1[0] - m2[0],
      m1[1] - m2[1]
    );
  }

  inline dmat2x3 operator*(const dmat2x3& m, double scalar) {
    return dmat2x3(
      m[0] * scalar,
      m[1] * scalar
    );
  }

  inline dmat2x3 operator*(double scalar, const dmat2x3& m) {
    return dmat2x3(
      scalar * m[0],
      scalar * m[1]
    );
  }

  inline dvec2 operator*(const dmat2x3& m, const dvec2& v) {
    return dvec2(
      m[0][0] * v.x + m[0][1] * v.y + m[0][2],
      m[1][0] * v.x + m[1][1] * v.y + m[1][2]
    );
  }

  inline dmat2x3 operator*(const dmat2x3& m1, const dmat2x3& m2) {
    const double src_a00 = m1[0][0];
    const double src_a01 = m1[0][1];
    const double src_a02 = m1[0][2];
    const double src_a10 = m1[1][0];
    const double src_a11 = m1[1][1];
    const double src_a12 = m1[1][2];

    const double src_b00 = m2[0][0];
    const double src_b01 = m2[0][1];
    const double src_b02 = m2[0][2];
    const double src_b10 = m2[1][0];
    const double src_b11 = m2[1][1];
    const double src_b12 = m2[1][2];

    dmat2x3 result;
    result[0][0] = src_a00 * src_b00 + src_a01 * src_b10;
    result[0][1] = src_a00 * src_b01 + src_a01 * src_b11;
    result[0][2] = src_a00 * src_b02 + src_a01 * src_b12 + src_a02;
    result[1][0] = src_a10 * src_b00 + src_a11 * src_b10;
    result[1][1] = src_a10 * src_b01 + src_a11 * src_b11;
    result[1][2] = src_a10 * src_b02 + src_a11 * src_b12 + src_a12;
    return result;
  }

  inline dmat2x3 operator/(const dmat2x3& m, double scalar) {
    return dmat2x3(
      m[0] / scalar,
      m[1] / scalar
    );
  }

  inline dmat2x3 operator/(double scalar, const dmat2x3& m) {
    return dmat2x3(
      scalar / m[0],
      scalar / m[1]
    );
  }

  inline dvec2 operator/(const dmat2x3& m, const dvec2& v) {
    return inverse(m) * v;
  }

  inline dmat2x3 operator/(const dmat2x3& m1, const dmat2x3& m2) {
    dmat2x3 m1_copy(m1);
    return m1_copy /= m2;
  }

  /* -- Boolean operators -- */

  inline bool operator==(const dmat2x3& m1, const dmat2x3& m2) {
    return (m1[0] == m2[0]) && (m1[1] == m2[1]);
  }

  inline bool operator!=(const dmat2x3& m1, const dmat2x3& m2) {
    return (m1[0] != m2[0]) || (m1[1] != m2[1]);
  }

  /* -- Address operator -- */

  inline const double* operator&(const dmat2x3& m) { return &(m[0].x); }

}
