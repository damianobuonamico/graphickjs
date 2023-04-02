#pragma once

#include "../vec2.h"

typedef unsigned int uint;

struct PathPoint {
  vec2 position;
  float pressure;
};

struct PathBezier {
  vec2 p0;
  vec2 p1;
  vec2 p2;
  vec2 p3;

  size_t start_index;
  size_t end_index;

  vec2 pressure{ 1.0f };

  vec2& operator[](uint8_t i) {
    switch (i) {
    default:
    case 0:
      return p0;
    case 1:
      return p1;
    case 2:
      return p2;
    case 3:
      return p3;
    }
  }

  const vec2& operator[](uint8_t i) const {
    switch (i) {
    default:
    case 0:
      return p0;
    case 1:
      return p1;
    case 2:
      return p2;
    case 3:
      return p3;
    }
  }
};
