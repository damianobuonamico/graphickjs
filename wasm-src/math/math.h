#pragma once

#include "vector.h"
#include "box.h"

bool is_point_in_circle(const vec2& point, const vec2& center, const float radius);
bool is_point_in_box(const vec2& point, const Box& box, const float threshold);

#include "math.inl"
