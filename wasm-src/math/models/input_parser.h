#pragma once

#include "path_point.h"

#include <vector>

std::vector<PathBezier> parse_input(
  const std::vector<PathPoint>& input,
  std::vector<uint>& corners
);
