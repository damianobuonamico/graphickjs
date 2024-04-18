/**
 * @file math/vec4.h
 * @brief This file contains the Vec4 struct, a templated 4D vector.
 */

#pragma once

#include <cstdint>
#include <limits>

namespace graphick::math {

  /**
   * @brief A 4D vector struct with x, y, z and w components.
   *
   * @struct Vec4
   */
  template <typename T>
  struct Vec4 {
    union { T x, r, s; };    /* The 0 component of the vector. */
    union { T y, g, t; };    /* The 1 component of the vector. */
    union { T z, b, p; };    /* The 2 component of the vector. */
    union { T w, a, q; };    /* The 3 component of the vector. */

    /* -- Component accesses -- */

    static constexpr uint8_t length() {
      return 4;
    }

    constexpr T& operator[](uint8_t i) {
      switch (i) {
      default:
      case 0:
        return x;
      case 1:
        return y;
      case 2:
        return z;
      case 3:
        return w;
      }
    }

    constexpr T const& operator[](uint8_t i) const {
      switch (i) {
      default:
      case 0:
        return x;
      case 1:
        return y;
      case 2:
        return z;
      case 3:
        return w;
      }
    }

    /* -- Constructors -- */

    Vec4() = default;

    constexpr Vec4(const Vec4<T>& v) :
      x(v.x),
      y(v.y),
      z(v.z),
      w(v.w) {}

    constexpr explicit Vec4(T scalar) :
      x(scalar),
      y(scalar),
      z(scalar),
      w(scalar) {}

    constexpr Vec4(T x, T y, T z, T w) :
      x(x),
      y(y),
      z(z),
      w(w) {}

    template <typename U>
    constexpr explicit Vec4(const Vec4<U>& v) :
      x(static_cast<T>(v.x)),
      y(static_cast<T>(v.y)),
      z(static_cast<T>(v.z)),
      w(static_cast<T>(v.w)) {}

    /* -- Static constructors -- */

    static constexpr Vec4<T> zero() {
      return Vec4<T>(T(0));
    }

    static constexpr Vec4<T> identity() {
      return Vec4<T>(T(1));
    }

    /* -- Assign operator -- */

    constexpr Vec4<T>& operator=(const Vec4<T>& v) {
      this->x = v.x;
      this->y = v.y;
      this->z = v.z;
      this->w = v.w;
      return *this;
    }

    /* -- Unary arithmetic operators -- */

    template <typename U>
    constexpr Vec4<T>& operator+=(U scalar) {
      this->x += static_cast<T>(scalar);
      this->y += static_cast<T>(scalar);
      this->z += static_cast<T>(scalar);
      this->w += static_cast<T>(scalar);
      return *this;
    }

    constexpr Vec4<T>& operator+=(const Vec4<T>& v) {
      this->x += v.x;
      this->y += v.y;
      this->z += v.z;
      this->w += v.w;
      return *this;
    }

    template <typename U>
    constexpr Vec4<T>& operator-=(U scalar) {
      this->x -= static_cast<T>(scalar);
      this->y -= static_cast<T>(scalar);
      this->z -= static_cast<T>(scalar);
      this->w -= static_cast<T>(scalar);
      return *this;
    }

    constexpr Vec4<T>& operator-=(const Vec4<T>& v) {
      this->x -= v.x;
      this->y -= v.y;
      this->z -= v.z;
      this->w -= v.w;
      return *this;
    }

    template <typename U>
    constexpr Vec4<T>& operator*=(U scalar) {
      this->x *= static_cast<T>(scalar);
      this->y *= static_cast<T>(scalar);
      this->z *= static_cast<T>(scalar);
      this->w *= static_cast<T>(scalar);
      return *this;
    }

    constexpr Vec4<T>& operator*=(const Vec4<T>& v) {
      this->x *= v.x;
      this->y *= v.y;
      this->z *= v.z;
      this->w *= v.w;
      return *this;
    }

    template <typename U>
    constexpr Vec4<T>& operator/=(U scalar) {
      this->x /= static_cast<T>(scalar);
      this->y /= static_cast<T>(scalar);
      this->z /= static_cast<T>(scalar);
      this->w /= static_cast<T>(scalar);
      return *this;
    }

    constexpr Vec4<T>& operator/=(const Vec4<T>& v) {
      this->x /= v.x;
      this->y /= v.y;
      this->z /= v.z;
      this->w /= v.w;
      return *this;
    }

    /* -- Increment/Decrement operators -- */

    constexpr Vec4<T>& operator++() {
      ++this->x;
      ++this->y;
      ++this->z;
      ++this->w;
      return *this;
    }

    constexpr Vec4<T>& operator--() {
      --this->x;
      --this->y;
      --this->z;
      --this->w;
      return *this;
    }
  };

  /* -- Unary operators */

  template <typename T>
  constexpr Vec4<T> operator+(const Vec4<T>& v) {
    return v;
  }

  template <typename T>
  constexpr Vec4<T> operator-(const Vec4<T>& v) {
    return Vec4<T>(-v.x, -v.y, -v.z, -v.w);
  }

  /* -- Binary operators -- */

  template <typename T, typename U>
  constexpr Vec4<T> operator+(const Vec4<T>& v, U scalar) {
    return Vec4<T>(
      v.x + static_cast<T>(scalar),
      v.y + static_cast<T>(scalar),
      v.z + static_cast<T>(scalar),
      v.w + static_cast<T>(scalar)
    );
  }

  template <typename T, typename U>
  constexpr Vec4<T> operator+(U scalar, const Vec4<T>& v) {
    return Vec4<T>(
      static_cast<T>(scalar) + v.x,
      static_cast<T>(scalar) + v.y,
      static_cast<T>(scalar) + v.z,
      static_cast<T>(scalar) + v.w
    );
  }

  template <typename T>
  constexpr Vec4<T> operator+(const Vec4<T>& v1, const Vec4<T>& v2) {
    return Vec4<T>(
      v1.x + v2.x,
      v1.y + v2.y,
      v1.z + v2.z,
      v1.w + v2.w
    );
  }

  template <typename T, typename U>
  constexpr Vec4<T> operator-(const Vec4<T>& v, U scalar) {
    return Vec4<T>(
      v.x - static_cast<T>(scalar),
      v.y - static_cast<T>(scalar),
      v.z - static_cast<T>(scalar),
      v.w - static_cast<T>(scalar)
    );
  }

  template <typename T, typename U>
  constexpr Vec4<T> operator-(U scalar, const Vec4<T>& v) {
    return Vec4<T>(
      static_cast<T>(scalar) - v.x,
      static_cast<T>(scalar) - v.y,
      static_cast<T>(scalar) - v.z,
      static_cast<T>(scalar) - v.w
    );
  }

  template <typename T>
  constexpr Vec4<T> operator-(const Vec4<T>& v1, const Vec4<T>& v2) {
    return Vec4<T>(
      v1.x - v2.x,
      v1.y - v2.y,
      v1.z - v2.z,
      v1.w - v2.w
    );
  }

  template <typename T, typename U>
  constexpr Vec4<T> operator*(const Vec4<T>& v, U scalar) {
    return Vec4<T>(
      v.x * static_cast<T>(scalar),
      v.y * static_cast<T>(scalar),
      v.z * static_cast<T>(scalar),
      v.w * static_cast<T>(scalar)
    );
  }

  template <typename T, typename U>
  constexpr Vec4<T> operator*(U scalar, const Vec4<T>& v) {
    return Vec4<T>(
      static_cast<T>(scalar) * v.x,
      static_cast<T>(scalar) * v.y,
      static_cast<T>(scalar) * v.z,
      static_cast<T>(scalar) * v.w
    );
  }

  template <typename T>
  constexpr Vec4<T> operator*(const Vec4<T>& v1, const Vec4<T>& v2) {
    return Vec4<T>(
      v1.x * v2.x,
      v1.y * v2.y,
      v1.z * v2.z,
      v1.w * v2.w
    );
  }

  template <typename T, typename U>
  constexpr Vec4<T> operator/(const Vec4<T>& v, U scalar) {
    return Vec4<T>(
      v.x / static_cast<T>(scalar),
      v.y / static_cast<T>(scalar),
      v.z / static_cast<T>(scalar),
      v.w / static_cast<T>(scalar)
    );
  }

  template <typename T, typename U>
  constexpr Vec4<T> operator/(U scalar, const Vec4<T>& v) {
    return Vec4<T>(
      static_cast<T>(scalar) / v.x,
      static_cast<T>(scalar) / v.y,
      static_cast<T>(scalar) / v.z,
      static_cast<T>(scalar) / v.w
    );
  }

  template <typename T>
  constexpr Vec4<T> operator/(const Vec4<T>& v1, const Vec4<T>& v2) {
    return Vec4<T>(
      v1.x / v2.x,
      v1.y / v2.y,
      v1.z / v2.z,
      v1.w / v2.w
    );
  }

  template <typename T, typename U>
  constexpr Vec4<T> operator%(const Vec4<T>& v, U scalar) {
    return Vec4<T>(
      static_cast<T>(static_cast<int>(v.x) % static_cast<int>(scalar)),
      static_cast<T>(static_cast<int>(v.y) % static_cast<int>(scalar)),
      static_cast<T>(static_cast<int>(v.z) % static_cast<int>(scalar)),
      static_cast<T>(static_cast<int>(v.w) % static_cast<int>(scalar))
    );
  }

  template <typename T, typename U>
  constexpr Vec4<T> operator%(U scalar, const Vec4<T>& v) {
    return Vec4<T>(
      static_cast<T>(static_cast<int>(scalar) % static_cast<int>(v.x)),
      static_cast<T>(static_cast<int>(scalar) % static_cast<int>(v.y)),
      static_cast<T>(static_cast<int>(scalar) % static_cast<int>(v.z)),
      static_cast<T>(static_cast<int>(scalar) % static_cast<int>(v.w))
    );
  }

  template <typename T>
  constexpr Vec4<T> operator%(const Vec4<T>& v1, const Vec4<T>& v2) {
    return Vec4<T>(
      static_cast<T>(static_cast<int>(v1.x) % static_cast<int>(v2.x)),
      static_cast<T>(static_cast<int>(v1.y) % static_cast<int>(v2.y)),
      static_cast<T>(static_cast<int>(v1.z) % static_cast<int>(v2.z)),
      static_cast<T>(static_cast<int>(v1.w) % static_cast<int>(v2.w))
    );
  }

  /* -- Boolean operators -- */

  template <typename T>
  constexpr bool operator==(const Vec4<T>& v1, const Vec4<T>& v2) {
    return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z && v1.w == v2.w;
  }

  template <typename T>
  constexpr bool operator!=(const Vec4<T>& v1, const Vec4<T>& v2) {
    return !(v1 == v2);
  }

  template <typename T>
  constexpr Vec4<T> operator&&(const Vec4<T>& v1, const Vec4<T>& v2) {
    return Vec4<T>(v1.x && v2.x, v1.y && v2.y, v1.z && v2.z, v1.w && v2.w);
  }

  template <typename T>
  constexpr Vec4<T> operator||(const Vec4<T>& v1, const Vec4<T>& v2) {
    return Vec4<T>(v1.x || v2.x, v1.y || v2.y, v1.z || v2.z, v1.w || v2.w);
  }
}

/* -- Aliases -- */

namespace graphick::math {

  using vec4 = math::Vec4<float>;
  using dvec4 = math::Vec4<double>;
  using ivec4 = math::Vec4<int32_t>;
  using uvec4 = math::Vec4<uint8_t>;

}

namespace graphick {

  using math::vec4;
  using math::dvec4;
  using math::ivec4;
  using math::uvec4;

}
