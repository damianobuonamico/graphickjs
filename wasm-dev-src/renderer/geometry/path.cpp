#include "path.h"

#include "../../math/math.h"
#include "../../math/mat2.h"
#include "../../math/vector.h"

#include "../../editor/editor.h"

#include "../../history/command_history.h"
#include "../../history/commands.h"

#include "../../utils/defines.h"
#include "../../utils/console.h"

namespace Graphick::History {

  class InsertInSegmentsVectorCommand : public InsertInVectorCommand<std::shared_ptr<Renderer::Geometry::Segment>> {
  public:
    InsertInSegmentsVectorCommand(Renderer::Geometry::Path* path, std::vector<std::shared_ptr<Renderer::Geometry::Segment>>* vector, const std::shared_ptr<Renderer::Geometry::Segment>& value)
      : InsertInVectorCommand(vector, value), m_path(path) {}

    InsertInSegmentsVectorCommand(Renderer::Geometry::Path* path, std::vector<std::shared_ptr<Renderer::Geometry::Segment>>* vector, const std::shared_ptr<Renderer::Geometry::Segment>& value, int index)
      : InsertInVectorCommand(vector, value, index), m_path(path) {}

    virtual void execute() override {
      clear_relative_handles();
      InsertInVectorCommand::execute();
      recalculate();
    }

    virtual void undo() override {
      clear_relative_handles();

      if (this->m_vector->size() == 1) {
        if (m_path->m_reversed) {
          m_path->m_last_point = this->m_vector->back()->m_p3;
        } else {
          m_path->m_last_point = this->m_vector->front()->m_p0;
        }
      }

      InsertInVectorCommand::undo();
      recalculate();
    }

    virtual bool merge_with(std::unique_ptr<Command>& command) override {
      if (command->type != Type::InsertInVector) return false;

      InsertInSegmentsVectorCommand* casted_command = static_cast<InsertInSegmentsVectorCommand*>(command.get());

      if (casted_command->m_path != this->m_path || casted_command->m_vector != this->m_vector) return false;
      casted_command->m_values.insert(casted_command->m_values.end(), m_values.begin(), m_values.end());
      casted_command->m_indices.insert(casted_command->m_indices.end(), m_indices.begin(), m_indices.end());

      return true;
    }
  private:
    void clear_relative_handles() {
      if (this->m_vector->empty()) {
        if (m_path->in_handle_ptr() != std::nullopt) m_path->m_last_point->remove_relative_handle(m_path->m_in_handle);
        if (m_path->out_handle_ptr() != std::nullopt) m_path->m_last_point->remove_relative_handle(m_path->m_out_handle);
      } else {
        if (m_path->in_handle_ptr() != std::nullopt) this->m_vector->front()->m_p0->remove_relative_handle(m_path->m_in_handle);
        if (m_path->out_handle_ptr() != std::nullopt) this->m_vector->back()->m_p3->remove_relative_handle(m_path->m_out_handle);
      }
    }

    void recalculate() {
      if (!this->m_vector->empty()) {
        if (m_path->m_reversed) {
          m_path->m_last_point = this->m_vector->front()->m_p0;
        } else {
          m_path->m_last_point = this->m_vector->back()->m_p3;
        }

        if (m_path->in_handle_ptr() != std::nullopt) this->m_vector->front()->m_p0->set_relative_handle(m_path->m_in_handle);
        if (m_path->out_handle_ptr() != std::nullopt) this->m_vector->back()->m_p3->set_relative_handle(m_path->m_out_handle);

        m_path->m_closed = this->m_vector->front()->p0_id() == this->m_vector->back()->p3_id();
      } else {
        if (m_path->in_handle_ptr() != std::nullopt) m_path->m_last_point->set_relative_handle(m_path->m_in_handle);
        if (m_path->out_handle_ptr() != std::nullopt) m_path->m_last_point->set_relative_handle(m_path->m_out_handle);

        m_path->m_closed = false;
      }

      Editor::Editor::scene().selection.clear();
      if (!m_vector->empty()) {
        if (m_path->m_reversed) {
          Editor::Editor::scene().selection.select_vertex(m_vector->front()->m_p0->id, m_path->id);
        } else {
          Editor::Editor::scene().selection.select_vertex(m_vector->back()->m_p3->id, m_path->id);
        }
      } else if (m_path->m_last_point) {
        Editor::Editor::scene().selection.select_vertex(m_path->m_last_point->id, m_path->id);
      }
    }
  private:
    Renderer::Geometry::Path* m_path;
  };

}

namespace Graphick::Renderer::Geometry {

  Path::Path(const uuid id) :
    id(id),
    m_in_handle(std::make_shared<History::Vec2Value>(std::numeric_limits<vec2>::lowest())),
    m_out_handle(std::make_shared<History::Vec2Value>(std::numeric_limits<vec2>::lowest())),
    m_segments(this) {}

  Path::Path(const uuid id, const Path& path) :
    id(id),
    m_closed(path.m_closed),
    m_reversed(path.m_reversed),
    m_in_handle(path.m_in_handle),
    m_out_handle(path.m_out_handle),
    m_segments(this, path.m_segments) {}

  Path::Path(const Path& path) :
    id(path.id),
    m_closed(path.m_closed),
    m_reversed(path.m_reversed),
    m_in_handle(path.m_in_handle),
    m_out_handle(path.m_out_handle),
    m_segments(this, path.m_segments) {}

  Path::Path(Path&& path) noexcept :
    id(path.id),
    m_closed(path.m_closed),
    m_reversed(path.m_reversed),
    m_in_handle(std::move(path.m_in_handle)),
    m_out_handle(std::move(path.m_out_handle)),
    m_segments(this, std::move(path.m_segments)) {}


  const std::vector<ControlPoint*> Path::vertices() const {
    if (m_segments.empty()) {
      if (m_last_point) {
        return { m_last_point.get() };
      }
      return {};
    }

    std::vector<ControlPoint*> vertices;

    for (const auto& segment : m_segments) {
      vertices.push_back(segment->m_p0.get());
    }

    if (!m_closed) {
      vertices.push_back(m_segments.back().m_p3.get());
    }

    return vertices;
  }

  const std::vector<uuid> Path::vertices_ids() const {
    if (m_segments.empty()) {
      if (m_last_point) {
        return { m_last_point->id };
      }
      return {};
    }

    std::vector<uuid> vertices;

    for (const auto& segment : m_segments) {
      vertices.push_back(segment->m_p0->id);
    }

    if (!m_closed) {
      vertices.push_back(m_segments.back().m_p3->id);
    }

    return vertices;
  }

  std::optional<Segment::ControlPointHandle> Path::in_handle_ptr() const {
    if (!m_in_handle || m_in_handle->get() == std::numeric_limits<vec2>::lowest()) return std::nullopt;
    return m_in_handle;
  }

  std::optional<Segment::ControlPointHandle> Path::out_handle_ptr() const {
    if (!m_out_handle || m_out_handle->get() == std::numeric_limits<vec2>::lowest()) return std::nullopt;
    return m_out_handle;
  }

  bool Path::is_open_end(const uuid id) const {
    if (m_segments.empty()) {
      if (m_last_point == nullptr) return false;
      return m_last_point->id == id;
    }

    return m_segments.front().p0_id() == id || m_segments.back().p3_id() == id;
  }

  void Path::move_to(vec2 p) {
    m_last_point = std::make_shared<ControlPoint>(p);
  }

  void Path::line_to(vec2 p) {
    Segment::ControlPointVertex point = std::make_shared<ControlPoint>(p);

    if (m_reversed) {
      m_segments.insert(std::make_shared<Segment>(point, m_last_point), 0);
    } else {
      m_segments.push_back(std::make_shared<Segment>(m_last_point, point));
    }
  }

  void Path::quadratic_to(vec2 p1, vec2 p2) {
    // if (m_segments.size()) {
    //   m_last_point = m_segments.back().m_p3;
    // }

    Segment::ControlPointVertex point = std::make_shared<ControlPoint>(p2);
    m_segments.push_back(std::make_shared<Segment>(m_last_point, p1, point, true));
    // m_last_point = point;
  }

  void Path::cubic_to(vec2 p1, vec2 p2, vec2 p3) {
    // if (m_segments.size()) {
    //   m_last_point = m_segments.back().m_p3;
    // }

    Segment::ControlPointVertex point = std::make_shared<ControlPoint>(p3);
    m_segments.push_back(std::make_shared<Segment>(m_last_point, p1, p2, point));
    // m_last_point = point;
  }

  void Path::cubic_to(vec2 p, vec2 p3, bool is_p1) {
    // if (m_segments.size()) {
    //   m_last_point = m_segments.back().m_p3;
    // }

    Segment::ControlPointVertex point = std::make_shared<ControlPoint>(p3);
    m_segments.push_back(std::make_shared<Segment>(m_last_point, p, point, false, is_p1));
    // m_last_point = point;
  }

  void Path::arc_to(vec2 c, vec2 radius, float x_axis_rotation, bool large_arc_flag, bool sweep_flag, vec2 p) {
    float sin_th = std::sinf(Math::degrees_to_radians(x_axis_rotation));
    float cos_th = std::cosf(Math::degrees_to_radians(x_axis_rotation));

    vec2 d0 = (c - p) / 2.0f;
    vec2 d1 = {
      cos_th * d0.x + sin_th * d0.y,
      -sin_th * d0.x + cos_th * d0.y
    };

    vec2 sq_r = radius * radius;
    vec2 sq_p = d1 * d1;

    float check = sq_p.x / sq_r.x + sq_p.y / sq_r.y;
    if (check > 1.0f) {
      radius *= std::sqrtf(check);
    }

    mat2 a = {
      cos_th / radius.x, sin_th / radius.x,
      -sin_th / radius.y, cos_th / radius.y
    };
    vec2 p0 = {
      Math::dot(a[0], c),
      Math::dot(a[1], c)
    };
    vec2 p1 = {
      Math::dot(a[0], p),
      Math::dot(a[1], p)
    };

    float d = Math::squared_length(p1 - p0);
    float sfactor_sq = 1.0f / d - 0.25f;
    if (sfactor_sq < 0.0f) sfactor_sq = 0.0f;

    float sfactor = std::sqrt(sfactor_sq);
    if (sweep_flag == large_arc_flag) sfactor = -sfactor;

    vec2 c1 = {
      0.5f * (p0.x + p1.x) - sfactor * (p1.y - p0.y),
      0.5f * (p0.y + p1.y) + sfactor * (p1.x - p0.x)
    };

    float th0 = std::atan2f(p0.y - c1.y, p0.x - c1.x);
    float th1 = std::atan2f(p1.y - c1.y, p1.x - c1.x);
    float th_arc = th1 - th0;

    if (th_arc < 0.0f && sweep_flag) {
      th_arc += MATH_TWO_PI;
    } else if (th_arc > 0.0f && !sweep_flag) {
      th_arc -= MATH_TWO_PI;
    }

    int n_segs = static_cast<int>(std::ceil(std::fabs(th_arc / (MATH_PI * 0.5f + 0.001f))));
    for (int i = 0; i < n_segs; i++) {
      float th2 = th0 + i * th_arc / n_segs;
      float th3 = th0 + (i + 1) * th_arc / n_segs;

      a = {
        cos_th * radius.x, -sin_th * radius.x,
        sin_th * radius.y, cos_th * radius.y
      };

      float th_half = 0.5f * (th3 - th2);
      float sin_half_th_half = std::sinf(th_half * 0.5f);
      float t = (8.0f / 3.0f) * sin_half_th_half * sin_half_th_half / std::sin(th_half);

      float sin_th2 = std::sinf(th2);
      float cos_th2 = std::cosf(th2);
      float sin_th3 = std::sinf(th3);
      float cos_th3 = std::cosf(th3);

      p1 = {
        c1.x + cos_th2 - t * sin_th2,
        c1.y + sin_th2 + t * cos_th2
      };
      vec2 p3 = {
        c1.x + cos_th3,
        c1.y + sin_th3
      };
      vec2 p2 = {
        p3.x + t * sin_th3,
        p3.y - t * cos_th3
      };

      vec2 bez1 = {
        Math::dot(a[0], p1),
        Math::dot(a[1], p1)
      };
      vec2 bez2 = {
        Math::dot(a[0], p2),
        Math::dot(a[1], p2)
      };
      vec2 bez3 = {
        Math::dot(a[0], p3),
        Math::dot(a[1], p3)
      };

      cubic_to(bez1, bez2, bez3);
    }
  }

  void Path::ellipse(vec2 c, vec2 radius) {
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

  void Path::circle(vec2 c, float radius) {
    ellipse(c, { radius, radius });
  }

  void Path::rect(vec2 p, vec2 size, bool centered) {
    if (centered) {
      p -= size * 0.5f;
    }

    move_to(p);
    line_to(p + vec2{ size.x, 0.0f });
    line_to(p + size);
    line_to(p + vec2{ 0.0f, size.y });
    close();
  }

  void Path::round_rect(vec2 p, vec2 size, float radius, bool centered) {
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

  void Path::close() {
    if (m_segments.size() < 1 || (m_segments.size() == 1 && m_segments.front().kind() == Segment::Kind::Linear)) return;

    Segment& first_segment = m_segments.front();
    Segment& last_segment = m_segments.back();

    if (is_almost_equal(last_segment.p3(), first_segment.p0())) {
      if (first_segment.has_p1()) {
        last_segment.m_p3->set_relative_handle(first_segment.m_p1);
      }

      first_segment.m_p0 = last_segment.m_p3;
    } else {
      auto in_handle = in_handle_ptr();
      auto out_handle = out_handle_ptr();

      if (in_handle && out_handle) {
        m_segments.push_back(std::make_shared<Segment>(m_last_point, out_handle.value()->get(), in_handle.value()->get(), first_segment.m_p0));
      } else if (in_handle) {
        m_segments.push_back(std::make_shared<Segment>(m_last_point, in_handle.value()->get(), first_segment.m_p0, false, false));
      } else if (out_handle) {
        m_segments.push_back(std::make_shared<Segment>(m_last_point, out_handle.value()->get(), first_segment.m_p0, false, true));
      } else {
        m_segments.push_back(std::make_shared<Segment>(m_last_point, first_segment.m_p0));
      }
    }

    m_closed = true;
  }

  void Path::reverse(bool reversed) {
    if (m_segments.empty()) return;

    m_reversed = reversed;

    if (reversed) {
      m_last_point = m_segments.front().m_p0;
    } else {
      m_last_point = m_segments.back().m_p3;
    }
  }

  Math::rect Path::bounding_rect() const {
    GK_TOTAL("Path::bounding_rect");
    // base ~ 3.0ms
    // stress ~ 2.7s
    // cache ~ 0.006ms
    if (m_bounding_rect_cache.has_value()) return m_bounding_rect_cache.value();

    if (m_segments.empty()) {
      if (m_last_point == nullptr) return { };

      vec2 p = m_last_point->get();
      return { p, p };
    }

    Math::rect rect{};

    for (const auto& segment : m_segments) {
      Math::rect segment_rect = segment->bounding_rect();

      min(rect.min, segment_rect.min, rect.min);
      max(rect.max, segment_rect.max, rect.max);
    }

    m_bounding_rect_cache = rect;

    return rect;
  }

  Math::rect Path::approx_bounding_rect() const {
    GK_TOTAL("Path::approx_bounding_rect");
    // base ~ 0.36ms
    // stress ~ 0.67ms
    // cache ~ 0.006ms
    if (m_approx_bounding_rect_cache.has_value()) return m_approx_bounding_rect_cache.value();

    if (m_segments.empty()) {
      if (m_last_point == nullptr) return { };

      vec2 p = m_last_point->get();
      return { p, p };
    }

    Math::rect rect{};

    for (const auto& segment : m_segments) {
      Math::rect segment_rect = segment->approx_bounding_rect();

      min(rect.min, segment_rect.min, rect.min);
      max(rect.max, segment_rect.max, rect.max);
    }

    m_approx_bounding_rect_cache = rect;

    return rect;
  }

  Math::rect Path::large_bounding_rect() const {
    GK_TOTAL("Path::large_bounding_rect");
    // Don't cache this one, it's not used often enough and in/out handles do not rehydrate cache.

    Math::rect rect{};

    if (m_segments.empty()) {
      if (m_last_point == nullptr) return { };

      vec2 p = m_last_point->get();
      rect = { p, p };
    } else {
      for (const auto& segment : m_segments) {
        Math::rect segment_rect = segment->approx_bounding_rect();

        min(rect.min, segment_rect.min, rect.min);
        max(rect.max, segment_rect.max, rect.max);
      }
    }

    if (m_in_handle != nullptr) {
      min(rect.min, m_in_handle->get(), rect.min);
      max(rect.max, m_in_handle->get(), rect.max);
    }
    if (m_out_handle != nullptr) {
      min(rect.min, m_out_handle->get(), rect.min);
      max(rect.max, m_out_handle->get(), rect.max);
    }

    return rect;
  }

  bool Path::is_inside(const vec2 position, bool deep_search, float threshold) const {
    GK_TOTAL("Path::is_inside");

    if (m_segments.empty()) {
      if (m_last_point && Math::is_point_in_circle(position, m_last_point->get(), threshold)) {
        return true;
      }
    } else {
      if (!Math::is_point_in_rect(position, deep_search ? large_bounding_rect() : approx_bounding_rect(), threshold)) {
        return false;
      }

      for (const auto& segment : m_segments) {
        if (segment->is_inside(position, deep_search, threshold)) {
          return true;
        }
      }
    }

    if (deep_search) {
      if (m_in_handle && Math::is_point_in_circle(position, m_in_handle->get(), threshold)) {
        return true;
      }
      if (m_out_handle && Math::is_point_in_circle(position, m_out_handle->get(), threshold)) {
        return true;
      }
    }

    return false;
  }

  // TODO: check if path is vacant or empty
  bool Path::intersects(const Math::rect& rect) const {
    GK_TOTAL("Path::intersects");

    if (m_segments.empty()) {
      if (m_last_point && Math::is_point_in_rect(m_last_point->get(), rect)) {
        return true;
      }
      return false;
    }

    Math::rect bounding_rect = this->approx_bounding_rect();

    if (!Math::does_rect_intersect_rect(rect, bounding_rect)) return false;

    for (const auto& segment : m_segments) {
      if (segment->intersects(rect)) return true;
    }

    return false;
  }

  bool Path::intersects(const Math::rect& rect, std::unordered_set<uuid>& vertices) const {
    GK_TOTAL("Path::intersects (deep)");

    if (m_segments.empty()) {
      if (m_last_point == nullptr) return false;

      if (Math::is_point_in_rect(m_last_point->get(), rect)) {
        vertices.insert(m_last_point->id);
        return true;
      }
    }

    Math::rect bounding_rect = this->approx_bounding_rect();

    if (!Math::does_rect_intersect_rect(rect, bounding_rect)) return false;

    bool found = false;
    for (const auto& segment : m_segments) {
      if (segment->intersects(rect, found, vertices)) found = true;
    }

    return found;
  }

  // TODO: fix relative handles
  void Path::create_in_handle(const vec2 position) {
    if (m_closed || vacant()) return;

    m_in_handle->set(position);

    if (m_segments.empty()) {
      m_last_point->set_relative_handle(m_in_handle);
      return;
    }

    m_segments.front().m_p0->set_relative_handle(m_in_handle);
  }

  // TODO: fix relative handles
  void Path::create_out_handle(const vec2 position) {
    if (m_closed || vacant()) return;

    m_out_handle->set(position);

    if (m_segments.empty()) {
      m_last_point->set_relative_handle(m_out_handle);
      return;
    }

    m_segments.back().m_p3->set_relative_handle(m_out_handle);
  }

  void Path::clear_in_handle() {
    m_in_handle->set(std::numeric_limits<vec2>::lowest());
  }

  void Path::clear_out_handle() {
    m_out_handle->set(std::numeric_limits<vec2>::lowest());
  }

  void Path::rehydrate_cache() const {
    GK_TOTAL("Path::rehydrate");

    int hash = (int)m_segments.size();
    bool rehydrate = m_hash != hash;

    m_hash = hash;

    for (const auto& segment : m_segments) {
      if (segment->rehydrate_cache()) rehydrate = true;
    }

    if (rehydrate) {
      m_bounding_rect_cache.reset();
      m_approx_bounding_rect_cache.reset();
    }
  }

  void Path::SegmentsVector::push_back(const std::shared_ptr<Segment>& value) {
    History::CommandHistory::add(std::make_unique<History::InsertInSegmentsVectorCommand>(m_path, &m_value, value));
  }

  void Path::SegmentsVector::insert(const std::shared_ptr<Segment>& value, size_t index) {
    History::CommandHistory::add(std::make_unique<History::InsertInSegmentsVectorCommand>(m_path, &m_value, value, index));
  }

}
