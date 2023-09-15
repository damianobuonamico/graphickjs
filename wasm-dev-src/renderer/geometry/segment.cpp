#include "segment.h"

#include "../../math/math.h"
#include "../../math/vector.h"

#include "../../utils/defines.h"
#include "../../utils/console.h"

#define SEGMENT_CALL(func, ...) (is_linear() ? linear_##func(__VA_ARGS__) : (is_cubic() ? cubic_##func(__VA_ARGS__) : quadratic_##func(__VA_ARGS__)))

namespace Graphick::Renderer::Geometry {

  Segment::Segment(vec2 p0, vec2 p3) :
    m_kind(Kind::Linear),
    m_p0(std::make_shared<ControlPoint>(p0)),
    m_p3(std::make_shared<ControlPoint>(p3)) {}

  Segment::Segment(vec2 p0, vec2 p1, vec2 p3, bool is_quadratic) :
    m_kind(is_quadratic ? Kind::Quadratic : Kind::Cubic),
    m_p0(std::make_shared<ControlPoint>(p0)),
    m_p1(std::make_shared<History::Vec2Value>(p1)),
    m_p3(std::make_shared<ControlPoint>(p3))
  {
    m_p0->set_relative_handle(m_p1);
  }

  Segment::Segment(vec2 p0, vec2 p1, vec2 p2, vec2 p3) :
    m_kind(Kind::Cubic),
    m_p0(std::make_shared<ControlPoint>(p0)),
    m_p1(std::make_shared<History::Vec2Value>(p1)),
    m_p2(std::make_shared<History::Vec2Value>(p2)),
    m_p3(std::make_shared<ControlPoint>(p3))
  {
    m_p0->set_relative_handle(m_p1);
    m_p3->set_relative_handle(m_p2);
  }

  Segment::Segment(ControlPointVertex p0, ControlPointVertex p3) :
    m_kind(Kind::Linear),
    m_p0(p0),
    m_p3(p3) {}

  Segment::Segment(ControlPointVertex p0, vec2 p1, ControlPointVertex p3, bool is_quadratic) :
    m_kind(is_quadratic ? Kind::Quadratic : Kind::Cubic),
    m_p0(p0),
    m_p1(std::make_shared<History::Vec2Value>(p1)),
    m_p3(p3)
  {
    m_p0->set_relative_handle(m_p1);
  }

  Segment::Segment(ControlPointVertex p0, vec2 p1, vec2 p2, ControlPointVertex p3) :
    m_kind(Kind::Cubic),
    m_p0(p0),
    m_p1(std::make_shared<History::Vec2Value>(p1)),
    m_p2(std::make_shared<History::Vec2Value>(p2)),
    m_p3(p3)
  {
    m_p0->set_relative_handle(m_p1);
    m_p3->set_relative_handle(m_p2);
  }

  Segment::Segment(const Segment& other) :
    m_kind(other.m_kind),
    m_p0(other.m_p0),
    m_p1(other.m_p1 ? std::make_shared<History::Vec2Value>(*other.m_p1) : nullptr),
    m_p2(other.m_p2 ? std::make_shared<History::Vec2Value>(*other.m_p2) : nullptr),
    m_p3(other.m_p3)
  {
    m_p0->set_relative_handle(m_p1);
    m_p3->set_relative_handle(m_p2);
  }

  Segment::Segment(Segment&& other) noexcept :
    m_kind(other.m_kind),
    m_p0(other.m_p0),
    m_p1(std::move(other.m_p1)),
    m_p2(std::move(other.m_p2)),
    m_p3(other.m_p3)
  {
    m_p0->set_relative_handle(m_p1);
    m_p3->set_relative_handle(m_p2);
  }

  Segment& Segment::operator=(const Segment& other) {
    m_kind = other.m_kind;
    m_p0 = other.m_p0;
    m_p1 = other.m_p1 ? std::make_shared<History::Vec2Value>(*other.m_p1) : nullptr;
    m_p2 = other.m_p2 ? std::make_shared<History::Vec2Value>(*other.m_p2) : nullptr;
    m_p3 = other.m_p3;

    m_p0->set_relative_handle(m_p1);
    m_p3->set_relative_handle(m_p2);

    return *this;
  }

  Segment& Segment::operator=(Segment&& other) noexcept {
    m_kind = other.m_kind;
    m_p0 = other.m_p0;
    m_p1 = std::move(other.m_p1);
    m_p2 = std::move(other.m_p2);
    m_p3 = other.m_p3;

    m_p0->set_relative_handle(m_p1);
    m_p3->set_relative_handle(m_p2);

    return *this;
  }

  bool Segment::is_masquerading_linear() const {
    if (kind() == Kind::Linear) return true;

    float error = Math::squared_distance(p0(), p3()) * GEOMETRY_MAX_INTERSECTION_ERROR;

    if (kind() == Kind::Quadratic) {
      return Math::collinear(p0(), p1(), p3(), error);
    }

    int linear = 0;
    int handles = 0;

    if (m_p1) {
      if (Math::collinear(p0(), m_p1->get(), p3(), error)) {
        linear++;
      }
      handles++;
    }

    if (m_p2) {
      if (Math::collinear(p0(), m_p2->get(), p3(), error)) {
        linear++;
      }
      handles++;
    }

    return linear == handles;
  }

  bool Segment::is_masquerading_quadratic(vec2& new_p1) const {
    if (kind() == Kind::Linear) return false;
    if (kind() == Kind::Quadratic) return true;

    if (!m_p1 || !m_p2) return false;

    vec2 d1 = 1.5f * (m_p1->get() - p0());
    vec2 d2 = 1.5f * (m_p2->get() - p3());

    vec2 p1 = p0() + d1;
    vec2 p2 = p3() + d2;

    new_p1 = Math::midpoint(p1, p2);

    // L1 norm
    vec2 diff = Math::abs(p1 - p2);
    float mag = diff.x + diff.y;

    // Manhattan distance of d1 and d2.
    float edges = std::fabsf(d1.x) + std::fabsf(d1.y) + std::fabsf(d2.x) + std::fabsf(d2.y);
    return mag * 4096 <= edges;
  }

  vec2 Segment::get(const float t) const {
    return SEGMENT_CALL(get, t);
  }

  rect Segment::bounding_rect() const {
    rect rect{};
    std::vector<vec2> points = extrema();

    for (vec2& point : points) {
      Math::min(rect.min, point, rect.min);
      Math::max(rect.max, point, rect.max);
    }

    return rect;
  }

  rect Segment::large_bounding_rect() const {
    rect rect = bounding_rect();

    if (m_p1) {
      vec2 p1 = m_p1->get();
      Math::min(rect.min, p1, rect.min);
      Math::max(rect.max, p1, rect.max);
    }
    if (m_p2) {
      vec2 p2 = m_p2->get();
      Math::min(rect.min, p2, rect.min);
      Math::max(rect.max, p2, rect.max);
    }

    return rect;
  }

  vec2 Segment::size() const {
    return bounding_rect().size();
  }

  bool Segment::is_inside(const vec2 position, bool deep_search, float threshold) const {
    if (!Math::is_point_in_rect(position, deep_search ? large_bounding_rect() : bounding_rect(), threshold)) {
      return false;
    }

    if (deep_search) {
      if (m_p1 != nullptr && Math::is_point_in_circle(position, p1(), threshold)) {
        return true;
      }
      if (m_p2 != nullptr && Math::is_point_in_circle(position, p2(), threshold)) {
        return true;
      }
    }

    auto a = SEGMENT_CALL(closest_to, position, 8).sq_distance;

    return a <= threshold * threshold;
  }

  bool Segment::intersects(const Math::rect& rect) const {
    Math::rect bounding_rect = this->bounding_rect();

    if (!Math::does_rect_intersect_rect(rect, bounding_rect)) return false;
    if (Math::is_point_in_rect(p0(), rect) || Math::is_point_in_rect(p3(), rect)) return true;

    for (Math::rect& line : Math::lines_from_rect(rect)) {
      if (intersects_line(line)) return true;
    }

    return false;
  }

  bool Segment::intersects(const Math::rect& rect, const bool found, std::unordered_set<uuid>& vertices) const {
    Math::rect bounding_rect = this->bounding_rect();

    if (found) {
      if (Math::is_point_in_rect(p0(), rect)) vertices.insert(m_p0->id);
      if (Math::is_point_in_rect(p3(), rect)) vertices.insert(m_p3->id);

      return false;
    }

    if (!Math::does_rect_intersect_rect(rect, bounding_rect)) return false;

    bool p0_inside = Math::is_point_in_rect(p0(), rect);
    bool p3_inside = Math::is_point_in_rect(p3(), rect);

    if (p0_inside) vertices.insert(m_p0->id);
    if (p3_inside) vertices.insert(m_p3->id);
    if (p0_inside || p3_inside) return true;

    for (Math::rect& line : Math::lines_from_rect(rect)) {
      if (intersects_line(line)) return true;
    }

    return false;
  }

  bool Segment::intersects_line(const Math::rect& line) const {
    return line_intersection_points(line).has_value();
  }

  vec2 Segment::linear_get(const float t) const {
    return Math::lerp(p0(), p3(), t);
  }

  vec2 Segment::quadratic_get(const float t) const {
    vec2 A = p0();
    vec2 B = p1();
    vec2 C = p2();

    vec2 a = A - 2.0f * B + C;
    vec2 b = 2.0f * (B - A);

    float t_sq = t * t;

    return a * t_sq + b * t + A;
  }

  vec2 Segment::cubic_get(const float t) const {
    return Math::bezier(p0(), p1(), p2(), p3(), t);
  }

  std::vector<vec2> Segment::extrema() const {
    std::vector <vec2> extrema;

    for (float t : SEGMENT_CALL(extrema)) {
      extrema.push_back(get(t));
    }

    return extrema;
  }

  std::vector<float> Segment::linear_extrema() const {
    return { 0.0f, 1.0f };
  }

  std::vector<float> Segment::quadratic_extrema() const {
    const vec2 A = p0();
    const vec2 B = p1();
    const vec2 C = p2();

    const vec2 a = A - 2.0f * B + C;
    const vec2 b = 2.0f * (B - A);

    std::vector<float> roots = { 0.0f, 1.0f };

    for (int i = 0; i < 2; i++) {
      if (Math::is_almost_zero(a[i] - b[i])) continue;
      roots.push_back(a[i] / (a[i] - b[i]));
    }

    return roots;
  }

  std::vector<float> Segment::cubic_extrema() const {
    return Math::bezier_extrema(p0(), p1(), p2(), p3());
  }

  Segment::SegmentPointDistance Segment::linear_closest_to(const vec2 position, int iterations) const {
    vec2 A = p0();
    vec2 B = p3();

    vec2 v = B - A;
    vec2 w = position - A;

    float len_sq = Math::squared_length(v);

    float t = len_sq == 0 ? -1.0f : Math::dot(v, w) / len_sq;

    if (t < 0.0f) {
      return { 0.0f, A, Math::squared_length(w) };
    } else if (t > 1.0f) {
      return { 1.0f, B, Math::squared_distance(B, position) };
    }

    vec2 point = A + t * v;

    return { t, point, Math::squared_distance(point, position) };
  }

  Segment::SegmentPointDistance Segment::quadratic_closest_to(const vec2 position, int iterations) const {
    // TODO: implement
    return { 0.0f, p0(), Math::squared_distance(p0(), position) };
  }

  Segment::SegmentPointDistance Segment::cubic_closest_to(const vec2 position, int iterations) const {
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

    SegmentPointDistance params = { 0.0f, A, Math::squared_distance(A, position) };

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
      float sq_dist = Math::squared_distance(point, position);

      if (sq_dist < params.sq_distance) {
        params.t = t;
        params.point = point;
        params.sq_distance = sq_dist;
      }
    }

    return params;
  }

  std::optional<std::vector<vec2>> Segment::line_intersection_points(const Math::rect& line) const {
    std::vector<float> intersections = line_intersections(line);
    if (intersections.empty()) return std::nullopt;

    std::vector<vec2> points{};
    Math::rect rect = { min(line.min, line.max), max(line.min, line.max) };

    for (float intersection : intersections) {
      vec2 point = get(intersection);
      if (Math::is_point_in_rect(point, rect, GEOMETRY_MAX_INTERSECTION_ERROR)) {
        points.push_back(point);
      }
    }

    if (points.empty()) return std::nullopt;
    return points;
  }

  std::vector<float> Segment::line_intersections(const rect& line) const {
    return SEGMENT_CALL(line_intersections, line);
  }

  std::vector<float> Segment::linear_line_intersections(const rect& line) const {
    const vec2 A = p0();
    const vec2 B = p3();

    float den = line.max.x - line.min.x;

    if (Math::is_almost_zero(den)) {
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

  std::vector<float> Segment::quadratic_line_intersections(const rect& line) const {
    return {};
  }

  std::vector<float> Segment::cubic_line_intersections(const rect& line) const {
    vec2 A = p0();
    vec2 B = p1();
    vec2 C = p2();
    vec2 D = p3();

    float a, b, c, d;
    float den = line.max.x - line.min.x;

    std::vector<float> roots{};

    if (Math::is_almost_zero(den)) {
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

      if (Math::is_almost_zero(delta)) {
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

    if (Math::is_almost_zero(p)) {
      roots.push_back(-std::cbrtf(q));
    } else if (Math::is_almost_zero(q)) {
      if (p < 0.0f) {
        float sqrt_p = std::sqrtf(-p);
        roots.insert(roots.begin(), { 0.0f, sqrt_p, -sqrt_p });
      } else {
        roots.push_back(0.0f);
      }
    } else {
      float s = q * q / 4.0f + p * p * p / 27.0f;

      if (Math::is_almost_zero(s)) {
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

}
