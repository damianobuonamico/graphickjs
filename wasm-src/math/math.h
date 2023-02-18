#pragma once

#include "vector.h"
#include "box.h"

#include <vector>

bool is_point_in_circle(const vec2& point, const vec2& center, const float radius);
bool is_point_in_box(const vec2& point, const Box& box, const float threshold);

bool does_box_intersect_box(const Box& a, const Box& b);
std::vector<float> line_line_intersections(const Box& a, const Box& b);
std::vector<vec2> line_line_intersection_points(const Box& a, const Box& b);

std::vector<Box> get_lines_from_box(const Box& box);

#include "math.inl"
