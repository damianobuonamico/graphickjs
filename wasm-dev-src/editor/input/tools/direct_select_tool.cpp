#include "direct_select_tool.h"

#include "../input_manager.h"

#include "../../editor.h"
#include "../../scene/entity.h"

#include "../../../utils/console.h"

namespace Graphick::Editor::Input {

  DirectSelectTool::DirectSelectTool() : Tool(ToolType::DirectSelect, CategoryDirect) {}

  void DirectSelectTool::on_pointer_down() {
    m_should_evaluate_selection = false;
    m_is_entity_added_to_selection = false;
    m_dragging_occurred = false;
    m_mode = Mode::ModeNone;
    m_entity = 0;
    m_vertex.reset();
    m_handle.reset();

    HoverState::HoverType hover_type = InputManager::hover.type();
    std::optional<Entity> entity = InputManager::hover.entity();
    console::log("HoverType", (int)hover_type);

    if (hover_type == HoverState::HoverType::None || !entity.has_value()) return;

    m_entity = entity.value().id();
    m_vertex = InputManager::hover.vertex();
    m_handle = InputManager::hover.handle();

    if (InputManager::keys.alt && m_entity && hover_type != HoverState::HoverType::Handle) {
      on_duplicate_pointer_down();

      console::log("DirectSelectTool::duplicate");
    } else if (hover_type == HoverState::HoverType::Element) {
      on_element_pointer_down();
      console::log("DirectSelectTool::element");
    } else if (hover_type == HoverState::HoverType::Vertex) {
      on_vertex_pointer_down();
      console::log("DirectSelectTool::vertex");
    } else if (hover_type == HoverState::HoverType::Handle) {
      on_handle_pointer_down();
      console::log("DirectSelectTool::handle");
    } else if (hover_type == HoverState::HoverType::Segment) {
      on_bezier_pointer_down();
      console::log("DirectSelectTool::bezier");
    } else if (hover_type == HoverState::HoverType::Entity /*&& m_entity->is_in_category(Entity::CategorySelectable)*/) {
      on_entity_pointer_down();
      console::log("DirectSelectTool::entity");
    } else {
      on_none_pointer_down();
    }

    // Entity* hovered = InputManager::hover.entity();
    // m_entity = hovered ? InputManager::hover.element() : nullptr;
    // m_element = dynamic_cast<ElementEntity*>(m_entity);
    // m_bezier = dynamic_cast<BezierEntity*>(hovered);
    // m_handle = dynamic_cast<HandleEntity*>(hovered);
    // m_vertex = nullptr;


    // if (m_handle) {
    //   VertexEntity* vertex = dynamic_cast<VertexEntity*>(m_handle->parent);
    //   if (vertex && vertex->position()->id == m_handle->id) {
    //     m_vertex = vertex;
    //   }
    // }

    // if (InputManager::keys.alt && m_entity && (!m_handle || m_vertex)) {
    //   on_duplicate_pointer_down();

    //   console::log("DirectSelectTool::duplicate");
    // } else if (m_element) {
    //   if (m_vertex) {
    //     on_vertex_pointer_down();
    //     console::log("DirectSelectTool::vertex");
    //   } else if (m_handle) {
    //     on_handle_pointer_down();
    //     console::log("DirectSelectTool::handle");
    //   } else if (m_bezier) {
    //     on_bezier_pointer_down();
    //     console::log("DirectSelectTool::bezier");
    //   } else {
    //     on_element_pointer_down();
    //     console::log("DirectSelectTool::element");
    //   }
    // } else if (m_entity && m_entity->is_in_category(Entity::CategorySelectable)) {
    //   on_entity_pointer_down();
    //   console::log("DirectSelectTool::entity");
    // } else {
    //   on_none_pointer_down();
    // }
  }

  void DirectSelectTool::on_pointer_move() {
    m_dragging_occurred = true;

    switch (m_mode) {
    case Mode::ModeDuplicate:
      on_duplicate_pointer_move();
      break;
    case Mode::ModeElement:
      on_element_pointer_move();
      break;
    case Mode::ModeVertex:
      on_vertex_pointer_move();
      break;
    case Mode::ModeHandle:
      on_handle_pointer_move();
      break;
    case Mode::ModeBezier:
      on_bezier_pointer_move();
      break;
    case Mode::ModeEntity:
      on_entity_pointer_move();
      break;
    default:
    case Mode::ModeNone:
      on_none_pointer_move();
      break;
    }
  }

  void DirectSelectTool::on_pointer_up() {
    switch (m_mode) {
    case Mode::ModeDuplicate:
      on_duplicate_pointer_up();
      break;
    case Mode::ModeElement:
      on_element_pointer_up();
      break;
    case Mode::ModeVertex:
      on_vertex_pointer_up();
      break;
    case Mode::ModeHandle:
      on_handle_pointer_up();
      break;
    case Mode::ModeBezier:
      on_bezier_pointer_up();
      break;
    case Mode::ModeEntity:
      on_entity_pointer_up();
      break;
    default:
    case Mode::ModeNone:
      on_none_pointer_up();
      break;
    }
  }

  // void DirectSelectTool::tessellate_overlays_outline(const vec4& color, const RenderingOptions& options, Geometry& geo) const {
  //   m_selection_rect.tessellate_outline(color, options, geo);
  // }

  // void DirectSelectTool::render_overlays(const RenderingOptions& options) const {
  //   m_selection_rect.render(options);
  // }

  // TODO: Implement rotation
  void DirectSelectTool::translate_selected() {
    // TEMP
    if (m_vertex.has_value()) {
      auto vertex = m_vertex.value().lock();
      if (vertex) {
        vertex->set_delta(InputManager::pointer.scene.delta);
      }
    }

    // for (auto id : Editor::scene().selection.selected()) {
      // if (entity->is_in_category(Entity::CategorySelectableChildren)) {
      //   ElementEntity* element = dynamic_cast<ElementEntity*>(entity);

      //   if (element && !element->selection()->full()) {
      //     for (Entity* vertex : element->selection()->entities()) {
      //       vertex->transform()->translate(InputManager::pointer.scene.movement);
      //     }
      //     continue;
      //   }
      // }

      // entity->transform()->translate(InputManager::pointer.scene.movement);
    // }
  }

  // void DirectSelectTool::shift_select_element(Entity* entity) {
  //   if (InputManager::keys.shift) {
  //     Editor::scene().selection.deselect(entity->id);
  //   } else {
  //     if (InputManager::pointer.button == InputManager::PointerButton::Left) {
  //       Editor::scene().selection.clear();
  //     }
  //     Editor::scene().selection.select(entity, true);
  //   }
  // }

  /* -- on_pointer_down -- */

  void DirectSelectTool::on_none_pointer_down() {
    // if (!InputManager::keys.shift) {
    //   Editor::scene().selection.clear();
    // }

    // m_selection_rect.set(InputManager::pointer.scene.position);

    m_mode = Mode::ModeNone;
  }

  void DirectSelectTool::on_duplicate_pointer_down() {
    // if (!Editor::scene().selection.has(m_entity->id)) {
    //   if (!InputManager::keys.shift) Editor::scene().selection.clear();

    //   Editor::scene().selection.select(m_entity);
    //   m_is_entity_added_to_selection = true;
    // }

    // std::vector<Entity*> entities = Editor::scene().selection.entities();
    // Editor::scene().selection.clear();

    // for (Entity* entity : entities) {
    //   Entity* duplicate = Editor::scene().duplicate(entity);
    //   if (duplicate) Editor::scene().selection.select(duplicate);
    // }

    m_mode = Mode::ModeDuplicate;
  }

  void DirectSelectTool::on_entity_pointer_down() {
    // if (!Editor::scene().selection.has(m_entity->id)) {
    //   if (!InputManager::keys.shift) Editor::scene().selection.clear();

    //   Editor::scene().selection.select(m_entity);
    //   m_is_entity_added_to_selection = true;
    // }

    m_mode = Mode::ModeEntity;
  }

  void DirectSelectTool::on_element_pointer_down() {
    // if (!Editor::scene().selection.has(m_element->id) || !m_element->selection()->full()) {
    //   if (!InputManager::keys.shift) Editor::scene().selection.clear();

    //   Editor::scene().selection.select(m_element, true);
    //   m_is_entity_added_to_selection = true;
    // }

    m_mode = Mode::ModeElement;
  }

  void DirectSelectTool::on_bezier_pointer_down() {
    // if (
    //   !Editor::scene().selection.has(m_element->id) ||
    //   !m_element->selection()->has(m_bezier->start().id) ||
    //   !m_element->selection()->has(m_bezier->end().id)
    //   ) {
    //   if (m_bezier->type() == BezierEntity::Type::Linear) {
    //     if (!InputManager::keys.shift) {
    //       Editor::scene().selection.clear();
    //     }

    //     m_element->selection()->select(&m_bezier->start());
    //     m_element->selection()->select(&m_bezier->end());

    //     m_is_entity_added_to_selection = true;
    //   } else {
    //     if (InputManager::keys.shift) {
    //       m_element->selection()->select(&m_bezier->start());
    //       m_element->selection()->select(&m_bezier->end());

    //       m_is_entity_added_to_selection = true;
    //     } else {
    //       Editor::scene().selection.clear();
    //       Editor::scene().selection.select(m_element, false);

    //       m_last_bezier_point = InputManager::pointer.scene.origin - m_element->transform()->position().get();
    //       m_last_bezier_p1 = m_bezier->p1();
    //       m_last_bezier_p2 = m_bezier->p2();
    //       m_closest = m_bezier->closest_to(m_last_bezier_point);


    //       m_should_evaluate_selection = true;
    //     }
    //   }
    // }

    m_mode = Mode::ModeBezier;
  }

  void DirectSelectTool::on_vertex_pointer_down() {
    // if (!m_element->selection()->has(m_vertex->id)) {
    //   if (!InputManager::keys.shift) Editor::scene().selection.clear();

    //   m_element->selection()->select(m_vertex);
    //   m_is_entity_added_to_selection = true;
    // }

    // console::log("vertex", m_vertex->transform()->position().get());
    // if (m_vertex->transform()->left()) console::log("left", m_vertex->transform()->left()->get());
    // if (m_vertex->transform()->right()) console::log("right", m_vertex->transform()->right()->get());

    m_mode = Mode::ModeVertex;
  }

  void DirectSelectTool::on_handle_pointer_down() {
    // m_vertex = dynamic_cast<VertexEntity*>(m_handle->parent);
    // m_element = dynamic_cast<ElementEntity*>(m_vertex->parent);

    m_mode = Mode::ModeHandle;
  }

  /* -- on_pointer_move -- */

  void DirectSelectTool::on_none_pointer_move() {
    //   if (m_selection_rect.active()) {
    //     m_selection_rect.size(InputManager::pointer.scene.delta);
    //     Box box = m_selection_rect.transform()->bounding_box();
    //     std::vector<Entity*> entities = Editor::scene().entities_in(box, false);

    //     for (Entity* entity : entities) {
    //       ElementEntity* element = dynamic_cast<ElementEntity*>(entity);
    //       if (element) {
    //         std::vector<Entity*> vertices{};
    //         element->entities_in(box, vertices, true);

    //         element->selection()->temp_select(vertices);
    //         if (element->selection()->empty()) {
    //           element->selection()->temp_all();
    //         }
    //       }
    //     }

    //     Editor::scene().selection.temp_select(entities);
    //   }
  }

  void DirectSelectTool::on_duplicate_pointer_move() {
    // translate_selected();
  }

  void DirectSelectTool::on_entity_pointer_move() {
    // translate_selected();
  }

  void DirectSelectTool::on_element_pointer_move() {
    // translate_selected();
  }

  // TODO: test with asymmetric handles
  void DirectSelectTool::on_bezier_pointer_move() {
    // if (!m_should_evaluate_selection) {
    //   translate_selected();
    //   return;
    // }

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

    // vec2 position = InputManager::pointer.scene.position - m_element->transform()->position().get();

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

    // float angle = std::fmodf(std::atan2f(p3.y - p0.y, p3.x - p0.x) - std::atan2f(position.y - p0.y, position.x - p0.x) + MATH_TWO_PI, MATH_TWO_PI);
    // float bc = (angle < 0.0f || angle > MATH_PI ? -1.0f : 1.0f) * distance(p0, p3) / 3.0f;
    // float de1 = ideal_t * bc;
    // float de2 = (1.0f - ideal_t) * bc;

    // vec2 tangent1 = { position.x - 10.0f * (position.y - center.y), position.y + 10.0f * (position.x - center.x) };
    // vec2 tangent2 = { position.x + 10.0f * (position.y - center.y), position.y - 10.0f * (position.x - center.x) };
    // vec2 direction = normalize(tangent2 - tangent1);

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

  void DirectSelectTool::on_vertex_pointer_move() {
    translate_selected();
  }

  void DirectSelectTool::on_handle_pointer_move() {
    // if (InputManager::keys.space) {
    //   m_vertex->transform()->translate(InputManager::pointer.scene.movement);
    //   return;
    // }

    // if (m_vertex->left() && m_handle->id == m_vertex->left()->id) {
    //   m_vertex->transform()->translate_left(InputManager::pointer.scene.movement, !InputManager::keys.alt);
    // } else if (m_vertex->right() && m_handle->id == m_vertex->right()->id) {
    //   m_vertex->transform()->translate_right(InputManager::pointer.scene.movement, !InputManager::keys.alt);
    // }
  }

  /* -- on_pointer_up -- */

  void DirectSelectTool::on_none_pointer_up() {
    // Editor::scene().selection.sync(true);

    // if (m_selection_rect.active()) {
    //   m_selection_rect.reset();
    // }
  }

  void DirectSelectTool::on_duplicate_pointer_up() {
    // if (m_dragging_occurred) {
    //   for (const auto& [id, entity] : Editor::scene().selection) {
    //     entity->transform()->apply();
    //   }
    // }
  }

  void DirectSelectTool::on_entity_pointer_up() {
    // if (m_dragging_occurred) {
    //   for (const auto& [id, entity] : Editor::scene().selection) {
    //     entity->transform()->apply();
    //   }
    // } else if (Editor::scene().selection.has(m_entity->id) && !m_is_entity_added_to_selection) {
    //   shift_select_element(m_entity);
    // }
  }

  void DirectSelectTool::on_element_pointer_up() {
    // if (m_dragging_occurred) {
    //   for (const auto& [id, entity] : Editor::scene().selection) {
    //     entity->transform()->apply();
    //   }
    // } else if (Editor::scene().selection.has(m_element->id) && !m_is_entity_added_to_selection) {
    //   shift_select_element(m_element);
    // }
  }

  void DirectSelectTool::on_bezier_pointer_up() {
    // if (m_dragging_occurred) {
    //   for (const auto& [id, entity] : Editor::scene().selection) {
    //     entity->transform()->apply();
    //   }
    // } else if (m_should_evaluate_selection) {
    //   m_element->selection()->select(&m_bezier->start());
    //   m_element->selection()->select(&m_bezier->end());
    // } else if (
    //   !m_is_entity_added_to_selection &&
    //   Editor::scene().selection.has(m_element->id) &&
    //   m_element->selection()->has(m_bezier->start().id) &&
    //   m_element->selection()->has(m_bezier->end().id)
    //   ) {
    //   if (InputManager::keys.shift) {
    //     m_element->selection()->deselect(m_bezier->start().id);
    //     m_element->selection()->deselect(m_bezier->end().id);
    //   } else {
    //     if (InputManager::pointer.button == InputManager::PointerButton::Left) {
    //       Editor::scene().selection.clear();
    //     }

    //     m_element->selection()->select(&m_bezier->start());
    //     m_element->selection()->select(&m_bezier->end());
    //   }
    // }
  }

  void DirectSelectTool::on_vertex_pointer_up() {
    if (m_vertex.has_value()) {
      auto vertex = m_vertex.value().lock();
      if (vertex) {
        vertex->apply();
      }
    }
    // if (m_dragging_occurred && !m_element->selection()->empty()) {
    //   for (const auto& [id, entity] : Editor::scene().selection) {
    //     entity->transform()->apply();
    //   }
    // } else if (m_element->selection()->has(m_vertex->id) && !m_is_entity_added_to_selection) {
    //   if (InputManager::keys.shift) {
    //     m_element->selection()->deselect(m_vertex->id);
    //   } else {
    //     if (InputManager::pointer.button == InputManager::PointerButton::Left) {
    //       Editor::scene().selection.clear();
    //     }

    //     m_element->selection()->select(m_vertex);
    //   }
    // }
  }

  void DirectSelectTool::on_handle_pointer_up() {
    // if (m_dragging_occurred) {
    //   m_element->transform()->apply();
    // }
  }

}
