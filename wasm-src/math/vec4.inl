#include <assert.h>

/* -- Component accesses -- */

constexpr float& vec4::operator[](uint8_t i) {
  assert(i >= 0 && i < this->length());
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

constexpr const float& vec4::operator[](uint8_t i) const {
  assert(i >= 0 && i < this->length());
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

constexpr vec4::vec4(float scalar): x(scalar), y(scalar), z(scalar), w(scalar) {}

constexpr vec4::vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

/* -- Assign operator -- */

constexpr vec4& vec4::operator=(const vec4& v) {
  this->x = v.x;
  this->y = v.y;
  this->z = v.z;
  this->w = v.w;
  return *this;
}

/* -- Unary arithmetic operators -- */

constexpr vec4& vec4::operator+=(float scalar) {
  this->x += scalar;
  this->y += scalar;
  this->z += scalar;
  this->w += scalar;
  return *this;
}

constexpr vec4& vec4::operator+=(const vec4& v) {
  this->x += v.x;
  this->y += v.y;
  this->z += v.z;
  this->w += v.w;
  return *this;
}

constexpr vec4& vec4::operator-=(float scalar) {
  this->x -= scalar;
  this->y -= scalar;
  this->z -= scalar;
  this->w -= scalar;
  return *this;
}

constexpr vec4& vec4::operator-=(const vec4& v) {
  this->x -= v.x;
  this->y -= v.y;
  this->z -= v.z;
  this->w -= v.w;
  return *this;
}

constexpr vec4& vec4::operator*=(float scalar) {
  this->x *= scalar;
  this->y *= scalar;
  this->z *= scalar;
  this->w *= scalar;
  return *this;
}

constexpr vec4& vec4::operator*=(const vec4& v) {
  this->x *= v.x;
  this->y *= v.y;
  this->z *= v.z;
  this->w *= v.w;
  return *this;
}

constexpr vec4& vec4::operator/=(float scalar) {
  this->x /= scalar;
  this->y /= scalar;
  this->z /= scalar;
  this->w /= scalar;
  return *this;
}

constexpr vec4& vec4::operator/=(const vec4& v) {
  this->x /= v.x;
  this->y /= v.y;
  this->z /= v.z;
  this->w /= v.w;
  return *this;
}


/* -- Increment/Decrement operators -- */

constexpr vec4& vec4::operator++() {
  ++this->x;
  ++this->y;
  ++this->z;
  ++this->w;
  return *this;
}

constexpr vec4& vec4::operator--() {
  --this->x;
  --this->y;
  --this->z;
  --this->w;
  return *this;
}

/* -- Unary operators */

constexpr vec4 operator+(const vec4& v) {
  return v;
}

constexpr vec4 operator-(const vec4& v) {
  return vec4(-v.x, -v.y, -v.z, -v.w);
}

/* -- Binary operators -- */

constexpr vec4 operator+(const vec4& v, float scalar) {
  return vec4(v.x + scalar, v.y + scalar, v.z + scalar, v.w + scalar);
}

constexpr vec4 operator+(float scalar, const vec4& v) {
  return vec4(scalar + v.x, scalar + v.y, scalar + v.z, scalar + v.w);
}

constexpr vec4 operator+(const vec4& v1, const vec4& v2) {
  return vec4(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w);
}

constexpr vec4 operator-(const vec4& v, float scalar) {
  return vec4(v.x - scalar, v.y - scalar, v.z - scalar, v.w - scalar);
}

constexpr vec4 operator-(float scalar, const vec4& v) {
  return vec4(scalar - v.x, scalar - v.y, scalar - v.z, scalar - v.w);
}

constexpr vec4 operator-(const vec4& v1, const vec4& v2) {
  return vec4(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w);
}

constexpr vec4 operator*(const vec4& v, float scalar) {
  return vec4(v.x * scalar, v.y * scalar, v.z * scalar, v.w * scalar);
}

constexpr vec4 operator*(float scalar, const vec4& v) {
  return vec4(scalar * v.x, scalar * v.y, scalar * v.z, scalar * v.w);
}

constexpr vec4 operator*(const vec4& v1, const vec4& v2) {
  return vec4(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z, v1.w * v2.w);
}

constexpr vec4 operator/(const vec4& v, float scalar) {
  return vec4(v.x / scalar, v.y / scalar, v.z / scalar, v.w / scalar);
}

constexpr vec4 operator/(float scalar, const vec4& v) {
  return vec4(scalar / v.x, scalar / v.y, scalar / v.z, scalar / v.w);
}

constexpr vec4 operator/(const vec4& v1, const vec4& v2) {
  return vec4(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z, v1.w / v2.w);
}

constexpr vec4 operator%(const vec4& v, float scalar) {
  return vec4(std::fmod(v.x, scalar), std::fmod(v.y, scalar), std::fmod(v.z, scalar), std::fmod(v.w, scalar));
}

constexpr vec4 operator%(float scalar, const vec4& v) {
  return vec4(std::fmod(scalar, v.x), std::fmod(scalar, v.y), std::fmod(scalar, v.z), std::fmod(scalar, v.w));
}

constexpr vec4 operator%(const vec4& v1, const vec4& v2) {
  return vec4(std::fmod(v1.x, v2.x), std::fmod(v1.y, v2.y), std::fmod(v1.z, v2.z), std::fmod(v1.w, v2.w));
}

/* -- Boolean operators -- */

constexpr bool operator==(const vec4& v1, const vec4& v2) {
  return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z && v1.w == v2.w;
}

constexpr bool operator!=(const vec4& v1, const vec4& v2) {
  return !(v1 == v2);
}

constexpr vec4 operator&&(const vec4& v1, const vec4& v2) {
  return vec4(v1.x && v2.x, v1.y && v2.y, v1.z && v2.z, v1.w && v2.w);
}

constexpr vec4 operator||(const vec4& v1, const vec4& v2) {
  return vec4(v1.x || v2.x, v1.y || v2.y, v1.z || v2.z, v1.w || v2.w);
}

/* -- Address operator -- */

constexpr const float* operator&(const vec4& v) { return &(v.x); }

/* -- Stream operator -- */

inline std::ostream& operator<<(std::ostream& os, const vec4& v) {
  os << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
  return os;
}
