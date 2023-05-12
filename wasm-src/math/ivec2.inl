#include <assert.h>

/* -- Component accesses -- */

constexpr int& ivec2::operator[](uint8_t i) {
  assert(i >= 0 && i < this->length());
  switch (i) {
  default:
  case 0:
    return x;
  case 1:
    return y;
  }
}

constexpr const int& ivec2::operator[](uint8_t i) const {
  assert(i >= 0 && i < this->length());
  switch (i) {
  default:
  case 0:
    return x;
  case 1:
    return y;
  }
}

/* -- Constructors -- */

constexpr ivec2::ivec2(int scalar) : x(scalar), y(scalar) {}

constexpr ivec2::ivec2(int x, int y) : x(x), y(y) {}

/* -- Assign operator -- */

constexpr ivec2& ivec2::operator=(const ivec2& v) {
  this->x = v.x;
  this->y = v.y;
  return *this;
}

/* -- Unary arithmetic operators -- */

constexpr ivec2& ivec2::operator+=(int scalar) {
  this->x += scalar;
  this->y += scalar;
  return *this;
}

constexpr ivec2& ivec2::operator+=(const ivec2& v) {
  this->x += v.x;
  this->y += v.y;
  return *this;
}

constexpr ivec2& ivec2::operator-=(int scalar) {
  this->x -= scalar;
  this->y -= scalar;
  return *this;
}

constexpr ivec2& ivec2::operator-=(const ivec2& v) {
  this->x -= v.x;
  this->y -= v.y;
  return *this;
}

constexpr ivec2& ivec2::operator*=(int scalar) {
  this->x *= scalar;
  this->y *= scalar;
  return *this;
}

constexpr ivec2& ivec2::operator*=(const ivec2& v) {
  this->x *= v.x;
  this->y *= v.y;
  return *this;
}

constexpr ivec2& ivec2::operator/=(int scalar) {
  this->x /= scalar;
  this->y /= scalar;
  return *this;
}

constexpr ivec2& ivec2::operator/=(const ivec2& v) {
  this->x /= v.x;
  this->y /= v.y;
  return *this;
}


/* -- Increment/Decrement operators -- */

constexpr ivec2& ivec2::operator++() {
  ++this->x;
  ++this->y;
  return *this;
}

constexpr ivec2& ivec2::operator--() {
  --this->x;
  --this->y;
  return *this;
}

/* -- Unary operators */

constexpr ivec2 operator+(const ivec2& v) {
  return v;
}

constexpr ivec2 operator-(const ivec2& v) {
  return ivec2(-v.x, -v.y);
}

/* -- Binary operators -- */

constexpr ivec2 operator+(const ivec2& v, int scalar) {
  return ivec2(v.x + scalar, v.y + scalar);
}

constexpr ivec2 operator+(int scalar, const ivec2& v) {
  return ivec2(scalar + v.x, scalar + v.y);
}

constexpr ivec2 operator+(const ivec2& v1, const ivec2& v2) {
  return ivec2(v1.x + v2.x, v1.y + v2.y);
}

constexpr ivec2 operator-(const ivec2& v, int scalar) {
  return ivec2(v.x - scalar, v.y - scalar);
}

constexpr ivec2 operator-(int scalar, const ivec2& v) {
  return ivec2(scalar - v.x, scalar - v.y);
}

constexpr ivec2 operator-(const ivec2& v1, const ivec2& v2) {
  return ivec2(v1.x - v2.x, v1.y - v2.y);
}

constexpr ivec2 operator*(const ivec2& v, int scalar) {
  return ivec2(v.x * scalar, v.y * scalar);
}

constexpr ivec2 operator*(int scalar, const ivec2& v) {
  return ivec2(scalar * v.x, scalar * v.y);
}

constexpr ivec2 operator*(const ivec2& v1, const ivec2& v2) {
  return ivec2(v1.x * v2.x, v1.y * v2.y);
}

constexpr ivec2 operator/(const ivec2& v, int scalar) {
  return ivec2(v.x / scalar, v.y / scalar);
}

constexpr ivec2 operator/(int scalar, const ivec2& v) {
  return ivec2(scalar / v.x, scalar / v.y);
}

constexpr ivec2 operator/(const ivec2& v1, const ivec2& v2) {
  return ivec2(v1.x / v2.x, v1.y / v2.y);
}

constexpr ivec2 operator%(const ivec2& v, int scalar) {
  return ivec2(v.x % scalar, v.y % scalar);
}

constexpr ivec2 operator%(int scalar, const ivec2& v) {
  return ivec2(scalar % v.x, scalar % v.y);
}

constexpr ivec2 operator%(const ivec2& v1, const ivec2& v2) {
  return ivec2(v1.x % v2.x, v1.y % v2.y);
}

/* -- Boolean operators -- */

constexpr bool operator==(const ivec2& v1, const ivec2& v2) {
  return v1.x == v2.x && v1.y == v2.y;
}

constexpr bool operator!=(const ivec2& v1, const ivec2& v2) {
  return !(v1 == v2);
}

constexpr ivec2 operator&&(const ivec2& v1, const ivec2& v2) {
  return ivec2(v1.x && v2.x, v1.y && v2.y);
}

constexpr ivec2 operator||(const ivec2& v1, const ivec2& v2) {
  return ivec2(v1.x || v2.x, v1.y || v2.y);
}

/* -- Address operator -- */

constexpr const int* operator&(const ivec2& v) { return &(v.x); }

/* -- Std -- */

namespace std {

  inline ostream& operator<<(ostream& os, const ivec2& v) {
    os << "(" << v.x << ", " << v.y << ")";
    return os;
  }

  template <> class numeric_limits<ivec2> {
  public:
    static inline ivec2 min() { return ivec2{ numeric_limits<int>::lowest() }; }
    static inline ivec2 max() { return ivec2{ numeric_limits<int>::max() }; }
  };

}

