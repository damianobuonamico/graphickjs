/**
 * @file pen_tool.cpp
 * @brief Implements the PenTool class.
 *
 * @todo esc to cancel pen and other tools
 * @todo shift to slow down movement
 * @todo create element at current position to avoid flickering at far distances
 * @todo carry attributes to new elements
 */

#include "pen_tool.h"

#include "common.h"

#include "../input_manager.h"

#include "../../editor.h"
#include "../../scene/entity.h"

#include "../../../math/vector.h"
#include "../../../math/scalar.h"

#include "../../../renderer/renderer.h"

namespace Graphick::Editor::Input {

  PenTool::PenTool() : Tool(ToolType::Pen, CategoryDirect) {}

  void PenTool::on_pointer_down() {
    m_vertex.reset();

    HoverState::HoverType hover_type = InputManager::hover.type();
    std::optional<Entity> entity = InputManager::hover.entity();

    if (!entity.has_value() || !entity->is_element()) {
      return on_new_pointer_down();
    }

    Scene& scene = Editor::scene();

    TransformComponent transform = entity->get_component<TransformComponent>();
    PathComponent path = entity->get_component<PathComponent>();

    if (hover_type == HoverState::HoverType::Vertex) {
      std::optional<size_t> active_vertex = m_vertex;

      m_vertex = InputManager::hover.vertex().value();

      if (path.data().is_open_end(m_vertex.value())) {
        if (entity->id() == m_element) {
          if (path.data().empty() || (active_vertex.has_value() && active_vertex.value() == m_vertex.value())) {
            return on_angle_pointer_down();
          } else {
            return on_close_pointer_down();
          }
        } else {
          if (m_element) {
            return on_join_pointer_down();
          } else {
            set_pen_element(entity->id());
            return on_start_pointer_down();
          }
        }
      } else if (scene.selection.has(entity->id())) {
        return on_sub_pointer_down();
      }
    } else if (hover_type == HoverState::HoverType::Segment && scene.selection.has(entity->id())) {
      return on_add_pointer_down();
    }

    on_new_pointer_down();
  }

  void PenTool::on_pointer_move() {
    if (!m_element || !m_vertex.has_value()) return;

    Entity entity = Editor::scene().get_entity(m_element);

    TransformComponent transform = entity.get_component<TransformComponent>();
    PathComponent path = entity.get_component<PathComponent>();

    switch (m_mode) {
    case Mode::Close:
      if (!path.data().closed()) return;
    case Mode::Join:
      return translate_control_point(path, m_vertex.value(), transform, nullptr, true, true, true, nullptr);
    case Mode::Add:
      return translate_control_point(path, m_vertex.value(), transform, nullptr, true, false, false, &m_direction);
      break;
    case Mode::Angle:
    case Mode::Start:
      return translate_control_point(path, m_vertex.value(), transform, nullptr, true, true, false, nullptr);
      break;
    case Mode::New:
      return translate_control_point(path, m_vertex.value(), transform, nullptr, true, false, false, nullptr);
      break;
    default:
    case Mode::Sub:
    case Mode::None:
      break;
    }
  }

  void PenTool::on_pointer_up() {
#if 0
    if (!m_path) return;

    if (m_mode == Mode::Close) {
      set_pen_element(uuid::null);
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
        } else {
          handles.out_handle->apply();
        }
      }
    } else if (!vertex_ptr.expired()) {
      vertex_ptr.lock()->apply();
    }
#endif
  }

  void PenTool::reset() {
    m_mode = Mode::New;
    set_pen_element(uuid::null);
  }

  void PenTool::render_overlays() const {
#if 0
    if (!m_element || !m_transform || InputManager::pointer.down) return;

    Entity entity = Editor::scene().get_entity(m_element);
    if (!entity.is_element()) return;
#endif

#if 0
    auto& path = entity.get_component<PathComponent>().path;
    if (path.vacant() || path.closed()) return;

    Renderer::Geometry::Internal::PathInternal segment{};
    mat2x3 transform = m_transform->get();
    History::Vec2Value* handle = nullptr;

    segment.move_to(transform * path.last().lock()->get());

    if (path.reversed()) {
      auto in_handle_ptr = path.in_handle_ptr();
      if (in_handle_ptr.has_value()) handle = in_handle_ptr->get();
    } else {
      auto out_handle_ptr = path.out_handle_ptr();
      if (out_handle_ptr.has_value()) handle = out_handle_ptr->get();
    }

    if (handle) {
      segment.cubic_to(transform * handle->get(), InputManager::pointer.scene.position, !path.reversed());
    } else {
      segment.line_to(InputManager::pointer.scene.position);
    }

    Renderer::Renderer::draw_outline(segment);
#endif
  }

  void PenTool::set_pen_element(const uuid id) {
    m_element = id;
    m_reverse = false;
  }

  /* -- on_pointer_down -- */

  void PenTool::on_new_pointer_down() {
    std::optional<Entity> entity = std::nullopt;

    Scene& scene = Editor::scene();

    if (!m_element) {
      entity = scene.create_element();
      entity->add_component<StrokeComponent>();
      entity->add_component<FillComponent>(vec4{ 0.8f, 0.3f, 0.3f, 1.0f });

      set_pen_element(entity->id());
    } else {
      if (!scene.has_entity(m_element) || !(entity = scene.get_entity(m_element))->is_element()) {
        set_pen_element(uuid::null);
        return;
      }
    }

    TransformComponent transform = entity->get_component<TransformComponent>();
    PathComponent path = entity->get_component<PathComponent>();

    const mat2x3 inverse_transform = transform.inverse();
    const vec2 pointer_position = inverse_transform * InputManager::pointer.scene.position;

    if (path.data().vacant()) {
      m_vertex = path.move_to(pointer_position);

      scene.selection.clear();
      scene.selection.select(m_element);
    } else {
      m_vertex = path.line_to(pointer_position, m_reverse);
    }

    // else if (m_reverse) {
    //   auto in_handle_ptr = m_path->in_handle_ptr();

    //   if (in_handle_ptr.has_value()) {
    //     m_path->cubic_to(in_handle_ptr.value()->get(), pointer_position, true);
    //     m_path->clear_in_handle();
    //   } else {
    //     m_path->line_to(pointer_position);
    //   }
    // } else {
    //   auto out_handle_ptr = m_path->out_handle_ptr();

    //   if (out_handle_ptr.has_value()) {
    //     m_path->cubic_to(out_handle_ptr.value()->get(), pointer_position, true);
    //     m_path->clear_out_handle();
    //   } else {
    //     m_path->line_to(pointer_position);
    //   }
    // }

    m_mode = Mode::New;
  }

  void PenTool::on_join_pointer_down() {
#if 0
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

    Entity first_entity = Editor::scene().get_entity(m_element);
    Entity second_entity = Editor::scene().get_entity(m_path->id);
    Entity new_entity = Editor::scene().create_element();
#endif

#if 0
    auto& first_path = first_entity.get_component<PathComponent>().path;
    auto& second_path = *m_path;
    auto& new_path = new_entity.get_component<PathComponent>().path;

    auto& first_segments = first_path.segments();
    auto& second_segments = second_path.segments();
    auto& new_segments = new_path.segments();

    mat2x3 first_transform = first_entity.get_component<TransformComponent>().get();
    mat2x3 second_transform = second_entity.get_component<TransformComponent>().get();
    History::Mat2x3Value* new_transform = new_entity.get_component<TransformComponent>()._value();

    std::shared_ptr<Renderer::Geometry::ControlPoint> p0 =
      first_path.empty() ? first_path.last().lock() :
      (first_path.reversed() ? first_segments.front().p0_ptr().lock() : first_segments.back().p3_ptr().lock());

    std::optional<vec2> p1 = std::nullopt;
    std::optional<vec2> p2 = std::nullopt;

    auto in_handle = first_path.in_handle_ptr();
    auto out_handle = first_path.out_handle_ptr();

    if (first_path.reversed()) {
      for (int i = (int)first_segments.size() - 1; i >= 0; i--) {
        std::shared_ptr<Renderer::Geometry::Segment> reversed = Renderer::Geometry::Segment::reverse(first_segments.at(i));
        Renderer::Geometry::Segment::transform(*reversed, first_transform, false);
        new_segments.push_back(reversed);
      }

      auto last_ptr = new_segments.back().p3_ptr().lock();
      last_ptr->set(first_transform * last_ptr->get());

      if (in_handle) p1 = first_transform * in_handle.value()->get();
      if (out_handle) new_path.create_in_handle(first_transform * out_handle.value()->get());
    } else {
      for (auto& segment : first_segments) {
        Renderer::Geometry::Segment::transform(*segment, first_transform, false);
        new_segments.push_back(segment);
      }

      auto last_ptr = new_segments.back().p3_ptr().lock();
      last_ptr->set(first_transform * last_ptr->get());

      if (out_handle) p1 = first_transform * out_handle.value()->get();
      if (in_handle) new_path.create_in_handle(first_transform * in_handle.value()->get());
    }

    in_handle = second_path.in_handle_ptr();
    out_handle = second_path.out_handle_ptr();

    if (second_path.empty()) {
      std::shared_ptr<Renderer::Geometry::ControlPoint> p3 = second_path.last().lock();
      p3->set(second_transform * p3->get());

      if (in_handle) p2 = second_transform * in_handle.value()->get();
      if (out_handle) new_path.create_out_handle(second_transform * out_handle.value()->get());

      new_segments.push_back(std::make_shared<Renderer::Geometry::Segment>(p0, p1, p2, p3));
    } else if (m_vertex->id == second_segments.front().p0_id()) {
      std::shared_ptr<Renderer::Geometry::ControlPoint> p3 = second_segments.front().p0_ptr().lock();

      if (in_handle) p2 = second_transform * in_handle.value()->get();
      if (out_handle) new_path.create_out_handle(second_transform * out_handle.value()->get());

      new_segments.push_back(std::make_shared<Renderer::Geometry::Segment>(p0, p1, p2, p3));

      for (auto& segment : second_segments) {
        Renderer::Geometry::Segment::transform(*segment, second_transform, false);
        new_segments.push_back(segment);
      }

      auto last_ptr = new_segments.back().p3_ptr().lock();
      last_ptr->set(second_transform * last_ptr->get());
    } else {
      std::shared_ptr<Renderer::Geometry::ControlPoint> p3 = second_segments.back().p3_ptr().lock();

      if (out_handle) p2 = second_transform * out_handle.value()->get();
      if (in_handle) new_path.create_out_handle(second_transform * in_handle.value()->get());

      new_segments.push_back(std::make_shared<Renderer::Geometry::Segment>(p0, p1, p2, p3));

      size_t size = new_segments.size();

      for (int i = (int)second_segments.size() - 1; i >= 0; i--) {
        std::shared_ptr<Renderer::Geometry::Segment> reversed = Renderer::Geometry::Segment::reverse(second_segments.at(i));
        Renderer::Geometry::Segment::transform(*reversed, second_transform, false);
        new_segments.push_back(reversed);
      }

      auto last_ptr = new_segments.back().p3_ptr().lock();
      last_ptr->set(second_transform * last_ptr->get());
    }

    Editor::scene().delete_entity(first_entity);
    Editor::scene().delete_entity(second_path.id);

    History::CommandHistory::add(std::make_unique<History::FunctionCommand>(
      [this, new_path, new_transform, vertex_id]() {
        Scene& scene = Editor::scene();

        scene.selection.clear();
        scene.selection.select_vertex(new_path.id, vertex_id);

        m_transform = new_transform;
        set_pen_element(uuid::null);
      },
      []() {}
    ));

    m_path = &new_path;
    m_mode = Mode::Join;
#endif
  }

  void PenTool::on_close_pointer_down() {
#if 0
    if (!m_element || !m_path) return;

    m_path->close();

    m_vertex = m_path->last().lock().get();
    m_mode = Mode::Close;
#endif
  }

  void PenTool::on_sub_pointer_down() {
    m_mode = Mode::Sub;
  }

  void PenTool::on_add_pointer_down() {
#if 0
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
#endif
  }

  void PenTool::on_angle_pointer_down() {
#if 0
    if (!m_element || !m_path) return;

    if (m_path->reversed()) {
      m_path->clear_in_handle();
    } else {
      m_path->clear_out_handle();
    }

    m_mode = Mode::Angle;
#endif
  }

  void PenTool::on_start_pointer_down() {
    if (!m_element || !m_vertex.has_value()) return;

    // Editor::scene().selection.select_vertex(m_entity, m_vertex.value());

    Entity entity = Editor::scene().get_entity(m_element);
    PathComponent path = entity.get_component<PathComponent>();

    if (!path.data().empty()) {
      if (m_vertex.value() == 0) {
        m_reverse = true;
      } else {
        m_reverse = false;
      }
    }

    m_mode = Mode::Start;
  }

}
