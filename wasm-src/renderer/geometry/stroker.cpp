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
    geometry.push_vertices({ from, to });
    return 2;
  }

  geometry.push_vertex(center);
  uint32_t offset = static_cast<uint32_t>(start_offset + 1);

  uint32_t center_index = offset;
  uint32_t from_index = offset + sides;

  geometry.push_vertex(rotate(from, center, increment));
  geometry.push_indices({ center_index, from_index, offset + 1 });
  offset += 1;

  for (int i = 2; i < sides; ++i) {
    geometry.push_vertex(rotate(from, center, i * increment));
    geometry.push_indices({ offset + 1, center_index, offset });
    offset += 1;
  }

  geometry.push_vertices({ from, to });
  geometry.push_indices({ offset + 2, center_index, offset });
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
  const vec2& from = geometry.vertices()[from_index].position;
  const vec2& to = geometry.vertices()[to_index].position;

  vec2 direction_from = from - center;
  vec2 direction_to = to - center;
  float radius = length(direction_from);
  float join_angle = angle(direction_from, direction_to);

  if (std::abs(join_angle) < GEOMETRY_MAX_INTERSECTION_ERROR) join_angle = MATH_PI;

  float increment = 2.0f * std::acos(1 - GEOMETRY_CURVE_ERROR / (radius * zoom));
  int sides = (int)(std::abs(join_angle) / increment);
  increment = join_angle / sides;

  if (!sides || sides < 1) {
    geometry.push_indices({ from_index, center_index, to_index });
    return 0;
  }

  uint32_t added = 0;

  geometry.push_vertex(rotate(from, center, increment));
  geometry.push_indices({ from_index, center_index, offset + added + 1 });
  added += 1;

  for (int i = 2; i < sides; ++i) {
    geometry.push_vertex(rotate(from, center, i * increment));
    geometry.push_indices({ offset + added, center_index, offset + added + 1 });
    added += 1;
  }

  geometry.push_vertices({ from, to });
  geometry.push_indices({ offset + added + 2, center_index, offset + added });
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

      geometry.push_vertices({ nvec_prev_pt, nvec_next_pt });
      offset += 2;

      geometry.push_indices({ last_left_index, last_right_index, offset - 1, last_right_index, offset - 1, offset });

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
          geometry.push_vertices({ nvec_prev_pt, nvec_next_pt });
          offset += 2;

          geometry.push_indices({ last_left_index, last_right_index, offset - 1 });

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
          geometry.push_vertex(nvec_prev_pt);
          offset += 1;

          geometry.push_indices({ last_left_index, last_right_index, offset });

          last_left_index = offset;
        }

        if (miter_length < prev_length && miter_length < next_length) {
          miter_right_pt = curr_pt + vec_miter_right;
        } else {
          negate(nvec_next, nvec_next);
          miter_right_pt = curr_pt + nvec_next;
        }

        geometry.push_vertex(miter_right_pt);
        offset += 1;

        geometry.push_indices({ last_left_index, last_right_index, offset });

        last_right_index = offset;
      } else {
        /* Bend to the right. */
        normalize_length(nvec_prev, -radius, nvec_prev);
        normalize_length(nvec_next, -radius, nvec_next);

        nvec_prev_pt = curr_pt + nvec_prev;
        nvec_next_pt = curr_pt + nvec_next;

        float distance = squared_distance(nvec_next_pt, nvec_prev_pt);

        if (distance > GEOMETRY_SQR_EPSILON) {
          geometry.push_vertices({ nvec_prev_pt, nvec_next_pt });
          offset += 2;

          geometry.push_indices({ last_left_index, last_right_index, offset - 1 });

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
          geometry.push_vertex(nvec_prev_pt);
          offset += 1;

          geometry.push_indices({ last_left_index, last_right_index, offset });

          last_right_index = offset;
        }

        if (miter_length < prev_length && miter_length < next_length) {
          miter_left_pt = curr_pt + vec_miter_left;
        } else {
          negate(nvec_prev, nvec_prev);
          miter_left_pt = curr_pt + nvec_prev;
        }

        geometry.push_vertex(miter_left_pt);
        offset += 1;

        geometry.push_indices({ last_left_index, last_right_index, offset });

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

  geometry.push_indices({ last_left_index, last_right_index, offset - 1, last_left_index, offset - 1, offset });

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

// TODO: Optimize
static vec2 bezier_t(const Bezier& curve, float t) {
  vec2 a = -curve.p0 + 3.0f * curve.p1 - 3.0f * curve.p2 + curve.p3;
  vec2 b = 3.0f * (curve.p0 - 2 * curve.p1 + curve.p2);
  vec2 c = -3.0f * (curve.p0 - curve.p1);
  vec2 d = curve.p0;

  return a * t * t * t + b * t * t + c * t + d;
}

static vec2 bezier_derivative_t(const Bezier& curve, float t) {
  vec2 a = 3.0f * (-curve.p0 + 3.0f * curve.p1 - 3.0f * curve.p2 + curve.p3);
  vec2 b = 6.0f * (curve.p0 - 2.0f * curve.p1 + curve.p2);
  vec2 c = -3.0f * (curve.p0 - curve.p1);

  return a * t * t + b * t + c;
}

static vec2 bezier_second_derivative_t(const Bezier& curve, float t) {
  vec2 a = 6.0f * (-curve.p0 + 3.0f * curve.p1 - 3.0f * curve.p2 + curve.p3);
  vec2 b = 6.0f * (curve.p0 - 2.0f * curve.p1 + curve.p2);

  return a * t + b;
}

static vec2 t_from_theta(const Bezier& curve, float theta) {
  vec2 A = 3.0f * (-curve.p0 + 3.0f * curve.p1 - 3.0f * curve.p2 + curve.p3);
  vec2 B = 6.0f * (curve.p0 - 2.0f * curve.p1 + curve.p2);
  vec2 C = -3.0f * (curve.p0 - curve.p1);

  float tan = tanf(theta);

  float a = A.y - tan * A.x;
  float b = B.y - tan * B.x;
  float c = C.y - tan * C.x;

  if (is_almost_zero(a)) {
    if (is_almost_zero(b)) {
      return { -1.0f, -1.0f };
    }
    return { -c / b, -1.0f };
  }

  float delta = b * b - 4.0f * a * c;

  if (is_almost_zero(delta)) {
    return { -b / (2.0f * a), -1.0f };
  } else if (delta > 0.0f) {
    float sqrt_delta = sqrtf(delta);
    float t1 = (-b + sqrt_delta) / (2.0f * a);
    float t2 = (-b - sqrt_delta) / (2.0f * a);

    return { t1, t2 };
  }

  return { -1.0f, -1.0f };
}

static void add_point(const vec2& point, const vec4& color, uint32_t& offset, Geometry& geo, float size = 0.5f) {
  vec2 off_d1{ size, size };
  vec2 off_d2{ -size, size };

  geo.push_vertices({ {point - off_d1, color}, {point + off_d2, color}, {point + off_d1, color}, {point - off_d2, color} });
  geo.push_indices({ offset + 0, offset + 1, offset + 2, offset + 2, offset + 3, offset + 0 });

  offset += 4;
}

/* "Inflection points of a cubic Bezier"
 * http://web.archive.org/web/20191210155614/http://www.caffeineowl.com/graphics/2d/vectorial/cubic-inflexion.html
 * 0 = cross(P'(t),P"(t)) = Px*Pyy-Py*Pxx
 *
 * Developed in Maple:
 *
 * with(linalg):
 * > unprotect(D);
 * > A:=vector([Ax,Ay,0]):
 * > B:=vector([Bx,By,0]):
 * > C:=vector([Cx,Cy,0]):
 * > D:=vector([Dx,Dy,0]):
 * > P:=A*t^3+B*t^2+C*t+D;
 * P := A*t^3+B*t^2+C*t+D
 * > Pt:=(diff(P,t));Ptt:=(diff(P,t,t));
 * Pt := 3*A*t^2+2*B*t+C
 * Ptt := 6*A*t+2*B
 * > v:=crossprod(Pt,Ptt);
 * v := vector([0, 0, (3*t^2*Ax+2*t*Bx+Cx)*(6*t*Ay+2*By)-(3*t^2*Ay+2*t*By+Cy)*(6*t*Ax+2*Bx)])
 * > collect(v[3]/2,t);  % dividing by 2 just scales coefficients without changing polynomial's roots
 * (3*Bx*Ay-3*Ax*By)*t^2+(3*Cx*Ay-3*Cy*Ax)*t+Cx*By-Cy*Bx
 *
 * So the inflection points are at the solutions (roots) of
 *
 *   0 = (3*Bx*Ay-3*Ax*By)*t^2+(3*Cx*Ay-3*Cy*Ax)*t+Cx*By-Cy*Bx
 *     = a*t^2+b*t+c
 *
 * where a, b, & c are:
 *   a = 3*(Bx*Ay-Ax*By)
 *   b = 3*(Cx*Ay-Cy*Ax)
 *   c = Cx*By-Cy*Bx
 * These are determinants!  Scaled by 3 for a & b.
 */
static vec2 find_inflections(const Bezier& curve) {
  vec2 A = curve.p1 - curve.p0;
  vec2 B = curve.p2 - curve.p1 - A;
  vec2 C = curve.p3 - curve.p2 - A - 2.0f * B;

  float a = B.x * C.y - B.y * C.x;
  float b = A.x * C.y - A.y * C.x;
  float c = A.x * B.y - A.y * B.x;

  // TODO: Check if other cases need to be handled
  if (is_almost_zero(a)) {
    if (is_almost_zero(b)) {
      return { 0.0f, 0.0f };
    }
    return { -c / b, -1.0f };
  }

  float delta = b * b - 4.0f * a * c;

  if (is_almost_zero(delta)) {
    return { -b / (2.0f * a), -1.0f };
  } else if (delta > 0.0f) {
    float sqrt_delta = sqrtf(delta);
    float t1 = (-b + sqrt_delta) / (2.0f * a);
    float t2 = (-b - sqrt_delta) / (2.0f * a);

    return { t1, t2 };
  }

  return { -1.0f, -1.0f };
}

// Compute absolute angle difference of theta1 and theta0 in radians.
static float absolute_angle_difference(float theta1, float theta0) {
  const float absolute_difference = abs(theta1 - theta0);

  // Is absolute difference between angles greater than 180 degrees?
  if (absolute_difference > MATH_PI) {
    // Yes, return absolute difference minus 360 degrees.
    return abs(absolute_difference - MATH_TWO_PI);
  }

  // No, return absolute difference as is.
  return absolute_difference;
}

struct BadIndex {
  size_t index;
  uint32_t offset;
};

void stroke_curve(const Bezier& curve, uint32_t& offset, Geometry& geo) {
  vec2 points[4] = { curve.p0, curve.p1, curve.p2, curve.p3 };
  int resolution = 100;

  // for (int i = 0; i <= resolution; i++) {
  //   float t = (float)i / (float)resolution;

  //   add_point(bezier_t(curve, t), vec4{ 0.2f, 0.2f, 1.0f, 1.0f }, offset, geo);
  //   add_point(bezier_derivative_t(curve, t), vec4{ 0.2f, 0.8f, 0.8f, 1.0f }, offset, geo);
  // }

  // for (vec2& point : points) {
  //   add_point(point, vec4{ 1.0f, 0.2f, 0.2f, 1.0f }, offset, geo, 1.0f);
  // }

  // vec2 inflections = (-curve.p0 + 2.0f * curve.p1 - curve.p2) / (-curve.p0 + 3.0f * curve.p1 - 3.0f * curve.p2 + curve.p3);
  vec2 inflections = find_inflections(curve);
  vec4 inflection_col{ 0.2f, 1.0f, 0.2f, 1.0f };

  std::vector<float> turning_points{};
  turning_points.reserve(4);

  // TODO: inline
  vec2 P0 = bezier_derivative_t(curve, 0.0f);
  turning_points.push_back(atan2(P0.y, P0.x));

  std::vector<float> inflection_points{};
  inflection_points.reserve(4);

  inflection_points.push_back(0.0f);

  // Sort inflections
  if (inflections.x > inflections.y) swap_coordinates(inflections, inflections);
  if (inflections.x > 0.0f && inflections.x < 1.0f) {
    vec2 P1 = bezier_derivative_t(curve, inflections.x);

    // add_point(bezier_t(curve, inflections.x), inflection_col, offset, geo, 1.0f);
    // add_point(P1, inflection_col, offset, geo, 1.0f);

    inflection_points.push_back(inflections.x);
    turning_points.push_back(atan2(P1.y, P1.x));
  }

  if (inflections.y > 0.0f && inflections.y < 1.0f && inflections.y != inflections.x) {
    vec2 P2 = bezier_derivative_t(curve, inflections.y);

    // add_point(bezier_t(curve, inflections.y), inflection_col, offset, geo, 1.0f);
    // add_point(P2, inflection_col, offset, geo, 1.0f);

    inflection_points.push_back(inflections.y);
    turning_points.push_back(atan2(P2.y, P2.x));
  }

  inflection_points.push_back(1.0f);

  // TODO: inline
  vec2 P3 = bezier_derivative_t(curve, 1.0f);
  turning_points.push_back(atan2(P3.y, P3.x));

  const float max_angle_difference = std::max(max_angle, MATH_PI / 300.0f);

  std::vector<vec2> t_values{};

  for (int i = 0; i < (int)turning_points.size() - 1; i++) {
    float difference = turning_points[i + 1] - turning_points[i];
    int increments = std::max(abs((int)ceilf(difference / max_angle_difference)), 1);
    float increment = difference / (float)increments;

    t_values.reserve(increments);

    t_values.push_back(vec2{ inflection_points[i], -666.17f });
    for (int j = 1; j < increments; j++) {
      float theta = turning_points[i] + (float)j * increment;
      t_values.push_back(t_from_theta(curve, theta));

      // if (i == 2) {
      //   for (int h = 0; h <= 300; h += 20) {
      //     add_point(vec2{ h * cosf(theta), h * sinf(theta) }, vec4{ 0.2f, 0.2f, 0.8f, 0.5f }, offset, geo);
      //   }
      // }
    }
  }

  t_values.push_back({ 1.0f, 1.0f });

  size_t t_values_len = t_values.size();
  std::vector<float> parsed_t_values(t_values_len);
  parsed_t_values[t_values_len - 1] = 1.0f;

  geo.reserve(t_values_len * 2, t_values_len * 6);

  float max_t = 0.0f;

  {
    vec2 point = bezier_t(curve, 0.0f);
    vec2 tangent = bezier_derivative_t(curve, 0.0f);
    normalize(tangent, tangent);
    vec2 normal = stroke_width * orthogonal(tangent);

    vec4 color = vec4{ 0.8f, 0.0f, 0.0f, 1.0f };

    geo.push_vertices({
      {point + normal, color},
      {point - normal, color}
      });

    offset += 2;
  }

  std::vector<BadIndex> bad_indices{};

  for (size_t i = 1; i < t_values_len - 1; i++) {
    vec2& prev = t_values[i - 1];
    vec2& values = t_values[i];
    vec2& next = t_values[i + 1];

    bool x_bad = values.x <= max_t || values.x >= 1 ||
      (values.x <= prev.x && values.x <= prev.y) ||
      ((next.x > 0 || next.y > 0) && values.x >= next.x && values.x >= next.y);
    bool y_bad = values.y <= max_t || values.y >= 1 ||
      (values.y <= prev.x && values.y <= prev.y) ||
      ((next.x > 0 || next.y > 0) && values.y >= next.x && values.y >= next.y);

    if (x_bad && y_bad) {
      float avg = (values.x + values.y) / 2.0f;
      bool avg_bad = avg <= max_t || avg >= 1 ||
        (avg <= prev.x && avg <= prev.y) ||
        (avg >= next.x && avg >= next.y);

      if (avg_bad) {
        geo.push_vertices({ {},{} });

        geo.push_indices({
          offset - 2, offset - 1, offset + 0, offset + 0, offset + 1, offset - 1
          });

        bad_indices.push_back({ i, offset });
        offset += 2;

        // parsed_t_values[i] = parsed_t_values[i - 1];
        continue;
      } else {
        parsed_t_values[i] = avg;
      }
    } else if (x_bad) {
      parsed_t_values[i] = values.y;
    } else {
      parsed_t_values[i] = values.x;
    }

    max_t = std::max(max_t, parsed_t_values[i]);

    vec2 point = bezier_t(curve, parsed_t_values[i]);
    vec2 tangent = bezier_derivative_t(curve, parsed_t_values[i]);
    normalize(tangent, tangent);
    vec2 normal = stroke_width * orthogonal(tangent);

    vec4 color = vec4{ 0.8f, 0.0f, 0.0f, 1.0f };

    geo.push_vertices({
      {point + normal, color},
      {point - normal, color}
      });

    geo.push_indices({
      offset - 2, offset - 1, offset + 0, offset + 0, offset + 1, offset - 1
      });

    offset += 2;


    // add_point(bezier_t(curve, parsed_t_values[i]), vec4{ 0.8f, 0.0f, 0.0f, 1.0f }, offset, geo, 0.2f);
    // add_point(bezier_derivative_t(curve, parsed_t_values[i]), vec4{ 0.8f, 0.0f, 0.0f, 1.0f }, offset, geo, 0.2f);
  }

  int bad_size = (int)bad_indices.size();

  for (int i = 0; i < bad_size; i++) {
    BadIndex& bad_index = bad_indices[i];

    float prev = parsed_t_values[bad_index.index - 1];
    float next;


    // for (int j = i + 1; j < (int)bad_indices.size(); j++) {
    //   if (bad_indices[j].index == bad_index.index + j - i)
    // }

    int j = 1;
    while (i + j < bad_size && bad_indices[i + j].index == bad_index.index + j) {
      j++;
    }

    next = parsed_t_values[bad_index.index + j];


    float t = (j * prev + next) / (float)(1 + j);

    parsed_t_values[bad_index.index] = t;

    vec2 point = bezier_t(curve, t);
    vec2 tangent = bezier_derivative_t(curve, t);
    normalize(tangent, tangent);
    vec2 normal = stroke_width * orthogonal(tangent);

    vec4 color = vec4{ 0.8f, 1.0f, 0.0f, 1.0f };

    //geo.vertices[bad_index.offset + 0].position += point + normal;
    //geo.vertices[bad_index.offset + 1].position += 10.0f;
    geo.vertices()[bad_index.offset + 0] = { point + normal, color };
    geo.vertices()[bad_index.offset + 1] = { point - normal, color };
  }

  {
    vec2 point = bezier_t(curve, 1.0f);
    vec2 tangent = bezier_derivative_t(curve, 1.0f);
    normalize(tangent, tangent);
    vec2 normal = stroke_width * orthogonal(tangent);

    vec4 color = vec4{ 0.8f, 0.0f, 0.0f, 1.0f };

    geo.push_vertices({
      {point + normal, color},
      {point - normal, color}
      });

    geo.push_indices({
      offset - 2, offset - 1, offset + 0, offset + 0, offset + 1, offset - 1
      });

    offset += 2;
  }
}

Geometry stroke_curves(const std::vector<Bezier>& curves) {
  Geometry geo;
  uint32_t offset = 0;

  Bezier test1{ {0.0f, 0.0f}, {25.0f, -45.0f}, {75.0f, -45.0f}, {100.0f, 0.0f} };
  Bezier test2{ {120.0f, 0.0f}, {120.0f, -45.0f}, {190.0f, -45.0f}, {190.0f, -100.0f} };
  Bezier test3{ {0.0f, 0.0f}, {100.0f, -55.0f}, {0.0f, -140.0f}, {85.0f, -100.0f} };
  Bezier test4{ {0.0f, 0.0f}, {60.0f, -50.0f}, {15.0f, -45.0f}, {100.0f, -10.0f} };
  Bezier test5{ {0.0f, 0.0f}, {110.0f, -100.0f}, {-10.0f, -100.0f}, {100.0f, 0.0f} };
  Bezier test6{ {0.0f, 0.0f}, {101.0f, -100.0f}, {-1.0f, -100.0f}, {100.0f, 0.0f} };
  Bezier test7{ {0.0f, 0.0f}, {100.0f, -100.0f}, {0.0f, -100.0f}, {100.0f, 0.0f} };
  Bezier test8{ {0.0f, 0.0f}, {10.0f, -60.0f}, {0.0f, -60.0f}, {10.0f, -50.0f} };
  Bezier test9{ {0.0f, 0.0f}, {62.78f / 10.0f, -21.56f / 10.0f}, {135.89f / 10.0f, 49.17f / 10.0f}, {46.88f / 10.0f, 104.0f / 10.0f} };

  //stroke_curve(test9, offset, geo);

  for (const Bezier& curve : curves) {
    stroke_curve(curve, offset, geo);
  }

  return geo;
}

void tessellate_join(
  const TessellationParams& params,
  const vec2& point, const vec2& direction, const vec2& normal,
  const uint32_t* override_end_index, Geometry& geo
) {
  uint32_t offset = geo.offset();
  float bend_direction = dot(direction, params.start_join_params.normal);

  vec2 h = 0.5f * (normal + params.start_join_params.normal);
  float height = length(h);
  float k = 2.0f * params.width - height;
  normalize(h, h);

  if (params.join == JoinType::Round) {
    float angle = std::acosf(dot(normal, params.start_join_params.normal) / (params.width * params.width));
    int increments = (int)std::ceilf(angle / params.facet_angle);

    if (increments > 1) {
      uint32_t end_index = override_end_index ? *override_end_index : offset + increments - 1;
      float increment = angle / (float)increments;
      float width = params.width;
      vec2 bended_normal = params.start_join_params.normal;

      geo.push_vertex({ point, params.color, 0.0f, width });

      if (bend_direction < 0.0f) {
        geo.push_indices({ offset, params.start_join_params.index + 1, offset + 1 });

        increment = -increment;
        end_index++;
      } else {
        geo.push_indices({ offset, params.start_join_params.index, offset + 1 });

        negate(bended_normal, bended_normal);
        width = -width;
      }

      for (int i = 1; i < increments; ++i) {
        float angle_offset = (float)i * increment;
        float sin = std::sinf(angle_offset);
        float cos = std::cosf(angle_offset);
        vec2 p = {
          bended_normal.x * cos - bended_normal.y * sin,
          bended_normal.x * sin + bended_normal.y * cos
        };

        geo.push_vertex({ point + p, params.color, width });
        geo.push_indices({ offset, offset + i, offset + i - 1 });
      }

      geo.push_indices({ offset, end_index + 1, offset + increments - 1 });

      return;
    }
  }

  uint32_t end_index = override_end_index ? *override_end_index : offset;

  if (params.join == JoinType::Miter) {
    float cos = dot(normal, params.start_join_params.normal) / (params.width * params.width);
    float miter_length = params.width / std::sqrtf(0.5f * (1.0f + cos));

    if (miter_length < params.miter_limit * params.width) {
      vec2 miter = h * miter_length;

      if (bend_direction < 0.0f) {
        geo.push_vertex({ point + miter, params.color, 0.5f * (miter_length + params.width) });
        geo.push_indices({ params.start_join_params.index, params.start_join_params.index + 1, offset });
        geo.push_indices({ offset, end_index + 1, end_index + 2 });
      } else {
        geo.push_vertex({ point - miter, params.color, -0.5f * (miter_length + params.width) });
        geo.push_indices({ params.start_join_params.index, params.start_join_params.index + 1, offset });
        geo.push_indices({ offset, end_index + 1, end_index + 2 });
      }

      return;
    }
  }

  vec2 inset = h * k;

  if (bend_direction < 0.0f) {
    geo.push_vertex({ point - inset, params.color, -params.width });
    geo.push_indices({ params.start_join_params.index + 1, offset, end_index + 2 });
  } else {
    geo.push_vertex({ point + inset, params.color, params.width });
    geo.push_indices({ params.start_join_params.index, offset, end_index + 1 });
  }
}

void tessellate_cap(
  const TessellationParams& params,
  const vec2& point, const vec2& normal,
  bool is_end_cap, Geometry& geo
) {
  uint32_t offset = geo.offset();
  uint32_t end_index = params.start_join_params.index;

  if (params.cap == CapType::Round) {
    float angle = MATH_PI;
    int increments = (int)std::ceilf(angle / params.facet_angle);

    if (increments > 1) {
      float increment = angle / (float)increments;

      if (is_end_cap) {
        increment = -increment;
      } else {
        end_index = offset + increments + 1;
      }

      geo.reserve(increments + 1, (increments + 1) * 3);
      geo.push_vertex({ point, params.color, 0.0f, params.width });
      geo.push_indices({ offset, end_index + 1, offset + 1 });

      for (int i = 1; i <= increments; ++i) {
        float angle_offset = (float)i * increment;
        float sin = std::sinf(angle_offset);
        float cos = std::cosf(angle_offset);
        vec2 p = {
          normal.x * cos - normal.y * sin,
          normal.x * sin + normal.y * cos
        };

        geo.push_vertex({ point + p, params.color, params.width });
        geo.push_indices({ offset, offset + i, offset + i - 1 });
      }

      return;
    }
  }

  if (params.cap == CapType::Butt) {
    float cap_length = 120.0f * GEOMETRY_BUTT_CAP_LENGTH / params.zoom;
    vec2 normal_ortho = cap_length * orthogonal(normal) / (params.width);

    if (!is_end_cap) {
      end_index = offset + 4;
    } else {
      negate(normal_ortho, normal_ortho);
    }

    vec2 offset_normal = normal / params.width * (params.width - cap_length);

    vec2 A = point + offset_normal;
    vec2 B = point - offset_normal;

    geo.reserve(4, 6);
    geo.push_vertex({ A, params.color, 0.0f, cap_length });
    geo.push_vertex({ B, params.color, 0.0f, cap_length });
    geo.push_vertex({ B + normal_ortho, params.color, cap_length });
    geo.push_vertex({ A + normal_ortho, params.color, cap_length });
    geo.push_indices({ offset, offset + 1, offset + 2 });
    geo.push_indices({ offset, offset + 2, offset + 3 });

    return;
  }

  vec2 normal_ortho = orthogonal(normal);

  if (!is_end_cap) {
    end_index = offset + 4;
  } else {
    negate(normal_ortho, normal_ortho);
  }

  geo.reserve(4, 9);
  geo.push_vertex({ point, params.color, 0.0f, params.width });
  geo.push_vertex({ point + normal + normal_ortho, params.color, params.width });
  geo.push_vertex({ point - normal + normal_ortho, params.color, params.width });
  geo.push_vertex({ point - normal, params.color, params.width });
  geo.push_indices({ offset, offset + 1, offset + 2 });
  geo.push_indices({ offset, end_index + 1, offset + 1 });
  geo.push_indices({ offset, offset + 2, offset + 3 });
}
