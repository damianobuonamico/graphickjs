#include "pen_tool.h"

#include "../input_manager.h"

#include "../../editor.h"
#include "../../scene/entity.h"

#include "../../../utils/console.h"

namespace Graphick::Editor::Input {

  PenTool::PenTool() : Tool(ToolType::Pen, CategoryDirect) {}

  void PenTool::on_pointer_down() {
    //m_element = 0;

    HoverState::HoverType hover_type = InputManager::hover.type();
    std::optional<Entity> entity = InputManager::hover.entity();

    on_none_pointer_down();
  }

  void PenTool::on_pointer_move() {
    switch (m_mode) {
    default:
    case Mode::None:
      on_none_pointer_move();
      break;
    }
  }

  void PenTool::on_pointer_up() {
    switch (m_mode) {
    default:
    case Mode::None:
      on_none_pointer_up();
      break;
    }
  }

  void PenTool::reset() {
    m_mode = Mode::None;
    m_element = 0;
  }

  void PenTool::render_overlays() const {

  }

  void PenTool::on_none_pointer_down() {
    std::optional<Entity> entity = std::nullopt;
    Scene& scene = Editor::scene();

    if (!m_element) {
      // TODO: set element position to pointer position and vertex to (0, 0)
      entity = scene.create_element();
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
    } else {
      auto out_handle_ptr = path.out_handle_ptr();
      if (out_handle_ptr.has_value()) {
        path.cubic_to(out_handle_ptr.value()->get(), InputManager::pointer.scene.position, true);
        path.clear_out_handle();
      } else {
        path.line_to(InputManager::pointer.scene.position);
      }
    }

    scene.selection.clear();
    scene.selection.select(m_element);

    m_mode = Mode::None;
  }

  void PenTool::on_none_pointer_move() {
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

    Renderer::Geometry::Segment& segment = *path.segments().back();

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

  void PenTool::on_none_pointer_up() {
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

}
