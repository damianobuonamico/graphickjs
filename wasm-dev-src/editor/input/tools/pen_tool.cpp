#include "pen_tool.h"

#include "../input_manager.h"

#include "../../editor.h"
#include "../../scene/entity.h"

#include "../../../history/command_history.h"

#include "../../../renderer/renderer.h"

#include "../../../utils/console.h"

// TODO: esc to cancel pen and other tools
// TODO: cleanup (code repetition, make more robus and consistent)
namespace Graphick::Editor::Input {

  PenTool::PenTool() : Tool(ToolType::Pen, CategoryDirect) {}

  // TODO: fix close move not working by caching path and vertex 
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
    switch (m_mode) {
    case Mode::Join:
      on_join_pointer_move();
      break;
    case Mode::Close:
      on_close_pointer_move();
      break;
    case Mode::Sub:
      on_sub_pointer_move();
      break;
    case Mode::Add:
      on_add_pointer_move();
      break;
    case Mode::Angle:
      on_angle_pointer_move();
      break;
    case Mode::Start:
      on_start_pointer_move();
      break;
    default:
    case Mode::New:
      on_new_pointer_move();
      break;
    }
  }

  void PenTool::on_pointer_up() {
    switch (m_mode) {
    case Mode::Join:
      on_join_pointer_up();
      break;
    case Mode::Close:
      on_close_pointer_up();
      break;
    case Mode::Sub:
      on_sub_pointer_up();
      break;
    case Mode::Add:
      on_add_pointer_up();
      break;
    case Mode::Angle:
      on_angle_pointer_up();
      break;
    case Mode::Start:
      on_start_pointer_up();
      break;
    default:
    case Mode::New:
      on_new_pointer_up();
      break;
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

    console::log("reversed", path.reversed());

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
    console::log("PenTool::new");

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
    console::log("PenTool::join");
    m_mode = Mode::Join;
  }

  void PenTool::on_close_pointer_down() {
    console::log("PenTool::close");
    if (!m_element || !m_path) return;

    m_path->close();

    m_vertex = m_path->last().lock().get();
    m_mode = Mode::Close;
  }

  void PenTool::on_sub_pointer_down() {
    console::log("PenTool::sub");
    m_mode = Mode::Sub;
  }

  void PenTool::on_add_pointer_down() {
    console::log("PenTool::add");
    m_mode = Mode::Add;
  }

  void PenTool::on_angle_pointer_down() {
    console::log("PenTool::angle");

    if (!m_element || !m_path) return;

    if (m_path->reversed()) {
      m_path->clear_in_handle();
    } else {
      m_path->clear_out_handle();
    }

    m_mode = Mode::Angle;
  }

  void PenTool::on_start_pointer_down() {
    console::log("PenTool::start");

    if (!m_element || !m_vertex || !m_path) return;

    Editor::scene().selection.select_vertex(m_vertex->id, m_element);

    if (m_vertex->id == m_path->segments().front().p0_id()) {
      m_path->reverse();
    } else {
      m_path->reverse(false);
    }

    m_mode = Mode::Start;
  }

  /* -- on_pointer_move -- */

  void PenTool::on_new_pointer_move() {
    if (!m_element || !m_path || !m_vertex) return;

    auto in_handle_ptr = m_path->in_handle_ptr();
    auto out_handle_ptr = m_path->out_handle_ptr();

    if (InputManager::keys.space) {
      m_vertex->add_delta(InputManager::pointer.scene.movement);
      return;
    }

    if (m_path->empty()) {
      if (!out_handle_ptr.has_value()) {
        m_path->create_out_handle(InputManager::pointer.scene.origin);
        out_handle_ptr = m_path->out_handle_ptr();
      }

      out_handle_ptr.value()->set_delta(InputManager::pointer.scene.delta);

      if (!InputManager::keys.alt) {
        if (!in_handle_ptr.has_value()) {
          m_path->create_in_handle(InputManager::pointer.scene.origin);
          in_handle_ptr = m_path->in_handle_ptr();
        }

        in_handle_ptr.value()->move_to(2.0f * m_vertex->get() - InputManager::pointer.scene.position);
      }

      return;
    }

    History::Vec2Value* p1_ptr = nullptr;
    History::Vec2Value* p2_ptr = nullptr;

    if (m_path->reversed()) {
      m_path->create_in_handle(InputManager::pointer.scene.origin);
      p1_ptr = m_path->in_handle_ptr().value().get();
    } else {
      m_path->create_out_handle(InputManager::pointer.scene.origin);
      p1_ptr = m_path->out_handle_ptr().value().get();
    }

    p1_ptr->set_delta(InputManager::pointer.scene.delta);

    if (InputManager::keys.alt || Math::is_almost_equal(p1_ptr->get(), m_vertex->get())) return;

    if (m_path->reversed()) {
      Renderer::Geometry::Segment& segment = m_path->segments().front();

      if (!segment.has_p1()) {
        segment.create_p1(InputManager::pointer.scene.origin);
      }

      p2_ptr = segment.p1_ptr().lock().get();
    } else {
      Renderer::Geometry::Segment& segment = m_path->segments().back();

      if (!segment.has_p2()) {
        segment.create_p2(InputManager::pointer.scene.origin);
      }

      p2_ptr = segment.p2_ptr().lock().get();
    }

    p2_ptr->move_to(2.0f * m_vertex->get() - InputManager::pointer.scene.position);
  }

  void PenTool::on_join_pointer_move() {}

  // TODO: fix reverse path history
  // TODO: snap handles to vertex destroys them on apply
  void PenTool::on_close_pointer_move() {
    if (!m_path || !m_vertex || !m_path->closed()) return;

    // TODO: space to move vertex
    auto& first_segment = m_path->segments().front();
    auto& last_segment = m_path->segments().back();

    History::Vec2Value* p1_ptr = nullptr;
    History::Vec2Value* p2_ptr = nullptr;

    if (m_path->reversed()) {
      if (!first_segment.has_p1()) {
        first_segment.create_p1(m_vertex->get());
      }

      p2_ptr = first_segment.p1_ptr().lock().get();
    } else {
      if (!last_segment.has_p2()) {
        last_segment.create_p2(m_vertex->get());
      }

      p2_ptr = last_segment.p2_ptr().lock().get();
    }

    p2_ptr->move_to(m_vertex->get() - InputManager::pointer.scene.delta);

    if (InputManager::keys.alt) return;

    if (m_path->reversed()) {
      if (!last_segment.has_p2()) {
        last_segment.create_p2(m_vertex->get());
      }

      p1_ptr = last_segment.p2_ptr().lock().get();
    } else {
      if (!first_segment.has_p1()) {
        first_segment.create_p1(m_vertex->get());
      }

      p1_ptr = first_segment.p1_ptr().lock().get();
    }

    vec2 dir = Math::normalize(m_vertex->get() - p2_ptr->get());
    float length = Math::length(p1_ptr->get() - p1_ptr->delta() - m_vertex->get() + m_vertex->delta());

    p1_ptr->move_to(dir * length + m_vertex->get());
  }

  void PenTool::on_sub_pointer_move() {}

  void PenTool::on_add_pointer_move() {}

  void PenTool::on_angle_pointer_move() {
    on_new_pointer_move();
  }

  void PenTool::on_start_pointer_move() {
    if (!m_element || !m_vertex) return;

    Entity entity = Editor::scene().get_entity(m_element);
    auto& path = entity.get_component<PathComponent>().path;

    History::Vec2Value* p1_ptr = nullptr;
    History::Vec2Value* p2_ptr = nullptr;

    if (path.reversed()) {
      path.create_in_handle(InputManager::pointer.scene.origin);
      p1_ptr = path.in_handle_ptr().value().get();
    } else {
      path.create_out_handle(InputManager::pointer.scene.origin);
      p1_ptr = path.out_handle_ptr().value().get();
    }

    p1_ptr->set_delta(InputManager::pointer.scene.delta);

    if (InputManager::keys.alt || Math::is_almost_equal(p1_ptr->get(), m_vertex->get())) return;

    if (path.reversed()) {
      Renderer::Geometry::Segment& segment = path.segments().front();

      if (!segment.has_p1()) {
        segment.create_p1(InputManager::pointer.scene.origin);
      }

      p2_ptr = segment.p1_ptr().lock().get();
    } else {
      Renderer::Geometry::Segment& segment = path.segments().back();

      if (!segment.has_p2()) {
        segment.create_p2(InputManager::pointer.scene.origin);
      }

      p2_ptr = segment.p2_ptr().lock().get();
    }

    vec2 dir = Math::normalize(m_vertex->get() - p1_ptr->get());
    float length = Math::length(p2_ptr->get() - p2_ptr->delta() - m_vertex->get() + m_vertex->delta());

    p2_ptr->move_to(dir * length + m_vertex->get());
  }

  /* -- on_pointer_up -- */

  // TODO: support reversed paths
  void PenTool::on_new_pointer_up() {
    if (!m_element || !m_path) return;

    auto in_handle_ptr = m_path->in_handle_ptr();
    auto out_handle_ptr = m_path->out_handle_ptr();

    if (in_handle_ptr.has_value()) {
      in_handle_ptr.value()->apply();
    }
    if (out_handle_ptr.has_value()) {
      out_handle_ptr.value()->apply();
    }

    auto vertex_ptr = m_path->last();
    if (vertex_ptr.expired()) return;

    vertex_ptr.lock()->deep_apply();
  }

  void PenTool::on_join_pointer_up() {}

  void PenTool::on_close_pointer_up() {
    if (!m_path) return;

    m_path->last().lock()->deep_apply();

    set_pen_element(0);
  }

  void PenTool::on_sub_pointer_up() {}

  void PenTool::on_add_pointer_up() {}

  void PenTool::on_angle_pointer_up() {
    on_new_pointer_up();
  }

  void PenTool::on_start_pointer_up() {
    on_new_pointer_up();
  }

}
