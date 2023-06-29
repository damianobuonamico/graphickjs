#include "segment.h"

#include "../../math/vector.h"

#include "../../utils/defines.h"

#define SEGMENT_CALL(func, ...) is_linear() ? linear_##func(__VA_ARGS__) : (is_cubic() ? cubic_##func(__VA_ARGS__) : quadratic_##func(__VA_ARGS__))

namespace Graphick::Renderer::Geometry {

  Segment::Segment(vec2 p0, vec2 p3) :
    m_kind(Kind::Linear),
    m_p0(std::make_shared<vec2>(p0)),
    m_p3(std::make_shared<vec2>(p3)) {}

  Segment::Segment(vec2 p0, vec2 p1, vec2 p3, bool is_quadratic) :
    m_kind(is_quadratic ? Kind::Quadratic : Kind::Cubic),
    m_p0(std::make_shared<vec2>(p0)),
    m_p1(std::make_unique<vec2>(p0)),
    m_p3(std::make_shared<vec2>(p3)) {}

  Segment::Segment(vec2 p0, vec2 p1, vec2 p2, vec2 p3) :
    m_kind(Kind::Cubic),
    m_p0(std::make_shared<vec2>(p0)),
    m_p1(std::make_unique<vec2>(p0)),
    m_p2(std::make_unique<vec2>(p0)),
    m_p3(std::make_shared<vec2>(p3)) {}

  Segment::Segment(Point p0, Point p3) :
    m_kind(Kind::Linear),
    m_p0(p0),
    m_p3(p3) {}

  Segment::Segment(Point p0, vec2 p1, Point p3, bool is_quadratic) :
    m_kind(is_quadratic ? Kind::Quadratic : Kind::Cubic),
    m_p0(p0),
    m_p1(std::make_unique<vec2>(p1)),
    m_p3(p3) {}

  Segment::Segment(Point p0, vec2 p1, vec2 p2, Point p3) :
    m_kind(Kind::Cubic),
    m_p0(p0),
    m_p1(std::make_unique<vec2>(p1)),
    m_p2(std::make_unique<vec2>(p2)),
    m_p3(p3) {}

  Segment::Segment(const Segment& other) :
    m_kind(other.m_kind),
    m_p0(other.m_p0),
    m_p1(other.m_p1 ? std::make_unique<vec2>(*other.m_p1) : nullptr),
    m_p2(other.m_p2 ? std::make_unique<vec2>(*other.m_p2) : nullptr),
    m_p3(other.m_p3) {}

  Segment::Segment(Segment&& other) noexcept :
    m_kind(other.m_kind),
    m_p0(other.m_p0),
    m_p1(std::move(other.m_p1)),
    m_p2(std::move(other.m_p2)),
    m_p3(other.m_p3) {}

  Segment& Segment::operator=(const Segment& other) {
    m_kind = other.m_kind;
    m_p0 = other.m_p0;
    m_p1 = other.m_p1 ? std::make_unique<vec2>(*other.m_p1) : nullptr;
    m_p2 = other.m_p2 ? std::make_unique<vec2>(*other.m_p2) : nullptr;
    m_p3 = other.m_p3;

    return *this;
  }

  Segment& Segment::operator=(Segment&& other) noexcept {
    m_kind = other.m_kind;
    m_p0 = other.m_p0;
    m_p1 = std::move(other.m_p1);
    m_p2 = std::move(other.m_p2);
    m_p3 = other.m_p3;

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
      if (Math::collinear(p0(), *m_p1, p3(), error)) {
        linear++;
      }
      handles++;
    }

    if (m_p2) {
      if (Math::collinear(p0(), *m_p2, p3(), error)) {
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

    vec2 d1 = 1.5f * (*m_p1 - p0());
    vec2 d2 = 1.5f * (*m_p2 - p3());

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
    rect rect{};
    std::vector<vec2> points = { p0(), p3() };

    if (m_p1) {
      points.push_back(*m_p1);
    }
    if (m_p2) {
      points.push_back(*m_p2);
    }

    for (vec2& point : points) {
      Math::min(rect.min, point, rect.min);
      Math::max(rect.max, point, rect.max);
    }

    return rect;
  }

  vec2 Segment::size() const {
    return bounding_rect().size();
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
    const vec2 A = p0();
    const vec2 B = p1();
    const vec2 C = p2();
    const vec2 D = p3();

    const vec2 a = 3.0f * (-A + 3.0f * B - 3.0f * C + D);
    const vec2 b = 6.0f * (A - 2.0f * B + C);
    const vec2 c = 3.0f * (B - A);

    std::vector<float> roots = { 0.0f, 1.0f };

    for (int i = 0; i < 2; i++) {
      if (Math::is_almost_zero(a[i])) {
        if (Math::is_almost_zero(b[i])) continue;

        float t = -c[i] / b[i];
        if (t > 0.0f && t < 1.0f) {
          roots.push_back(t);
        }

        continue;
      }

      float delta = b[i] * b[i] - 4.0f * a[i] * c[i];

      if (Math::is_almost_zero(delta)) {
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

}
