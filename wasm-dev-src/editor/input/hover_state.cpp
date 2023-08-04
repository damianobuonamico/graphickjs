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

  void HoverState::set_hovered(const uuid id, const vec2 position, bool lower_level, float threshold) {
    reset();

    m_entity = id;

    if (id == 0 || !Editor::scene().has_entity(id)) {
      m_type = HoverType::None;
      return;
    }

    Entity entity = Editor::scene().get_entity(id);

    // TODO: internal check for different entity types
    if (!entity.has_components<PathComponent, TransformComponent>()) {
      m_type = HoverType::Entity;
      return;
    }

    Renderer::Geometry::Path& path = entity.get_component<PathComponent>().path;
    //Blaze::Matrix transform = entity.get_component<TransformComponent>().get_matrix().Inverse();
   // auto pos = transform.Map(position.x, position.y);
    //vec2 transformed_pos = { (float)pos.X, (float)pos.Y };
    vec2 transformed_pos = position;

    if (path.empty()) {
      m_type = HoverType::Entity;
      return;
    }

    for (const auto& segment : path.segments()) {
      vec2 p0 = segment.p0();

      if (Math::is_point_in_circle(transformed_pos, p0, threshold)) {
        m_type = HoverType::Vertex;
        m_vertex = segment.p0_ptr();
        return;
      }

      if (lower_level && (segment.is_quadratic() || segment.is_cubic())) {
        vec2 p1 = segment.p1();
        vec2 p2 = segment.p2();
        vec2 p3 = segment.p3();

        if (segment.has_p1() && Math::is_point_in_circle(transformed_pos, p1, threshold)) {
          m_type = HoverType::Handle;
          m_vertex = segment.p0_ptr();
          m_handle = segment.p1_ptr();
          return;
        }

        if (segment.is_cubic()) {
          if (segment.has_p2() && Math::is_point_in_circle(transformed_pos, p2, threshold)) {
            m_type = HoverType::Handle;
            m_vertex = segment.p3_ptr();
            m_handle = segment.p2_ptr();
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

    m_type = HoverType::Element;
  }

  void HoverState::reset() {
    m_type = HoverType::None;
    m_entity = 0;
    m_vertex.reset();
    m_handle.reset();
  }

}
