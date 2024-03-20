/**
 * @file pen_tool.cpp
 * @brief Contains the implementation of the PenTool class.
 *
 * @todo esc to cancel pen and other tools
 * @todo create element at current position to avoid flickering at far distances
 * @todo carry attributes to new elements
 */

#include "pen_tool.h"

#include "common.h"

#include "../input_manager.h"

#include "../../editor.h"
#include "../../scene/entity.h"

#include "../../../math/math.h"

#include "../../../renderer/renderer.h"

namespace Graphick::Editor::Input {

  PenTool::PenTool() : Tool(ToolType::Pen, CategoryDirect) {}

  void PenTool::on_pointer_down() {
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
            m_temp_element = entity->id();
            return on_join_pointer_down();
          } else {
            set_pen_element(entity->id());
            return on_start_pointer_down();
          }
        }
      } else if (scene.selection.has(entity->id())) {
        m_temp_element = entity->id();
        return on_sub_pointer_down();
      }
    } else if (hover_type == HoverState::HoverType::Segment && scene.selection.has(entity->id())) {
      m_temp_element = entity->id();
      return on_add_pointer_down();
    }

    on_new_pointer_down();
  }

  void PenTool::on_pointer_move() {
    if ((!m_element && !m_temp_element) || !m_vertex.has_value()) return;

    Entity entity = Editor::scene().get_entity(m_temp_element ? m_temp_element : m_element);

    TransformComponent transform = entity.get_component<TransformComponent>();
    PathComponent path = entity.get_component<PathComponent>();

    int direction = m_reverse ? -1 : 1;

    switch (m_mode) {
    case Mode::Close:
      if (!path.data().closed()) return;
    case Mode::Join:
      m_vertex = translate_control_point(path, m_vertex.value(), transform, nullptr, true, true, true, nullptr);
      break;
    case Mode::Add:
      m_vertex = translate_control_point(path, m_vertex.value(), transform, nullptr, true, false, false, &m_direction);
      break;
    case Mode::Start:
    case Mode::Angle:
      m_vertex = translate_control_point(path, m_vertex.value(), transform, nullptr, true, true, false, &direction);
      break;
    case Mode::New:
      m_vertex = translate_control_point(path, m_vertex.value(), transform, nullptr, true, false, false, &direction);
      break;
    default:
    case Mode::Sub:
    case Mode::None:
      break;
    }
  }

  void PenTool::on_pointer_up() {
    Scene& scene = Editor::scene();

    if (m_mode == Mode::Sub) {
      if (m_vertex.has_value() && Math::squared_length(InputManager::pointer.scene.delta) < 10.0f / scene.viewport.zoom()) {
        PathComponent path = scene.get_entity(m_temp_element).get_component<PathComponent>();
        path.remove(m_vertex.value(), InputManager::keys.shift);
      }

      m_temp_element = uuid::null;
      return;
    }

    m_temp_element = uuid::null;

    if (m_mode == Mode::Join) {
      return;
    }

    if (!m_element || !m_vertex.has_value()) return;

    Entity entity = scene.get_entity(m_element);
    PathComponent path = entity.get_component<PathComponent>();

    const Renderer::Geometry::Path::VertexNode node = path.data().node_at(m_vertex.value());
    const float threshold = 2.5f / Editor::scene().viewport.zoom();

    if (node.in >= 0) {
      const vec2 in_handle = path.data().point_at(static_cast<size_t>(node.in));
      const vec2 vertex = path.data().point_at(node.vertex);

      if (Math::is_almost_equal(in_handle, vertex, threshold)) {
        path.translate(static_cast<size_t>(node.in), vertex - in_handle);

        if (node.in_command >= 0) {
          const Renderer::Geometry::Path::Segment segment = path.data().at(static_cast<size_t>(node.in_command), Renderer::Geometry::Path::IndexType::Command);

          if (segment.is_line()) {
            path.to_cubic(static_cast<size_t>(node.in_command));
          }
        }
      }
    }

    if (node.out >= 0) {
      const vec2 out_handle = path.data().point_at(static_cast<size_t>(node.out));
      const vec2 vertex = path.data().point_at(node.vertex);

      if (Math::is_almost_equal(out_handle, vertex, threshold)) {
        path.translate(static_cast<size_t>(node.out), vertex - out_handle);

        if (node.out_command >= 0) {
          const Renderer::Geometry::Path::Segment segment = path.data().at(static_cast<size_t>(node.out_command), Renderer::Geometry::Path::IndexType::Command);

          if (segment.is_line()) {
            path.to_cubic(static_cast<size_t>(node.out_command));
          }
        }
      }
    }

    if (m_mode == Mode::Close) {
      set_pen_element(uuid::null);
    }
  }

  void PenTool::reset() {
    m_mode = Mode::New;
    set_pen_element(uuid::null);
  }

  void PenTool::render_overlays() const {
    if (!m_element || InputManager::pointer.down) return;

    const Entity entity = Editor::scene().get_entity(m_element);
    const PathComponent path = entity.get_component<PathComponent>();
    const mat2x3 transform = entity.get_component<TransformComponent>();

    if (path.data().vacant() || path.data().closed()) return;

    Renderer::Geometry::Path segment;
    std::optional<vec2> handle = std::nullopt;

    if (m_reverse) {
      segment.move_to(transform * path.data().point_at(0));

      if (path.data().has_in_handle()) {
        handle = path.data().point_at(Renderer::Geometry::Path::in_handle_index);
      }
    } else {
      segment.move_to(transform * path.data().point_at(path.data().points_size() - 1));

      if (path.data().has_out_handle()) {
        handle = path.data().point_at(Renderer::Geometry::Path::out_handle_index);
      }
    }

    if (handle) {
      segment.cubic_to(transform * handle.value(), InputManager::pointer.scene.position, !m_reverse);
    } else {
      segment.line_to(InputManager::pointer.scene.position);
    }

    Renderer::Renderer::draw_outline(segment);
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
    } else if (m_reverse) {
      const bool has_in_handle = path.data().has_in_handle();

      if (has_in_handle) {
        m_vertex = path.cubic_to(path.data().point_at(Renderer::Geometry::Path::in_handle_index), pointer_position, pointer_position, m_reverse);
      } else {
        m_vertex = path.line_to(pointer_position, m_reverse);
      }
    } else {
      const bool has_out_handle = path.data().has_out_handle();

      if (has_out_handle) {
        m_vertex = path.cubic_to(path.data().point_at(Renderer::Geometry::Path::out_handle_index), pointer_position, pointer_position, m_reverse);
      } else {
        m_vertex = path.line_to(pointer_position, m_reverse);
      }
    }

    m_mode = Mode::New;
  }

  void PenTool::on_join_pointer_down() {
    if (!m_element || !m_temp_element || !m_vertex) return;

    Scene& scene = Editor::scene();
    Entity first_entity = scene.get_entity(m_element);
    Entity second_entity = scene.get_entity(m_temp_element);
    Entity new_entity = scene.create_element();

    const uuid first_id = first_entity.id();
    const uuid second_id = second_entity.id();
    const uuid new_id = new_entity.id();

    const PathComponent first_path = first_entity.get_component<PathComponent>();
    const PathComponent second_path = second_entity.get_component<PathComponent>();
    const mat2x3 first_transform = first_entity.get_component<TransformComponent>();
    const mat2x3 second_transform = second_entity.get_component<TransformComponent>();

    PathComponent new_path = new_entity.get_component<PathComponent>();
    new_entity.add_component<StrokeComponent>(vec4{ 1.0f, 0.0f, 0.4f, 1.0f });

    first_path.data().for_each(
      [&](const vec2 p0) {
        new_path.move_to(first_transform * p0);
      },
      [&](const vec2 p1) {
        new_path.line_to(first_transform * p1);
      },
      [&](const vec2 p1, const vec2 p2) {
        // TODO: implement and test quadratic
        // new_path.quadratic_to(p1, p2);
      },
      [&](const vec2 p1, const vec2 p2, const vec2 p3) {
        new_path.cubic_to(first_transform * p1, first_transform * p2, first_transform * p3);
      }
    );

    const vec2 in_p1 = first_path.data().has_out_handle() ? first_path.data().point_at(Renderer::Geometry::Path::out_handle_index) : first_path.data().point_at(first_path.data().points_size() - 1);
    const vec2 in_p2 = second_path.data().has_in_handle() ? second_path.data().point_at(Renderer::Geometry::Path::in_handle_index) : second_path.data().point_at(0);

    second_path.data().for_each(
      [&](const vec2 p0) {
        new_path.cubic_to(first_transform * in_p1, second_transform * in_p2, second_transform * p0);
      },
      [&](const vec2 p1) {
        new_path.line_to(second_transform * p1);
      },
      [&](const vec2 p1, const vec2 p2) {
        // TODO: implement and test quadratic
        // new_path.quadratic_to(p1, p2);
      },
      [&](const vec2 p1, const vec2 p2, const vec2 p3) {
        new_path.cubic_to(second_transform * p1, second_transform * p2, second_transform * p3);
      }
    );

#if 0
    // std::shared_ptr<Renderer::Geometry::ControlPoint> p0 =
    //   first_path.empty() ? first_path.last().lock() :
    //   (first_path.reversed() ? first_segments.front().p0_ptr().lock() : first_segments.back().p3_ptr().lock());

    // std::optional<vec2> p1 = std::nullopt;
    // std::optional<vec2> p2 = std::nullopt;

    // auto in_handle = first_path.in_handle_ptr();
    // auto out_handle = first_path.out_handle_ptr();

    // if (first_path.reversed()) {
    //   for (int i = (int)first_segments.size() - 1; i >= 0; i--) {
    //     std::shared_ptr<Renderer::Geometry::Segment> reversed = Renderer::Geometry::Segment::reverse(first_segments.at(i));
    //     Renderer::Geometry::Segment::transform(*reversed, first_transform, false);
    //     new_segments.push_back(reversed);
    //   }

    //   auto last_ptr = new_segments.back().p3_ptr().lock();
    //   last_ptr->set(first_transform * last_ptr->get());

    //   if (in_handle) p1 = first_transform * in_handle.value()->get();
    //   if (out_handle) new_path.create_in_handle(first_transform * out_handle.value()->get());
    // } else {
    //   for (auto& segment : first_segments) {
    //     Renderer::Geometry::Segment::transform(*segment, first_transform, false);
    //     new_segments.push_back(segment);
    //   }

    //   auto last_ptr = new_segments.back().p3_ptr().lock();
    //   last_ptr->set(first_transform * last_ptr->get());

    //   if (out_handle) p1 = first_transform * out_handle.value()->get();
    //   if (in_handle) new_path.create_in_handle(first_transform * in_handle.value()->get());
    // }

    // in_handle = second_path.in_handle_ptr();
    // out_handle = second_path.out_handle_ptr();

    // if (second_path.empty()) {
    //   std::shared_ptr<Renderer::Geometry::ControlPoint> p3 = second_path.last().lock();
    //   p3->set(second_transform * p3->get());

    //   if (in_handle) p2 = second_transform * in_handle.value()->get();
    //   if (out_handle) new_path.create_out_handle(second_transform * out_handle.value()->get());

    //   new_segments.push_back(std::make_shared<Renderer::Geometry::Segment>(p0, p1, p2, p3));
    // } else if (m_vertex->id == second_segments.front().p0_id()) {
    //   std::shared_ptr<Renderer::Geometry::ControlPoint> p3 = second_segments.front().p0_ptr().lock();

    //   if (in_handle) p2 = second_transform * in_handle.value()->get();
    //   if (out_handle) new_path.create_out_handle(second_transform * out_handle.value()->get());

    //   new_segments.push_back(std::make_shared<Renderer::Geometry::Segment>(p0, p1, p2, p3));

    //   for (auto& segment : second_segments) {
    //     Renderer::Geometry::Segment::transform(*segment, second_transform, false);
    //     new_segments.push_back(segment);
    //   }

    //   auto last_ptr = new_segments.back().p3_ptr().lock();
    //   last_ptr->set(second_transform * last_ptr->get());
    // } else {
    //   std::shared_ptr<Renderer::Geometry::ControlPoint> p3 = second_segments.back().p3_ptr().lock();

    //   if (out_handle) p2 = second_transform * out_handle.value()->get();
    //   if (in_handle) new_path.create_out_handle(second_transform * in_handle.value()->get());

    //   new_segments.push_back(std::make_shared<Renderer::Geometry::Segment>(p0, p1, p2, p3));

    //   size_t size = new_segments.size();

    //   for (int i = (int)second_segments.size() - 1; i >= 0; i--) {
    //     std::shared_ptr<Renderer::Geometry::Segment> reversed = Renderer::Geometry::Segment::reverse(second_segments.at(i));
    //     Renderer::Geometry::Segment::transform(*reversed, second_transform, false);
    //     new_segments.push_back(reversed);
    //   }

    //   auto last_ptr = new_segments.back().p3_ptr().lock();
    //   last_ptr->set(second_transform * last_ptr->get());
    // }
#endif 

    Editor::scene().delete_entity(first_id);
    Editor::scene().delete_entity(second_entity.id());

    // History::CommandHistory::add(std::make_unique<History::FunctionCommand>(
    //   [this, new_path, new_transform, vertex_id]() {
    //     Scene& scene = Editor::scene();

    //     scene.selection.clear();
    //     scene.selection.select_vertex(new_path.id, vertex_id);

    //     m_transform = new_transform;
    //     set_pen_element(uuid::null);
    //   },
    //   []() {}
    // ));

    m_element = new_id;
    m_vertex = 0;
    m_mode = Mode::Join;
  }


  void PenTool::on_close_pointer_down() {
    if (!m_element || !m_vertex.has_value()) return;

    Entity entity = Editor::scene().get_entity(m_element);
    PathComponent path = entity.get_component<PathComponent>();

    m_vertex = path.close(m_reverse);
    m_mode = Mode::Close;
  }

  void PenTool::on_sub_pointer_down() {
    m_mode = Mode::Sub;
  }

  void PenTool::on_add_pointer_down() {
    if (!m_temp_element) return;

    std::optional<size_t> segment_hover = InputManager::hover.segment();
    if (!segment_hover.has_value()) return;

    Entity entity = Editor::scene().get_entity(m_temp_element);
    PathComponent path = entity.get_component<PathComponent>();
    TransformComponent transform = entity.get_component<TransformComponent>();

    const Renderer::Geometry::Path::Segment segment = path.data().at(segment_hover.value());
    const mat2x3 inverse_transform = transform.inverse();
    const vec2 p = inverse_transform * InputManager::pointer.scene.position;

    float t;

    switch (segment.type) {
    case Renderer::Geometry::Path::Command::Cubic:
      t = Math::cubic_closest_to(segment.p0, segment.p1, segment.p2, segment.p3, p);
      break;
    case Renderer::Geometry::Path::Command::Quadratic:
      t = Math::quadratic_closest_to(segment.p0, segment.p1, segment.p2, p);
      break;
    default:
      t = Math::linear_closest_to(segment.p0, segment.p1, p);
      break;
    }

    m_vertex = path.split(segment_hover.value(), t);
    m_direction = 0;
    m_mode = Mode::Add;
  }

  void PenTool::on_angle_pointer_down() {
    if (!m_element || !m_vertex.has_value()) return;

    Entity entity = Editor::scene().get_entity(m_element);
    PathComponent path = entity.get_component<PathComponent>();

    if (m_reverse) {
      path.translate(
        Renderer::Geometry::Path::in_handle_index,
        path.data().point_at(0) - path.data().point_at(Renderer::Geometry::Path::in_handle_index)
      );
    } else {
      path.translate(
        Renderer::Geometry::Path::out_handle_index,
        path.data().point_at(path.data().points_size() - 1) - path.data().point_at(Renderer::Geometry::Path::out_handle_index)
      );
    }

    m_mode = Mode::Angle;
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
