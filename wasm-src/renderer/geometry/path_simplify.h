#pragma once

#include "stroker.h"
#include "../../math/vector.h"
#include "../../utils/console.h"

// square distance from a point to a segment
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

// basic distance-based simplification
static std::vector<FreehandPathPoint> simplify_radial_dist(const std::vector<FreehandPathPoint>& points, float sq_tolerance) {

  const FreehandPathPoint* prev_point = &points[0];
  const FreehandPathPoint* point = prev_point;
  const size_t size = points.size();

  std::vector<FreehandPathPoint> new_points(size / 2);
  new_points.push_back(*prev_point);

  for (size_t i = 1; i < size; i++) {
    point = &points[i];

    if (squared_distance(point->position, prev_point->position) > sq_tolerance) {
      new_points.push_back(*point);
      prev_point = point;
    }
  }

  if (prev_point != point) new_points.push_back(*point);

  return new_points;
}

static void simplify_dp_step(const std::vector<FreehandPathPoint>& points, size_t first, size_t last, float sq_tolerance, std::vector<FreehandPathPoint>& simplified) {
  float max_sq_dist = sq_tolerance;
  size_t index;

  for (size_t i = first + 1; i < last; i++) {
    float sq_dist = sq_seg_dist(points[i].position, points[first].position, points[last].position);

    if (sq_dist > max_sq_dist) {
      index = i;
      max_sq_dist = sq_dist;
    }
  }

  if (max_sq_dist > sq_tolerance) {
    if (index - first > 1) simplify_dp_step(points, first, index, sq_tolerance, simplified);

    simplified.push_back(points[index]);

    if (last - index > 1) simplify_dp_step(points, index, last, sq_tolerance, simplified);
  }
}

// simplification using Ramer-Douglas-Peucker algorithm
static std::vector<FreehandPathPoint> simplify_douglas_peucker(const std::vector<FreehandPathPoint>& points, float sq_tolerance) {
  const size_t last = points.size() - 1;
  std::vector<FreehandPathPoint> simplified(last / 2);

  simplified.push_back(points[0]);

  simplify_dp_step(points, 0, last, sq_tolerance, simplified);

  simplified.push_back(points[last]);

  return simplified;
}

// both algorithms combined for awesome performance
static std::vector<FreehandPathPoint> simplify_path(const std::vector<FreehandPathPoint>& points, float tolerance, bool highest_quality) {
  if (points.size() <= 2) return points;

  float sq_tolerance = tolerance * tolerance;

  return simplify_douglas_peucker(highest_quality ? points : simplify_radial_dist(points, sq_tolerance), sq_tolerance);
}
