#include "path_simplifier.h"

#include "path_fitter.h"
#include "../vector.h"

static float sq_seg_dist(const vec2& p, const vec2 p1, const vec2& p2) {
  vec2 delta = p2 - p1;

  if (not_zero(delta)) {

    float t = ((p.x - p1.x) * delta.x + (p.y - p1.y) * delta.y) / squared_length(delta);

    if (t > 1) {
      delta = p - p2;
    } else if (t > 0) {
      delta = p - (p1 + delta * t);
    } else {
      delta = p - p1;
    }
  } else {
    delta = p - p1;
  }

  return squared_length(delta);
}


static void simplify_dp_step(
  const std::vector<PathPoint>& points,
  uint first, uint last, float sq_threshold,
  std::vector<uint>& result
) {
  float max_sq_dist = sq_threshold;
  uint index;

  for (uint i = first + 1; i < last; i++) {
    float sq_dist = sq_seg_dist(points[i].position, points[first].position, points[last].position);

    if (sq_dist > max_sq_dist) {
      index = i;
      max_sq_dist = sq_dist;
    }
  }

  if (max_sq_dist > sq_threshold) {
    if (index - first > 1) simplify_dp_step(points, first, index, sq_threshold, result);

    result.push_back(index);

    if (last - index > 1) simplify_dp_step(points, index, last, sq_threshold, result);
  }
}

std::vector<uint> simplify_path(
  const std::vector<PathPoint>& path,
  uint start_index,
  uint end_index,
  float threshold
) {
  size_t path_size = path.size();

  if (path_size < 3 || is_almost_zero(threshold)) {
    std::vector<uint> result(path.size());

    for (uint i = 0; i < path.size(); i++) {
      result[i] = i;
    }

    return result;
  }

  std::vector<uint> result;
  result.reserve(end_index - start_index);

  result.push_back(start_index);
  simplify_dp_step(path, start_index, end_index, threshold * threshold, result);
  result.push_back(end_index);

  return result;
}

uint simplify_spline(
  Knot* knots, const uint knots_len, uint knots_len_remaining,
  const float error_sq_max
) {
  return knots_len_remaining;
}
