#include "pen_tool.h"

#include "../input_manager.h"

#include "../../editor.h"
#include "../../scene/entity.h"

#include "../../../renderer/renderer.h"

#include "../../../utils/console.h"

// TODO: esc to cancel pen and other tools
namespace Graphick::Editor::Input {

  PenTool::PenTool() : Tool(ToolType::Pen, CategoryDirect) {}

  void PenTool::on_pointer_down() {
    HoverState::HoverType hover_type = InputManager::hover.type();
    std::optional<Entity> entity = InputManager::hover.entity();

    if (!entity.has_value() || !entity->is_element()) {
      on_new_pointer_down();
      return;
    }

    Renderer::Geometry::Path& path = entity->get_component<PathComponent>().path;
    Scene& scene = Editor::scene();
    m_vertex = InputManager::hover.vertex();

    if (hover_type == HoverState::HoverType::Vertex) {
      auto vertex = m_vertex->lock();

      if (path.is_open_end(vertex->id)) {
        if (entity->id() == m_element.get()) {
          if (path.empty() ||
            (m_reverse ? path.segments().front().p0_id() == vertex->id : path.segments().back().p3_id() == vertex->id)
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
    m_element = 0;
  }

  void PenTool::render_overlays() const {
    if (!m_element || InputManager::pointer.down) return;

    Entity entity = Editor::scene().get_entity(m_element);
    if (!entity.is_element()) return;

    auto& path = entity.get_component<PathComponent>().path;
    if (path.vacant() || path.closed()) return;

    Renderer::Geometry::Internal::PathInternal segment{};
    auto out_handle = path.out_handle_ptr();
    segment.move_to(path.last().lock()->get());

    if (out_handle.has_value()) {
      segment.cubic_to(out_handle.value()->get(), InputManager::pointer.scene.position, true);
    } else {
      segment.line_to(InputManager::pointer.scene.position);
    }

    Renderer::Renderer::draw_outline(segment, { 0.0f, 0.0f });
  }

  /* -- on_pointer_down -- */

  void PenTool::on_new_pointer_down() {
    console::log("PenTool::down");

    std::optional<Entity> entity = std::nullopt;
    Scene& scene = Editor::scene();

    if (!m_element) {
      // TODO: set element position to pointer position and vertex to (0, 0)
      entity = scene.create_element();
      // TODO: always add this history to batch
      m_element = entity->id();
    } else {
      if (!scene.has_entity(m_element) || !(entity = scene.get_entity(m_element))->is_element()) {
        m_element = 0;
        return;
      }
    }

    Renderer::Geometry::Path& path = entity->get_component<PathComponent>().path;

    if (path.vacant()) {
      path.move_to(InputManager::pointer.scene.position);

      scene.selection.clear();
      scene.selection.select(m_element);
    } else {
      auto out_handle_ptr = path.out_handle_ptr();
      if (out_handle_ptr.has_value()) {
        path.cubic_to(out_handle_ptr.value()->get(), InputManager::pointer.scene.position, true);
        path.clear_out_handle();
      } else {
        path.line_to(InputManager::pointer.scene.position);
      }
    }

    m_mode = Mode::New;
  }

  void PenTool::on_join_pointer_down() {
    console::log("PenTool::join");
    m_mode = Mode::New;
  }

  void PenTool::on_close_pointer_down() {
    console::log("PenTool::close");
    if (!m_element) return;

    Editor::scene().get_entity(m_element).get_component<PathComponent>().path.close();

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
    m_mode = Mode::Angle;
  }

  void PenTool::on_start_pointer_down() {
    console::log("PenTool::start");
    m_mode = Mode::Start;
  }

  /* -- on_pointer_move -- */

  void PenTool::on_new_pointer_move() {
    if (!m_element) return;

    Renderer::Geometry::Path& path = Editor::scene().get_entity(m_element).get_component<PathComponent>().path;

    auto in_handle_ptr = path.in_handle_ptr();
    auto out_handle_ptr = path.out_handle_ptr();
    auto vertex_ptr = path.last();

    if (InputManager::keys.space) {
      if (!vertex_ptr.expired()) {
        vertex_ptr.lock()->add_delta(InputManager::pointer.scene.movement);
      }

      return;
    }

    if (path.empty()) {
      if (!out_handle_ptr.has_value()) {
        path.create_out_handle(InputManager::pointer.scene.origin);
        out_handle_ptr = path.out_handle_ptr();
      }

      out_handle_ptr.value()->set_delta(InputManager::pointer.scene.delta);

      if (!InputManager::keys.alt) {
        if (!in_handle_ptr.has_value()) {
          path.create_in_handle(InputManager::pointer.scene.origin);
          in_handle_ptr = path.in_handle_ptr();
        }

        in_handle_ptr.value()->move_to(2.0f * vertex_ptr.lock()->get() - InputManager::pointer.scene.position);
      }

      return;
    }

    if (!out_handle_ptr.has_value()) {
      path.create_out_handle(InputManager::pointer.scene.origin);
      out_handle_ptr = path.out_handle_ptr();
    }

    if (vertex_ptr.expired()) return;

    out_handle_ptr.value()->set_delta(InputManager::pointer.scene.delta);

    if (InputManager::keys.alt) return;

    Renderer::Geometry::Segment& segment = path.segments().back();

    if (!segment.has_p2()) {
      segment.create_p2(InputManager::pointer.scene.origin);
    }

    auto vertex = vertex_ptr.lock();
    auto handles = vertex->relative_handles();
    if (!vertex || handles.empty() || Math::is_almost_equal(out_handle_ptr.value()->get(), vertex->get())) return;

    for (auto h : handles) {
      auto h_ptr = h.lock();
      if (h_ptr && h_ptr != out_handle_ptr) {
        h_ptr->move_to(2.0f * vertex->get() - InputManager::pointer.scene.position);
      }
    }
  }

  void PenTool::on_join_pointer_move() {}

  void PenTool::on_close_pointer_move() {
    if (!m_element) return;

    Renderer::Geometry::Path& path = Editor::scene().get_entity(m_element).get_component<PathComponent>().path;
    auto vertex_ptr = path.last();

    if (!path.closed() || vertex_ptr.expired()) return;

    auto& first_segment = path.segments().front();
    auto& last_segment = path.segments().back();
    auto vertex = vertex_ptr.lock();

    if (!last_segment.has_p2()) {
      last_segment.create_p2(vertex->get());
    }

    auto p2_ptr = last_segment.p2_ptr().lock();
    p2_ptr->move_to(vertex->get() - InputManager::pointer.scene.delta);

    if (InputManager::keys.alt) return;

    if (!first_segment.has_p1()) {
      first_segment.create_p1(vertex->get());
    }

    auto p1_ptr = first_segment.p1_ptr().lock();

    vec2 dir = Math::normalize(InputManager::pointer.scene.delta);
    float length = Math::length(p1_ptr->get() - p1_ptr->delta() - vertex->get() + vertex->delta());

    p1_ptr->move_to(dir * length + vertex->get());
  }

  void PenTool::on_sub_pointer_move() {}

  void PenTool::on_add_pointer_move() {}

  void PenTool::on_angle_pointer_move() {}

  void PenTool::on_start_pointer_move() {}

  /* -- on_pointer_up -- */

  void PenTool::on_new_pointer_up() {
    if (!m_element) return;

    Renderer::Geometry::Path& path = Editor::scene().get_entity(m_element).get_component<PathComponent>().path;

    auto in_handle_ptr = path.in_handle_ptr();
    auto out_handle_ptr = path.out_handle_ptr();

    if (in_handle_ptr.has_value()) {
      in_handle_ptr.value()->apply();
    }
    if (out_handle_ptr.has_value()) {
      out_handle_ptr.value()->apply();
    }

    path.last().lock()->deep_apply();
  }

  void PenTool::on_join_pointer_up() {}

  void PenTool::on_close_pointer_up() {
    if (!m_element) return;

    Renderer::Geometry::Path& path = Editor::scene().get_entity(m_element).get_component<PathComponent>().path;
    path.last().lock()->deep_apply();

    m_element = 0;
  }

  void PenTool::on_sub_pointer_up() {}

  void PenTool::on_add_pointer_up() {}

  void PenTool::on_angle_pointer_up() {}

  void PenTool::on_start_pointer_up() {}

}
