#include "direct_select_tool.h"

#include "../input_manager.h"
#include "../../editor.h"
#include "../../../utils/console.h"

DirectSelectTool::DirectSelectTool(): Tool(ToolType::DirectSelect, CategoryDirect) {}

void DirectSelectTool::on_pointer_down() {
  // TODO: selection rect
  m_is_entity_added_to_selection = false;
  m_dragging_occurred = false;
  m_mode = ModeNone;

  Entity* hovered = InputManager::hover.entity();
  m_entity = hovered ? InputManager::hover.element() : nullptr;
  m_element = dynamic_cast<ElementEntity*>(m_entity);
  m_bezier = dynamic_cast<BezierEntity*>(hovered);
  m_handle = dynamic_cast<HandleEntity*>(hovered);
  m_vertex = nullptr;


  if (m_handle) {
    VertexEntity* vertex = dynamic_cast<VertexEntity*>(m_handle->parent);
    if (vertex && vertex->position()->id == m_handle->id) {
      m_vertex = vertex;
    }
  }

  if (InputManager::keys.alt && m_entity && (!m_handle || m_vertex)) {
    on_duplicate_pointer_down();

    console::log("DirectSelectTool::duplicate");
  } else if (m_element) {
    if (m_vertex) {
      on_vertex_pointer_down();
      console::log("DirectSelectTool::vertex");
    } else if (m_handle) {
      on_handle_pointer_down();
      console::log("DirectSelectTool::handle");
    } else if (m_bezier) {
      on_bezier_pointer_down();
      console::log("DirectSelectTool::bezier");
    } else {
      on_element_pointer_down();
      console::log("DirectSelectTool::element");
    }
  } else if (m_entity && m_entity->is_in_category(Entity::CategorySelectable)) {
    on_entity_pointer_down();
    console::log("DirectSelectTool::entity");
  } else {
    on_none_pointer_down();
  }
}

void DirectSelectTool::on_pointer_move() {
  m_dragging_occurred = true;

  switch (m_mode) {
  case ModeDuplicate:
    on_duplicate_pointer_move();
    break;
  case ModeElement:
    on_element_pointer_move();
    break;
  case ModeVertex:
    on_vertex_pointer_move();
    break;
  case ModeHandle:
    on_handle_pointer_move();
    break;
  case ModeBezier:
    on_bezier_pointer_move();
    break;
  case ModeEntity:
    on_entity_pointer_move();
    break;
  default:
    on_none_pointer_move();
    break;
  }
}

void DirectSelectTool::on_pointer_up(bool abort) {
  switch (m_mode) {
  case ModeDuplicate:
    on_duplicate_pointer_up();
    break;
  case ModeElement:
    on_element_pointer_up();
    break;
  case ModeVertex:
    on_vertex_pointer_up();
    break;
  case ModeHandle:
    on_handle_pointer_up();
    break;
  case ModeBezier:
    on_bezier_pointer_up();
    break;
  case ModeEntity:
    on_entity_pointer_up();
    break;
  default:
    on_none_pointer_up();
    break;
  }
}

// TODO: Implement rotation
void DirectSelectTool::translate_selected() {
  for (const auto& [id, entity] : Editor::scene.selection) {
    if (entity->is_in_category(Entity::CategorySelectableChildren)) {
      ElementEntity* element = dynamic_cast<ElementEntity*>(entity);

      if (element && !element->selection()->full()) {
        for (Entity* vertex : element->selection()->entities()) {
          vertex->transform().translate(InputManager::pointer.scene.movement);
        }
        continue;
      }
    }

    entity->transform().translate(InputManager::pointer.scene.movement);
  }
}

void DirectSelectTool::shift_select_element(Entity* entity) {
  if (InputManager::keys.shift) {
    Editor::scene.selection.deselect(entity->id);
  } else {
    if (InputManager::pointer.button == InputManager::PointerButton::Left) {
      Editor::scene.selection.clear();
    }
    Editor::scene.selection.select(entity, true);
  }
}

/* -- on_pointer_down -- */

void DirectSelectTool::on_none_pointer_down() {
  if (!InputManager::keys.shift) {
    Editor::scene.selection.clear();
  }

  m_mode = ModeNone;
}

void DirectSelectTool::on_duplicate_pointer_down() {
  if (!Editor::scene.selection.has(m_entity->id)) {
    if (!InputManager::keys.shift) Editor::scene.selection.clear();

    Editor::scene.selection.select(m_entity);
    m_is_entity_added_to_selection = true;
  }

  std::vector<Entity*> entities = Editor::scene.selection.entities();
  Editor::scene.selection.clear();

  for (Entity* entity : entities) {
    Entity* duplicate = Editor::scene.duplicate(entity);
    if (duplicate) Editor::scene.selection.select(duplicate);
  }

  m_mode = ModeDuplicate;
}

void DirectSelectTool::on_entity_pointer_down() {
  if (!Editor::scene.selection.has(m_entity->id)) {
    if (!InputManager::keys.shift) Editor::scene.selection.clear();

    Editor::scene.selection.select(m_entity);
    m_is_entity_added_to_selection = true;
  }

  m_mode = ModeEntity;
}

void DirectSelectTool::on_element_pointer_down() {
  if (!Editor::scene.selection.has(m_element->id) || !m_element->selection()->full()) {
    if (!InputManager::keys.shift) Editor::scene.selection.clear();

    Editor::scene.selection.select(m_element, true);
    m_is_entity_added_to_selection = true;
  }

  m_mode = ModeElement;
}

void DirectSelectTool::on_bezier_pointer_down() {
  m_mode = ModeBezier;
}

void DirectSelectTool::on_vertex_pointer_down() {
  if (!m_element->selection()->has(m_vertex->id)) {
    if (!InputManager::keys.shift) Editor::scene.selection.clear();

    m_element->selection()->select(m_vertex);
    m_is_entity_added_to_selection = true;
  }

  m_mode = ModeVertex;
}

void DirectSelectTool::on_handle_pointer_down() {
  m_vertex = dynamic_cast<VertexEntity*>(m_handle->parent);
  m_element = dynamic_cast<ElementEntity*>(m_vertex->parent);

  m_mode = ModeHandle;
}

/* -- on_pointer_move -- */

void DirectSelectTool::on_none_pointer_move() {
  // TODO: Implement
}

void DirectSelectTool::on_duplicate_pointer_move() {
  translate_selected();
}

void DirectSelectTool::on_entity_pointer_move() {
  translate_selected();
}

void DirectSelectTool::on_element_pointer_move() {
  translate_selected();
}

void DirectSelectTool::on_bezier_pointer_move() {
  // TODO: Implement
}

void DirectSelectTool::on_vertex_pointer_move() {
  translate_selected();
}

void DirectSelectTool::on_handle_pointer_move() {
  if (InputManager::keys.space) {
    m_vertex->transform().translate(InputManager::pointer.scene.movement);
    return;
  }

  if (m_vertex->left() && m_handle->id == m_vertex->left()->id) {
    m_vertex->transform().translate_left(InputManager::pointer.scene.movement, !InputManager::keys.alt);
  } else if (m_vertex->right() && m_handle->id == m_vertex->right()->id) {
    m_vertex->transform().translate_right(InputManager::pointer.scene.movement, !InputManager::keys.alt);
  }
}

/* -- on_pointer_up -- */

void DirectSelectTool::on_none_pointer_up() {}

void DirectSelectTool::on_duplicate_pointer_up() {
  if (m_dragging_occurred) {
    for (const auto& [id, entity] : Editor::scene.selection) {
      entity->transform().apply();
    }
  }
}

void DirectSelectTool::on_entity_pointer_up() {
  if (m_dragging_occurred) {
    for (const auto& [id, entity] : Editor::scene.selection) {
      entity->transform().apply();
    }
  } else if (Editor::scene.selection.has(m_entity->id) && !m_is_entity_added_to_selection) {
    shift_select_element(m_entity);
  }
}

void DirectSelectTool::on_element_pointer_up() {
  if (m_dragging_occurred) {
    for (const auto& [id, entity] : Editor::scene.selection) {
      entity->transform().apply();
    }
  } else if (Editor::scene.selection.has(m_element->id) && !m_is_entity_added_to_selection) {
    shift_select_element(m_element);
  }
}

void DirectSelectTool::on_bezier_pointer_up() {}

void DirectSelectTool::on_vertex_pointer_up() {
  if (m_dragging_occurred && !m_element->selection()->empty()) {
    for (const auto& [id, entity] : Editor::scene.selection) {
      entity->transform().apply();
    }
  } else if (m_element->selection()->has(m_vertex->id) && !m_is_entity_added_to_selection) {
    if (InputManager::keys.shift) {
      m_element->selection()->deselect(m_vertex->id);
    } else {
      if (InputManager::pointer.button == InputManager::PointerButton::Left) {
        Editor::scene.selection.clear();
      }

      m_element->selection()->select(m_vertex);
    }
  }
}

void DirectSelectTool::on_handle_pointer_up() {
  if (m_dragging_occurred) {
    m_element->transform().apply();
  }
}
