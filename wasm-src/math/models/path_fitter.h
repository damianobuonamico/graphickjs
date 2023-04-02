#pragma once

#include "path_point.h"
#include "../vec2.h"

#include <vector>

void fit_cubic(
  const std::vector<PathPoint>& points, size_t first, size_t last,
  vec2 t_hat_1, vec2 t_hat_2, float error,
  std::vector<PathBezier>& r_curves
);

void fit_path(
  const std::vector<PathPoint>& points,
  const uint start_index,
  const uint end_index,
  const float error,
  std::vector<PathBezier>& beziers
);

void refit_path(
  const std::vector<PathPoint>& points,
  const uint start_index,
  const uint end_index,
  const float error,
  std::vector<PathBezier>& beziers
);
