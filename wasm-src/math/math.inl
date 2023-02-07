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

inline bool does_box_intersect_box(const Box& a, const Box& b) {
  return b.max.x >= a.min.x && a.max.x >= b.min.x && b.max.y >= a.min.y && a.max.y >= b.min.y;
}

inline std::vector<Box> get_lines_from_box(const Box& box) {
  return {
    { box.min, { box.max.x, box.min.y }},
    { { box.max.x, box.min.y }, box.max },
    { box.max, { box.min.x, box.max.y } },
    { { box.min.x, box.max.y }, box.min }
  };
}
