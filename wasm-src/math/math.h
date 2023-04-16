#pragma once

#include "vector.h"
#include "box.h"

#include <vector>

bool is_point_in_circle(const vec2& point, const vec2& center, const float radius);
bool is_point_in_box(const vec2& point, const Box& box, const float threshold);

bool does_box_intersect_box(const Box& a, const Box& b);
bool is_box_in_box(const Box& a, const Box& b);
std::vector<float> line_line_intersections(const Box& a, const Box& b);
std::vector<vec2> line_line_intersection_points(const Box& a, const Box& b);
std::vector<vec2> line_circle_intersection_points(const Box& line, const vec2& center, const float radius);

std::vector<Box> lines_from_box(const Box& box);
vec2 circle_center(const vec2& a, const vec2& b, const vec2& c);

bool clockwise(const std::vector<vec2>& points);

#include "math.inl"
