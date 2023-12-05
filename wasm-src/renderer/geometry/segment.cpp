#include "segment.h"

#include "../../math/math.h"
#include "../../math/vector.h"
#include "../../math/matrix.h"

#include "../../history/command_history.h"
#include "../../history/commands.h"

#include "../../utils/defines.h"
#include "../../utils/console.h"

#define SEGMENT_CALL(func, ...) (is_linear() ? linear_##func(__VA_ARGS__) : (is_cubic() ? cubic_##func(__VA_ARGS__) : quadratic_##func(__VA_ARGS__)))

namespace Graphick::History {

  class CreateHandleCommand : public Command {
  public:
    CreateHandleCommand(Renderer::Geometry::Segment* segment, std::shared_ptr<History::Vec2Value> handle, bool is_p1)
      : Command(Type::CreateHandle), m_segment(segment), m_handle(handle), m_is_p1(is_p1) {}

    inline virtual void execute() override {
      if (m_is_p1) {
        m_segment->m_p1 = m_handle;
      } else {
        m_segment->m_p2 = m_handle;
      }

      m_segment->recalculate_kind();
    }

    inline virtual void undo() override {
      if (m_is_p1) {
        m_segment->m_p1.reset();
      } else {
        m_segment->m_p2.reset();
      }

      m_segment->recalculate_kind();
    }

    inline virtual bool merge_with(std::unique_ptr<Command>& command) override {
      return false;
    }

    inline virtual uintptr_t pointer() override {
      return reinterpret_cast<uintptr_t>(m_handle.get());
    }
  private:
    Renderer::Geometry::Segment* m_segment;
    std::shared_ptr<History::Vec2Value> m_handle;
    bool m_is_p1;
  };

  class RemoveHandleCommand : public Command {
  public:
    RemoveHandleCommand(Renderer::Geometry::Segment* segment, std::shared_ptr<History::Vec2Value> handle, bool is_p1)
      : Command(Type::RemoveHandle), m_segment(segment), m_handle(handle), m_is_p1(is_p1) {}

    inline virtual void execute() override {
      if (m_is_p1) {
        m_segment->m_p1.reset();
      } else {
        m_segment->m_p2.reset();
      }

      m_segment->recalculate_kind();
    }

    inline virtual void undo() override {
      if (m_is_p1) {
        m_segment->m_p1 = m_handle;
      } else {
        m_segment->m_p2 = m_handle;
      }

      m_segment->recalculate_kind();
    }

    inline virtual bool merge_with(std::unique_ptr<Command>& command) override {
      return false;
    }

    inline virtual uintptr_t pointer() override {
      return reinterpret_cast<uintptr_t>(m_handle.get());
    }
  private:
    Renderer::Geometry::Segment* m_segment;
    std::shared_ptr<History::Vec2Value> m_handle;
    bool m_is_p1;
  };

}

namespace Graphick::Renderer::Geometry {

  Segment::Segment(vec2 p0, vec2 p3) :
    m_kind(Kind::Linear),
    m_p0(std::make_shared<ControlPoint>(p0)),
    m_p3(std::make_shared<ControlPoint>(p3)) {}

  Segment::Segment(vec2 p0, vec2 p, vec2 p3, bool is_quadratic, bool is_p1) :
    m_kind(is_quadratic ? Kind::Quadratic : Kind::Cubic),
    m_p0(std::make_shared<ControlPoint>(p0)),
    m_p1((is_quadratic || is_p1) ? (p == p0 || p == p3 ? nullptr : std::make_shared<History::Vec2Value>(p)) : nullptr),
    m_p2((!is_quadratic && !is_p1) ? (p == p0 || p == p3 ? nullptr : std::make_shared<History::Vec2Value>(p)) : nullptr),
    m_p3(std::make_shared<ControlPoint>(p3)) {}

  Segment::Segment(vec2 p0, vec2 p1, vec2 p2, vec2 p3) :
    m_kind(Kind::Cubic),
    m_p0(std::make_shared<ControlPoint>(p0)),
    m_p1(p1 == p0 ? nullptr : std::make_shared<History::Vec2Value>(p1)),
    m_p2(p2 == p3 ? nullptr : std::make_shared<History::Vec2Value>(p2)),
    m_p3(std::make_shared<ControlPoint>(p3)) {}

  Segment::Segment(ControlPointVertex p0, ControlPointVertex p3) :
    m_kind(Kind::Linear),
    m_p0(p0),
    m_p3(p3) {}

  Segment::Segment(ControlPointVertex p0, vec2 p, ControlPointVertex p3, bool is_quadratic, bool is_p1) :
    m_kind(is_quadratic ? Kind::Quadratic : Kind::Cubic),
    m_p0(p0),
    m_p1((is_quadratic || is_p1) ? (p == p0->get() || p == p3->get() ? nullptr : std::make_shared<History::Vec2Value>(p)) : nullptr),
    m_p2((!is_quadratic && !is_p1) ? (p == p0->get() || p == p3->get() ? nullptr : std::make_shared<History::Vec2Value>(p)) : nullptr),
    m_p3(p3) {}

  Segment::Segment(ControlPointVertex p0, vec2 p1, vec2 p2, ControlPointVertex p3) :
    m_kind(Kind::Cubic),
    m_p0(p0),
    m_p1(p1 == p0->get() ? nullptr : std::make_shared<History::Vec2Value>(p1)),
    m_p2(p2 == p3->get() ? nullptr : std::make_shared<History::Vec2Value>(p2)),
    m_p3(p3) {}

  Segment::Segment(ControlPointVertex p0, std::optional<vec2> p1, std::optional<vec2> p2, ControlPointVertex p3)
    : m_kind(Kind::Cubic),
    m_p0(p0),
    m_p1(p1.has_value() && p1.value() != p0->get() ? std::make_shared<History::Vec2Value>(p1.value()) : nullptr),
    m_p2(p2.has_value() && p2.value() != p3->get() ? std::make_shared<History::Vec2Value>(p2.value()) : nullptr),
    m_p3(p3)
  {
    if (!m_p1 && !m_p2) {
      m_kind = Kind::Linear;
    }
  }

  Segment::Segment(const Segment& other) :
    m_kind(other.m_kind),
    m_p0(other.m_p0),
    m_p1(other.m_p1),
    m_p2(other.m_p2),
    m_p3(other.m_p3) {}

  Segment::Segment(Segment&& other) noexcept :
    m_kind(other.m_kind),
    m_p0(other.m_p0),
    m_p1(other.m_p1),
    m_p2(other.m_p2),
    m_p3(other.m_p3) {}

  Segment& Segment::operator=(const Segment& other) {
    m_kind = other.m_kind;
    m_p0 = other.m_p0;
    m_p1 = other.m_p1 ? std::make_shared<History::Vec2Value>(*other.m_p1) : nullptr;
    m_p2 = other.m_p2 ? std::make_shared<History::Vec2Value>(*other.m_p2) : nullptr;
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

  bool Segment::operator==(const Segment& other) const {
    if (m_p0 != other.m_p0) return false;
    if (m_p3 != other.m_p3) return false;

    if (m_p1 != other.m_p1) return false;
    if (m_p2 != other.m_p2) return false;

    return true;
  }

  static float threshold_angle = 0.1f;
  static float min_threshold_angle = 0.01f;

  const std::vector<float>& Segment::parameterize(const float zoom) const {
    // TODO: better cache recalculation
    if (m_parameterization.has_value() && zoom == m_parameterization->first) return m_parameterization->second;

    GK_TOTAL("Segment::parameterize");

    const std::vector<vec2> angles = turning_angles();
    m_parameterization = std::make_pair(zoom, std::vector<float>{});

    const float facet_angle = std::max(threshold_angle / static_cast<float>(zoom), min_threshold_angle);
    float last_t = 0.0f;

    const vec2 A = p0();
    const vec2 B = p1();
    const vec2 C = p2();
    const vec2 D = p3();

    const vec2 a = 3.0f * (-A + 3.0f * B - 3.0f * C + D);
    const vec2 b = 6.0f * (A - 2.0f * B + C);
    const vec2 c = -3.0f * (A - B);

    // TODO: do better job with m_parameterization->reserve()

    for (size_t i = 0; i < angles.size() - 1; i++) {
      float checkpoint_t = 0.5f * (angles[i].x + angles[i + 1].x);
      vec2 checkpoint = a * checkpoint_t * checkpoint_t + b * checkpoint_t + c;
      float checkpoint_angle = std::atan2f(checkpoint.y, checkpoint.x);

      float difference = angles[i + 1].y - angles[i].y;

      float k1 = (checkpoint_angle - angles[i].y) / difference;
      float k2 = (checkpoint_angle + MATH_F_TWO_PI - angles[i].y) / difference;

      if (!(Math::is_normalized(k1) || Math::is_normalized(k2))) {
        difference += -Math::sign(difference) * MATH_F_TWO_PI;
      }

      int increments = std::max(std::abs((int)std::ceilf(difference / facet_angle)), 1);
      float increment = difference / (float)increments;

      m_parameterization->second.reserve(increments);
      m_parameterization->second.push_back(last_t = angles[i].x);

      for (int j = 1; j < increments; j++) {
        float theta = angles[i].y + (float)j * increment;
        float tan = std::tanf(theta);

        float p = a.y - tan * a.x;
        float q = b.y - tan * b.x;
        float r = c.y - tan * c.x;

        vec2 t_values;

        if (Math::is_almost_zero(p)) {
          if (Math::is_almost_zero(q)) continue;
          else t_values = { -r / q, -1.0f };
        } else {
          float delta = q * q - 4.0f * p * r;

          if (Math::is_almost_zero(delta)) {
            t_values = { -q / (2.0f * p), -1.0f };
          } else if (delta > 0.0f) {
            float sqrt_delta = std::sqrtf(delta);
            float t1 = (-q + sqrt_delta) / (2.0f * p);
            float t2 = (-q - sqrt_delta) / (2.0f * p);

            t_values = { t1, t2 };
          } else continue;
        }

        bool is_t1_bad = !Math::is_in_range(t_values.x, last_t, angles[i + 1].x, false);
        bool is_t2_bad = !Math::is_in_range(t_values.y, last_t, angles[i + 1].x, false);

        if (is_t1_bad || is_t2_bad) {
          if (is_t1_bad && is_t2_bad) continue;

          m_parameterization->second.push_back(last_t = (float)is_t1_bad * t_values.y + (float)is_t2_bad * t_values.x);
        } else if (t_values.x - last_t < t_values.y - last_t) {
          m_parameterization->second.push_back(last_t = t_values.x);
        } else {
          m_parameterization->second.push_back(last_t = t_values.y);
        }
      }
    }

    m_parameterization->second.push_back(1.0f);

    // console::log("params", m_parameterization->second.size());

    return m_parameterization->second;
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

    /* L1 norm */
    vec2 diff = Math::abs(p1 - p2);
    float mag = diff.x + diff.y;

    /* Manhattan distance of d1 and d2. */
    float edges = std::fabsf(d1.x) + std::fabsf(d1.y) + std::fabsf(d2.x) + std::fabsf(d2.y);
    return mag * 4096 <= edges;
  }

  bool Segment::is_point() const {
    vec2 p0 = this->p0();
    vec2 p1 = this->p1();
    vec2 p2 = this->p2();
    vec2 p3 = this->p3();

    return Math::is_almost_equal(p0, p3) && Math::is_almost_equal(p0, p1) && Math::is_almost_equal(p0, p2);
  }

  vec2 Segment::get(const float t) const {
    return SEGMENT_CALL(get, t);
  }

  // TODO: check if segment level cache is worth it (probably is invalidated when path level cache is invalidated)
  rect Segment::bounding_rect() const {
    GK_TOTAL("Segment::bounding_rect");

    if (m_bounding_rect_cache.has_value()) return m_bounding_rect_cache.value();

    rect rect{};
    std::vector<vec2> points = extrema();

    for (vec2& point : points) {
      Math::min(rect.min, point, rect.min);
      Math::max(rect.max, point, rect.max);
    }

    m_bounding_rect_cache = rect;

    return rect;
  }

  rect Segment::bounding_rect(const mat2x3& transform) const {
    GK_TOTAL("Segment::bounding_rect(transform)");

    rect rect{};
    std::vector<vec2> points = extrema(transform);

    for (vec2& point : points) {
      Math::min(rect.min, point, rect.min);
      Math::max(rect.max, point, rect.max);
    }

    return rect;
  }


  rect Segment::approx_bounding_rect() const {
    GK_TOTAL("Segment::approx_bounding_rect");

    vec2 p0 = m_p0->get();
    vec2 p3 = m_p3->get();

    rect rect{ p0, p0 };

    Math::min(rect.min, p3, rect.min);
    Math::max(rect.max, p3, rect.max);

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

  Segment::SegmentPointDistance Segment::closest_to(const vec2 position, int iterations) const {
    return SEGMENT_CALL(closest_to, position, iterations);
  }

  bool Segment::is_inside(const vec2 position, bool deep_search, vec2 threshold) const {
    if (!Math::is_point_in_rect(position, deep_search ? approx_bounding_rect() : bounding_rect(), threshold)) {
      return false;
    }

    if (Math::is_point_in_ellipse(position, p0(), threshold) || Math::is_point_in_ellipse(position, p3(), threshold)) {
      return true;
    }

    if (deep_search) {
      if (m_p1 != nullptr && Math::is_point_in_ellipse(position, p1(), threshold)) {
        return true;
      }
      if (m_p2 != nullptr && Math::is_point_in_ellipse(position, p2(), threshold)) {
        return true;
      }
    }

    auto a = SEGMENT_CALL(closest_to, position, 8).sq_distance;
    float mean_threshold = (threshold.x + threshold.y) / 2.0f;

    // TODO: fix threshold for exclusive horizontal or vertical stretches
    return a <= mean_threshold * mean_threshold / 3.0f;   /* Adjusted threshold for segment hover. */
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

  bool Segment::intersects(const Math::rect& rect, const mat2x3& transform) const {
    Math::rect bounding_rect = transform * this->bounding_rect();

    if (!Math::does_rect_intersect_rect(rect, bounding_rect)) return false;
    if (Math::is_point_in_rect(transform * p0(), rect) || Math::is_point_in_rect(transform * p3(), rect)) return true;

    for (Math::rect& line : Math::lines_from_rect(rect)) {
      if (intersects_line(line, transform)) return true;
    }

    return false;
  }

  bool Segment::intersects(const Math::rect& rect, const bool found, std::unordered_set<uuid>& vertices) const {
    if (found) {
      if (Math::is_point_in_rect(p0(), rect)) vertices.insert(m_p0->id);
      if (Math::is_point_in_rect(p3(), rect)) vertices.insert(m_p3->id);

      return false;
    }

    Math::rect bounding_rect = this->bounding_rect();
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

  bool Segment::intersects(const Math::rect& rect, const bool found, const mat2x3& transform, std::unordered_set<uuid>& vertices) const {
    if (found) {
      if (Math::is_point_in_rect(transform * p0(), rect)) vertices.insert(m_p0->id);
      if (Math::is_point_in_rect(transform * p3(), rect)) vertices.insert(m_p3->id);

      return false;
    }

    Math::rect bounding_rect = transform * this->bounding_rect();
    if (!Math::does_rect_intersect_rect(rect, bounding_rect)) return false;

    bool p0_inside = Math::is_point_in_rect(transform * p0(), rect);
    bool p3_inside = Math::is_point_in_rect(transform * p3(), rect);

    if (p0_inside) vertices.insert(m_p0->id);
    if (p3_inside) vertices.insert(m_p3->id);
    if (p0_inside || p3_inside) return true;

    for (Math::rect& line : Math::lines_from_rect(rect)) {
      if (intersects_line(line, transform)) return true;
    }

    return false;
  }

  bool Segment::intersects_line(const Math::rect& line) const {
    return line_intersection_points(line).has_value();
  }

  bool Segment::intersects_line(const Math::rect& line, const mat2x3& transform) const {
    return line_intersection_points(line, transform).has_value();
  }

  void Segment::create_p1(const vec2 position) {
    if (m_p1) {
      m_p1->set(position);
      return;
    }

    History::CommandHistory::add(std::make_unique<History::CreateHandleCommand>(this, std::make_shared<History::Vec2Value>(position), true));
  }

  void Segment::create_p2(const vec2 position) {
    if (m_p2) {
      m_p2->set(position);
      return;
    }

    History::CommandHistory::add(std::make_unique<History::CreateHandleCommand>(this, std::make_shared<History::Vec2Value>(position), false));
  }

  void Segment::remove_p1() {
    if (!m_p1) return;

    History::CommandHistory::add(std::make_unique<History::RemoveHandleCommand>(this, m_p1, true));
  }

  void Segment::remove_p2() {
    if (!m_p2) return;

    History::CommandHistory::add(std::make_unique<History::RemoveHandleCommand>(this, m_p2, false));
  }

  void Segment::recalculate_kind() {
    if (!m_p1 && !m_p2) {
      m_kind = Kind::Linear;
    } else {
      m_kind = Kind::Cubic;
    }
  }

  bool Segment::rehydrate_cache() const {
    vec2 p0 = m_p0->get();
    vec2 p3 = m_p3->get();

    int hash = 0;

    if (m_p1 && m_p2) {
      vec2 p1 = m_p1->get();
      vec2 p2 = m_p2->get();
      hash = Math::hash({ p0.x, p0.y, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y });
    } else if (m_p1) {
      vec2 p1 = m_p1->get();
      hash = Math::hash({ p0.x, p0.y, p1.x, p1.y, p3.x, p3.y });
    } else if (m_p2) {
      vec2 p2 = m_p2->get();
      hash = Math::hash({ p0.x, p0.y, p2.x, p2.y, p3.x, p3.y });
    } else {
      hash = Math::hash({ p0.x, p0.y, p3.x, p3.y });
    }

    if (m_hash == hash) return false;

    m_hash = hash;

    m_bounding_rect_cache.reset();
    m_parameterization.reset();

    return true;
  }

  std::shared_ptr<Segment> Segment::reverse(const Segment& segment) {
    if (segment.is_linear()) {
      return std::make_shared<Renderer::Geometry::Segment>(segment.p3_ptr().lock(), segment.p0_ptr().lock());
    } else if (segment.has_p1() && segment.has_p2()) {
      return std::make_shared<Renderer::Geometry::Segment>(segment.p3_ptr().lock(), segment.p2(), segment.p1(), segment.p0_ptr().lock());
    } else if (segment.has_p1()) {
      return std::make_shared<Renderer::Geometry::Segment>(segment.p3_ptr().lock(), segment.p1(), segment.p0_ptr().lock(), false, false);
    }

    return std::make_shared<Renderer::Geometry::Segment>(segment.p3_ptr().lock(), segment.p2(), segment.p0_ptr().lock(), false, true);
  }

  void Segment::transform(Segment& segment, const mat2x3& matrix, bool transform_p3) {
    segment.p0_ptr().lock()->set(matrix * segment.p0());

    if (segment.has_p1()) segment.m_p1->set(matrix * segment.p1());
    if (segment.has_p2()) segment.m_p2->set(matrix * segment.p2());

    if (transform_p3) segment.p3_ptr().lock()->set(matrix * segment.p3());
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

  std::vector<vec2> Segment::extrema(const mat2x3& transform) const {
    std::vector <vec2> extrema;

    for (float t : SEGMENT_CALL(extrema, transform)) {
      extrema.push_back(transform * get(t));
    }

    return extrema;
  }

  std::vector<float> Segment::linear_extrema() const {
    return { 0.0f, 1.0f };
  }

  std::vector<float> Segment::linear_extrema(const mat2x3& transform) const {
    return { 0.0f, 1.0f };
  }

  // TODO: extract quadratic_extrema in math.h
  std::vector<float> Segment::quadratic_extrema() const {
    const vec2 A = p0();
    const vec2 B = p1();
    const vec2 C = p3();

    const vec2 a = A - 2.0f * B + C;
    const vec2 b = 2.0f * (B - A);

    std::vector<float> roots = { 0.0f, 1.0f };

    for (int i = 0; i < 2; i++) {
      if (Math::is_almost_zero(a[i] - b[i])) continue;

      float t = a[i] / (a[i] - b[i]);
      if (t > 0.0f && t < 1.0f) {
        roots.push_back(t);
      }
    }

    return roots;
  }

  std::vector<float> Segment::quadratic_extrema(const mat2x3& transform) const {
    const vec2 A = transform * p0();
    const vec2 B = transform * p1();
    const vec2 C = transform * p3();

    const vec2 a = A - 2.0f * B + C;
    const vec2 b = 2.0f * (B - A);

    std::vector<float> roots = { 0.0f, 1.0f };

    for (int i = 0; i < 2; i++) {
      if (Math::is_almost_zero(a[i] - b[i])) continue;

      float t = a[i] / (a[i] - b[i]);
      if (t > 0.0f && t < 1.0f) {
        roots.push_back(t);
      }
    }

    return roots;
  }

  std::vector<float> Segment::cubic_extrema() const {
    return Math::bezier_extrema(p0(), p1(), p2(), p3());
  }

  std::vector<float> Segment::cubic_extrema(const mat2x3& transform) const {
    return Math::bezier_extrema(transform * p0(), transform * p1(), transform * p2(), transform * p3());
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

  std::optional<std::vector<vec2>> Segment::line_intersection_points(const Math::rect& line, const float threshold) const {
    std::vector<float> intersections = line_intersections(line);
    if (intersections.empty()) return std::nullopt;

    std::vector<vec2> points{};
    Math::rect rect = { min(line.min, line.max), max(line.min, line.max) };

    if (Math::is_almost_equal(rect.min.x, rect.max.x, GEOMETRY_MAX_INTERSECTION_ERROR)) {
      for (float intersection : intersections) {
        vec2 point = get(intersection);

        if (point.y >= rect.min.y && point.y <= rect.max.y) {
          points.push_back(point);
        }
      }
    } else if (Math::is_almost_equal(rect.min.y, rect.max.y, GEOMETRY_MAX_INTERSECTION_ERROR)) {
      for (float intersection : intersections) {
        vec2 point = get(intersection);

        if (point.x >= rect.min.x && point.x <= rect.max.x) {
          points.push_back(point);
        }
      }
    } else {
      for (float intersection : intersections) {
        vec2 point = get(intersection);

        if (Math::is_point_in_rect(point, rect)) {
          points.push_back(point);
        }
      }
    }

    return points.empty() ? std::nullopt : std::optional{ points };
  }

  std::optional<std::vector<vec2>> Segment::line_intersection_points(const rect& line, const mat2x3& transform, const float threshold) const {
    std::vector<float> intersections = line_intersections(line, transform);
    if (intersections.empty()) return std::nullopt;

    std::vector<vec2> points{};
    Math::rect rect = { min(line.min, line.max), max(line.min, line.max) };

    if (Math::is_almost_equal(rect.min.x, rect.max.x, GEOMETRY_MAX_INTERSECTION_ERROR)) {
      for (float intersection : intersections) {
        vec2 point = transform * get(intersection);

        if (point.y >= rect.min.y && point.y <= rect.max.y) {
          points.push_back(point);
        }
      }
    } else if (Math::is_almost_equal(rect.min.y, rect.max.y, GEOMETRY_MAX_INTERSECTION_ERROR)) {
      for (float intersection : intersections) {
        vec2 point = transform * get(intersection);

        if (point.x >= rect.min.x && point.x <= rect.max.x) {
          points.push_back(point);
        }
      }
    } else {
      for (float intersection : intersections) {
        vec2 point = transform * get(intersection);

        if (Math::is_point_in_rect(point, rect)) {
          points.push_back(point);
        }
      }
    }

    return points.empty() ? std::nullopt : std::optional{ points };
  }

  std::vector<float> Segment::line_intersections(const rect& line) const {
    return SEGMENT_CALL(line_intersections, line);
  }

  std::vector<float> Segment::line_intersections(const rect& line, const mat2x3& transform) const {
    return SEGMENT_CALL(line_intersections, line, transform);
  }

  // TOOD: extract in math.h
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

  std::vector<float> Segment::linear_line_intersections(const rect& line, const mat2x3& transform) const {
    const vec2 A = transform * p0();
    const vec2 B = transform * p3();

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

  std::vector<float> Segment::quadratic_line_intersections(const rect& line, const mat2x3& transform) const {
    return {};
  }

  // TODO: extract in math.h
  std::vector<float> Segment::cubic_line_intersections(const rect& line) const {
    float alpha = -std::atan2f(line.max.y - line.min.y, line.max.x - line.min.x);
    float sin_alpha = std::sinf(alpha);
    float cos_alpha = std::cosf(alpha);

    vec2 A = Math::rotate(p0() - line.min, { 0.0f, 0.0f }, sin_alpha, cos_alpha);
    vec2 B = Math::rotate(p1() - line.min, { 0.0f, 0.0f }, sin_alpha, cos_alpha);
    vec2 C = Math::rotate(p2() - line.min, { 0.0f, 0.0f }, sin_alpha, cos_alpha);
    vec2 D = Math::rotate(p3() - line.min, { 0.0f, 0.0f }, sin_alpha, cos_alpha);

    float pa = A.y;
    float pb = B.y;
    float pc = C.y;
    float pd = D.y;

    float d = -pa + 3 * pb - 3 * pc + pd;
    float a = 3 * pa - 6 * pb + 3 * pc;
    float b = -3 * pa + 3 * pb;
    float c = pa;

    std::vector<float> roots = {};

    if (Math::is_almost_zero(d)) {
      /* This is not a cubic curve. */
      if (Math::is_almost_zero(a)) {
        /* This is not a quadratic curve either. */
        if (Math::is_almost_zero(b)) {
          return {};
        }

        roots.push_back(-c / b);
      } else {
        float q = std::sqrt(b * b - 4 * a * c);
        float a2 = 2 * a;

        roots.insert(roots.end(), { (q - b) / a2, (-b - q) / a2 });
      }
    } else {
      a /= d;
      b /= d;
      c /= d;

      float p = (3 * b - a * a) / 3;
      float p3 = p / 3;
      float q = (2 * a * a * a - 9 * a * b + 27 * c) / 27;
      float q2 = q / 2;
      float discriminant = q2 * q2 + p3 * p3 * p3;

      float u1, v1, x1, x2, x3;
      if (discriminant < 0) {
        float mp3 = -p / 3;
        float mp33 = mp3 * mp3 * mp3;
        float r = sqrt(mp33);
        float t = -q / (2 * r);
        float cosphi = t < -1 ? -1 : t > 1 ? 1 : t;
        float phi = acos(cosphi);
        float cbrtr = std::cbrt(r);
        float t1 = 2 * cbrtr;

        x1 = t1 * std::cosf(phi / 3) - a / 3;
        x2 = t1 * std::cosf((phi + MATH_F_TWO_PI) / 3) - a / 3;
        x3 = t1 * std::cosf((phi + 2 * MATH_F_TWO_PI) / 3) - a / 3;

        roots.insert(roots.end(), { x1, x2, x3 });
      } else if (Math::is_almost_zero(discriminant)) {
        u1 = q2 < 0 ? std::cbrt(-q2) : -std::cbrt(q2);
        x1 = 2 * u1 - a / 3;
        x2 = -u1 - a / 3;

        roots.insert(roots.end(), { x1, x2 });
      } else {
        float sd = std::sqrt(discriminant);
        u1 = std::cbrt(-q2 + sd);
        v1 = std::cbrt(q2 + sd);

        roots.push_back(u1 - v1 - a / 3);
      }
    }

    std::vector<float> parsed_roots = {};

    for (float t : roots) {
      if (t >= 0.0f && t <= 1.0f) {
        parsed_roots.push_back(t);
      }
    }

    return parsed_roots;
  }

  std::vector<float> Segment::cubic_line_intersections(const rect& line, const mat2x3& transform) const {
    float alpha = -std::atan2f(line.max.y - line.min.y, line.max.x - line.min.x);
    float sin_alpha = std::sinf(alpha);
    float cos_alpha = std::cosf(alpha);

    vec2 A = Math::rotate(transform * p0() - line.min, { 0.0f, 0.0f }, sin_alpha, cos_alpha);
    vec2 B = Math::rotate(transform * p1() - line.min, { 0.0f, 0.0f }, sin_alpha, cos_alpha);
    vec2 C = Math::rotate(transform * p2() - line.min, { 0.0f, 0.0f }, sin_alpha, cos_alpha);
    vec2 D = Math::rotate(transform * p3() - line.min, { 0.0f, 0.0f }, sin_alpha, cos_alpha);

    float pa = A.y;
    float pb = B.y;
    float pc = C.y;
    float pd = D.y;

    float d = -pa + 3 * pb - 3 * pc + pd;
    float a = 3 * pa - 6 * pb + 3 * pc;
    float b = -3 * pa + 3 * pb;
    float c = pa;

    std::vector<float> roots = {};

    if (Math::is_almost_zero(d)) {
      /* This is not a cubic curve. */
      if (Math::is_almost_zero(a)) {
        /* This is not a quadratic curve either. */
        if (Math::is_almost_zero(b)) {
          return {};
        }

        roots.push_back(-c / b);
      } else {
        float q = std::sqrt(b * b - 4 * a * c);
        float a2 = 2 * a;

        roots.insert(roots.end(), { (q - b) / a2, (-b - q) / a2 });
      }
    } else {
      a /= d;
      b /= d;
      c /= d;

      float p = (3 * b - a * a) / 3;
      float p3 = p / 3;
      float q = (2 * a * a * a - 9 * a * b + 27 * c) / 27;
      float q2 = q / 2;
      float discriminant = q2 * q2 + p3 * p3 * p3;

      float u1, v1, x1, x2, x3;
      if (discriminant < 0) {
        float mp3 = -p / 3;
        float mp33 = mp3 * mp3 * mp3;
        float r = sqrt(mp33);
        float t = -q / (2 * r);
        float cosphi = t < -1 ? -1 : t > 1 ? 1 : t;
        float phi = acos(cosphi);
        float cbrtr = std::cbrt(r);
        float t1 = 2 * cbrtr;

        x1 = t1 * std::cosf(phi / 3) - a / 3;
        x2 = t1 * std::cosf((phi + MATH_F_TWO_PI) / 3) - a / 3;
        x3 = t1 * std::cosf((phi + 2 * MATH_F_TWO_PI) / 3) - a / 3;

        roots.insert(roots.end(), { x1, x2, x3 });
      } else if (Math::is_almost_zero(discriminant)) {
        u1 = q2 < 0 ? std::cbrt(-q2) : -std::cbrt(q2);
        x1 = 2 * u1 - a / 3;
        x2 = -u1 - a / 3;

        roots.insert(roots.end(), { x1, x2 });
      } else {
        float sd = std::sqrt(discriminant);
        u1 = std::cbrt(-q2 + sd);
        v1 = std::cbrt(q2 + sd);

        roots.push_back(u1 - v1 - a / 3);
      }
    }

    std::vector<float> parsed_roots = {};

    for (float t : roots) {
      if (t >= 0.0f && t <= 1.0f) {
        parsed_roots.push_back(t);
      }
    }

    return parsed_roots;
  }

  std::vector<float> Segment::inflections() const {
    vec2 P1 = p1();
    vec2 P2 = p2();

    vec2 A = P1 - p0();
    vec2 B = P2 - P1 - A;
    vec2 C = p3() - P2 - A - 2.0f * B;

    float a = B.x * C.y - B.y * C.x;
    float b = A.x * C.y - A.y * C.x;
    float c = A.x * B.y - A.y * B.x;

    if (Math::is_almost_zero(a)) {
      if (Math::is_almost_zero(b)) {
        return { 0.0f, 1.0f };
      }

      float t = -c / b;
      if (t > 0.0f && t < 1.0f) {
        return { 0.0f, t, 1.0f };
      }

      return { 0.0f, 1.0f };
    }

    float delta = b * b - 4.0f * a * c;

    if (Math::is_almost_zero(delta)) {
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

  std::vector<vec2> Segment::turning_angles() const {
    std::vector<float> inflections;

    const vec2 A = p0();
    const vec2 B = p1();
    const vec2 C = p2();
    const vec2 D = p3();

    const vec2 a = 3.0f * (-A + 3.0f * B - 3.0f * C + D);
    const vec2 b = 6.0f * (A - 2.0f * B + C);
    const vec2 c = -3.0f * (A - B);

    const vec2 a_prime = 2.0f * a;
    const vec2 b_prime = b;

    if (A == B || C == D) {
      // TODO: test if necessary
      inflections = { 0.0f, 1.0f };
    } else {
      // TODO: cache inflections
      inflections = this->inflections();
    }

    size_t inflections_num = inflections.size();

    std::vector<vec2> turning_angles(inflections_num);

    for (size_t i = 0; i < inflections_num; i++) {
      float t = inflections[i];
      vec2 gradient = a * t * t + b * t + c;

      if (Math::is_almost_zero(gradient, GEOMETRY_CURVE_ERROR)) {
        vec2 curvature = (1.0f - t * 2.0f) * a * t + b;
        turning_angles[i] = { t, std::atan2f(curvature.y, curvature.x) };
      } else {
        turning_angles[i] = { t, std::atan2f(gradient.y, gradient.x) };
      }
    }

    return turning_angles;
  }
}
