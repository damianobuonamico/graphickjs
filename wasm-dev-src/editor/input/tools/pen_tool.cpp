#include "pen_tool.h"

#include "../input_manager.h"

#include "../../editor.h"
#include "../../scene/entity.h"

#include "../../../history/command_history.h"
#include "../../../history/commands.h"

#include "../../../renderer/renderer.h"

#include "../../../utils/console.h"

// TODO: esc to cancel pen and other tools
// TODO: fix pen for translated elements
namespace Graphick::Editor::Input {

  static void pen_pointer_move(Renderer::Geometry::Path& path, Renderer::Geometry::ControlPoint& vertex, bool keep_in_handle_length = false, bool swap_in_out = false, int* direction = nullptr) {
    if (InputManager::keys.space) {
      vertex.add_delta(InputManager::pointer.scene.movement);
      return;
    }

    auto handles = path.relative_handles(vertex.id);

    if (path.empty()) {
      if (!handles.out_handle) {
        path.create_out_handle(InputManager::pointer.scene.origin);
        handles.out_handle = path.out_handle_ptr()->get();
      }

      handles.out_handle->set_delta(InputManager::pointer.scene.delta);

      if (InputManager::keys.alt) return;

      if (!handles.in_handle) {
        path.create_in_handle(InputManager::pointer.scene.origin);
        handles.in_handle = path.in_handle_ptr()->get();
      }

      handles.in_handle->move_to(2.0f * vertex.get() - InputManager::pointer.scene.position);

      return;
    }

    if (direction) {
      if (*direction == 0) {
        float cos = 0;

        if (handles.out_handle) {
          cos = Math::dot(-InputManager::pointer.scene.delta, handles.out_handle->get() - vertex.get());
        } else if (handles.out_segment) {
          cos = Math::dot(-InputManager::pointer.scene.delta, (handles.out_segment->has_p2() ? handles.out_segment->p2() : handles.out_segment->p3()) - vertex.get());
        }

        if (cos > 0) *direction = -1;
        else *direction = 1;
      }

      if (*direction < 0) {
        std::swap(handles.in_handle, handles.out_handle);
        std::swap(handles.in_segment, handles.out_segment);
      }
    }

    // TODO: shift to slow down movement
    vec2 out_handle_position = InputManager::pointer.scene.position;
    vec2 in_handle_position = 2.0f * vertex.get() - InputManager::pointer.scene.position;

    if (swap_in_out) {
      std::swap(handles.in_segment, handles.out_segment);
      std::swap(handles.in_handle, handles.out_handle);
      std::swap(out_handle_position, in_handle_position);
    }

    bool should_reverse_out = (!direction && path.reversed()) || (direction && *direction > 0);

    if (!handles.out_handle) {
      if (handles.out_segment) {
        if (should_reverse_out) {
          handles.out_segment->create_p1(InputManager::pointer.scene.position);
          handles.out_handle = handles.out_segment->p1_ptr().lock().get();
        } else {
          handles.out_segment->create_p2(InputManager::pointer.scene.position);
          handles.out_handle = handles.out_segment->p2_ptr().lock().get();
        }
      } else {
        if (should_reverse_out) {
          path.create_in_handle(InputManager::pointer.scene.origin);
          handles.out_handle = path.in_handle_ptr()->get();
        } else {
          path.create_out_handle(InputManager::pointer.scene.origin);
          handles.out_handle = path.out_handle_ptr()->get();
        }
      }
    }

    handles.out_handle->move_to(out_handle_position);

    if (InputManager::keys.alt || Math::is_almost_equal(handles.out_handle->get(), vertex.get()) || (!handles.in_handle && keep_in_handle_length)) return;

    if (!handles.in_handle) {
      if ((!direction && path.reversed() == swap_in_out) || (direction && *direction > 0)) {
        handles.in_segment->create_p2(InputManager::pointer.scene.position);
        handles.in_handle = handles.in_segment->p2_ptr().lock().get();
      } else {
        handles.in_segment->create_p1(InputManager::pointer.scene.position);
        handles.in_handle = handles.in_segment->p1_ptr().lock().get();
      }
    }

    if (keep_in_handle_length) {
      vec2 dir = Math::normalize(vertex.get() - handles.out_handle->get());
      float length = Math::length(handles.in_handle->get() - handles.in_handle->delta() - vertex.get() + vertex.delta());

      in_handle_position = dir * length + vertex.get();
    }

    handles.in_handle->move_to(in_handle_position);
  }

  PenTool::PenTool() : Tool(ToolType::Pen, CategoryDirect) {}

  void PenTool::on_pointer_down() {
    HoverState::HoverType hover_type = InputManager::hover.type();
    std::optional<Entity> entity = InputManager::hover.entity();

    m_vertex = nullptr;
    m_path = nullptr;

    if (!entity.has_value() || !entity->is_element()) {
      on_new_pointer_down();
      return;
    }

    Scene& scene = Editor::scene();
    auto hovered_vertex = InputManager::hover.vertex();

    m_path = &entity->get_component<PathComponent>().path;

    if (hover_type == HoverState::HoverType::Vertex && hovered_vertex.has_value()) {
      m_vertex = hovered_vertex->lock().get();

      if (m_path->is_open_end(m_vertex->id)) {
        if (entity->id() == m_element) {
          if (m_path->empty() ||
            (m_path->reversed() ? m_path->segments().front().p0_id() == m_vertex->id : m_path->segments().back().p3_id() == m_vertex->id)
            ) {
            on_angle_pointer_down();
            return;
          } else {
            on_close_pointer_down();
            return;
          }
        } else {
          if (m_element) {
            on_join_pointer_down();
            return;
          } else {
            set_pen_element(entity->id());
            on_start_pointer_down();
            return;
          }
        }
      } else if (scene.selection.has(entity->id())) {
        on_sub_pointer_down();
        return;
      }
    } else if (hover_type == HoverState::HoverType::Segment && scene.selection.has(entity->id())) {
      on_add_pointer_down();
      return;
    }

    on_new_pointer_down();
  }

  void PenTool::on_pointer_move() {
    if (!m_path || !m_vertex) return;

    switch (m_mode) {
    case Mode::Close:
      if (!m_path->closed()) return;
    case Mode::Join:
      return pen_pointer_move(*m_path, *m_vertex, true, true);
    case Mode::Add:
      pen_pointer_move(*m_path, *m_vertex, false, false, &m_direction);
      break;
    case Mode::Angle:
    case Mode::Start:
      pen_pointer_move(*m_path, *m_vertex, true, false);
      break;
    case Mode::New:
      pen_pointer_move(*m_path, *m_vertex);
      break;
    default:
    case Mode::Sub:
    case Mode::None:
      break;
    }
  }

  void PenTool::on_pointer_up() {
    if (!m_path) return;

    if (m_mode == Mode::Close) {
      set_pen_element(0);
    } else if (m_mode == Mode::Sub) {
      if (m_vertex && Math::squared_length(InputManager::pointer.scene.delta) < 10.0f / Editor::scene().viewport.zoom()) {
        m_path->remove(m_vertex->id, InputManager::keys.shift);
      }

      return;
    }

    auto in_handle = m_path->in_handle_ptr();
    auto out_handle = m_path->out_handle_ptr();
    auto vertex_ptr = m_path->last();

    if (in_handle) in_handle.value()->apply();
    if (out_handle) out_handle.value()->apply();

    if (m_vertex) {
      auto handles = m_path->relative_handles(m_vertex->id);
      float threshold = 2.5f / Editor::scene().viewport.zoom();

      m_vertex->apply();

      if (handles.in_handle) {
        if (Math::is_almost_equal(handles.in_handle->get(), m_vertex->get(), threshold)) {
          if (handles.in_segment) {
            if (m_path->reversed()) handles.in_segment->remove_p1();
            else handles.in_segment->remove_p2();
          } else {
            if (m_path->reversed()) m_path->clear_out_handle();
            else m_path->clear_in_handle();
          }
          console::log("collapsed in handle");
        } else {
          handles.in_handle->apply();
        }
      }

      if (handles.out_handle) {
        if (Math::is_almost_equal(handles.out_handle->get(), m_vertex->get(), threshold)) {
          if (handles.out_segment) {
            if (m_path->reversed()) handles.out_segment->remove_p2();
            else handles.out_segment->remove_p1();
          } else {
            if (m_path->reversed()) m_path->clear_in_handle();
            else m_path->clear_out_handle();
          }
          console::log("collapsed out handle");
        } else {
          handles.out_handle->apply();
        }
      }
    } else if (!vertex_ptr.expired()) {
      vertex_ptr.lock()->deep_apply();
    }
  }

  void PenTool::reset() {
    m_mode = Mode::New;
    set_pen_element(0);
  }

  void PenTool::render_overlays() const {
    if (!m_element || InputManager::pointer.down) return;

    Entity entity = Editor::scene().get_entity(m_element);
    if (!entity.is_element()) return;

    auto& path = entity.get_component<PathComponent>().path;
    if (path.vacant() || path.closed()) return;

    Renderer::Geometry::Internal::PathInternal segment{};
    History::Vec2Value* handle = nullptr;

    segment.move_to(path.last().lock()->get());

    if (path.reversed()) {
      auto in_handle_ptr = path.in_handle_ptr();
      if (in_handle_ptr.has_value()) handle = in_handle_ptr->get();
    } else {
      auto out_handle_ptr = path.out_handle_ptr();
      if (out_handle_ptr.has_value()) handle = out_handle_ptr->get();
    }

    if (handle) {
      segment.cubic_to(handle->get(), InputManager::pointer.scene.position, !path.reversed());
    } else {
      segment.line_to(InputManager::pointer.scene.position);
    }

    Renderer::Renderer::draw_outline(segment, { 0.0f, 0.0f });
  }

  /* -- on_pointer_down -- */

  void PenTool::on_new_pointer_down() {
    std::optional<Entity> entity = std::nullopt;
    Scene& scene = Editor::scene();

    if (!m_element) {
      entity = scene.create_element();
      set_pen_element(entity->id());
    } else {
      if (!scene.has_entity(m_element) || !(entity = scene.get_entity(m_element))->is_element()) {
        set_pen_element(0);
        return;
      }
    }

    m_path = &entity->get_component<PathComponent>().path;

    if (m_path->vacant()) {
      m_path->move_to(InputManager::pointer.scene.position);

      scene.selection.clear();
      scene.selection.select(m_element);
    } else if (m_path->reversed()) {
      auto in_handle_ptr = m_path->in_handle_ptr();

      if (in_handle_ptr.has_value()) {
        m_path->cubic_to(in_handle_ptr.value()->get(), InputManager::pointer.scene.position, true);
        m_path->clear_in_handle();
      } else {
        m_path->line_to(InputManager::pointer.scene.position);
      }
    } else {
      auto out_handle_ptr = m_path->out_handle_ptr();

      if (out_handle_ptr.has_value()) {
        m_path->cubic_to(out_handle_ptr.value()->get(), InputManager::pointer.scene.position, true);
        m_path->clear_out_handle();
      } else {
        m_path->line_to(InputManager::pointer.scene.position);
      }
    }

    m_vertex = m_path->last().lock().get();
    m_mode = Mode::New;
  }

  void PenTool::on_join_pointer_down() {
    if (!m_element || !m_path || !m_vertex) return;

    uuid vertex_id = m_vertex->id;
    uuid first_entity_id = m_element;
    uuid second_entity_id = m_path->id;

    History::CommandHistory::add(std::make_unique<History::FunctionCommand>(
      []() {},
      [this, first_entity_id, second_entity_id]() {
        Scene& scene = Editor::scene();

        scene.selection.clear();
        scene.selection.select(first_entity_id);
        scene.selection.select(second_entity_id);

        set_pen_element(first_entity_id);
      }
    ));

    Entity new_entity = Editor::scene().create_element();
    Entity first_entity = Editor::scene().get_entity(m_element);

    auto& first_path = first_entity.get_component<PathComponent>().path;
    auto& second_path = *m_path;
    auto& new_path = new_entity.get_component<PathComponent>().path;

    auto& first_segments = first_path.segments();
    auto& second_segments = second_path.segments();
    auto& new_segments = new_path.segments();

    std::shared_ptr<Renderer::Geometry::ControlPoint> p0 =
      first_path.empty() ? first_path.last().lock() :
      (first_path.reversed() ? first_segments.front().p0_ptr().lock() : first_segments.back().p3_ptr().lock());

    std::optional<vec2> p1 = std::nullopt;
    std::optional<vec2> p2 = std::nullopt;

    auto in_handle = first_path.in_handle_ptr();
    auto out_handle = first_path.out_handle_ptr();

    if (first_path.reversed()) {
      for (int i = (int)first_segments.size() - 1; i >= 0; i--) {
        new_segments.push_back(Renderer::Geometry::Segment::reverse(first_segments.at(i)));
      }

      if (in_handle) p1 = in_handle.value()->get();
      if (out_handle) new_path.create_in_handle(out_handle.value()->get());
    } else {
      for (auto& segment : first_segments) {
        new_segments.push_back(segment);
      }

      if (out_handle) p1 = out_handle.value()->get();
      if (in_handle) new_path.create_in_handle(in_handle.value()->get());
    }

    in_handle = second_path.in_handle_ptr();
    out_handle = second_path.out_handle_ptr();

    if (second_path.empty()) {
      std::shared_ptr<Renderer::Geometry::ControlPoint> p3 = second_path.last().lock();

      if (in_handle) p2 = in_handle.value()->get();
      if (out_handle) new_path.create_out_handle(out_handle.value()->get());

      new_segments.push_back(std::make_shared<Renderer::Geometry::Segment>(p0, p1, p2, p3));
    } else if (m_vertex->id == second_segments.front().p0_id()) {
      std::shared_ptr<Renderer::Geometry::ControlPoint> p3 = second_segments.front().p0_ptr().lock();

      if (in_handle) p2 = in_handle.value()->get();
      if (out_handle) new_path.create_out_handle(out_handle.value()->get());

      new_segments.push_back(std::make_shared<Renderer::Geometry::Segment>(p0, p1, p2, p3));

      for (auto& segment : second_segments) {
        new_segments.push_back(segment);
      }
    } else {
      std::shared_ptr<Renderer::Geometry::ControlPoint> p3 = second_segments.back().p3_ptr().lock();

      if (out_handle) p2 = out_handle.value()->get();
      if (in_handle) new_path.create_out_handle(in_handle.value()->get());

      new_segments.push_back(std::make_shared<Renderer::Geometry::Segment>(p0, p1, p2, p3));

      for (int i = (int)second_segments.size() - 1; i >= 0; i--) {
        new_segments.push_back(Renderer::Geometry::Segment::reverse(second_segments.at(i)));
      }
    }

    Editor::scene().delete_entity(first_entity);
    Editor::scene().delete_entity(second_path.id);

    History::CommandHistory::add(std::make_unique<History::FunctionCommand>(
      [this, new_path, vertex_id]() {
        Scene& scene = Editor::scene();

        scene.selection.clear();
        scene.selection.select_vertex(vertex_id, new_path.id);

        set_pen_element(0);
      },
      []() {}
    ));

    m_path = &new_path;
    m_mode = Mode::Join;
  }

  void PenTool::on_close_pointer_down() {
    if (!m_element || !m_path) return;

    m_path->close();

    m_vertex = m_path->last().lock().get();
    m_mode = Mode::Close;
  }

  void PenTool::on_sub_pointer_down() {
    m_mode = Mode::Sub;
  }

  void PenTool::on_add_pointer_down() {
    if (!m_path) return;

    std::optional<std::pair<std::weak_ptr<Renderer::Geometry::Segment>, float>> segment_hover = InputManager::hover.segment();
    if (!segment_hover.has_value()) return;

    auto& [segment_ptr, t] = segment_hover.value();
    if (segment_ptr.expired()) return;

    auto new_vertex = m_path->split(*segment_ptr.lock(), t);

    if (!new_vertex.has_value() || new_vertex->expired()) {
      m_mode = Mode::None;
      return;
    }

    m_vertex = new_vertex->lock().get();
    m_direction = 0;
    m_mode = Mode::Add;
  }

  void PenTool::on_angle_pointer_down() {
    if (!m_element || !m_path) return;

    if (m_path->reversed()) {
      m_path->clear_in_handle();
    } else {
      m_path->clear_out_handle();
    }

    m_mode = Mode::Angle;
  }

  void PenTool::on_start_pointer_down() {
    if (!m_element || !m_vertex || !m_path) return;

    Editor::scene().selection.select_vertex(m_vertex->id, m_element);

    if (!m_path->segments().empty()) {
      if (m_vertex->id == m_path->segments().front().p0_id()) {
        m_path->reverse();
      } else {
        m_path->reverse(false);
      }
    }

    m_mode = Mode::Start;
  }

}
