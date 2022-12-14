#pragma once

#include "../math/vec2.h"

struct Vertex {
  Vertex() = default;
  Vertex(const vec2& position)
    : position(position) {};

  vec2 position;
};
