#include "math.h"
#include "../utils/defines.h"

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

inline std::vector<float> line_line_intersections(const Box& a, const Box& b) {
  if (!does_box_intersect_box(a, b)) {
    return {};
  }

  float den = b.max.x - b.min.x;

  if (is_almost_zero(den)) {
    float t = (b.min.x - a.min.x) / (a.max.x - a.min.x);
    if (t >= 0.0f && t <= 1.0f) {
      return { t };
    }

    return {};
  }

  float m = (b.max.y - b.min.y) / den;

  float t = (m * b.min.x - b.min.y + a.min.y - m * a.min.x) / (m * (a.max.x - a.min.x) + a.min.y - a.max.y);
  if (t >= 0.0f && t <= 1.0f) {
    return { t };
  }

  return {};
}

inline std::vector<vec2> line_line_intersection_points(const Box& a, const Box& b) {
  std::vector<float> values = line_line_intersections(a, b);
  std::vector<vec2> points{};

  for (float value : values) {
    vec2 point = lerp(a.min, a.max, value);
    if (is_point_in_box(point, b, GEOMETRY_MAX_INTERSECTION_ERROR)) {
      points.push_back(point);
    }
  }

  return points;
}

inline std::vector<Box> get_lines_from_box(const Box& box) {
  return {
    { box.min, { box.max.x, box.min.y }},
    { { box.max.x, box.min.y }, box.max },
    { box.max, { box.min.x, box.max.y } },
    { { box.min.x, box.max.y }, box.min }
  };
}
