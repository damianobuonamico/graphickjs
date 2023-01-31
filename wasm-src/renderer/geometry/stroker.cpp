#include "stroker.h"

#include "../../math/vector.h"
#include "../../utils/defines.h"

uint32_t generate_round_cap(
  const vec2& from,
  const vec2& to,
  float radius,
  Geometry& geometry,
  int start_offset,
  float zoom
) {
  vec2 center = midpoint(from, to);
  vec2 direction_from = from - center;
  vec2 direction_to = to - center;
  float cap_angle = MATH_PI;

  float increment = 2.0f * std::acos(1 - GEOMETRY_CURVE_ERROR / (radius * zoom));
  int sides = (int)(std::abs(cap_angle) / increment);
  increment = cap_angle / sides;

  if (!sides || sides < 1) {
    geometry.vertices.insert(geometry.vertices.end(), { from, to });
    return 2;
  }

  geometry.vertices.push_back(center);
  uint32_t offset = static_cast<uint32_t>(start_offset + 1);

  uint32_t center_index = offset;
  uint32_t from_index = offset + sides;

  geometry.vertices.push_back(rotate(from, center, increment));
  geometry.indices.insert(
    geometry.indices.end(),
    { center_index, from_index, offset + 1 }
  );
  offset += 1;

  for (int i = 2; i < sides; ++i) {
    geometry.vertices.push_back(rotate(from, center, i * increment));
    geometry.indices.insert(
      geometry.indices.end(),
      { offset + 1, center_index, offset }
    );
    offset += 1;
  }

  geometry.vertices.insert(geometry.vertices.end(), { from, to });
  geometry.indices.insert(
    geometry.indices.end(),
    { offset + 2, center_index, offset }
  );
  offset += 2;

  return offset - center_index + 1;
}

uint32_t generate_round_join(
  uint32_t from_index,
  uint32_t to_index,
  uint32_t center_index,
  const vec2& center,
  Geometry& geometry,
  uint32_t offset,
  float zoom
) {
  vec2& from = geometry.vertices[from_index].position;
  vec2& to = geometry.vertices[to_index].position;

  vec2 direction_from = from - center;
  vec2 direction_to = to - center;
  float radius = length(direction_from);
  float join_angle = angle(direction_from, direction_to);

  if (std::abs(join_angle) < GEOMETRY_MAX_INTERSECTION_ERROR) join_angle = MATH_PI;

  float increment = 2.0f * std::acos(1 - GEOMETRY_CURVE_ERROR / (radius * zoom));
  int sides = (int)(std::abs(join_angle) / increment);
  increment = join_angle / sides;

  if (!sides || sides < 1) {
    geometry.indices.insert(
      geometry.indices.end(),
      { from_index, center_index, to_index }
    );
    return 0;
  }

  uint32_t added = 0;

  geometry.vertices.push_back(rotate(from, center, increment));
  geometry.indices.insert(
    geometry.indices.end(),
    { from_index, center_index, offset + added + 1 }
  );
  added += 1;

  for (int i = 2; i < sides; ++i) {
    geometry.vertices.push_back(rotate(from, center, i * increment));
    geometry.indices.insert(
      geometry.indices.end(),
      { offset + added, center_index, offset + added + 1 }
    );
    added += 1;
  }

  geometry.vertices.insert(geometry.vertices.end(), { from, to });
  geometry.indices.insert(
    geometry.indices.end(),
    { offset + added + 2, center_index, offset + added }
  );
  added += 2;

  return added;
}

Geometry stroke_freehand_path(const std::vector<FreehandPathPoint>& points, float thickness, float zoom) {
  using Point = FreehandPathPoint;

  const size_t totpoints = points.size();

  Geometry geometry{};

  /* Sanity check. */
  if (totpoints < 1) return geometry;

  const float stroke_radius = thickness / 2.0f;

  Point const* first = &points[0];
  Point const* last = &points[totpoints - 1];

  float first_radius = stroke_radius * first->pressure;
  float last_radius = stroke_radius * last->pressure;

  Point const* first_next;
  Point const* last_prev;
  if (totpoints > 1) {
    first_next = &points[1];
    last_prev = &points[totpoints - 2];
  } else {
    first_next = first;
    last_prev = last;
  }

  vec2 first_pt = first->position;
  vec2 last_pt = last->position;
  vec2 first_next_pt = first_next->position;
  vec2 last_prev_pt = last_prev->position;

  /* Edge-case if single point. */
  if (totpoints == 1) {
    first_next_pt.x += 0.01f;
    last_prev_pt.x -= 0.01f;
  }

  /* Start cap. */
  vec2 vec_first = first_pt - first_next_pt;

  if (is_zero(normalize(vec_first, vec_first))) {
    vec_first.x = 1.0f;
    vec_first.y = 0.0f;
  }

  vec2 nvec_first = {
    -vec_first.y * first_radius,
    vec_first.x * first_radius,
  };

  uint32_t offset = generate_round_cap(
    first_pt - nvec_first,
    first_pt + nvec_first,
    first_radius,
    geometry,
    -1,
    zoom
  ) - 1;

  uint32_t last_left_index = offset - 1;
  uint32_t last_right_index = offset;

  /* Generate perimeter points. */
  vec2 curr_pt, next_pt, prev_pt;
  vec2 vec_next, vec_prev;
  vec2 nvec_next, nvec_prev;
  vec2 nvec_next_pt, nvec_prev_pt;
  vec2 vec_tangent;

  vec2 vec_miter_left, vec_miter_right;
  vec2 miter_left_pt, miter_right_pt;

  for (int i = 1; i < totpoints - 1; ++i) {
    Point const* curr = &points[i];
    Point const* prev = &points[i - 1];
    Point const* next = &points[i + 1];
    float radius = stroke_radius * curr->pressure;

    curr_pt = curr->position;
    next_pt = next->position;
    prev_pt = prev->position;

    vec_prev = curr_pt - prev_pt;
    vec_next = next_pt - curr_pt;

    float prev_length = length(vec_prev);
    float next_length = length(vec_next);

    if (is_zero(normalize(vec_prev, vec_prev))) {
      vec_prev.x = 1.0f;
      vec_prev.y = 0.0f;
    }
    if (is_zero(normalize(vec_next, vec_next))) {
      vec_next.x = 1.0f;
      vec_next.y = 0.0f;
    }

    nvec_prev.x = -vec_prev.y;
    nvec_prev.y = vec_prev.x;

    nvec_next.x = -vec_next.y;
    nvec_next.y = vec_next.x;

    vec_tangent = vec_prev + vec_next;
    if (is_zero(normalize(vec_tangent, vec_tangent))) {
      vec_tangent = nvec_prev;
    }

    vec_miter_left.x = -vec_tangent.y;
    vec_miter_left.y = vec_tangent.x;

    /* Calculate miter length. */
    float an1 = dot(vec_miter_left, nvec_prev);
    if (an1 == 0.0f) {
      an1 = 1.0f;
    }

    float miter_length = radius / an1;
    if (miter_length <= 0.0f) {
      miter_length = 0.01f;
    }

    normalize_length(vec_miter_left, miter_length, vec_miter_left);

    vec_miter_right = negate(vec_miter_left);

    float angle = dot(vec_next, nvec_prev);
    /* Add two points if angle is close to being straight. */
    if (std::abs(angle) < GEOMETRY_MAX_INTERSECTION_ERROR) {
      normalize_length(nvec_prev, radius, nvec_prev);
      normalize_length(nvec_next, radius, nvec_next);

      nvec_prev_pt = curr_pt + nvec_prev;

      negate(nvec_next, nvec_next);
      nvec_next_pt = curr_pt + nvec_next;

      geometry.vertices.insert(geometry.vertices.end(), { nvec_prev_pt, nvec_next_pt });
      offset += 2;

      geometry.indices.insert(
        geometry.indices.end(),
        { last_left_index, last_right_index, offset - 1, last_right_index, offset - 1, offset }
      );

      last_left_index = offset - 1;
      last_right_index = offset;
    } else {
      /* Bend to the left. */
      if (angle < 0.0f) {
        normalize_length(nvec_prev, radius, nvec_prev);
        normalize_length(nvec_next, radius, nvec_next);

        nvec_prev_pt = curr_pt + nvec_prev;
        nvec_next_pt = curr_pt + nvec_next;

        float distance = squared_distance(nvec_next_pt, nvec_prev_pt);

        if (distance > GEOMETRY_SQR_EPSILON) {
          geometry.vertices.insert(geometry.vertices.end(), { nvec_prev_pt, nvec_next_pt });
          offset += 2;

          geometry.indices.insert(
            geometry.indices.end(),
            { last_left_index, last_right_index, offset - 1 }
          );

          last_left_index = offset;

          offset += generate_round_join(
            offset - 1,
            offset,
            last_right_index,
            curr_pt,
            geometry,
            offset,
            zoom
          );
        } else {
          geometry.vertices.push_back(nvec_prev_pt);
          offset += 1;

          geometry.indices.insert(
            geometry.indices.end(),
            { last_left_index, last_right_index, offset }
          );

          last_left_index = offset;
        }

        if (miter_length < prev_length && miter_length < next_length) {
          miter_right_pt = curr_pt + vec_miter_right;
        } else {
          negate(nvec_next, nvec_next);
          miter_right_pt = curr_pt + nvec_next;
        }

        geometry.vertices.push_back(miter_right_pt);
        offset += 1;

        geometry.indices.insert(
          geometry.indices.end(),
          { last_left_index, last_right_index, offset }
        );

        last_right_index = offset;
      } else {
        /* Bend to the right. */
        normalize_length(nvec_prev, -radius, nvec_prev);
        normalize_length(nvec_next, -radius, nvec_next);

        nvec_prev_pt = curr_pt + nvec_prev;
        nvec_next_pt = curr_pt + nvec_next;

        float distance = squared_distance(nvec_next_pt, nvec_prev_pt);

        if (distance > GEOMETRY_SQR_EPSILON) {
          geometry.vertices.insert(geometry.vertices.end(), { nvec_prev_pt, nvec_next_pt });
          offset += 2;

          geometry.indices.insert(
            geometry.indices.end(),
            { last_left_index, last_right_index, offset - 1 }
          );

          last_right_index = offset;

          offset += generate_round_join(
            offset - 1,
            offset,
            last_left_index,
            curr_pt,
            geometry,
            offset,
            zoom
          );
        } else {
          geometry.vertices.push_back(nvec_prev_pt);
          offset += 1;

          geometry.indices.insert(
            geometry.indices.end(),
            { last_left_index, last_right_index, offset }
          );

          last_right_index = offset;
        }

        if (miter_length < prev_length && miter_length < next_length) {
          miter_left_pt = curr_pt + vec_miter_left;
        } else {
          negate(nvec_prev, nvec_prev);
          miter_left_pt = curr_pt + nvec_prev;
        }

        geometry.vertices.push_back(miter_left_pt);
        offset += 1;

        geometry.indices.insert(
          geometry.indices.end(),
          { last_left_index, last_right_index, offset }
        );

        last_left_index = offset;
      }
    }
  }

  vec2 vec_last = last_prev_pt - last_pt;

  if (is_zero(normalize(vec_last, vec_last))) {
    vec_last.x = 1.0f;
    vec_last.y = 0.0f;
  }

  vec2 nvec_last = {
    -vec_last.y * last_radius,
    vec_last.x * last_radius
  };

  offset += generate_round_cap(
    last_pt + nvec_last,
    last_pt - nvec_last,
    last_radius,
    geometry,
    offset,
    zoom
  );

  geometry.indices.insert(
    geometry.indices.end(),
    { last_left_index, last_right_index, offset - 1, last_left_index, offset - 1, offset }
  );

  return geometry;
}

std::vector<FreehandPathPoint> smooth_freehand_path(const std::vector<FreehandPathPoint>& points, int kernel_size) {
  std::vector<FreehandPathPoint> result(points);

  for (int i = 0; i < static_cast<int>(points.size()); ++i) {
    int left_offset = i - kernel_size;
    int from = left_offset >= 0 ? left_offset : 0;
    int to = i + kernel_size + 1;

    int count = 0;
    float sum = 0;
    for (size_t j = from; j < to && j < points.size(); ++j) {
      sum += points[j].pressure;
      count += 1;
    }

    result[i].pressure = sum / count;
  }

  return result;
}
