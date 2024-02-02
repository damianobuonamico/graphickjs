/**
 * @file mat4.h
 * @brief This file contains the Mat4 struct, a templated 4D matrix.
 */
#pragma once

#include "vec4.h"

namespace Graphick::Math {

  /**
   * @brief A 4D matrix struct.
   *
   * @struct Mat4
   */
  template<typename T>
  struct Mat4 {
    /* -- Component accesses -- */

    static constexpr uint8_t length() {
      return 4;
    }

    constexpr Vec4<T>& operator[](uint8_t i) {
      return this->value[i];
    }

    constexpr Vec4<T> const& operator[](uint8_t i) const {
      return this->value[i];
    }

    /* -- Constructors -- */

    constexpr Mat4() : value{
      Vec4<T>(1, 0, 0, 0),
      Vec4<T>(0, 1, 0, 0),
      Vec4<T>(0, 0, 1, 0),
      Vec4<T>(0, 0, 0, 1)
    } {}

    constexpr Mat4(const Mat4& m) : value{
      Vec4<T>(m[0]),
      Vec4<T>(m[1]),
      Vec4<T>(m[2]),
      Vec4<T>(m[3])
    } {}

    constexpr explicit Mat4(T scalar) : value{
      Vec4<T>(scalar, 0, 0, 0),
      Vec4<T>(0, scalar, 0, 0),
      Vec4<T>(0, 0, scalar, 0),
      Vec4<T>(0, 0, 0, scalar)
    } {}

    constexpr Mat4(const Vec4<T>& v0, const Vec4<T>& v1, const Vec4<T>& v2, const Vec4<T>& v3) : value{
      Vec4<T>(v0),
      Vec4<T>(v1),
      Vec4<T>(v2),
      Vec4<T>(v3)
    } {}

    constexpr Mat4(
      T x0, T y0, T z0, T w0,
      T x1, T y1, T z1, T w1,
      T x2, T y2, T z2, T w2,
      T x3, T y3, T z3, T w3
    ) : value{
      Vec4<T>(x0, y0, z0, w0),
      Vec4<T>(x1, y1, z1, w1),
      Vec4<T>(x2, y2, z2, w2),
      Vec4<T>(x3, y3, z3, w3)
    } {}

    template<typename U>
    constexpr explicit Mat4(const Mat4<U>& m) : value{
      Vec4<T>(m[0]),
      Vec4<T>(m[1]),
      Vec4<T>(m[2]),
      Vec4<T>(m[3])
    } {}

    /* -- Assign operator -- */

    constexpr Mat4<T>& operator=(const Mat4<T>& m) {
      this->value[0] = m[0];
      this->value[1] = m[1];
      this->value[2] = m[2];
      this->value[3] = m[3];
      return *this;
    }

    /* -- Unary arithmetic operators -- */

    template<typename U>
    constexpr Mat4<T>& operator+=(U scalar) {
      this->value[0] += scalar;
      this->value[1] += scalar;
      this->value[2] += scalar;
      this->value[3] += scalar;
      return *this;
    }

    constexpr Mat4<T>& operator+=(const Mat4<T>& m) {
      this->value[0] += m[0];
      this->value[1] += m[1];
      this->value[2] += m[2];
      this->value[3] += m[3];
      return *this;
    }

    template<typename U>
    constexpr Mat4<T>& operator-=(U scalar) {
      this->value[0] -= scalar;
      this->value[1] -= scalar;
      this->value[2] -= scalar;
      this->value[3] -= scalar;
      return *this;
    }

    constexpr Mat4<T>& operator-=(const Mat4<T>& m) {
      this->value[0] -= m[0];
      this->value[1] -= m[1];
      this->value[2] -= m[2];
      this->value[3] -= m[3];
      return *this;
    }

    template<typename U>
    constexpr Mat4<T>& operator*=(U scalar) {
      this->value[0] *= scalar;
      this->value[1] *= scalar;
      this->value[2] *= scalar;
      this->value[3] *= scalar;
      return *this;
    }

    constexpr Mat4<T>& operator*=(const Mat4<T>& m) {
      return (*this = *this * m);
    }

    template<typename U>
    constexpr Mat4<T>& operator/=(U scalar) {
      this->value[0] /= scalar;
      this->value[1] /= scalar;
      this->value[2] /= scalar;
      this->value[3] /= scalar;
      return *this;
    }

    /* -- Increment/Decrement operators -- */

    constexpr Mat4<T>& operator++() {
      ++this->value[0];
      ++this->value[1];
      ++this->value[2];
      ++this->value[3];
      return *this;
    }

    constexpr Mat4<T>& operator--() {
      --this->value[0];
      --this->value[1];
      --this->value[2];
      --this->value[3];
      return *this;
    }
  private:
    Vec4<T> value[4];
  };

  /* -- Unary operators */

  template<typename T>
  constexpr Mat4<T> operator+(const Mat4<T>& m) {
    return m;
  }

  template<typename T>
  constexpr Mat4<T> operator-(const Mat4<T>& m) {
    return Mat4<T>(
      -m[0],
      -m[1],
      -m[2],
      -m[3]
    );
  }

  /* -- Binary operators -- */

  template<typename T, typename U>
  constexpr Mat4<T> operator+(const Mat4<T>& m, U scalar) {
    return Mat4<T>(
      m[0] + scalar,
      m[1] + scalar,
      m[2] + scalar,
      m[3] + scalar
    );
  }

  template<typename T, typename U>
  constexpr Mat4<T> operator+(U scalar, const Mat4<T>& m) {
    return Mat4<T>(
      scalar + m[0],
      scalar + m[1],
      scalar + m[2],
      scalar + m[3]
    );
  }

  template<typename T>
  constexpr Mat4<T> operator+(const Mat4<T>& m1, const Mat4<T>& m2) {
    return Mat4<T>(
      m1[0] + m2[0],
      m1[1] + m2[1],
      m1[2] + m2[2],
      m1[3] + m2[3]
    );
  }

  template<typename T, typename U>
  constexpr Mat4<T> operator-(const Mat4<T>& m, U scalar) {
    return Mat4<T>(
      m[0] - scalar,
      m[1] - scalar,
      m[2] - scalar,
      m[3] - scalar
    );
  }

  template<typename T, typename U>
  constexpr Mat4<T> operator-(U scalar, const Mat4<T>& m) {
    return Mat4<T>(
      scalar - m[0],
      scalar - m[1],
      scalar - m[2],
      scalar - m[3]
    );
  }

  template<typename T>
  constexpr Mat4<T> operator-(const Mat4<T>& m1, const Mat4<T>& m2) {
    return Mat4<T>(
      m1[0] - m2[0],
      m1[1] - m2[1],
      m1[2] - m2[2],
      m1[3] - m2[3]
    );
  }

  template<typename T, typename U>
  constexpr Mat4<T> operator*(const Mat4<T>& m, U scalar) {
    return Mat4<T>(
      m[0] * scalar,
      m[1] * scalar,
      m[2] * scalar,
      m[3] * scalar
    );
  }

  template<typename T, typename U>
  constexpr Mat4<T> operator*(U scalar, const Mat4<T>& m) {
    return Mat4<T>(
      scalar * m[0],
      scalar * m[1],
      scalar * m[2],
      scalar * m[3]
    );
  }

  template<typename T>
  constexpr Vec4<T> operator*(const Mat4<T>& m, const Vec4<T>& v) {
    return Vec4<T>(
      m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3] * v.w,
      m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3] * v.w,
      m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3] * v.w,
      m[3][0] * v.x + m[3][1] * v.y + m[3][2] * v.z + m[3][3] * v.w
    );
  }

  template<typename T>
  constexpr Mat4<T> operator*(const Mat4<T>& m1, const Mat4<T>& m2) {
    return Mat4<T>(
      m1[0][0] * m2[0][0] + m1[0][1] * m2[1][0] + m1[0][2] * m2[2][0] + m1[0][3] * m2[3][0],
      m1[0][0] * m2[0][1] + m1[0][1] * m2[1][1] + m1[0][2] * m2[2][1] + m1[0][3] * m2[3][1],
      m1[0][0] * m2[0][2] + m1[0][1] * m2[1][2] + m1[0][2] * m2[2][2] + m1[0][3] * m2[3][2],
      m1[0][0] * m2[0][3] + m1[0][1] * m2[1][3] + m1[0][2] * m2[2][3] + m1[0][3] * m2[3][3],
      m1[1][0] * m2[0][0] + m1[1][1] * m2[1][0] + m1[1][2] * m2[2][0] + m1[1][3] * m2[3][0],
      m1[1][0] * m2[0][1] + m1[1][1] * m2[1][1] + m1[1][2] * m2[2][1] + m1[1][3] * m2[3][1],
      m1[1][0] * m2[0][2] + m1[1][1] * m2[1][2] + m1[1][2] * m2[2][2] + m1[1][3] * m2[3][2],
      m1[1][0] * m2[0][3] + m1[1][1] * m2[1][3] + m1[1][2] * m2[2][3] + m1[1][3] * m2[3][3],
      m1[2][0] * m2[0][0] + m1[2][1] * m2[1][0] + m1[2][2] * m2[2][0] + m1[2][3] * m2[3][0],
      m1[2][0] * m2[0][1] + m1[2][1] * m2[1][1] + m1[2][2] * m2[2][1] + m1[2][3] * m2[3][1],
      m1[2][0] * m2[0][2] + m1[2][1] * m2[1][2] + m1[2][2] * m2[2][2] + m1[2][3] * m2[3][2],
      m1[2][0] * m2[0][3] + m1[2][1] * m2[1][3] + m1[2][2] * m2[2][3] + m1[2][3] * m2[3][3],
      m1[3][0] * m2[0][0] + m1[3][1] * m2[1][0] + m1[3][2] * m2[2][0] + m1[3][3] * m2[3][0],
      m1[3][0] * m2[0][1] + m1[3][1] * m2[1][1] + m1[3][2] * m2[2][1] + m1[3][3] * m2[3][1],
      m1[3][0] * m2[0][2] + m1[3][1] * m2[1][2] + m1[3][2] * m2[2][2] + m1[3][3] * m2[3][2],
      m1[3][0] * m2[0][3] + m1[3][1] * m2[1][3] + m1[3][2] * m2[2][3] + m1[3][3] * m2[3][3]
    );
  }

  template<typename T, typename U>
  constexpr Mat4<T> operator/(const Mat4<T>& m, U scalar) {
    return Mat4<T>(
      m[0] / scalar,
      m[1] / scalar,
      m[2] / scalar,
      m[3] / scalar
    );
  }

  template<typename T, typename U>
  constexpr Mat4<T> operator/(U scalar, const Mat4<T>& m) {
    return Mat4<T>(
      scalar / m[0],
      scalar / m[1],
      scalar / m[2],
      scalar / m[3]
    );
  }

  template<typename T>
  constexpr Mat4<T> operator/(const Mat4<T>& m1, const Mat4<T>& m2) {
    Mat4<T> m1_copy(m1);
    return m1_copy /= m2;
  }

  /* -- Boolean operators -- */

  template<typename T>
  constexpr bool operator==(const Mat4<T>& m1, const Mat4<T>& m2) {
    return m1[0] == m2[0] && m1[1] == m2[1] && m1[2] == m2[2] && m1[3] == m2[3];
  }

  template<typename T>
  constexpr bool operator!=(const Mat4<T>& m1, const Mat4<T>& m2) {
    return !(m1 == m2);
  }

  /* -- Address operator -- */

  template<typename T>
  constexpr const T* operator&(const Mat4<T>& m) {
    return &(m[0][0]);
  }

}

namespace Graphick {

  using mat4 = Math::Mat4<float>;
  using dmat4 = Math::Mat4<double>;
  using imat4 = Math::Mat4<int32_t>;
  using umat4 = Math::Mat4<uint32_t>;

}
