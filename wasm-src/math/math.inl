#include "math.h"

inline bool is_point_in_circle(const vec2& point, const vec2& center, const float radius) {
  return squared_distance(point, center) <= radius * radius;
}

inline bool is_point_in_box(const vec2& point, const Box& box, const float threshold = 0.0f) {
  return (
    point.x + threshold >= box.min.x &&
    point.x - threshold <= box.max.x &&
    point.y + threshold >= box.min.y &&
    point.y - threshold <= box.max.y
    );
}
