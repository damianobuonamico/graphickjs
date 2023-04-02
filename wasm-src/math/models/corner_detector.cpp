#include "corner_detector.h"

#include "../vector.h"
#include "../math.h"

static float cos(const vec2& v0, const vec2& v1, const vec2& v2) {
  vec2 dvec0 = normalize(v0 - v1);
  vec2 dvec1 = normalize(v1 - v2);
  return clamp(dot(dvec0, dvec1), -1.0, 1.0);
}

static float angle(const vec2& v0, const vec2& v1, const vec2& v2) {
  return acos(cos(v0, v1, v2));
}

static bool point_corner_measure(
  const std::vector<PathPoint>& points,
  const uint points_len,
  const uint i,
  const uint i_prev_init,
  const uint i_next_init,
  const float radius,
  const uint samples_max,
  vec2& r_p_prev, uint* r_i_prev_next,
  vec2& r_p_next, uint* r_i_next_prev) {
  const vec2 p = points[i].position;

  uint i_prev = i_prev_init;
  uint i_prev_next = i_prev + 1;

  uint sample = 0;

  while (true) {
    if ((i_prev == -1) || (sample++ > samples_max)) {
      return false;
    } else if (squared_distance(p, points[i_prev].position) < radius) {
      i_prev -= 1;
    } else {
      break;
    }
  }

  uint i_next = i_next_init;
  uint i_next_prev = i_next - 1;

  sample = 0;

  while (true) {
    if ((i_next == points_len) || (sample++ > samples_max)) {
      return false;
    } else if (squared_distance(p, points[i_next].position) < radius) {
      i_next += 1;
    } else {
      break;
    }
  }

  std::vector<vec2> intersections = line_circle_intersection_points(
    Box{ points[i_prev].position, points[i_prev_next].position }, p, radius
  );

  if (intersections.empty()) {
    return false;
  }

  r_p_prev = intersections[0];

  intersections = line_circle_intersection_points(
    Box{ points[i_next].position, points[i_next_prev].position }, p, radius
  );

  if (intersections.empty()) {
    return false;
  }

  r_p_next = intersections[0];

  *r_i_prev_next = i_prev_next;
  *r_i_next_prev = i_next_prev;

  return true;
}

static float point_corner_angle(
  const std::vector<PathPoint>& points,
  const uint points_len,
  const uint i,
  const float radius_mid,
  const float radius_max,
  const float angle_threshold,
  const float angle_threshold_cos,
  const uint samples_max
) {
  if (i == 0 || i == points_len - 1) {
    return 0.0f;
  }

  const vec2& p = points[i].position;

  if (cos(points[i - 1].position, p, points[i + 1].position) > angle_threshold_cos) {
    return 0.0f;
  }

  vec2 p_mid_prev;
  vec2 p_mid_next;

  uint i_mid_prev_next, i_mid_next_prev;
  if (point_corner_measure(
    points, points_len,
    i, i - 1, i + 1,
    radius_mid,
    samples_max,
    p_mid_prev, &i_mid_prev_next,
    p_mid_next, &i_mid_next_prev
  )) {
    const float angle_mid_cos = cos(p_mid_prev, p, p_mid_next);

    if (angle_mid_cos < angle_threshold_cos) {
      vec2 p_max_prev;
      vec2 p_max_next;

      uint i_max_prev_next, i_max_next_prev;
      if (point_corner_measure(
        points, points_len,
        i, i - 1, i + 1,
        radius_max,
        samples_max,
        p_max_prev, &i_max_prev_next,
        p_max_next, &i_max_next_prev))
      {
        const float angle_mid = acos(angle_mid_cos);
        const float angle_max = 0.5f * angle(p_max_prev, p, p_max_next);
        const float angle_diff = angle_mid - angle_max;

        if (angle_diff > angle_threshold) {
          return angle_diff;
        }
      }
    }
  }

  return 0.0f;
}


std::vector<uint> detect_corners(
  const std::vector<PathPoint>& points,
  const float radius_min,
  const float radius_max,
  const float angle_threshold,
  const float min_distance,
  const uint samples_max
) {
  const float radius_mid = 0.5f * (radius_min + radius_max);
  const float angle_threshold_cos = cos(angle_threshold);
  const uint points_len = (uint)points.size();

  std::vector<float> points_angle(points_len);
  std::vector<uint> corners;

  uint corners_len = 0;

  for (uint i = 0; i < points_len; i++) {
    points_angle[i] = point_corner_angle(
      points, points_len, i,
      radius_mid, radius_max,
      angle_threshold, angle_threshold_cos,
      samples_max
    );

    if (points_angle[i] != 0.0f) {
      corners_len++;
    }
  }

  if (corners_len == 0) {
    return { 0, points_len - 1 };
  }

  const float radius_min_sq = radius_min * radius_min;
  const float min_distance_sq = min_distance * min_distance;

  uint i_span_start = 0;

  while (i_span_start < points_len) {
    uint i_span_end = i_span_start;

    if (points_angle[i_span_start] != 0.0f) {
      uint i_next = i_span_start + 1;
      uint i_best = i_span_start;

      while (i_next < points_len) {
        if ((points_angle[i_next] == 0.0f) ||
          (squared_distance(
            points[i_next - 1].position,
            points[i_next].position) > radius_min_sq))
        {
          break;
        } else {
          if (points_angle[i_best] < points_angle[i_next]) {
            i_best = i_next;
          }

          i_span_end = i_next;
          i_next += 1;
        }
      }

      if (i_span_start != i_span_end) {
        uint i = i_span_start;

        while (i <= i_span_end) {
          if (i != i_best) {
            points_angle[i] = 0.0f;
            corners_len--;
          }

          i += 1;
        }
      }
    }

    i_span_start = i_span_end + 1;
  }

  corners_len += 2;
  corners.push_back(0);

  bool is_prev_corner = false;

  for (uint i = 1; i < points_len - 1; i++) {
    if (points_angle[i] != 0.0f) {
      if (is_prev_corner && squared_distance(points[i - 1].position, points[i].position) < min_distance_sq) {
        if (points_angle[i + 1] != 0.0f && squared_distance(points[i].position, points[i + 1].position) < min_distance_sq) {
          is_prev_corner = true;
          corners[corners.size() - 1] = i;
        } else {
          is_prev_corner = false;
        }

        continue;
      }

      corners.push_back(i);
      is_prev_corner = true;
    } else {
      is_prev_corner = false;
    }
  }

  if (points_len > 2 && is_prev_corner && squared_distance(points[points_len - 1].position, points[points_len - 2].position) < min_distance_sq
    ) {
    corners[corners.size() - 1] = points_len - 1;
  } else {
    corners.push_back(points_len - 1);
  }

  return corners;
}
