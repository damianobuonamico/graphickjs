#pragma once

#include "../../math/vec2.h"
#include "../../math/vec4.h"

struct Vertex {
  Vertex() = default;
  Vertex(const vec2& position)
    : position(position), color({ 0.5f, 0.5f, 0.5f, 1.0f }), normal(0.0f), max_normal(0.0f) {};
  Vertex(const vec2& position, const vec4& color)
    : position(position), color(color), normal(0.0f), max_normal(0.0f) {};
  Vertex(const vec2& position, const vec4& color, float normal)
    : position(position), color(color), normal(normal), max_normal(std::fabsf(normal)) {};
  Vertex(const vec2& position, const vec4& color, float normal, float max_normal)
    : position(position), color(color), normal(normal), max_normal(max_normal) {};

  vec2 position;
  vec4 color;
  float normal;
  float max_normal;
};
