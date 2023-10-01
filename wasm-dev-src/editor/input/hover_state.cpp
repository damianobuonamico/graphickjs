#include "hover_state.h"

#include "../scene/entity.h"
#include "../editor.h"

#include "../../math/math.h"

namespace Graphick::Editor::Input {

  std::optional<Entity> HoverState::entity() const {
    if (m_entity == 0 || !Editor::scene().has_entity(m_entity)) return std::nullopt;
    return Editor::scene().get_entity(m_entity);
  }

  std::optional<std::weak_ptr<Renderer::Geometry::ControlPoint>> HoverState::vertex() const {
    if (m_entity == 0 || !Editor::scene().has_entity(m_entity)) return std::nullopt;
    return m_vertex;
  }

  std::optional<std::weak_ptr<History::Vec2Value>> HoverState::handle() const {
    if (m_entity == 0 || !Editor::scene().has_entity(m_entity)) return std::nullopt;
    return m_handle;
  }

  void HoverState::set_hovered(const uuid id, const vec2 position, bool deep_search, float threshold) {
    reset();

    Scene& scene = Editor::scene();
    m_entity = id;

    if (id == 0 || !scene.has_entity(id)) {
      m_type = HoverType::None;
      return;
    }

    Entity entity = scene.get_entity(id);

    if (!entity.is_element()) {
      m_type = HoverType::Entity;
      return;
    }

    if (!deep_search) {
      m_type = HoverType::Element;
      return;
    }

    const Renderer::Geometry::Path& path = entity.get_component<PathComponent>().path;
    const vec2 translation = entity.get_component<TransformComponent>().position.get();
    const vec2 transformed_pos = position - translation;

    bool deep = deep_search && scene.selection.has(id);

    if (path.empty()) {
      if (path.vacant()) {
        m_entity = 0;
        m_type = HoverType::None;
        return;
      }

      auto last = path.last();

      if (Math::is_point_in_circle(transformed_pos, last.lock()->get(), threshold)) {
        m_type = HoverType::Vertex;
        m_vertex = last;
        return;
      }
    } else {
      for (const auto& segment : path.segments()) {
        vec2 p0 = segment->p0();

        if (Math::is_point_in_circle(transformed_pos, p0, threshold)) {
          m_type = HoverType::Vertex;
          m_vertex = segment->p0_ptr();
          return;
        }

        if (deep && (segment->is_quadratic() || segment->is_cubic())) {
          vec2 p1 = segment->p1();
          vec2 p2 = segment->p2();
          vec2 p3 = segment->p3();

          if (segment->has_p1() && Math::is_point_in_circle(transformed_pos, p1, threshold)) {
            m_type = HoverType::Handle;
            m_vertex = segment->p0_ptr();
            m_handle = segment->p1_ptr();
            return;
          }

          if (segment->is_cubic()) {
            if (segment->has_p2() && Math::is_point_in_circle(transformed_pos, p2, threshold)) {
              m_type = HoverType::Handle;
              m_vertex = segment->p3_ptr();
              m_handle = segment->p2_ptr();
              return;
            }
          }
        }
      }

      if (Math::is_point_in_circle(transformed_pos, path.segments().back().p3(), threshold)) {
        m_type = HoverType::Vertex;
        m_vertex = path.segments().back().p3_ptr();
        m_handle.reset();
        return;
      }
    }

    auto in_handle = path.in_handle_ptr();
    auto out_handle = path.out_handle_ptr();

    if (in_handle) {
      if (Math::is_point_in_circle(transformed_pos, in_handle.value()->get(), threshold)) {
        m_type = HoverType::Handle;
        m_vertex = path.empty() ? path.last() : path.segments().front().p0_ptr();
        m_handle = path.in_handle_ptr();
        return;
      }
    }
    if (out_handle) {
      if (Math::is_point_in_circle(transformed_pos, out_handle.value()->get(), threshold)) {
        m_type = HoverType::Handle;
        m_vertex = path.empty() ? path.last() : path.segments().back().p3_ptr();
        m_handle = path.out_handle_ptr();
        return;
      }
    }

    m_type = HoverType::Element;
  }

  void HoverState::reset() {
    m_type = HoverType::None;
    m_entity = 0;
    m_vertex.reset();
    m_handle.reset();
  }

}
