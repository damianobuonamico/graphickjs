#include "select_tool.h"

#include "../input_manager.h"
#include "../../editor.h"
#include "../../../utils/console.h"

SelectTool::SelectTool(): Tool(ToolType::Select, CategoryNone) {}

void SelectTool::on_pointer_down() {
  m_element = InputManager::hover.element();

  if (!InputManager::keys.shift && (!m_element || Editor::scene.selection.has(m_element->id))) {
    Editor::scene.selection.clear();
  }

  if (m_element) {
    if (!Editor::scene.selection.has(m_element->id)) {
      Editor::scene.selection.select(m_element);
      m_is_element_added_to_selection = true;
    }

    if (InputManager::keys.alt) {
      std::vector<Entity*> entities = Editor::scene.selection.entities();
      Editor::scene.selection.clear();

      // TODO: Duplication
      // for (Entity* entity : entities) {
      //   Entity* duplicate = SceneManager::duplicate(entity);
      //   if (duplicate) Editor::scene.selection.select(duplicate);
      // }
    }
  } else {
    // TODO: Selection box
  }
}

void SelectTool::on_pointer_move() {
  if ((m_element && Editor::scene.selection.has(m_element->id)) || InputManager::keys.alt) {
    if (!Editor::scene.selection.empty()) {
      vec2 delta = InputManager::pointer.scene.delta;
      // TODO: Snapping

      m_dragging_occurred = true;
      for (auto& [id, entity] : Editor::scene.selection) {
        entity->transform().translate(delta - entity->transform().position().delta());
      }
    }
  } // TODO: Selection box
}

void SelectTool::on_pointer_up(bool abort) {
  // TODO: abort
  Editor::scene.selection.sync();
  // TODO: Selection box

  if (m_dragging_occurred && !Editor::scene.selection.empty()) {
    for (auto& [id, entity] : Editor::scene.selection) {
      entity->transform().position().apply();
    }
  } else if (m_element && Editor::scene.selection.has(m_element->id) && !m_is_element_added_to_selection) {
    if (InputManager::keys.shift) {
      Editor::scene.selection.deselect(m_element->id);
    } else {
      if (InputManager::pointer.button == InputManager::PointerButton::Left) {
        Editor::scene.selection.clear();
      }
      Editor::scene.selection.select(m_element);
    }
  }
}