#include "path.h"

#include "../../math/math.h"
#include "../../math/mat2.h"
#include "../../math/vector.h"
#include "../../math/algorithms/fit.h"

#include "../../editor/editor.h"
#include "../../editor/input/tools/pen_tool.h"

#include "../../history/command_history.h"
#include "../../history/commands.h"

#include "../../utils/defines.h"
#include "../../utils/console.h"

namespace Graphick::History {

  class InsertInSegmentsVectorCommand : public InsertInVectorCommand<std::shared_ptr<Renderer::Geometry::Segment>> {
  public:
    InsertInSegmentsVectorCommand(Renderer::Geometry::Path* path, std::vector<std::shared_ptr<Renderer::Geometry::Segment>>* vector, const std::shared_ptr<Renderer::Geometry::Segment>& value)
      : InsertInVectorCommand(vector, value), m_path(path) {
      Editor::Input::PenTool* pen = Editor::Editor::scene().tool_state.pen();
      if (!pen) return;

      m_pen = pen->pen_element() == m_path->id;
    }

    InsertInSegmentsVectorCommand(Renderer::Geometry::Path* path, std::vector<std::shared_ptr<Renderer::Geometry::Segment>>* vector, const std::shared_ptr<Renderer::Geometry::Segment>& value, int index)
      : InsertInVectorCommand(vector, value, index), m_path(path) {
      Editor::Input::PenTool* pen = Editor::Editor::scene().tool_state.pen();
      if (!pen) return;

      m_pen = pen->pen_element() == m_path->id;
    }

    virtual void execute() override {
      InsertInVectorCommand::execute();
      recalculate();

      Editor::Scene& scene = Editor::Editor::scene();

      scene.selection.clear();
      if (!m_vector->empty()) {
        if (m_path->m_reversed) {
          scene.selection.select_vertex(m_values.back()->m_p0->id, m_path->id);
        } else {
          scene.selection.select_vertex(m_values.back()->m_p3->id, m_path->id);
        }
      } else if (m_path->m_last_point) {
        scene.selection.select_vertex(m_path->m_last_point->id, m_path->id);
      }
    }

    virtual void undo() override {
      if (m_vector->size() == 1) {
        if (m_path->m_reversed) {
          m_path->m_last_point = m_vector->back()->m_p3;
        } else {
          m_path->m_last_point = m_vector->front()->m_p0;
        }
      }

      InsertInVectorCommand::undo();
      recalculate();

      Editor::Scene& scene = Editor::Editor::scene();

      scene.selection.clear();
      if (!m_vector->empty()) {
        if (m_path->m_reversed) {
          scene.selection.select_vertex(m_vector->front()->m_p0->id, m_path->id);
        } else {
          scene.selection.select_vertex(m_vector->back()->m_p3->id, m_path->id);
        }
      } else if (m_path->m_last_point) {
        scene.selection.select_vertex(m_path->m_last_point->id, m_path->id);
      }
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
    void recalculate() {
      if (!m_vector->empty()) {
        if (m_path->m_reversed) {
          m_path->m_last_point = m_vector->front()->m_p0;
        } else {
          m_path->m_last_point = m_vector->back()->m_p3;
        }

        m_path->m_closed = m_vector->front()->p0_id() == m_vector->back()->p3_id();
      } else {
        m_path->m_closed = false;
      }

      Editor::Input::PenTool* pen = Editor::Editor::scene().tool_state.pen();
      if (!pen) return;

      if (m_path->vacant() || m_path->closed()) {
        pen->set_pen_element(0);
      } else if (m_pen) {
        pen->set_pen_element(m_path->id);
      }
    }
  private:
    Renderer::Geometry::Path* m_path;
    bool m_pen;
  };

  class EraseFromSegmentsVectorCommand : public EraseFromVectorCommand<std::shared_ptr<Renderer::Geometry::Segment>> {
  public:
    EraseFromSegmentsVectorCommand(Renderer::Geometry::Path* path, std::vector<std::shared_ptr<Renderer::Geometry::Segment>>* vector, int index)
      : EraseFromVectorCommand(vector, vector->at(index), index), m_path(path) {
      Editor::Input::PenTool* pen = Editor::Editor::scene().tool_state.pen();
      if (!pen) return;

      m_pen = pen->pen_element() == m_path->id;
    }

    virtual void execute() override {
      if (m_vector->size() == 1) {
        if (m_path->m_reversed) {
          m_path->m_last_point = m_vector->back()->m_p3;
        } else {
          m_path->m_last_point = m_vector->front()->m_p0;
        }
      }

      EraseFromVectorCommand::execute();
      recalculate();
    }

    virtual void undo() override {
      EraseFromVectorCommand::undo();
      recalculate();
    }

    virtual bool merge_with(std::unique_ptr<Command>& command) override {
      if (command->type != Type::EraseFromVector) return false;

      EraseFromSegmentsVectorCommand* casted_command = static_cast<EraseFromSegmentsVectorCommand*>(command.get());

      if (casted_command->m_path != this->m_path || casted_command->m_vector != this->m_vector) return false;
      casted_command->m_values.insert(casted_command->m_values.end(), m_values.begin(), m_values.end());
      casted_command->m_indices.insert(casted_command->m_indices.end(), m_indices.begin(), m_indices.end());

      return true;
    }
  private:
    void recalculate() {
      if (!m_vector->empty()) {
        if (m_path->m_reversed) {
          m_path->m_last_point = m_vector->front()->m_p0;
        } else {
          m_path->m_last_point = m_vector->back()->m_p3;
        }

        m_path->m_closed = m_vector->front()->p0_id() == m_vector->back()->p3_id();
      } else {
        m_path->m_closed = false;
      }

      Editor::Scene& scene = Editor::Editor::scene();

      scene.selection.clear();
      if (!m_vector->empty()) {
        if (m_path->m_reversed) {
          scene.selection.select_vertex(m_vector->front()->m_p0->id, m_path->id);
        } else {
          scene.selection.select_vertex(m_vector->back()->m_p3->id, m_path->id);
        }
      } else if (m_path->m_last_point) {
        scene.selection.select_vertex(m_path->m_last_point->id, m_path->id);
      }

      Editor::Input::PenTool* pen = scene.tool_state.pen();
      if (!pen) return;

      if (m_path->vacant() || m_path->closed()) {
        pen->set_pen_element(0);
      } else if (m_pen) {
        pen->set_pen_element(m_path->id);
      }
    }
  private:
    Renderer::Geometry::Path* m_path;
    bool m_pen;
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

  Path::RelativeHandles Path::relative_handles(const uuid id) {
    GK_TOTAL("Path::relative_handles");

    RelativeHandles handles;

    if (vacant()) return handles;

    auto in_handle = in_handle_ptr();
    auto out_handle = out_handle_ptr();

    size_t i = 0;

    if (m_segments.empty()) {
      if (in_handle) handles.in_handle = in_handle->get();
      if (out_handle) handles.out_handle = out_handle->get();

      return handles;
    }

    for (auto& segment : m_segments) {
      if (segment->p0_id() == id) {
        handles.out_segment = segment.get();
        if (segment->has_p1()) handles.out_handle = segment->m_p1.get();

        break;
      }

      i++;
    }

    if (i == 0) {
      if (m_closed) {
        handles.in_segment = &m_segments.back();
        if (handles.in_segment->has_p2()) {
          handles.in_handle = m_segments.back().m_p2.get();
        }
      } else if (in_handle) {
        handles.in_handle = in_handle->get();
      }
    } else if (i >= m_segments.size()) {
      if (m_segments.back().p3_id() == id) {
        handles.in_segment = &m_segments.back();
        if (m_segments.back().has_p2()) handles.in_handle = m_segments.back().m_p2.get();

        if (m_closed) {
          handles.out_segment = &m_segments.front();
          if (handles.out_segment->has_p1()) handles.out_handle = handles.out_segment->m_p1.get();
        } else if (out_handle) {
          handles.out_handle = out_handle->get();
        }
      }
    } else {
      handles.in_segment = &m_segments[i - 1];
      if (handles.in_segment->has_p2()) handles.in_handle = handles.in_segment->m_p2.get();
    }

    if (m_reversed) {
      std::swap(handles.in_segment, handles.out_segment);
      std::swap(handles.in_handle, handles.out_handle);
    }

    return handles;
  }

  bool Path::is_open_end(const uuid id) const {
    if (m_closed) return false;

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
    Segment::ControlPointVertex point = std::make_shared<ControlPoint>(p2);

    if (m_reversed) {
      m_segments.insert(std::make_shared<Segment>(point, p1, m_last_point, false), 0);
    } else {
      m_segments.push_back(std::make_shared<Segment>(m_last_point, p1, point, true));
    }
  }

  void Path::cubic_to(vec2 p1, vec2 p2, vec2 p3) {
    Segment::ControlPointVertex point = std::make_shared<ControlPoint>(p3);

    if (m_reversed) {
      m_segments.insert(std::make_shared<Segment>(point, p2, p1, m_last_point), 0);
    } else {
      m_segments.push_back(std::make_shared<Segment>(m_last_point, p1, p2, point));
    }
  }

  void Path::cubic_to(vec2 p, vec2 p3, bool is_p1) {
    Segment::ControlPointVertex point = std::make_shared<ControlPoint>(p3);

    if (m_reversed) {
      m_segments.insert(std::make_shared<Segment>(point, p, m_last_point, false, !is_p1), 0);
    } else {
      m_segments.push_back(std::make_shared<Segment>(m_last_point, p, point, false, is_p1));
    }
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
      first_segment.m_p0 = last_segment.m_p3;
    } else {
      History::Vec2Value* in_handle = nullptr;
      History::Vec2Value* out_handle = nullptr;

      auto in_ptr = in_handle_ptr();
      auto out_ptr = out_handle_ptr();

      if (in_ptr.has_value()) in_handle = in_ptr->get();
      if (out_ptr.has_value()) out_handle = out_ptr->get();

      if (m_reversed) {
        if (in_handle && out_handle) {
          m_segments.insert(std::make_shared<Segment>(last_segment.m_p3, out_handle->get(), in_handle->get(), m_last_point), 0);
        } else if (in_handle) {
          m_segments.insert(std::make_shared<Segment>(last_segment.m_p3, in_handle->get(), m_last_point, false, false), 0);
        } else if (out_handle) {
          m_segments.insert(std::make_shared<Segment>(last_segment.m_p3, out_handle->get(), m_last_point, false, true), 0);
        } else {
          m_segments.insert(std::make_shared<Segment>(last_segment.m_p3, m_last_point), 0);
        }
      } else {
        if (in_handle && out_handle) {
          m_segments.push_back(std::make_shared<Segment>(m_last_point, out_handle->get(), in_handle->get(), first_segment.m_p0));
        } else if (in_handle) {
          m_segments.push_back(std::make_shared<Segment>(m_last_point, in_handle->get(), first_segment.m_p0, false, false));
        } else if (out_handle) {
          m_segments.push_back(std::make_shared<Segment>(m_last_point, out_handle->get(), first_segment.m_p0, false, true));
        } else {
          m_segments.push_back(std::make_shared<Segment>(m_last_point, first_segment.m_p0));
        }
      }
    }

    m_closed = true;
  }

  void Path::reverse(bool reversed) {
    if (m_segments.empty() || m_reversed == reversed) return;

    m_reversed = reversed;

    if (reversed) {
      History::CommandHistory::add(std::make_unique<History::FunctionCommand>(
        [this]() {
          Editor::Scene& scene = Editor::Editor::scene();
          m_last_point = m_segments.front().m_p0;

          scene.selection.clear();
          scene.selection.select_vertex(m_segments.front().m_p0->id, id);
        },
        [this]() {
          Editor::Scene& scene = Editor::Editor::scene();
          m_last_point = m_segments.back().m_p3;

          scene.selection.clear();
          scene.selection.select_vertex(m_segments.back().m_p3->id, id);
        }
      ));
    } else {
      History::CommandHistory::add(std::make_unique<History::FunctionCommand>(
        [this]() {
          Editor::Scene& scene = Editor::Editor::scene();
          m_last_point = m_segments.back().m_p3;

          scene.selection.clear();
          scene.selection.select_vertex(m_segments.back().m_p3->id, id);
        },
        [this]() {
          Editor::Scene& scene = Editor::Editor::scene();
          m_last_point = m_segments.front().m_p0;

          scene.selection.clear();
          scene.selection.select_vertex(m_segments.front().m_p0->id, id);
        }
      ));
    }
  }

  void Path::remove(const uuid id, bool fit_shape) {
    if (m_segments.empty()) return;

    // TODO: implement object/vertex deletion
    if (m_segments.size() == 1 || (m_closed && m_segments.size() == 2)) {
      std::shared_ptr<ControlPoint> p = nullptr;
      std::optional<vec2> in_handle = std::nullopt;
      std::optional<vec2> out_handle = std::nullopt;

      if (m_segments.front().p0_id() == id) {
        p = m_segments.front().m_p3;

        if (m_segments.front().has_p2()) {
          in_handle = m_segments.front().p2();
        }

        if (m_closed) {
          if (m_segments.back().has_p1()) {
            out_handle = m_segments.back().p1();
          }
        }
      } else if (m_segments.front().p3_id() == id) {
        p = m_segments.front().m_p0;

        if (m_segments.front().has_p1()) {
          out_handle = m_segments.front().p1();
        }

        if (m_closed) {
          if (m_segments.back().has_p2()) {
            in_handle = m_segments.back().p2();
          }
        }
      } else {
        return;
      }

      m_segments.clear();

      if (p == nullptr) return;

      if (in_handle.has_value()) {
        create_in_handle(in_handle.value());
      } else {
        clear_in_handle();
      }
      if (out_handle.has_value()) {
        create_out_handle(out_handle.value());
      } else {
        clear_out_handle();
      }

      History::CommandHistory::add(std::make_unique<History::FunctionCommand>(
        [this, p]() {
          Editor::Editor::scene().selection.select_vertex(p->id, this->id);
          m_last_point = p;
        },
        []() {}
      ));

      return;
    }

    int index = 0;

    for (; index < m_segments.size(); index++) {
      if (m_segments.at(index).p0_id() == id) break;
    }

    int first_segment_index = 0;
    int second_segment_index = 0;

    if (m_closed && (index == 0 || index == m_segments.size())) {
      first_segment_index = (int)m_segments.size() - 1;
      second_segment_index = 0;
    } else if (index == 0) {
      if (m_segments.front().has_p2()) {
        create_in_handle(m_segments.front().p2());
      } else {
        clear_in_handle();
      }

      m_segments.erase(index);

      return;
    } else if (index >= m_segments.size()) {
      if (m_segments.back().has_p1()) {
        create_out_handle(m_segments.back().p1());
      } else {
        clear_out_handle();
      }

      m_segments.pop_back();

      return;
    } else {
      first_segment_index = index - 1;
      second_segment_index = index;
    }

    Segment& first_segment = m_segments[first_segment_index];
    Segment& second_segment = m_segments[second_segment_index];

    std::shared_ptr<Segment> new_segment = nullptr;

    if (fit_shape) {
      const int n_points = 25;
      std::vector<vec2> points(n_points * 2 + 1);

      for (int i = 0; i < n_points; i++) {
        float t = (float)i / (float)n_points;

        points[i] = first_segment.get(t);
        points[n_points + i] = second_segment.get(t);
      }

      points.back() = second_segment.get(1.0f);

      auto cubic = Math::Algorithms::fit_points_to_cubic(points, 0.01f);

      new_segment = std::make_shared<Segment>(first_segment.m_p0, cubic.p1, cubic.p2, second_segment.m_p3);
    } else {
      new_segment = std::make_shared<Segment>(
        first_segment.m_p0,
        first_segment.has_p1() ? std::optional{ first_segment.p1() } : std::nullopt,
        second_segment.has_p2() ? std::optional{ second_segment.p2() } : std::nullopt,
        second_segment.m_p3
      );
    }

    int min_index = std::min(first_segment_index, second_segment_index);
    int max_index = std::max(first_segment_index, second_segment_index);

    m_segments.erase(max_index);
    m_segments.erase(min_index);

    m_segments.insert(new_segment, min_index);
  }

  std::optional<std::weak_ptr<ControlPoint>> Path::split(Segment& segment, float t) {
    if (m_segments.empty()) return std::nullopt;

    int index = 0;

    for (int i = 0; i < m_segments.size(); i++) {
      if (&m_segments[i] == &segment) {
        index = i;
        break;
      }
    }

    std::optional<std::shared_ptr<ControlPoint>> new_vertex = std::nullopt;
    std::shared_ptr<ControlPoint> first_vertex = segment.m_p0;
    std::shared_ptr<ControlPoint> last_vertex = segment.m_p3;

    if (segment.is_linear()) {
      new_vertex = std::make_shared<ControlPoint>(segment.get(t));
      m_segments.erase(index);

      if (m_reversed) {
        m_segments.insert(std::make_shared<Segment>(first_vertex, new_vertex.value()), index);
        m_segments.insert(std::make_shared<Segment>(new_vertex.value(), last_vertex), index + 1);
      } else {
        m_segments.insert(std::make_shared<Segment>(new_vertex.value(), last_vertex), index);
        m_segments.insert(std::make_shared<Segment>(first_vertex, new_vertex.value()), index);
      }
    } else {
      auto [p, in_p1, in_p2, out_p1, out_p2] = Math::split_bezier(segment.p0(), segment.p1(), segment.p2(), segment.p3(), t);

      new_vertex = std::make_shared<ControlPoint>(p);
      m_segments.erase(index);

      if (m_reversed) {
        m_segments.insert(std::make_shared<Segment>(first_vertex, in_p1, in_p2, new_vertex.value()), index);
        m_segments.insert(std::make_shared<Segment>(new_vertex.value(), out_p1, out_p2, last_vertex), index + 1);
      } else {
        m_segments.insert(std::make_shared<Segment>(new_vertex.value(), out_p1, out_p2, last_vertex), index);
        m_segments.insert(std::make_shared<Segment>(first_vertex, in_p1, in_p2, new_vertex.value()), index);
      }
    }

    uuid new_vertex_id = new_vertex.value()->id;
    uuid element_id = id;

    return new_vertex;
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
    if (m_large_bounding_rect_cache.has_value()) return m_large_bounding_rect_cache.value();

    Math::rect rect = approx_bounding_rect();

    auto in_handle = in_handle_ptr();
    auto out_handle = out_handle_ptr();

    if (in_handle) {
      min(rect.min, in_handle.value()->get(), rect.min);
      max(rect.max, in_handle.value()->get(), rect.max);
    }
    if (out_handle) {
      min(rect.min, out_handle.value()->get(), rect.min);
      max(rect.max, out_handle.value()->get(), rect.max);
    }

    m_large_bounding_rect_cache = rect;

    return rect;
  }

  bool Path::is_inside(const vec2 position, bool filled_search, bool deep_search, float threshold) const {
    GK_TOTAL("Path::is_inside");

    if (m_segments.empty()) {
      if (m_last_point && Math::is_point_in_circle(position, m_last_point->get(), threshold)) {
        return true;
      }
    } else {
      if (!Math::is_point_in_rect(position, deep_search ? large_bounding_rect() : approx_bounding_rect(), threshold)) {
        return false;
      }

      if (true/*filled_search*/) {
        // TODO: implement non-zero winding rule

        Math::rect line = { position, { std::numeric_limits<float>::max(), position.y } };
        int intersections = 0;

        for (const auto& segment : m_segments) {
          auto points = segment->line_intersection_points(line);
          if (points) intersections += (int)points->size();
        }

        if (!m_closed) {
          intersections += (int)Math::line_line_intersection_points({ m_segments.back().p3(), m_segments.front().p0() }, line).size();
        }

        if (intersections % 2 != 0) return true;
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

  void Path::create_in_handle(const vec2 position) {
    if (m_closed || vacant()) return;

    m_in_handle->set(position);
  }

  void Path::create_out_handle(const vec2 position) {
    if (m_closed || vacant()) return;

    m_out_handle->set(position);
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
      m_large_bounding_rect_cache.reset();
    }
  }

  void Path::SegmentsVector::push_back(const std::shared_ptr<Segment>& value) {
    History::CommandHistory::add(std::make_unique<History::InsertInSegmentsVectorCommand>(m_path, &m_value, value));
  }

  void Path::SegmentsVector::insert(const std::shared_ptr<Segment>& value, int index) {
    if (m_value.size() < index || index < 0) return;
    History::CommandHistory::add(std::make_unique<History::InsertInSegmentsVectorCommand>(m_path, &m_value, value, index));
  }

  void Path::SegmentsVector::pop_back() {
    erase((int)m_value.size() - 1);
  }

  void Path::SegmentsVector::erase(int index) {
    if (m_value.size() <= index || index < 0) return;
    History::CommandHistory::add(std::make_unique<History::EraseFromSegmentsVectorCommand>(m_path, &m_value, index));
  }

  void Path::SegmentsVector::clear() {
    if (m_value.empty()) return;

    for (int i = (int)m_value.size() - 1; i >= 0; i--) {
      History::CommandHistory::add(std::make_unique<History::EraseFromSegmentsVectorCommand>(m_path, &m_value, i));
    }
  }

}
