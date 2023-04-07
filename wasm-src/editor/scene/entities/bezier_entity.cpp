#include "bezier_entity.h"

#define BEZIER_CALL(func, ...) type() == Type::Linear ? linear_##func(__VA_ARGS__) : cubic_##func(__VA_ARGS__)
#define STRICT_BEZIER_CALL(func, ...) strict_type() == Type::Linear ? linear_##func(__VA_ARGS__) : cubic_##func(__VA_ARGS__)

BezierEntity::Type BezierEntity::strict_type() const {
  if (type() == Type::Linear) {
    return Type::Linear;
  }

  vec2 p0 = m_start.transform()->position().get();
  Vec2Value* p1 = m_start.transform()->right();
  Vec2Value* p2 = m_end.transform()->left();
  vec2 p3 = m_end.transform()->position().get();

  int linear = 0;
  int handles = 0;
  float dist = squared_distance(p0, p3);

  if (p1) {
    if (collinear(p0, p0 + p1->get(), p3, GEOMETRY_MAX_INTERSECTION_ERROR * dist)) {
      linear++;
    }
    handles++;
  }
  if (p2) {
    if (collinear(p0, p3 + p2->get(), p3, GEOMETRY_MAX_INTERSECTION_ERROR * dist)) {
      linear++;
    }
    handles++;
  }

  return linear == handles ? Type::Linear : Type::Cubic;
}

std::vector<vec2> BezierEntity::extrema() const {
  std::vector<float> roots = STRICT_BEZIER_CALL(extrema);
  std::vector<vec2> extrema;

  for (float root : roots) {
    extrema.push_back(get(root));
  }

  return extrema;
}

std::vector<float> BezierEntity::inflections() const {
  return BEZIER_CALL(inflections);
};

std::vector<vec2> BezierEntity::turning_angles() const {
  return BEZIER_CALL(turning_angles);
}

std::vector<float> BezierEntity::triangulation_params(const RenderingOptions& options) const {
  return BEZIER_CALL(triangulation_params, options);
}

Box BezierEntity::bounding_box() const {
  std::vector<vec2> points = extrema();

  Box box = { std::numeric_limits<vec2>::max(), std::numeric_limits<vec2>::min() };

  for (vec2& point : points) {
    box.min = min(box.min, point);
    box.max = max(box.max, point);
  }

  return box;
}

Box BezierEntity::large_bounding_box() const {
  Box box = { std::numeric_limits<vec2>::max(), std::numeric_limits<vec2>::min() };

  std::vector<vec2> points = {
    p0(), p1(), p2(), p3()
  };

  for (vec2& point : points) {
    box.min = min(box.min, point);
    box.max = max(box.max, point);
  }

  return box;
}

vec2 BezierEntity::size() const {
  Box box = bounding_box();
  return box.max - box.min;
}

bool BezierEntity::clockwise(int resolution = 50) const {
  float sum = 0.0f;
  vec2 last = get(0.0f);

  for (int i = 1; i <= resolution; ++i) {
    vec2 point = get((float)i / (float)resolution);
    sum += (point.x - last.x) * (point.y + last.y);
    last = point;
  }

  return sum >= 0.0f;
}

vec2 BezierEntity::get(float t) const {
  return BEZIER_CALL(get, t);
}

vec2 BezierEntity::gradient(float t) const {
  return BEZIER_CALL(gradient, t);
}

BezierEntity::BezierPointDistance BezierEntity::closest_to(const vec2& position, int iterations) const {
  return (BEZIER_CALL(closest_to, position, iterations));
}

float BezierEntity::closest_t_to(const vec2& position, int iterations) const {
  return (BEZIER_CALL(closest_to, position, iterations)).t;
}

vec2 BezierEntity::closest_point_to(const vec2& position, int iterations) const {
  return (BEZIER_CALL(closest_to, position, iterations)).point;
}

float BezierEntity::distance_from(const vec2& position, int iterations) const {
  return std::sqrtf((BEZIER_CALL(closest_to, position, iterations)).sq_distance);
}

BezierEntity::BezierABC BezierEntity::abc(float t, const vec2& B) const {
  return BEZIER_CALL(abc, t, B);
}

std::vector<float> BezierEntity::line_intersections(const Box& line) const {
  Box box = { min(line.min, line.max), max(line.min, line.max) };
  if (!does_box_intersect_box(bounding_box(), box)) {
    return {};
  }

  return BEZIER_CALL(line_intersections, line);
}

std::vector<vec2> BezierEntity::line_intersection_points(const Box& line) const {
  std::vector<float> intersections = line_intersections(line);
  std::vector<vec2> points{};

  Box box = { min(line.min, line.max), max(line.min, line.max) };

  for (float intersection : intersections) {
    vec2 point = get(intersection);
    if (is_point_in_box(point, box, GEOMETRY_MAX_INTERSECTION_ERROR)) {
      points.push_back(point);
    }
  }

  return points;
}

bool BezierEntity::intersects_line(const Box& line) const {
  return line_intersection_points(line).size() > 0;
}

std::vector<vec2> BezierEntity::box_intersection_points(const Box& box) const {
  std::vector<Box> lines = lines_from_box(box);
  std::vector<vec2> points{};

  for (Box& line : lines) {
    std::vector<vec2> line_points = line_intersection_points(line);
    points.insert(points.end(), line_points.begin(), line_points.end());
  }

  return points;
}

bool BezierEntity::intersects_box(const Box& box) const {
  if (!does_box_intersect_box(box, bounding_box())) {
    return false;
  }

  if (is_point_in_box(p0(), box)) {
    return true;
  }

  return box_intersection_points(box).size() > 0;
}

void BezierEntity::tessellate(TessellationParams& params, Geometry& geo) const {
  STRICT_BEZIER_CALL(tessellate, params, geo);
}

void BezierEntity::tessellate_outline(TessellationParams& params, Geometry& geo) const {
  STRICT_BEZIER_CALL(tessellate_outline, params, geo);
}

void BezierEntity::render(const RenderingOptions& options) const {
  STRICT_BEZIER_CALL(render, options);
}

Entity* BezierEntity::entity_at(const vec2& position, bool lower_level, float threshold) {
  if (!is_point_in_box(position, bounding_box(), threshold)) {
    return nullptr;
  }

  if ((BEZIER_CALL(closest_to, position, 8)).sq_distance <= threshold * threshold) {
    return this;
  }

  return nullptr;
}

bool BezierEntity::is_masquerading_quadratic(vec2& B) const {
  VertexEntity& start = m_start;
  VertexTransformComponent* transform = start.transform();
  Vec2Value* d1 = m_start.transform()->right();
  Vec2Value* d2 = m_end.transform()->left();

  if (!d1 || !d2) {
    return false;
  }

  vec2 P0 = p0();
  vec2 P3 = p3();

  vec2 D1 = 1.5f * d1->get();
  vec2 D2 = 1.5f * d2->get();

  vec2 P1 = P0 + D1;
  vec2 P2 = P3 + D2;

  B = midpoint(P1, P2);

  // L1 norm
  vec2 diff = abs(P1 - P2);
  float mag = diff.x + diff.y;

  // Manhattan distance of D1 and D2.
  float edges = std::fabsf(D1.x) + std::fabsf(D1.y) + std::fabsf(D2.x) + std::fabsf(D2.y);

  return mag * 4096 <= edges;
}

std::vector<float> BezierEntity::linear_extrema() const {
  return { 0.0f, 1.0f };
}

// TODO: check if is masquerading quadratic in every cubic method
std::vector<float> BezierEntity::cubic_extrema() const {
  const vec2 A = p0();
  const vec2 B = p1();
  const vec2 C = p2();
  const vec2 D = p3();

  const vec2 a = 3.0f * (-A + 3.0f * B - 3.0f * C + D);
  const vec2 b = 6.0f * (A - 2.0f * B + C);
  const vec2 c = 3.0f * (B - A);

  std::vector<float> roots{ 0.0f, 1.0f };

  for (int i = 0; i < 2; i++) {
    if (is_almost_zero(a[i])) {
      if (is_almost_zero(b[i])) continue;

      float t = -c[i] / b[i];
      if (t > 0.0f && t < 1.0f) {
        roots.push_back(t);
      }

      continue;
    }

    float delta = b[i] * b[i] - 4.0f * a[i] * c[i];

    if (is_almost_zero(delta)) {
      roots.push_back(-b[i] / (2.0f * a[i]));
    } else if (delta < 0.0f) {
      continue;
    } else {
      float sqrt_delta = std::sqrtf(delta);

      float t1 = (-b[i] + sqrt_delta) / (2.0f * a[i]);
      float t2 = (-b[i] - sqrt_delta) / (2.0f * a[i]);

      if (t1 > 0.0f && t1 < 1.0f) {
        roots.push_back(t1);
      }
      if (t2 > 0.0f && t2 < 1.0f) {
        roots.push_back(t2);
      }
    }
  }

  return roots;
}

std::vector<float> BezierEntity::cubic_inflections() const {
  vec2 P1 = p1();
  vec2 P2 = p2();

  vec2 A = P1 - p0();
  vec2 B = P2 - P1 - A;
  vec2 C = p3() - P2 - A - 2.0f * B;

  float a = B.x * C.y - B.y * C.x;
  float b = A.x * C.y - A.y * C.x;
  float c = A.x * B.y - A.y * B.x;

  if (is_almost_zero(a)) {
    if (is_almost_zero(b)) {
      return { 0.0f, 1.0f };
    }

    float t = -c / b;
    if (t > 0.0f && t < 1.0f) {
      return { 0.0f, t, 1.0f };
    }

    return { 0.0f, 1.0f };
  }

  float delta = b * b - 4.0f * a * c;

  if (is_almost_zero(delta)) {
    float t = -b / (2.0f * a);
    if (t > 0.0f && t < 1.0f) {
      return { 0.0f, t, 1.0f };
    }
  } else if (delta > 0.0f) {
    float sqrt_delta = sqrtf(delta);
    float t1 = (-b + sqrt_delta) / (2.0f * a);
    float t2 = (-b - sqrt_delta) / (2.0f * a);

    if (t1 > t2) {
      std::swap(t1, t2);
    }

    std::vector<float> values = { 0.0f };
    if (t1 > 0.0f && t1 < 1.0f) {
      values.push_back(t1);
    }
    if (t2 > 0.0f && t2 < 1.0f) {
      values.push_back(t2);
    }

    values.push_back(1.0f);

    return values;
  }

  return { 0.0f, 1.0f };
}

std::vector<vec2> BezierEntity::cubic_turning_angles() const {
  std::vector<float> inflections;

  if (p0() == p1() || p2() == p3()) {
    inflections = { 0.0f, 1.0f };
  } else {
    inflections = cubic_inflections();
  }

  int inflections_num = (int)inflections.size();

  std::vector<vec2> turning_angles(inflections_num);

  for (int i = 0; i < inflections_num; i++) {
    float inflection = inflections[i];
    vec2 gradient = cubic_gradient(inflection);

    if (is_almost_zero(gradient, GEOMETRY_CURVE_ERROR)) {
      vec2 curvature = -(inflections[i] * 2.0f - 1.0f) * cubic_curvature(inflection);
      turning_angles[i] = { inflection, std::atan2f(curvature.y, curvature.x) };
    } else {
      turning_angles[i] = { inflection, std::atan2f(gradient.y, gradient.x) };
    }
  }

  return turning_angles;
}


// TODO: cache
vec2 BezierEntity::cubic_t_from_theta(float theta) const {
  vec2 P0 = p0();
  vec2 P1 = p1();
  vec2 P2 = p2();
  vec2 P3 = p3();

  vec2 A = 3.0f * (-P0 + 3.0f * P1 - 3.0f * P2 + P3);
  vec2 B = 6.0f * (P0 - 2.0f * P1 + P2);
  vec2 C = -3.0f * (P0 - P1);

  float tan = std::tanf(theta);

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
    float sqrt_delta = std::sqrtf(delta);
    float t1 = (-b + sqrt_delta) / (2.0f * a);
    float t2 = (-b - sqrt_delta) / (2.0f * a);

    return { t1, t2 };
  }

  return { -1.0f, -1.0f };
}

std::vector<float> BezierEntity::quadratic_triangulation_params(const vec2& B, const RenderingOptions& options) const {
  vec2 A = p0();
  vec2 C = p3();

  vec2 a = 2.0f * (A - 2.0f * B + C);
  vec2 b = 2.0f * (B - A);

  vec2 start = b;
  vec2 end = a + b;

  float start_angle = std::atan2f(start.y, start.x);
  float end_angle = std::atan2f(end.y, end.x);

  std::vector<float> triangulation_params{};

  float facet_angle = std::max(options.facet_angle, GEOMETRY_MIN_FACET_ANGLE) * 0.25f;

  float difference = end_angle - start_angle;
  int increments = std::max(std::abs((int)std::ceilf(difference / facet_angle)), 1);
  float increment = difference / (float)increments;

  triangulation_params.reserve(increments);
  triangulation_params.push_back(0.0f);

  for (int j = 1; j < increments; j++) {
    float theta = start_angle + (float)j * increment;
    float tan = std::tanf(theta);
    float t = (tan * b.x - b.y) / (a.y - tan * a.x);

    triangulation_params.push_back(t);
  }

  triangulation_params.push_back(1.0f);

  return triangulation_params;
}

std::vector<float> BezierEntity::cubic_triangulation_params(const RenderingOptions& options) const {
  if (vec2 B; is_masquerading_quadratic(B)) {
    return quadratic_triangulation_params(B, options);
  }

  std::vector<vec2> turning_angles = cubic_turning_angles();
  std::vector<float> triangulation_params{};

  float facet_angle = std::max(options.facet_angle, GEOMETRY_MIN_FACET_ANGLE) * 0.25f;
  float last_t = 0.0f;

  for (int i = 0; i < turning_angles.size() - 1; i++) {
    vec2 checkpoint = cubic_gradient(0.5f * (turning_angles[i].x + turning_angles[i + 1].x));
    float checkpoint_angle = std::atan2f(checkpoint.y, checkpoint.x);

    float difference = turning_angles[i + 1].y - turning_angles[i].y;

    float k1 = (checkpoint_angle - turning_angles[i].y) / difference;
    float k2 = (checkpoint_angle + MATH_TWO_PI - turning_angles[i].y) / difference;

    if (!(is_normalized(k1) || is_normalized(k2))) {
      difference += -sign(difference) * MATH_TWO_PI;
    }

    int increments = std::max(std::abs((int)std::ceilf(difference / facet_angle)), 1);
    float increment = difference / (float)increments;

    triangulation_params.reserve(increments);
    triangulation_params.push_back(last_t = turning_angles[i].x);

    for (int j = 1; j < increments; j++) {
      float theta = turning_angles[i].y + (float)j * increment;

      vec2 t_values = cubic_t_from_theta(theta);

      bool is_t1_bad = !is_in_range(t_values.x, last_t, turning_angles[i + 1].x, false);
      bool is_t2_bad = !is_in_range(t_values.y, last_t, turning_angles[i + 1].x, false);

      if (is_t1_bad || is_t2_bad) {
        if (is_t1_bad && is_t2_bad) {
          continue;
        }
        triangulation_params.push_back(last_t = (float)is_t1_bad * t_values.y + (float)is_t2_bad * t_values.x);
      } else if (t_values.x - last_t < t_values.y - last_t) {
        triangulation_params.push_back(last_t = t_values.x);
      } else {
        triangulation_params.push_back(last_t = t_values.y);
      }
    }
  }

  triangulation_params.push_back(1.0f);

  return triangulation_params;
}

vec2 BezierEntity::linear_get(float t) const {
  return lerp(p0(), p3(), t);
}

// TODO: Cache
vec2 BezierEntity::cubic_get(float t) const {
  vec2 A = p0();
  vec2 B = p1();
  vec2 C = p2();
  vec2 D = p3();

  vec2 a = -A + 3.0f * B - 3.0f * C + D;
  vec2 b = 3.0f * A - 6.0f * B + 3.0f * C;
  vec2 c = -3.0f * A + 3.0f * B;

  float t_sq = t * t;

  return a * t_sq * t + b * t_sq + c * t + A;
}

vec2 BezierEntity::linear_gradient(float t) const {
  return p3() - p0();
}

// TODO: Cache
vec2 BezierEntity::cubic_gradient(float t) const {
  vec2 A = p0();
  vec2 B = p1();
  vec2 C = p2();
  vec2 D = p3();

  vec2 a = 3.0f * (-A + 3.0f * B - 3.0f * C + D);
  vec2 b = 6.0f * (A - 2.0f * B + C);
  vec2 c = -3.0f * (A - B);

  return a * t * t + b * t + c;
}

// TODO: Cache
vec2 BezierEntity::cubic_curvature(float t) const {
  vec2 A = p0();
  vec2 B = p1();
  vec2 C = p2();
  vec2 D = p3();

  vec2 a = 6.0f * (-A + 3.0f * B - 3.0f * C + D);
  vec2 b = 6.0f * (A - 2.0f * B + C);

  return a * t + b;
}

BezierEntity::BezierPointDistance BezierEntity::linear_closest_to(const vec2& position, int iterations = 4) const {
  vec2 A = p0();
  vec2 B = p3();

  vec2 v = B - A;
  vec2 w = position - A;

  float len_sq = squared_length(v);

  float t = len_sq == 0 ? -1.0f : dot(v, w) / len_sq;

  if (t < 0.0f) {
    return { 0.0f, A, squared_length(w) };
  } else if (t > 1.0f) {
    return { 1.0f, B, squared_distance(B, position) };
  }

  vec2 point = A + t * v;

  return { t, point, squared_distance(point, position) };
}

// TODO: Cache
BezierEntity::BezierPointDistance BezierEntity::cubic_closest_to(const vec2& position, int iterations = 4) const {
  vec2 A = p0();
  vec2 B = p1();
  vec2 C = p2();
  vec2 D = p3();

  vec2 A_sq = A * A;
  vec2 B_sq = B * B;
  vec2 C_sq = C * C;
  vec2 D_sq = D * D;

  vec2 AB = A * B;
  vec2 AC = A * C;
  vec2 AD = A * D;
  vec2 BC = B * C;
  vec2 BD = B * D;
  vec2 CD = C * D;

  vec2 Apos = A * position;
  vec2 Bpos = B * position;
  vec2 Cpos = C * position;
  vec2 Dpos = D * position;

  float a = 0.0f;
  float b = 0.0f;
  float c = 0.0f;
  float d = 0.0f;
  float e = 0.0f;
  float f = 0.0f;

  for (int i = 0; i < 2; ++i) {
    a +=
      6.0f * A_sq[i] -
      36.0f * AB[i] +
      36.0f * AC[i] -
      12.0f * AD[i] +
      54.0f * B_sq[i] -
      108.0f * BC[i] +
      36.0f * BD[i] +
      54.0f * C_sq[i] -
      36.0f * CD[i] +
      6.0f * D_sq[i];

    b +=
      -30.0f * A_sq[i] +
      150.0f * AB[i] -
      120.0f * AC[i] +
      30.0f * AD[i] -
      180.0f * B_sq[i] +
      270.0f * BC[i] -
      60.0f * BD[i] -
      90.0f * C_sq[i] +
      30.0f * CD[i];

    c +=
      60.0f * A_sq[i] -
      240.0f * AB[i] +
      144.0f * AC[i] -
      24.0f * AD[i] +
      216.0f * B_sq[i] -
      216.0f * BC[i] +
      24.0f * BD[i] +
      36.0f * C_sq[i];

    d +=
      -60.0f * A_sq[i] +
      180.0f * AB[i] -
      72.0f * AC[i] +
      6.0f * AD[i] +
      6.0f * Apos[i] -
      108.0f * B_sq[i] +
      54.0f * BC[i] -
      18.0f * Bpos[i] +
      18.0f * Cpos[i] -
      6.0f * Dpos[i];

    e +=
      30.0f * A_sq[i] -
      60.0f * AB[i] +
      12.0f * AC[i] -
      12.0f * Apos[i] +
      18.0f * B_sq[i] +
      24.0f * Bpos[i] -
      12.0f * Cpos[i];

    f +=
      -6.0f * A_sq[i] + 6.0f * AB[i] + 6.0f * Apos[i] - 6.0f * Bpos[i];
  }

  BezierPointDistance params = { 0.0f, A, squared_distance(A, position) };

  for (int i = 0; i <= iterations; ++i) {
    float t = (float)i / (float)iterations;

    for (int j = 0; j < 5; ++j) {
      float t_sq = t * t;
      float t_cu = t_sq * t;
      float t_qu = t_cu * t;
      float t_qui = t_qu * t;

      t -= (a * t_qui + b * t_qu + c * t_cu + d * t_sq + e * t + f) /
        (5.0f * a * t_qu + 4.0f * b * t_cu + 3.0f * c * t_sq + 2.0f * d * t + e);
    }

    if (t < 0 || t > 1) continue;

    vec2 point = cubic_get(t);
    float sq_dist = squared_distance(point, position);

    if (sq_dist < params.sq_distance) {
      params.t = t;
      params.point = point;
      params.sq_distance = sq_dist;
    }
  }

  return params;
}

float BezierEntity::projection_ratio(float t) const {
  if (t == 0.0f || t == 1.0f) {
    return t;
  }

  float n = (float)std::pow(1.0f - t, 3);
  return n / (n + (float)std::pow(t, 3));
}

float BezierEntity::abc_ratio(float t) const {
  if (t == 0.0f || t == 1.0f) {
    return t;
  }

  float d = (float)std::pow(t, 3) + (float)std::pow(1.0f - t, 3);
  return std::fabsf((d - 1.0f) / d);
}

BezierEntity::BezierABC BezierEntity::linear_abc(float t, const vec2& B) const {
  vec2 point = linear_get(t);
  return { point, B, point };
}

BezierEntity::BezierABC BezierEntity::cubic_abc(float t, const vec2& B) const {
  float u = projection_ratio(t);
  float um = 1.0f - u;
  float s = abc_ratio(t);

  vec2 C = p0() * u + p3() * um;
  vec2 A = B + (B - C) / s;

  return { A, B, C };
}

std::vector<float> BezierEntity::linear_line_intersections(const Box& line) const {
  const vec2 A = p0();
  const vec2 B = p3();

  float den = line.max.x - line.min.x;

  if (is_almost_zero(den)) {
    float t = (line.min.x - A.x) / (B.x - A.x);
    if (t >= 0.0f && t <= 1.0f) {
      return { t };
    }

    return {};
  }

  float m = (line.max.y - line.min.y) / den;

  float t = (m * line.min.x - line.min.y + A.y - m * A.x) / (m * (B.x - A.x) + A.y - B.y);
  if (t >= 0.0f && t <= 1.0f) {
    return { t };
  }

  return {};
}

std::vector<float> BezierEntity::cubic_line_intersections(const Box& line) const {
  vec2 A = p0();
  vec2 B = p1();
  vec2 C = p2();
  vec2 D = p3();

  float a, b, c, d;
  float den = line.max.x - line.min.x;

  std::vector<float> roots{};

  if (is_almost_zero(den)) {
    a = -A.x + 3.0f * B.x - 3.0f * C.x + D.x;
    b = 3.0f * A.x - 6.0f * B.x + 3.0f * C.x;
    c = -3.0f * A.x + 3.0f * B.x;
    d = A.x - line.min.x;
  } else {
    float m = (line.max.y - line.min.y) / den;

    a = m * (-A.x + 3.0f * B.x - 3.0f * C.x + D.x) +
      1 * (A.y - 3.0f * B.y + 3.0f * C.y - D.y);
    b = m * (3.0f * A.x - 6.0f * B.x + 3.0f * C.x) +
      1 * (-3.0f * A.y + 6.0f * B.y - 3.0f * C.y);
    c = m * (-3.0f * A.x + 3.0f * B.x) + 1 * (3.0f * A.y - 3.0f * B.y);
    d = m * (A.x - line.min.x) - A.y + line.min.y;
  }

  // If the cubic bezier is an approximation of a quadratic curve, ignore the third degree term
  if (std::abs(a) < GEOMETRY_MAX_INTERSECTION_ERROR) {
    float delta = c * c - 4.0f * b * d;

    if (is_almost_zero(delta)) {
      roots.push_back(-c / (2.0f * b));
    } else if (delta > 0.0f) {
      float sqrt_delta = std::sqrtf(delta);

      float t1 = (-c + sqrt_delta) / (2.0f * b);
      float t2 = (-c - sqrt_delta) / (2.0f * b);

      if (t1 > 0.0f && t1 < 1.0f) {
        roots.push_back(t1);
      }
      if (t2 > 0.0f && t2 < 1.0f && t2 != t1) {
        roots.push_back(t2);
      }
    }

    return roots;
  }

  float a_sq = a * a;
  float b_sq = b * b;

  float p = (3.0f * a * c - b_sq) / (3.0f * a_sq);
  float q = (2.0f * b_sq * b - 9.0f * a * b * c + 27.0f * a_sq * d) / (27.0f * a_sq * a);

  if (is_almost_zero(p)) {
    roots.push_back(-std::cbrtf(q));
  } else if (is_almost_zero(q)) {
    if (p < 0.0f) {
      float sqrt_p = std::sqrtf(-p);
      roots.insert(roots.begin(), { 0.0f, sqrt_p, -sqrt_p });
    } else {
      roots.push_back(0.0f);
    }
  } else {
    float s = q * q / 4.0f + p * p * p / 27.0f;

    if (is_almost_zero(s)) {
      roots.insert(roots.begin(), { -1.5f * q / p, 3.0f * q / p });
    } else if (s > 0.0f) {
      float u = std::cbrtf(-0.5f * q - std::sqrtf(s));
      roots.push_back(u - p / (3.0f * u));
    } else {
      float u = 2.0f * std::sqrtf(-p / 3.0f);
      float t = std::acosf(3.0f * q / p / u) / 3.0f;
      float k = MATH_TWO_PI / 3.0f;

      roots.insert(roots.begin(), { u * std::cosf(t), u * std::cosf(t - k), u * std::cosf(t - 2.0f * k) });
    }
  }

  std::vector<float> parsed_roots{};

  for (float root : roots) {
    float t = root - b / (3.0f * a);
    if (t >= 0.0f && t <= 1.0f) {
      parsed_roots.push_back(t);
    }
  }

  return parsed_roots;
}

void BezierEntity::linear_tessellate(TessellationParams& params, Geometry& geo) const {
  vec2 A = params.offset + p0();
  vec2 B = params.offset + p3();

  float width_start = params.width * m_start.taper().get();
  float width_end = params.width * m_end.taper().get();

  vec2 direction = B - A;
  vec2 normal = orthogonal(direction);
  normalize(normal, normal);

  vec2 normal_start = normal * width_start;
  vec2 normal_end = normal * width_end;

  if (params.start_join) {
    tessellate_join(params, A, direction, normal_start, width_start, nullptr, geo);
  } else if (params.start_cap) {
    tessellate_cap(params, A, normal_start, false, width_start, geo);
  }

  uint32_t offset = geo.offset();

  if (params.is_first_segment) {
    params.end_join_params.direction = direction;
    params.end_join_params.normal = normal_start;
    params.end_join_params.index = offset - 1;
  }

  geo.push_vertices({
    { A - normal_start, params.color, -width_start }, { A + normal_start, params.color, width_start },
    { B - normal_end, params.color, -width_end }, { B + normal_end, params.color, width_end }
    });
  geo.push_indices({ offset, offset + 1, offset + 2, offset + 2, offset + 3, offset + 1 });

  params.start_join_params.direction = direction;
  params.start_join_params.normal = normal_end;
  params.start_join_params.index = offset + 2;

  if (params.end_join) {
    tessellate_join(params, B, params.end_join_params.direction, params.end_join_params.normal, width_end, &params.end_join_params.index, geo);
  } else if (params.end_cap) {
    tessellate_cap(params, B, normal_end, true, width_end, geo);
  }
}

void BezierEntity::cubic_tessellate(TessellationParams& params, Geometry& geo) const {
  std::vector<float> triangulation_params = cubic_triangulation_params(params.rendering_options);
  vec2 point, direction, normal;

  float width_start = params.width * m_start.taper().get();
  float width_end = params.width * m_end.taper().get();

  point = params.offset + p0();
  direction = cubic_gradient(0.0f);

  if (is_almost_zero(direction, GEOMETRY_CURVE_ERROR)) {
    direction = cubic_curvature(0.0f);
  }

  normal = orthogonal(direction);
  normalize_length(normal, width_start, normal);

  if (params.start_join) {
    tessellate_join(params, point, direction, normal, width_start, nullptr, geo);
  } else if (params.start_cap) {
    tessellate_cap(params, point, normal, false, width_start, geo);
  }

  uint32_t offset = geo.offset();

  if (params.is_first_segment) {
    params.end_join_params.direction = direction;
    params.end_join_params.normal = normal;
    params.end_join_params.index = offset - 1;
  }

  geo.push_vertices({ { point - normal, params.color, -width_start }, { point + normal, params.color, width_start } });
  offset += 2;

  for (size_t i = 1; i < triangulation_params.size() - 1; i++) {
    float t = triangulation_params[i];
    float width = lerp(width_start, width_end, t * t * t);
    // float width = lerp(width_start, width_end, t == 0
    //   ? 0
    //   : t == 1
    //   ? 1
    //   : t < 0.5f ? std::powf(2, 20 * t - 10) / 2
    //   : (2 - std::pow(2, -20 * t + 10)) / 2);

    point = params.offset + cubic_get(t);
    direction = cubic_gradient(t);
    normal = orthogonal(direction);
    normalize_length(normal, width, normal);

    geo.push_vertices({ { point - normal, params.color, -width }, { point + normal, params.color, width } });
    geo.push_indices({ offset - 2, offset - 1, offset, offset, offset + 1, offset - 1 });
    offset += 2;
  }

  point = params.offset + p3();
  direction = cubic_gradient(1.0f);

  if (is_almost_zero(direction, GEOMETRY_CURVE_ERROR)) {
    direction = -cubic_curvature(1.0f);
  }

  normal = orthogonal(direction);
  normalize_length(normal, width_end, normal);

  geo.push_vertices({ { point - normal, params.color, -width_end }, { point + normal, params.color, width_end } });
  geo.push_indices({ offset - 2, offset - 1, offset, offset, offset + 1, offset - 1 });

  params.start_join_params.direction = direction;
  params.start_join_params.normal = normal;
  params.start_join_params.index = offset;

  if (params.end_join) {
    tessellate_join(params, point, params.end_join_params.direction, params.end_join_params.normal, width_end, &params.end_join_params.index, geo);
  } else if (params.end_cap) {
    tessellate_cap(params, point, normal, true, width_end, geo);
  }
}

void BezierEntity::linear_tessellate_outline(TessellationParams& params, Geometry& geo) const {
  uint32_t offset = geo.offset();

  geo.push_vertices({ { params.offset + p0(), params.color }, { params.offset + p3(), params.color } });
  geo.push_indices({ offset, offset + 1 });
}

// TODO: move std::vector.reserve() outside
void BezierEntity::cubic_tessellate_outline(TessellationParams& params, Geometry& geo) const {
  std::vector<float> triangulation_params = cubic_triangulation_params(params.rendering_options);
  uint32_t offset = geo.offset();

  geo.reserve(triangulation_params.size(), (triangulation_params.size() - 1) * 2);
  for (int i = 0; i < triangulation_params.size() - 1; i++) {
    geo.push_vertices({ { params.offset + cubic_get(triangulation_params[i]), params.color } });
    geo.push_indices({ offset + i, offset + i + 1 });
  }

  geo.push_vertices({ { params.offset + p3(), params.color } });
}

void BezierEntity::linear_render(const RenderingOptions& options) const {
  Geometry geo;

  vec2 A = p0();
  vec2 B = p3();

  if (parent) {
    vec2 offset = parent->transform()->position().get();
    A += offset;
    B += offset;
  }

  float width = 2.0f / options.zoom;
  float dx = B.x - A.x;

  if (is_almost_zero(dx)) {
    vec2 offset{ width, 0.0f };
    geo.push_vertices({ A - offset, A + offset, B + offset, B - offset });
  } else {
    vec2 direction{ dx, B.y - A.y };
    normalize_length(direction, width, direction);
    orthogonal(direction, direction);

    geo.push_vertices({ A - direction, A + direction, B + direction, B - direction });
  }

  geo.push_indices({ 0, 1, 2, 2, 3, 0 });

  Renderer::draw(geo);
}

void BezierEntity::cubic_render(const RenderingOptions& options) const {
  vec2 offset{ 0.0f };
  if (parent) offset = parent->transform()->position().get();

  Geometry geo = stroke_curves({ Bezier{ offset + p0(), offset + p1(), offset + p2(), offset + p3() } });
  Renderer::draw(geo);

  Box box = bounding_box();
  box.min += offset;
  box.max += offset;

  Geometry box_geometry{};
  box_geometry.push_quad(box, vec4{ 0.0f, 1.0f, 0.5f, 0.2f });

  Renderer::draw(box_geometry);
}
