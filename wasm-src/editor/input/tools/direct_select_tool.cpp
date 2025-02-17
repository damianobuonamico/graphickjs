/**
 * @file direct_select_tool.cpp
 * @brief Contains the implementation of the DirectSelectTool class.
 *
 * @todo snapping and maybe grid
 * @todo curve molding
 * @todo fix moving handles of non selected elements
 * @todo fix moving the last segment in a closed path (I think)
 */

#include "direct_select_tool.h"

#include "../input_manager.h"

#include "../../editor.h"
#include "../../scene/entity.h"

#include "../../../renderer/renderer.h"

#include "../../../utils/console.h"

#include "../../../math/matrix.h"
#include "../../../math/vector.h"

namespace graphick::editor::input {

DirectSelectTool::DirectSelectTool() : Tool(ToolType::DirectSelect, CategoryDirect) {}

void DirectSelectTool::on_pointer_down()
{
  m_is_entity_added_to_selection = false;
  m_should_evaluate_selection = false;
  m_dragging_occurred = false;
  m_mode = Mode::None;
  m_entity = 0;

  m_segment.reset();
  m_vertex.reset();
  m_handle.reset();

  HoverState::HoverType hover_type = InputManager::hover.type();
  std::optional<Entity> entity = InputManager::hover.entity();

  if (hover_type == HoverState::HoverType::None || !entity.has_value()) {
    on_none_pointer_down();
    return;
  }

  m_entity = entity->id();
  m_segment = InputManager::hover.segment();
  m_vertex = InputManager::hover.vertex();
  m_handle = InputManager::hover.handle();

  if (InputManager::keys.alt && m_entity && hover_type != HoverState::HoverType::Handle) {
    on_duplicate_pointer_down();
  } else if (hover_type == HoverState::HoverType::Element) {
    on_element_pointer_down();
  } else if (hover_type == HoverState::HoverType::Vertex) {
    on_vertex_pointer_down();
  } else if (hover_type == HoverState::HoverType::Handle) {
    on_handle_pointer_down();
  } else if (hover_type == HoverState::HoverType::Segment) {
    on_segment_pointer_down();
  } else if (hover_type == HoverState::HoverType::Entity &&
             entity->is_in_category(CategoryComponent::Category::Selectable))
  {
    on_entity_pointer_down();
  } else {
    on_none_pointer_down();
  }
}

void DirectSelectTool::on_pointer_move()
{
  m_dragging_occurred = true;

  switch (m_mode) {
    case Mode::Duplicate:
    case Mode::Entity:
    case Mode::Element:
    case Mode::Vertex:
      translate_selected();
      break;
    case Mode::Handle:
      on_handle_pointer_move();
      break;
    case Mode::Bezier:
      on_segment_pointer_move();
      break;
    default:
    case Mode::None:
      on_none_pointer_move();
      break;
  }
}

void DirectSelectTool::on_pointer_up()
{
  switch (m_mode) {
    case Mode::Duplicate:
      break;
    case Mode::Entity:
    case Mode::Element:
      on_entity_pointer_up();
      break;
    case Mode::Vertex:
      on_vertex_pointer_up();
      break;
    case Mode::Handle:
      on_handle_pointer_up();
      break;
    case Mode::Bezier:
      on_segment_pointer_up();
      break;
    default:
    case Mode::None:
      on_none_pointer_up();
      break;
  }

  if (m_selection_rect.active()) {
    m_selection_rect.reset();
  }
}

void DirectSelectTool::render_overlays() const
{
  if (!m_selection_rect.active())
    return;

  renderer::Outline outline{nullptr, false, Settings::Renderer::ui_primary_color};
  renderer::Renderer::draw(m_selection_rect.path(),
                           m_selection_rect.transform(),
                           renderer::DrawingOptions{nullptr, nullptr, &outline});
  renderer::Renderer::ui_rect(m_selection_rect.bounding_rect(),
                              Settings::Renderer::ui_primary_transparent);
}

void DirectSelectTool::translate_selected()
{
  vec2 absolute_movenent = InputManager::pointer.scene.movement;
  vec2 movement = absolute_movenent;

  Scene& scene = Editor::scene();

  if (m_vertex.has_value()) {
    Entity entity = scene.get_entity(m_entity);

    TransformComponent transform = entity.get_component<TransformComponent>();
    PathComponent path = entity.get_component<PathComponent>();

    const mat2x3 inverse_transform = math::inverse(transform.matrix());

    const vec2 position = inverse_transform * InputManager::pointer.scene.position;
    const vec2 vertex_position = path.data().at(m_vertex.value());

    absolute_movenent = InputManager::pointer.scene.position -
                        transform.transform(vertex_position);
    movement = position - vertex_position;
  }

  for (auto& [id, entry] : scene.selection.selected()) {
    Entity entity = scene.get_entity(id);
    TransformComponent transform = entity.get_component<TransformComponent>();

    if (entry.full()) {
      transform.translate(absolute_movenent);
    } else {
      for (size_t i : entry.indices) {
        PathComponent path = entity.get_component<PathComponent>();
        translate_control_point(path, i, transform, &movement, false, true, false, nullptr);
      }
    }
  }
}

/* -- on_pointer_down -- */

void DirectSelectTool::on_none_pointer_down()
{
  if (!InputManager::keys.shift) {
    Editor::scene().selection.clear();
  }

  m_selection_rect.set(InputManager::pointer.scene.position);
  m_mode = Mode::None;
}

void DirectSelectTool::on_duplicate_pointer_down()
{
  on_entity_pointer_down();

  Scene& scene = Editor::scene();

  if (!scene.selection.has(m_entity)) {
    return;
  }

  std::vector<uuid> duplicated;

  for (const auto& [id, _] : scene.selection.selected()) {
    duplicated.push_back(scene.duplicate_entity(id).id());
  }

  scene.selection.clear();

  for (uuid id : duplicated) {
    scene.selection.select(id);
  }

  m_vertex.reset();
  m_handle.reset();
  m_segment.reset();

  m_mode = Mode::Duplicate;
}

void DirectSelectTool::on_entity_pointer_down()
{
  Scene& scene = Editor::scene();

  if (!scene.selection.has(m_entity)) {
    if (!InputManager::keys.shift)
      scene.selection.clear();

    scene.selection.select(m_entity);
    m_is_entity_added_to_selection = true;
  }

  m_mode = Mode::Entity;
}

void DirectSelectTool::on_element_pointer_down()
{
  Scene& scene = Editor::scene();

  if (!scene.selection.has(m_entity) || !scene.selection.get(m_entity).full()) {
    if (!InputManager::keys.shift)
      scene.selection.clear();

    scene.selection.select(m_entity);
    m_is_entity_added_to_selection = true;
  }

  m_mode = Mode::Element;
}

void DirectSelectTool::on_segment_pointer_down()
{
  Scene& scene = Editor::scene();
  Entity entity = scene.get_entity(m_entity);

  if (!entity.is_element()) {
    return;
  }

  const PathComponent path = entity.get_component<PathComponent>();
  const geom::path::Iterator it(path.data(), m_segment.value(), geom::path::IndexType::Segment);
  const geom::path::Segment segment = *it;

  const size_t start_index = std::max(uint32_t(1), it.point_index()) - 1;
  const size_t end_index = start_index + static_cast<size_t>(segment.type);

  m_mode = Mode::Bezier;

  if (scene.selection.has(m_entity) && scene.selection.has_child(m_entity, start_index) &&
      scene.selection.has_child(m_entity, end_index))
  {
    return;
  }

  if (segment.type == geom::path::Command::Cubic) {
    if (InputManager::keys.shift) {
      m_is_entity_added_to_selection = true;
    } else {
      scene.selection.clear();

      // m_last_bezier_point = InputManager::pointer.scene.origin -
      // m_element->transform()->position().get(); m_last_bezier_p1 = m_bezier->p1();
      // m_last_bezier_p2 = m_bezier->p2();
      // m_closest = m_bezier->closest_to(m_last_bezier_point);

      m_should_evaluate_selection = true;
    }
  } else {
    if (!InputManager::keys.shift) {
      scene.selection.clear();
    }

    m_is_entity_added_to_selection = true;
  }

  scene.selection.select_child(m_entity, start_index);
  scene.selection.select_child(m_entity, end_index);
}

void DirectSelectTool::on_vertex_pointer_down()
{
  Scene& scene = Editor::scene();

  if (!scene.selection.has_child(m_entity, m_vertex.value())) {
    if (!InputManager::keys.shift)
      scene.selection.clear();

    scene.selection.select_child(m_entity, m_vertex.value());
    m_is_entity_added_to_selection = true;
  }

  m_mode = Mode::Vertex;
}

void DirectSelectTool::on_handle_pointer_down()
{
  m_mode = Mode::Handle;
}

/* -- on_pointer_move -- */

void DirectSelectTool::on_none_pointer_move()
{
  if (m_selection_rect.active()) {
    Scene& scene = Editor::scene();

    m_selection_rect.size(InputManager::pointer.scene.delta);
    scene.selection.temp_select(scene.entities_in(m_selection_rect.bounding_rect(), true));
  }
}

void DirectSelectTool::on_segment_pointer_move()
{
  if (!m_should_evaluate_selection) {
    translate_selected();
    return;
  }

  // vec2 p0 = m_bezier->p0();
  // vec2 p1 = m_last_bezier_p1;
  // vec2 p2 = m_last_bezier_p2;
  // vec2 p3 = m_bezier->p3();

  // // Molded curve

  // vec2 v1 = lerp(p0, p1, m_closest.t);
  // vec2 A = lerp(p1, p2, m_closest.t);
  // vec2 v2 = lerp(p2, p3, m_closest.t);

  // vec2 e1 = lerp(v1, A, m_closest.t);
  // vec2 e2 = lerp(A, v2, m_closest.t);

  // vec2 d1 = e1 - m_closest.point;
  // vec2 d2 = e2 - m_closest.point;

  // vec2 position = InputManager::pointer.scene.position -
  // m_element->transform()->position().get();

  // vec2 ne1 = position + d1;
  // vec2 ne2 = position + d2;

  // BezierEntity::BezierABC abc = m_bezier->abc(m_closest.t, position);

  // vec2 h = abc.a - (abc.a - ne1) / (1.0f - m_closest.t);
  // vec2 k = abc.a - (abc.a - ne2) / m_closest.t;

  // vec2 np1 = p0 + (h - p0) / m_closest.t;
  // vec2 np2 = p3 + (k - p3) / (1.0f - m_closest.t);

  // // Idealised curve

  // vec2 center = circle_center(p0, position, p3);

  // float ideal_d1 = distance(p0, position);
  // float ideal_d2 = distance(p3, position);
  // float ideal_t = ideal_d1 / (ideal_d1 + ideal_d2);

  // BezierEntity::BezierABC ideal_abc = m_bezier->abc(ideal_t, position);

  // float angle = std::fmodf(std::atan2f(p3.y - p0.y, p3.x - p0.x) - std::atan2f(position.y -
  // p0.y, position.x - p0.x) + MATH_F_TWO_PI, MATH_F_TWO_PI); float bc = (angle < 0.0f || angle >
  // MATH_F_PI ? -1.0f : 1.0f) * distance(p0, p3) / 3.0f; float de1 = ideal_t * bc; float de2 =
  // (1.0f - ideal_t) * bc;

  // vec2 tangent1 = { position.x - 10.0f * (position.y - center.y), position.y + 10.0f *
  // (position.x - center.x) }; vec2 tangent2 = { position.x + 10.0f * (position.y - center.y),
  // position.y - 10.0f * (position.x - center.x) }; vec2 direction = normalize(tangent2 -
  // tangent1);

  // vec2 ideal_e1 = position + de1 * direction;
  // vec2 ideal_e2 = position - de2 * direction;

  // vec2 ideal_h = ideal_abc.a - (ideal_abc.a - ideal_e1) / (1.0f - ideal_t);
  // vec2 ideal_k = ideal_abc.a - (ideal_abc.a - ideal_e2) / ideal_t;

  // vec2 ideal_np1 = p0 + (ideal_h - p0) / ideal_t;
  // vec2 ideal_np2 = p3 + (ideal_k - p3) / (1.0f - ideal_t);

  // // Interpolated curve

  // float d = distance(m_last_bezier_point, position);
  // float u = std::min(44.0f, d) / 44.0f;

  // vec2 lerp_np1 = lerp(np1, ideal_np1, u);
  // vec2 lerp_np2 = lerp(np2, ideal_np2, u);

  // m_bezier->start().transform()->translate_right_to(lerp_np1 - p0);
  // m_bezier->end().transform()->translate_left_to(lerp_np2 - p3);
}

void DirectSelectTool::on_handle_pointer_move()
{
  Scene& scene = Editor::scene();
  Entity entity = scene.get_entity(m_entity);

  PathComponent path = entity.get_component<PathComponent>();
  TransformComponent transform = entity.get_component<TransformComponent>();

  translate_control_point(path, m_handle.value(), transform, nullptr, false, true, false, nullptr);
}

/* -- on_pointer_up -- */

void DirectSelectTool::on_none_pointer_up()
{
  Editor::scene().selection.sync();
}

void DirectSelectTool::on_entity_pointer_up()
{
  if (m_dragging_occurred)
    return;

  Scene& scene = Editor::scene();

  if (scene.selection.has(m_entity) && !m_is_entity_added_to_selection) {
    if (InputManager::keys.shift) {
      scene.selection.deselect(m_entity);
    } else {
      if (InputManager::pointer.button == InputManager::PointerButton::Left) {
        scene.selection.clear();
      }
      scene.selection.select(m_entity);
    }
  }
}

void DirectSelectTool::on_segment_pointer_up()
{
  if (m_dragging_occurred)
    return;

  Scene& scene = Editor::scene();
  Entity entity = scene.get_entity(m_entity);

  const PathComponent path = entity.get_component<PathComponent>();
  const geom::path::Iterator it(path.data(), m_segment.value(), geom::path::IndexType::Segment);
  const geom::path::Segment segment = *it;

  const size_t start_index = std::max(uint32_t(1), it.point_index()) - 1;
  const size_t end_index = start_index + static_cast<size_t>(segment.type);

  if (m_should_evaluate_selection) {
    scene.selection.select_child(m_entity, start_index);
    scene.selection.select_child(m_entity, end_index);
  } else if (!m_is_entity_added_to_selection && scene.selection.has(m_entity) &&
             scene.selection.has_child(m_entity, start_index) &&
             scene.selection.has_child(m_entity, end_index))
  {
    if (InputManager::keys.shift) {
      scene.selection.deselect_child(m_entity, start_index);
      scene.selection.deselect_child(m_entity, end_index);
    } else {
      if (InputManager::pointer.button == InputManager::PointerButton::Left) {
        Editor::scene().selection.clear();
      }

      scene.selection.select_child(m_entity, start_index);
      scene.selection.select_child(m_entity, end_index);
    }
  }
}

void DirectSelectTool::on_vertex_pointer_up()
{
  if (m_dragging_occurred) {
    return;
  }

  Scene& scene = Editor::scene();

  if (scene.selection.has_child(m_entity, m_vertex.value()) && !m_is_entity_added_to_selection) {
    if (InputManager::keys.shift) {
      scene.selection.deselect_child(m_entity, m_vertex.value());
    } else {
      if (InputManager::pointer.button == InputManager::PointerButton::Left) {
        scene.selection.clear();
      }

      scene.selection.select_child(m_entity, m_vertex.value());
    }
  }
}

void DirectSelectTool::on_handle_pointer_up()
{
  if (!m_entity || !m_handle.has_value())
    return;

  Entity entity = Editor::scene().get_entity(m_entity);
  PathComponent path = entity.get_component<PathComponent>();

  const geom::path::VertexNode node = path.data().node_at(m_handle.value());
  const float threshold = 2.5f / Editor::scene().viewport.zoom();

  if (node.in >= 0) {
    const vec2 in_handle = path.data().at(static_cast<size_t>(node.in));
    const vec2 vertex = path.data().at(node.vertex);

    if (math::is_almost_equal(in_handle, vertex, threshold)) {
      path.translate(static_cast<size_t>(node.in), vertex - in_handle);

      if (node.in_command >= 0) {
        const geom::path::Segment segment = path.data().segment_at(
            static_cast<size_t>(node.in_command), geom::path::IndexType::Command);

        if (segment.is_line()) {
          path.to_cubic(static_cast<size_t>(node.in_command));
        }
      }
    }
  }

  if (node.out >= 0) {
    const vec2 out_handle = path.data().at(static_cast<size_t>(node.out));
    const vec2 vertex = path.data().at(node.vertex);

    if (math::is_almost_equal(out_handle, vertex, threshold)) {
      path.translate(static_cast<size_t>(node.out), vertex - out_handle);

      if (node.out_command >= 0) {
        const geom::path::Segment segment = path.data().segment_at(
            static_cast<size_t>(node.out_command), geom::path::IndexType::Command);

        if (segment.is_line()) {
          path.to_cubic(static_cast<size_t>(node.out_command));
        }
      }
    }
  }
}

}  // namespace graphick::editor::input
