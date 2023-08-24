#include "internal.h"

#include "../../math/vector.h"

#include "../../utils/defines.h"

#define SEGMENT_CALL(func, ...) (is_linear() ? linear_##func(__VA_ARGS__) : cubic_##func(__VA_ARGS__))

namespace Graphick::Renderer::Geometry::Internal {

  /* -- SegmentInternal -- */

  SegmentInternal::SegmentInternal(vec2 p0, vec2 p3) :
    m_kind(Kind::Linear),
    m_p0(std::make_shared<vec2>(p0)),
    m_p3(std::make_shared<vec2>(p3)) {}

  SegmentInternal::SegmentInternal(vec2 p0, vec2 p1, vec2 p3, bool is_p1) :
    m_kind(Kind::Cubic),
    m_p0(std::make_shared<vec2>(p0)),
    m_p1(std::make_shared<vec2>(p1)),
    m_p3(std::make_shared<vec2>(p3)) {}

  SegmentInternal::SegmentInternal(vec2 p0, vec2 p1, vec2 p2, vec2 p3) :
    m_kind(Kind::Cubic),
    m_p0(std::make_shared<vec2>(p0)),
    m_p1(std::make_shared<vec2>(p1)),
    m_p2(std::make_shared<vec2>(p2)),
    m_p3(std::make_shared<vec2>(p3)) {}

  SegmentInternal::SegmentInternal(ControlPoint p0, ControlPoint p3) :
    m_kind(Kind::Linear),
    m_p0(p0),
    m_p3(p3) {}

  SegmentInternal::SegmentInternal(ControlPoint p0, vec2 p1, ControlPoint p3, bool is_p1) :
    m_kind(Kind::Cubic),
    m_p0(p0),
    m_p1(std::make_shared<vec2>(p1)),
    m_p3(p3) { }

  SegmentInternal::SegmentInternal(ControlPoint p0, vec2 p1, vec2 p2, ControlPoint p3) :
    m_kind(Kind::Cubic),
    m_p0(p0),
    m_p1(std::make_shared<vec2>(p1)),
    m_p2(std::make_shared<vec2>(p2)),
    m_p3(p3) {}

  SegmentInternal::SegmentInternal(const SegmentInternal& other) :
    m_kind(other.m_kind),
    m_p0(other.m_p0),
    m_p1(other.m_p1 ? std::make_shared<vec2>(*other.m_p1) : nullptr),
    m_p2(other.m_p2 ? std::make_shared<vec2>(*other.m_p2) : nullptr),
    m_p3(other.m_p3) {}

  SegmentInternal::SegmentInternal(SegmentInternal&& other) noexcept :
    m_kind(other.m_kind),
    m_p0(other.m_p0),
    m_p1(std::move(other.m_p1)),
    m_p2(std::move(other.m_p2)),
    m_p3(other.m_p3) {}

  SegmentInternal& SegmentInternal::operator=(const SegmentInternal& other) {
    m_kind = other.m_kind;
    m_p0 = other.m_p0;
    m_p1 = other.m_p1 ? std::make_shared<vec2>(*other.m_p1) : nullptr;
    m_p2 = other.m_p2 ? std::make_shared<vec2>(*other.m_p2) : nullptr;
    m_p3 = other.m_p3;

    return *this;
  }

  SegmentInternal& SegmentInternal::operator=(SegmentInternal&& other) noexcept {
    m_kind = other.m_kind;
    m_p0 = other.m_p0;
    m_p1 = std::move(other.m_p1);
    m_p2 = std::move(other.m_p2);
    m_p3 = other.m_p3;

    return *this;
  }

  vec2 SegmentInternal::get(const float t) const {
    return SEGMENT_CALL(get, t);
  }

  rect SegmentInternal::bounding_rect() const {
    rect rect{};
    std::vector<vec2> points = extrema();

    for (vec2& point : points) {
      Math::min(rect.min, point, rect.min);
      Math::max(rect.max, point, rect.max);
    }

    return rect;
  }

  vec2 SegmentInternal::size() const {
    return bounding_rect().size();
  }

  vec2 SegmentInternal::linear_get(const float t) const {
    return Math::lerp(p0(), p3(), t);
  }

  vec2 SegmentInternal::cubic_get(const float t) const {
    return Math::bezier(p0(), p1(), p2(), p3(), t);
  }

  std::vector<vec2> SegmentInternal::extrema() const {
    std::vector <vec2> extrema;

    for (float t : SEGMENT_CALL(extrema)) {
      extrema.push_back(get(t));
    }

    return extrema;
  }

  std::vector<float> SegmentInternal::linear_extrema() const {
    return { 0.0f, 1.0f };
  }

  std::vector<float> SegmentInternal::cubic_extrema() const {
    return Math::bezier_extrema(p0(), p1(), p2(), p3());
  }

  /* -- PathInternal -- */


  void PathInternal::move_to(vec2 p) {
    m_last_point = std::make_shared<vec2>(p);
  }

  void PathInternal::line_to(vec2 p) {
    SegmentInternal::ControlPoint point = std::make_shared<vec2>(p);
    m_segments.emplace_back(m_last_point, point);
    m_last_point = point;
  }

  void PathInternal::cubic_to(vec2 p1, vec2 p2, vec2 p3) {
    SegmentInternal::ControlPoint point = std::make_shared<vec2>(p3);
    m_segments.emplace_back(m_last_point, p1, p2, point);
    m_last_point = point;
  }

  void PathInternal::ellipse(vec2 c, vec2 radius) {
    vec2 top_left = c - radius;
    vec2 bottom_right = c + radius;

    vec2 cp = radius * GEOMETRY_CIRCLE_RATIO;

    move_to({ cp.x, top_left.y });
    cubic_to({ c.x + cp.x, top_left.y }, { bottom_right.x, c.y - cp.y }, { bottom_right.x, c.y });
    cubic_to({ bottom_right.x, c.y + cp.y }, { c.x + cp.x, bottom_right.y }, { c.x, bottom_right.y });
    cubic_to({ c.x - cp.x, bottom_right.y }, { top_left.x, c.y + cp.y }, { top_left.x, c.y });
    cubic_to({ top_left.x, c.y - cp.y }, { c.x - cp.x, top_left.y }, { c.x, top_left.y });
    close();
  }

  void PathInternal::circle(vec2 c, float radius) {
    ellipse(c, { radius, radius });
  }

  void PathInternal::rect(vec2 p, vec2 size, bool centered) {
    if (centered) {
      p -= size * 0.5f;
    }

    move_to(p);
    line_to(p + vec2{ size.x, 0.0f });
    line_to(p + size);
    line_to(p + vec2{ 0.0f, size.y });
    close();
  }

  void PathInternal::round_rect(vec2 p, vec2 size, float radius, bool centered) {
    if (centered) {
      p -= size * 0.5f;
    }

    if (radius > size.x * 0.5f) radius = size.x * 0.5f;
    if (radius > size.y * 0.5f) radius = size.y * 0.5f;

    move_to({ p.x + radius, p.y });
    line_to({ p.x + size.x - radius, p.y });
    cubic_to({ p.x + size.x - radius * GEOMETRY_CIRCLE_RATIO, p.y }, { p.x + size.x, p.y + radius * GEOMETRY_CIRCLE_RATIO }, { p.x + size.x, p.y + radius });
    line_to({ p.x + size.x, p.y + size.y - radius });
    cubic_to({ p.x + size.x, p.y + size.y - radius * GEOMETRY_CIRCLE_RATIO }, { p.x + size.x - radius * GEOMETRY_CIRCLE_RATIO, p.y + size.y }, { p.x + size.x - radius, p.y + size.y });
    line_to({ p.x + radius, p.y + size.y });
    cubic_to({ p.x + radius * GEOMETRY_CIRCLE_RATIO, p.y + size.y }, { p.x, p.y + size.y - radius * GEOMETRY_CIRCLE_RATIO }, { p.x, p.y + size.y - radius });
    line_to({ p.x, p.y + radius });
    cubic_to({ p.x, p.y + radius * GEOMETRY_CIRCLE_RATIO }, { p.x + radius * GEOMETRY_CIRCLE_RATIO, p.y }, { p.x + radius, p.y });
    close();
  }

  void PathInternal::close() {
    if (m_segments.size() < 2) return;

    SegmentInternal& first_segment = m_segments.front();
    SegmentInternal& last_segment = m_segments.back();

    if (is_almost_equal(last_segment.p3(), first_segment.p0())) {
      first_segment.m_p0 = last_segment.m_p3;
    } else {
      m_segments.emplace_back(m_last_point, first_segment.m_p0);
      m_last_point = m_segments.front().m_p0;
    }

    m_closed = true;
  }

  Math::rect PathInternal::bounding_rect() const {
    Math::rect rect{};

    for (const auto& segment : m_segments) {
      Math::rect segment_rect = segment.bounding_rect();

      min(rect.min, segment_rect.min, rect.min);
      max(rect.max, segment_rect.max, rect.max);
    }

    return rect;
  }

}
