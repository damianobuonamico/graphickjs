#include "bezier_entity.h"
#include "../../../renderer/geometry/stroker.h"

#define BEZIER_CALL(func, ...) type() == Type::Linear ? linear_##func(__VA_ARGS__) : cubic_##func(__VA_ARGS__)

std::vector<vec2> BezierEntity::extrema() const {
  std::vector<float> roots = BEZIER_CALL(extrema);
  std::vector<vec2> extrema;

  for (float root : roots) {
    extrema.push_back(get(root));
  }

  return extrema;
}

std::vector<float> BezierEntity::inflections() const {
  return BEZIER_CALL(inflections);
};

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

float BezierEntity::closest_t_to(const vec2& position, int iterations) const {
  return (BEZIER_CALL(closest_to, position, iterations)).t;
}

vec2 BezierEntity::closest_point_to(const vec2& position, int iterations) const {
  return (BEZIER_CALL(closest_to, position, iterations)).point;
}

float BezierEntity::distance_from(const vec2& position, int iterations) const {
  return std::sqrtf((BEZIER_CALL(closest_to, position, iterations)).sq_distance);
}

std::vector<float> BezierEntity::line_intersections(const Box& line) const {
  if (!does_box_intersect_box(bounding_box(), line)) return {};

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
  std::vector<Box> lines = get_lines_from_box(box);
  std::vector<vec2> points{};

  for (Box& line : lines) {
    std::vector<vec2> line_points = line_intersection_points(line);
    points.insert(points.end(), line_points.begin(), line_points.end());
  }

  return points;
}

bool BezierEntity::intersects_box(const Box& box) const {
  return box_intersection_points(box).size() > 0;
}

void BezierEntity::render(float zoom) const {
  BEZIER_CALL(render, zoom);
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
      float u = std::cbrtf(-q / 2.0f - std::sqrtf(s));
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

void BezierEntity::linear_render(float zoom) const {
  Geometry geo;

  vec2 A = p0();
  vec2 B = p3();

  if (parent) {
    vec2 offset = parent->transform().position().get();
    A += offset;
    B += offset;
  }

  float width = 2.0f / zoom;
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

void BezierEntity::cubic_render(float zoom) const {
  vec2 offset{ 0.0f };
  if (parent) offset = parent->transform().position().get();

  Geometry geo = stroke_curves({ Bezier{ offset + p0(), offset + p1(), offset + p2(), offset + p3() } });
  Renderer::draw(geo);

  Box box = bounding_box();
  box.min += offset;
  box.max += offset;

  Geometry box_geometry{};
  box_geometry.push_quad(box, vec4{ 0.0f, 1.0f, 0.5f, 0.2f });

  Renderer::draw(box_geometry);
}
