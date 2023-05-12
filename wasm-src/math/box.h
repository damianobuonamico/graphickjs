#pragma once

#include "vec2.h"

struct Box {
  vec2 min;
  vec2 max;

  inline vec2 size() const { return max - min; };
};
