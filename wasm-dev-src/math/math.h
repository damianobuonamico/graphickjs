#pragma once

#include "vector.h"
#include "rect.h"

#include <vector>

namespace Graphick::Math {

  bool is_point_in_circle(const vec2 point, const vec2 center, const float radius);
  bool is_point_in_rect(const vec2 point, const rect& rect, const float threshold);

  bool does_rect_intersect_rect(const rect& a, const rect& b);
  bool is_rect_in_rect(const rect& a, const rect& b);
  float rect_rect_intersection_area(const rect& a, const rect& b);
  std::vector<float> line_line_intersections(const rect& a, const rect& b);
  std::vector<vec2> line_line_intersection_points(const rect& a, const rect& b);
  vec2 line_line_fast_intersection_points(const rect& a, const rect& b);
  std::vector<vec2> line_circle_intersection_points(const rect& line, const vec2 center, const float radius);

  std::vector<rect> lines_from_rect(const rect& rect);
  vec2 circle_center(const vec2 a, const vec2 b, const vec2 c);

  bool clockwise(const std::vector<vec2>& points);

  int hash(std::initializer_list<float> floats);
}

#include "math.inl"
