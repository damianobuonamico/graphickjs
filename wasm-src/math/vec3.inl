#include <assert.h>

/* -- Component accesses -- */

constexpr float& vec3::operator[](uint8_t i) {
  assert(i >= 0 && i < this->length());
  switch (i) {
  default:
  case 0:
    return x;
  case 1:
    return y;
  case 2:
    return z;
  }
}

constexpr const float& vec3::operator[](uint8_t i) const {
  assert(i >= 0 && i < this->length());
  switch (i) {
  default:
  case 0:
    return x;
  case 1:
    return y;
  case 2:
    return z;
  }
}

/* -- Constructors -- */

constexpr vec3::vec3(float scalar): x(scalar), y(scalar), z(scalar) {}

constexpr vec3::vec3(float x, float y, float z) : x(x), y(y), z(z) {}

/* -- Assign operator -- */

constexpr vec3& vec3::operator=(const vec3& v) {
  this->x = v.x;
  this->y = v.y;
  this->z = v.z;
  return *this;
}

/* -- Unary arithmetic operators -- */

constexpr vec3& vec3::operator+=(float scalar) {
  this->x += scalar;
  this->y += scalar;
  this->z += scalar;
  return *this;
}

constexpr vec3& vec3::operator+=(const vec3& v) {
  this->x += v.x;
  this->y += v.y;
  this->z += v.z;
  return *this;
}

constexpr vec3& vec3::operator-=(float scalar) {
  this->x -= scalar;
  this->y -= scalar;
  this->z -= scalar;
  return *this;
}

constexpr vec3& vec3::operator-=(const vec3& v) {
  this->x -= v.x;
  this->y -= v.y;
  this->z -= v.z;
  return *this;
}

constexpr vec3& vec3::operator*=(float scalar) {
  this->x *= scalar;
  this->y *= scalar;
  this->z *= scalar;
  return *this;
}

constexpr vec3& vec3::operator*=(const vec3& v) {
  this->x *= v.x;
  this->y *= v.y;
  this->z *= v.z;
  return *this;
}

constexpr vec3& vec3::operator/=(float scalar) {
  this->x /= scalar;
  this->y /= scalar;
  this->z /= scalar;
  return *this;
}

constexpr vec3& vec3::operator/=(const vec3& v) {
  this->x /= v.x;
  this->y /= v.y;
  this->z /= v.z;
  return *this;
}


/* -- Increment/Decrement operators -- */

constexpr vec3& vec3::operator++() {
  ++this->x;
  ++this->y;
  ++this->z;
  return *this;
}

constexpr vec3& vec3::operator--() {
  --this->x;
  --this->y;
  --this->z;
  return *this;
}

/* -- Unary operators */

constexpr vec3 operator+(const vec3& v) {
  return v;
}

constexpr vec3 operator-(const vec3& v) {
  return vec3(-v.x, -v.y, -v.z);
}

/* -- Binary operators -- */

constexpr vec3 operator+(const vec3& v, float scalar) {
  return vec3(v.x + scalar, v.y + scalar, v.z + scalar);
}

constexpr vec3 operator+(float scalar, const vec3& v) {
  return vec3(scalar + v.x, scalar + v.y, scalar + v.z);
}

constexpr vec3 operator+(const vec3& v1, const vec3& v2) {
  return vec3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}

constexpr vec3 operator-(const vec3& v, float scalar) {
  return vec3(v.x - scalar, v.y - scalar, v.z - scalar);
}

constexpr vec3 operator-(float scalar, const vec3& v) {
  return vec3(scalar - v.x, scalar - v.y, scalar - v.z);
}

constexpr vec3 operator-(const vec3& v1, const vec3& v2) {
  return vec3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}

constexpr vec3 operator*(const vec3& v, float scalar) {
  return vec3(v.x * scalar, v.y * scalar, v.z * scalar);
}

constexpr vec3 operator*(float scalar, const vec3& v) {
  return vec3(scalar * v.x, scalar * v.y, scalar * v.z);
}

constexpr vec3 operator*(const vec3& v1, const vec3& v2) {
  return vec3(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z);
}

constexpr vec3 operator/(const vec3& v, float scalar) {
  return vec3(v.x / scalar, v.y / scalar, v.z / scalar);
}

constexpr vec3 operator/(float scalar, const vec3& v) {
  return vec3(scalar / v.x, scalar / v.y, scalar / v.z);
}

constexpr vec3 operator/(const vec3& v1, const vec3& v2) {
  return vec3(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z);
}

constexpr vec3 operator%(const vec3& v, float scalar) {
  return vec3((int)v.x % (int)scalar, (int)v.y % (int)scalar, (int)v.z % (int)scalar);
}

constexpr vec3 operator%(float scalar, const vec3& v) {
  return vec3((int)scalar % (int)v.x, (int)scalar % (int)v.y, (int)scalar % (int)v.z);
}

constexpr vec3 operator%(const vec3& v1, const vec3& v2) {
  return vec3((int)v1.x % (int)v2.x, (int)v1.y % (int)v2.y, (int)v1.z % (int)v2.z);
}

/* -- Boolean operators -- */

constexpr bool operator==(const vec3& v1, const vec3& v2) {
  return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z;
}

constexpr bool operator!=(const vec3& v1, const vec3& v2) {
  return !(v1 == v2);
}

constexpr vec3 operator&&(const vec3& v1, const vec3& v2) {
  return vec3(v1.x && v2.x, v1.y && v2.y, v1.z && v2.z);
}

constexpr vec3 operator||(const vec3& v1, const vec3& v2) {
  return vec3(v1.x || v2.x, v1.y || v2.y, v1.z || v2.z);
}

/* -- Address operator -- */

constexpr const float* operator&(const vec3& v) { return &(v.x); }

/* -- Stream operator -- */

inline std::ostream& operator<<(std::ostream& os, const vec3& v) {
  os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
  return os;
}
