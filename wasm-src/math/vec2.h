/**
 * @file vec2.h
 * @brief This file contains the Vec2 struct, a templated 2D vector.
 */

#pragma once

#include <cstdint>
#include <limits>

namespace Graphick::Math {

  /**
   * @brief A 2D vector struct with x and y components.
   *
   * @struct Vec2
   */
  template<typename T>
  struct Vec2 {
    union { T x, r, s; };    /* The 0 component of the vector. */
    union { T y, g, t; };    /* The 1 component of the vector. */

    /* -- Component accesses -- */

    static constexpr uint8_t length() {
      return 2;
    }

    constexpr T& operator[](uint8_t i) {
      switch (i) {
      default:
      case 0:
        return x;
      case 1:
        return y;
      }
    }

    constexpr T const& operator[](uint8_t i) const {
      switch (i) {
      default:
      case 0:
        return x;
      case 1:
        return y;
      }
    }

    /* -- Constructors -- */

    Vec2() = default;

    constexpr Vec2(const Vec2<T>& v) = default;

    constexpr explicit Vec2(T scalar) :
      x(scalar),
      y(scalar) {}

    constexpr Vec2(T x, T y) :
      x(x),
      y(y) {}

    template<typename U>
    constexpr explicit Vec2(const Vec2<U>& v) :
      x(static_cast<T>(v.x)),
      y(static_cast<T>(v.y)) {}

    /* -- Assign operator -- */

    constexpr Vec2<T>& operator=(const Vec2<T> v) {
      this->x = v.x;
      this->y = v.y;
      return *this;
    }

    /* -- Unary arithmetic operators -- */

    template <typename U>
    constexpr Vec2<T>& operator+=(U scalar) {
      this->x += static_cast<T>(scalar);
      this->y += static_cast<T>(scalar);
      return *this;
    }

    constexpr Vec2<T>& operator+=(const Vec2<T> v) {
      this->x += v.x;
      this->y += v.y;
      return *this;
    }

    template <typename U>
    constexpr Vec2<T>& operator-=(U scalar) {
      this->x -= static_cast<T>(scalar);
      this->y -= static_cast<T>(scalar);
      return *this;
    }

    constexpr Vec2<T>& operator-=(const Vec2<T> v) {
      this->x -= v.x;
      this->y -= v.y;
      return *this;
    }

    template <typename U>
    constexpr Vec2<T>& operator*=(U scalar) {
      this->x *= static_cast<T>(scalar);
      this->y *= static_cast<T>(scalar);
      return *this;
    }

    constexpr Vec2<T>& operator*=(const Vec2<T> v) {
      this->x *= v.x;
      this->y *= v.y;
      return *this;
    }

    template <typename U>
    constexpr Vec2<T>& operator/=(U scalar) {
      this->x /= static_cast<T>(scalar);
      this->y /= static_cast<T>(scalar);
      return *this;
    }

    constexpr Vec2<T>& operator/=(const Vec2<T> v) {
      this->x /= v.x;
      this->y /= v.y;
      return *this;
    }

    /* -- Increment/Decrement operators -- */

    constexpr Vec2<T>& operator++() {
      ++this->x;
      ++this->y;
      return *this;
    }

    constexpr Vec2<T>& operator--() {
      --this->x;
      --this->y;
      return *this;
    }
  };

  /* -- Unary operators */

  template<typename T>
  constexpr Vec2<T> operator+(const Vec2<T> v) {
    return v;
  }

  template<typename T>
  constexpr Vec2<T> operator-(const Vec2<T> v) {
    return Vec2<T>(
      -v.x,
      -v.y
    );
  }

  /* -- Binary operators -- */

  template<typename T, typename U>
  constexpr Vec2<T> operator+(const Vec2<T> v, U scalar) {
    return Vec2<T>(
      v.x + static_cast<T>(scalar),
      v.y + static_cast<T>(scalar)
    );
  }

  template<typename T, typename U>
  constexpr Vec2<T> operator+(U scalar, const Vec2<T> v) {
    return Vec2<T>(
      static_cast<T>(scalar) + v.x,
      static_cast<T>(scalar) + v.y
    );
  }

  template<typename T>
  constexpr Vec2<T> operator+(const Vec2<T> v1, const Vec2<T> v2) {
    return Vec2<T>(
      v1.x + v2.x,
      v1.y + v2.y
    );
  }

  template<typename T, typename U>
  constexpr Vec2<T> operator-(const Vec2<T> v, U scalar) {
    return Vec2<T>(
      v.x - static_cast<T>(scalar),
      v.y - static_cast<T>(scalar)
    );
  }

  template<typename T, typename U>
  constexpr Vec2<T> operator-(U scalar, const Vec2<T> v) {
    return Vec2<T>(
      static_cast<T>(scalar) - v.x,
      static_cast<T>(scalar) - v.y
    );
  }

  template<typename T>
  constexpr Vec2<T> operator-(const Vec2<T> v1, const Vec2<T> v2) {
    return Vec2<T>(
      v1.x - v2.x,
      v1.y - v2.y
    );
  }

  template<typename T, typename U>
  constexpr Vec2<T> operator*(const Vec2<T> v, U scalar) {
    return Vec2<T>(
      v.x * static_cast<T>(scalar),
      v.y * static_cast<T>(scalar)
    );
  }

  template<typename T, typename U>
  constexpr Vec2<T> operator*(U scalar, const Vec2<T> v) {
    return Vec2<T>(
      static_cast<T>(scalar) * v.x,
      static_cast<T>(scalar) * v.y
    );
  }

  template<typename T>
  constexpr Vec2<T> operator*(const Vec2<T> v1, const Vec2<T> v2) {
    return Vec2<T>(
      v1.x * v2.x,
      v1.y * v2.y
    );
  }

  template<typename T, typename U>
  constexpr Vec2<T> operator/(const Vec2<T> v, U scalar) {
    return Vec2<T>(
      v.x / static_cast<T>(scalar),
      v.y / static_cast<T>(scalar)
    );
  }

  template<typename T, typename U>
  constexpr Vec2<T> operator/(U scalar, const Vec2<T> v) {
    return Vec2<T>(
      static_cast<T>(scalar) / v.x,
      static_cast<T>(scalar) / v.y
    );
  }

  template<typename T>
  constexpr Vec2<T> operator/(const Vec2<T> v1, const Vec2<T> v2) {
    return Vec2<T>(
      v1.x / v2.x,
      v1.y / v2.y
    );
  }

  template<typename T, typename U>
  constexpr Vec2<T> operator%(const Vec2<T> v, U scalar) {
    return Vec2<T>(
      static_cast<T>(static_cast<int>(v.x) % static_cast<int>(scalar)),
      static_cast<T>(static_cast<int>(v.y) % static_cast<int>(scalar))
    );
  }

  template<typename T, typename U>
  constexpr Vec2<T> operator%(U scalar, const Vec2<T> v) {
    return Vec2<T>(
      static_cast<T>(static_cast<int>(scalar) % static_cast<int>(v.x)),
      static_cast<T>(static_cast<int>(scalar) % static_cast<int>(v.y))
    );
  }

  template<typename T>
  constexpr Vec2<T> operator%(const Vec2<T> v1, const Vec2<T> v2) {
    return Vec2<T>(
      static_cast<T>(static_cast<int>(v1.x) % static_cast<int>(v2.x)),
      static_cast<T>(static_cast<int>(v1.y) % static_cast<int>(v2.y))
    );
  }

  /* -- Boolean operators -- */

  template<typename T>
  constexpr bool operator==(const Vec2<T> v1, const Vec2<T> v2) {
    return v1.x == v2.x && v1.y == v2.y;
  }

  template<typename T>
  constexpr bool operator!=(const Vec2<T> v1, const Vec2<T> v2) {
    return !(v1 == v2);
  }

  template<typename T>
  constexpr Vec2<T> operator&&(const Vec2<T> v1, const Vec2<T> v2) {
    return Vec2<T>(v1.x && v2.x, v1.y && v2.y);
  }

  template<typename T>
  constexpr Vec2<T> operator||(const Vec2<T> v1, const Vec2<T> v2) {
    return Vec2<T>(v1.x || v2.x, v1.y || v2.y);
  }

  /* -- Address operator -- */

  template<typename T>
  constexpr const T* operator&(const Vec2<T>& v) {
    return &(v.x);
  }

}

namespace std {

  /* -- numeric_limits -- */

  template<typename T>
  class numeric_limits<Graphick::Math::Vec2<T>> {
  public:
    static inline Graphick::Math::Vec2<T> min() {
      return Graphick::Math::Vec2{ numeric_limits<T>::min() };
    }

    static inline Graphick::Math::Vec2<T> max() {
      return Graphick::Math::Vec2{ numeric_limits<T>::max() };
    }

    static inline Graphick::Math::Vec2<T> lowest() {
      return Graphick::Math::Vec2{ numeric_limits<T>::lowest() };
    }
  };

}

namespace Graphick::Math {

  using vec2 = Math::Vec2<float>;
  using dvec2 = Math::Vec2<double>;
  using ivec2 = Math::Vec2<int32_t>;
  using uvec2 = Math::Vec2<uint32_t>;

}

namespace Graphick {

  using vec2 = Math::vec2;
  using dvec2 = Math::dvec2;
  using ivec2 = Math::ivec2;
  using uvec2 = Math::uvec2;

}
