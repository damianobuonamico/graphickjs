#include "pen_tool.h"

#include "../input_manager.h"

#include "../../editor.h"
#include "../../scene/entity.h"

#include "../../../history/command_history.h"
#include "../../../history/commands.h"

#include "../../../renderer/renderer.h"

#include "../../../utils/console.h"

// TODO: esc to cancel pen and other tools
// TODO: cleanup (code repetition, make more robus and consistent)
// TODO: fix pen for translated elements
namespace Graphick::Editor::Input {

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
    case Mode::New:
      on_new_pointer_move();
      break;
    default:
    case Mode::None:
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
    case Mode::New:
      on_new_pointer_up();
      break;
    default:
    case Mode::None:
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

    if (first_path.reversed()) {
      for (int i = (int)first_segments.size() - 1; i >= 0; i--) {
        // TODO: Create utility for reversing a segment, maybe static method in segment class
        auto& segment = first_segments.at(i);

        if (segment.is_linear()) {
          new_segments.push_back(std::make_shared<Renderer::Geometry::Segment>(segment.p3_ptr().lock(), segment.p0_ptr().lock()));
        } else if (segment.has_p1() && segment.has_p2()) {
          new_segments.push_back(std::make_shared<Renderer::Geometry::Segment>(segment.p3_ptr().lock(), segment.p2(), segment.p1(), segment.p0_ptr().lock()));
        } else if (segment.has_p1()) {
          new_segments.push_back(std::make_shared<Renderer::Geometry::Segment>(segment.p3_ptr().lock(), segment.p1(), segment.p0_ptr().lock(), false, false));
        } else {
          new_segments.push_back(std::make_shared<Renderer::Geometry::Segment>(segment.p3_ptr().lock(), segment.p2(), segment.p0_ptr().lock(), false, true));
        }
      }

      auto in_handle = first_path.in_handle_ptr();
      auto out_handle = first_path.out_handle_ptr();

      if (in_handle.has_value()) {
        p1 = in_handle.value()->get();
      }
      if (out_handle.has_value()) {
        new_path.create_in_handle(out_handle.value()->get());
      }
    } else {
      for (auto& segment : first_segments) {
        new_segments.push_back(segment);
      }

      auto in_handle = first_path.in_handle_ptr();
      auto out_handle = first_path.out_handle_ptr();

      if (out_handle.has_value()) {
        p1 = out_handle.value()->get();
      }
      if (in_handle.has_value()) {
        new_path.create_in_handle(in_handle.value()->get());
      }
    }

    if (second_path.empty()) {
      std::shared_ptr<Renderer::Geometry::ControlPoint> p3 = second_path.last().lock();

      auto in_handle = second_path.in_handle_ptr();
      auto out_handle = second_path.out_handle_ptr();

      if (in_handle) {
        p2 = in_handle.value()->get();
      }
      if (out_handle) {
        new_path.create_out_handle(out_handle.value()->get());
      }

      if (p1.has_value() && p2.has_value()) {
        new_segments.push_back(std::make_shared<Renderer::Geometry::Segment>(p0, p1.value(), p2.value(), p3));
      } else if (p1.has_value()) {
        new_segments.push_back(std::make_shared<Renderer::Geometry::Segment>(p0, p1.value(), p3, false, true));
      } else if (p2.has_value()) {
        new_segments.push_back(std::make_shared<Renderer::Geometry::Segment>(p0, p2.value(), p3, false, false));
      } else {
        new_segments.push_back(std::make_shared<Renderer::Geometry::Segment>(p0, p3));
      }
    } else if (m_vertex->id == second_segments.front().p0_id()) {
      std::shared_ptr<Renderer::Geometry::ControlPoint> p3 = second_segments.front().p0_ptr().lock();

      auto in_handle = second_path.in_handle_ptr();
      auto out_handle = second_path.out_handle_ptr();

      if (in_handle) {
        p2 = in_handle.value()->get();
      }
      if (out_handle) {
        new_path.create_out_handle(out_handle.value()->get());
      }

      if (p1.has_value() && p2.has_value()) {
        new_segments.push_back(std::make_shared<Renderer::Geometry::Segment>(p0, p1.value(), p2.value(), p3));
      } else if (p1.has_value()) {
        new_segments.push_back(std::make_shared<Renderer::Geometry::Segment>(p0, p1.value(), p3, false, true));
      } else if (p2.has_value()) {
        new_segments.push_back(std::make_shared<Renderer::Geometry::Segment>(p0, p2.value(), p3, false, false));
      } else {
        new_segments.push_back(std::make_shared<Renderer::Geometry::Segment>(p0, p3));
      }

      for (auto& segment : second_path.segments()) {
        new_segments.push_back(segment);
      }
    } else {
      std::shared_ptr<Renderer::Geometry::ControlPoint> p3 = second_segments.back().p3_ptr().lock();

      auto in_handle = second_path.in_handle_ptr();
      auto out_handle = second_path.out_handle_ptr();

      if (out_handle) {
        p2 = out_handle.value()->get();
      }
      if (in_handle) {
        new_path.create_out_handle(in_handle.value()->get());
      }

      if (p1.has_value() && p2.has_value()) {
        new_segments.push_back(std::make_shared<Renderer::Geometry::Segment>(p0, p1.value(), p2.value(), p3));
      } else if (p1.has_value()) {
        new_segments.push_back(std::make_shared<Renderer::Geometry::Segment>(p0, p1.value(), p3, false, true));
      } else if (p2.has_value()) {
        new_segments.push_back(std::make_shared<Renderer::Geometry::Segment>(p0, p2.value(), p3, false, false));
      } else {
        new_segments.push_back(std::make_shared<Renderer::Geometry::Segment>(p0, p3));
      }

      for (int i = (int)second_segments.size() - 1; i >= 0; i--) {
        auto& segment = second_segments.at(i);

        if (segment.is_linear()) {
          new_segments.push_back(std::make_shared<Renderer::Geometry::Segment>(segment.p3_ptr().lock(), segment.p0_ptr().lock()));
        } else if (segment.has_p1() && segment.has_p2()) {
          new_segments.push_back(std::make_shared<Renderer::Geometry::Segment>(segment.p3_ptr().lock(), segment.p2(), segment.p1(), segment.p0_ptr().lock()));
        } else if (segment.has_p1()) {
          new_segments.push_back(std::make_shared<Renderer::Geometry::Segment>(segment.p3_ptr().lock(), segment.p1(), segment.p0_ptr().lock(), false, false));
        } else {
          new_segments.push_back(std::make_shared<Renderer::Geometry::Segment>(segment.p3_ptr().lock(), segment.p2(), segment.p0_ptr().lock(), false, true));
        }
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
    console::log("PenTool::close");
    if (!m_element || !m_path) return;

    m_path->close();

    m_vertex = m_path->last().lock().get();
    m_mode = Mode::Close;

    Editor::Editor::prepare_refresh();
    Editor::Editor::refresh();
  }

  void PenTool::on_sub_pointer_down() {
    console::log("PenTool::sub");
    m_mode = Mode::Sub;
  }

  void PenTool::on_add_pointer_down() {
    console::log("PenTool::add");
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
    m_add_direction = 0;
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

  // TODO: fix creating bezier on linear segment or empty path
  void PenTool::on_start_pointer_down() {
    console::log("PenTool::start");

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

  void PenTool::on_join_pointer_move() {
    on_add_pointer_move();
  }

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

  void PenTool::on_add_pointer_move() {
    if (!m_path || !m_vertex) return;

    auto handles = m_path->relative_handles(m_vertex->id);
    if (!handles.in_segment || !handles.out_segment) return;

    // TODO: space to move vertex

    if (m_add_direction == 0) {
      float cos = 0;

      if (handles.out_handle) {
        cos = Math::dot(-InputManager::pointer.scene.delta, handles.out_handle->get() - m_vertex->get());
      } else if (handles.out_segment) {
        cos = Math::dot(-InputManager::pointer.scene.delta, (handles.out_segment->has_p2() ? handles.out_segment->p2() : handles.out_segment->p3()) - m_vertex->get());
      }

      if (cos > 0) m_add_direction = -1;
      else m_add_direction = 1;
    }

    if (m_add_direction < 0) {
      std::swap(handles.in_handle, handles.out_handle);
      std::swap(handles.in_segment, handles.out_segment);
    }

    if (!handles.out_handle) {
      if (m_add_direction < 0) {
        handles.out_segment->create_p2(m_vertex->get());
        handles.out_handle = handles.out_segment->p2_ptr().lock().get();
      } else {
        handles.out_segment->create_p1(m_vertex->get());
        handles.out_handle = handles.out_segment->p1_ptr().lock().get();
      }
    }

    handles.out_handle->move_to(m_vertex->get() + InputManager::pointer.scene.delta);

    if (InputManager::keys.alt) return;

    if (!handles.in_handle) {
      if (m_add_direction < 0) {
        handles.in_segment->create_p1(m_vertex->get());
        handles.in_handle = handles.in_segment->p1_ptr().lock().get();
      } else {
        handles.in_segment->create_p2(m_vertex->get());
        handles.in_handle = handles.in_segment->p2_ptr().lock().get();
      }
    }

    handles.in_handle->move_to(m_vertex->get() - InputManager::pointer.scene.delta);
  }

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


    if (path.empty()) {
      if (!path.in_handle_ptr().has_value()) {
        path.create_in_handle(InputManager::pointer.scene.origin);
      }

      p2_ptr = path.in_handle_ptr().value().get();
    } else if (path.reversed()) {
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

  void PenTool::on_join_pointer_up() {
    on_add_pointer_up();
  }

  void PenTool::on_close_pointer_up() {
    if (!m_path) return;

    m_path->last().lock()->deep_apply();

    set_pen_element(0);
  }

  void PenTool::on_sub_pointer_up() {
    // TODO: replace with scene delta (dpr issue)
    if (!m_vertex || !m_path || Math::squared_length(InputManager::pointer.client.delta) > 10.0f) return;

    // TODO: keep shape if shift key is down 
    m_path->remove(m_vertex->id);
  }

  void PenTool::on_add_pointer_up() {
    if (!m_path || !m_vertex) return;

    auto handles = m_path->relative_handles(m_vertex->id);
    if (!handles.in_segment || !handles.out_segment) return;

    if (handles.in_handle) {
      handles.in_handle->apply();
    }
    if (handles.out_handle) {
      handles.out_handle->apply();
    }

    m_vertex->deep_apply();
  }

  void PenTool::on_angle_pointer_up() {
    on_new_pointer_up();
  }

  void PenTool::on_start_pointer_up() {
    on_new_pointer_up();
  }

}
