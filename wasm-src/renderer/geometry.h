#pragma once

#include "vertex.h"

#include <vector>

struct Geometry {
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
};
