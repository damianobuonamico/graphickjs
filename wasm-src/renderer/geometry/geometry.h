#pragma once

#include "vertex.h"

#include <vector>

// TODO: refactor, dynamically remember current offset
struct Geometry {
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
};
