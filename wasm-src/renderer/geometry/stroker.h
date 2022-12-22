#pragma once

#include "geometry.h"

struct FreehandPathPoint {
  vec2 position;
  float pressure;
};

Geometry stroke_freehand_path(const std::vector<FreehandPathPoint>& points, float thickness, float zoom);

std::vector<FreehandPathPoint> smooth_freehand_path(const std::vector<FreehandPathPoint>& points, int kernel_size);
