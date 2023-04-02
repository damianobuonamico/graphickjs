#pragma once

#include "path_point.h"

#include <vector>

std::vector<uint> detect_corners(
  const std::vector<PathPoint>& points,
  const float radius_min,
  const float radius_max,
  const float angle_threshold,
  const float min_distance,
  const uint samples_max
);
