/**
 * @file hover_state.cpp
 * @brief Contains the implementation of the HoverState class.
 *
 * @todo hover priority: handle > vertex > segment > element > none
 */

#include "hover_state.h"

#include "../editor.h"
#include "../scene/entity.h"

#include "../../math/math.h"
#include "../../math/matrix.h"

#include "../../utils/console.h"

namespace graphick::editor::input {

HoverState::~HoverState() {}

std::optional<Entity> HoverState::entity() const
{
  if (m_entity == 0 || !Editor::scene().has_entity(m_entity))
    return std::nullopt;
  return Editor::scene().get_entity(m_entity);
}

std::optional<size_t> HoverState::segment() const
{
  if (m_entity == 0 || m_segment < 0 || !Editor::scene().has_entity(m_entity))
    return std::nullopt;
  return static_cast<size_t>(m_segment);
}

std::optional<size_t> HoverState::vertex() const
{
  if (m_entity == 0 || m_vertex < 0 || !Editor::scene().has_entity(m_entity))
    return std::nullopt;
  return static_cast<size_t>(m_vertex);
}

std::optional<size_t> HoverState::handle() const
{
  if (m_entity == 0 || m_handle < 0 || !Editor::scene().has_entity(m_entity))
    return std::nullopt;
  return static_cast<size_t>(m_handle);
}

void HoverState::set_hovered(
    const uuid id, const vec2 position, const bool deep_search, float threshold, const double zoom)
{
  reset();

  const Scene& scene = Editor::scene();
  m_entity = id;

  if (id == uuid::null || !scene.has_entity(id)) {
    m_type = HoverType::None;
    return;
  }

  const Hierarchy hierarchy = scene.get_hierarchy(id);
  const uuid entity_id = deep_search || hierarchy.entries.empty() ? id :
                                                                    hierarchy.entries.back().id;

  const Entity entity = scene.get_entity(entity_id);

  if (!entity.is_element()) {
    m_type = HoverType::Entity;
    return;
  }

  m_type = HoverType::Element;

  if (!deep_search) {
    return;
  }

  const PathComponent path = entity.get_component<PathComponent>();
  const TransformComponent transform_component = entity.get_component<TransformComponent>();

  const mat2x3 transform = hierarchy.transform() * transform_component.matrix();

  threshold = static_cast<float>(static_cast<double>(threshold) / zoom);

  const float point_threshold = threshold * 1.5f;
  const bool deep = deep_search && scene.selection.has(entity_id);

  if (path.data().empty() && path.data().vacant()) {
    m_type = HoverType::None;
    m_entity = 0;
    return;
  }

  for (uint32_t i = 0; i < path.data().points_count(); i++) {
    if (path.data().is_point_inside_point(i, position, transform, point_threshold)) {
      m_segment = -1;

      if (path.data().is_vertex(i)) {
        m_type = HoverType::Vertex;
        m_vertex = i;
        m_handle = -1;
      } else if (deep) {
        m_type = HoverType::Handle;
        m_vertex = -1;
        m_handle = i;
      }

      return;
    }
  }

  if (path.data().has_in_handle()) {
    if (path.data().is_point_inside_point(
            geom::path::in_handle_index, position, transform, point_threshold))
    {
      m_type = HoverType::Handle;
      m_segment = -1;
      m_vertex = -1;
      m_handle = geom::path::in_handle_index;
      return;
    }
  }

  if (path.data().has_out_handle()) {
    if (path.data().is_point_inside_point(
            geom::path::out_handle_index, position, transform, point_threshold))
    {
      m_type = HoverType::Handle;
      m_segment = -1;
      m_vertex = -1;
      m_handle = geom::path::out_handle_index;
      return;
    }
  }

  if (!path.data().empty()) {
    const geom::StrokingOptions<float> stroke = {
        static_cast<float>(Settings::Renderer::stroking_tolerance),
        0.0f,
        0.0f,
        geom::LineCap::Square,
        geom::LineJoin::Bevel};

    for (uint32_t i = 0; i < path.data().size(); i++) {
      if (path.data().is_point_inside_segment(i, position, stroke, transform, threshold)) {
        m_type = HoverType::Segment;
        m_segment = i;
        m_vertex = -1;
        m_handle = -1;
        return;
      }
    }
  }
}

void HoverState::reset()
{
  m_type = HoverType::None;
  m_entity = uuid::null;
  m_segment = -1;
  m_vertex = -1;
  m_handle = -1;
}

}  // namespace graphick::editor::input
