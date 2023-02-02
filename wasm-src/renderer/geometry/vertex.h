#pragma once

#include "../../math/vec2.h"
#include "../../math/vec4.h"

struct Vertex {
  Vertex() = default;
  Vertex(const vec2& position)
    : position(position), color({ 0.5f, 0.5f, 0.5f, 1.0f }) {};
  Vertex(const vec2& position, const vec4& color)
    : position(position), color(color) {};

  vec2 position;
  vec4 color;
};
