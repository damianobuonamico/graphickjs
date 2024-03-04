#include "hover_state.h"

#include "../scene/entity.h"
#include "../editor.h"

#include "../../math/math.h"
#include "../../math/matrix.h"

#include "../../utils/console.h"

// TODO: hover priority: handle > vertex > segment > element > none
namespace Graphick::Editor::Input {

  HoverState::~HoverState() {}

  std::optional<Entity> HoverState::entity() const {
    if (m_entity == 0 || !Editor::scene().has_entity(m_entity)) return std::nullopt;
    return Editor::scene().get_entity(m_entity);
  }

  std::optional<size_t> HoverState::segment() const {
    if (m_entity == 0 || m_segment < 0 || !Editor::scene().has_entity(m_entity)) return std::nullopt;
    return static_cast<size_t>(m_segment);
  }

  std::optional<size_t> HoverState::vertex() const {
    if (m_entity == 0 || m_vertex < 0 || !Editor::scene().has_entity(m_entity)) return std::nullopt;
    return static_cast<size_t>(m_vertex);
  }

  std::optional<size_t> HoverState::handle() const {
    if (m_entity == 0 || m_handle < 0 || !Editor::scene().has_entity(m_entity)) return std::nullopt;
    return static_cast<size_t>(m_handle);
  }

  void HoverState::set_hovered(const uuid id, const vec2 position, const bool deep_search, float threshold, const double zoom) {
    reset();

    const Scene& scene = Editor::scene();
    m_entity = id;

    if (id == uuid::null || !scene.has_entity(id)) {
      m_type = HoverType::None;
      return;
    }

    const Entity entity = scene.get_entity(id);

    if (!entity.is_element()) {
      m_type = HoverType::Entity;
      return;
    }

    m_type = HoverType::Element;

    if (!deep_search) {
      return;
    }

    const PathComponent path = entity.get_component<PathComponent>();
    const TransformComponent transform = entity.get_component<TransformComponent>();

    threshold /= zoom;

    const bool deep = deep_search && scene.selection.has(id);

    if (path.data().empty()) {
      if (path.data().vacant()) {
        m_type = HoverType::None;
        m_entity = 0;
        return;
      }
    } else {
      for (size_t i = 0; i < path.data().points_size(); i++) {
        if (path.data().is_point_inside_point(i, position, transform, threshold)) {
          m_segment = -1;

          if (path.data().is_vertex(i)) {
            m_type = HoverType::Vertex;
            m_vertex = i;
            m_handle = -1;
          } else {
            m_type = HoverType::Handle;
            m_vertex = -1;
            m_handle = i;
          }

          return;
        }
      }

      for (size_t i = 0; i < path.data().size(); i++) {
        if (path.data().is_point_inside_segment(i, position, nullptr, transform, threshold, zoom)) {
          m_type = HoverType::Segment;
          m_segment = i;
          m_vertex = -1;
          m_handle = -1;
          return;
        }
      }
    }

    if (path.data().has_in_handle()) {
      if (path.data().is_point_inside_point(Renderer::Geometry::Path::in_handle_index, position, transform, threshold)) {
        m_type = HoverType::Handle;
        m_segment = -1;
        m_vertex = -1;
        m_handle = Renderer::Geometry::Path::in_handle_index;
        return;
      }
    }

    if (path.data().has_out_handle()) {
      if (path.data().is_point_inside_point(Renderer::Geometry::Path::out_handle_index, position, transform, threshold)) {
        m_type = HoverType::Handle;
        m_segment = -1;
        m_vertex = -1;
        m_handle = Renderer::Geometry::Path::out_handle_index;
        return;
      }
    }
  }

  void HoverState::reset() {
    m_type = HoverType::None;
    m_entity = uuid::null;
    m_segment = -1;
    m_vertex = -1;
    m_handle = -1;
  }

}
