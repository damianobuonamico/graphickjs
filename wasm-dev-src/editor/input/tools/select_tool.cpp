#include "select_tool.h"

#include "../input_manager.h"

#include "../../editor.h"

#include "../../../utils/console.h"

namespace Graphick::Editor::Input {

  SelectTool::SelectTool() : Tool(ToolType::Select, CategoryNone) {}

  void SelectTool::on_pointer_down() {
    m_is_element_added_to_selection = false;
    m_dragging_occurred = false;
    // m_element = InputManager::hover.element();

    // if (!InputManager::keys.shift && (!m_element || !Editor::scene().selection.has(m_element->id))) {
    //   Editor::scene().selection.clear();
    // }

    // if (m_element) {
    //   if (!Editor::scene().selection.has(m_element->id)) {
    //     Editor::scene().selection.select(m_element);
    //     m_is_element_added_to_selection = true;
    //   }

    //   if (InputManager::keys.alt) {
    //     std::vector<Entity*> entities = Editor::scene().selection.entities();
    //     Editor::scene().selection.clear();

    //     // TODO: Duplication
    //     for (Entity* entity : entities) {
    //       Entity* duplicate = Editor::scene().duplicate(entity);
    //       if (duplicate) Editor::scene().selection.select(duplicate);
    //     }
    //   }
    // } else {
    //   m_selection_rect.set(InputManager::pointer.scene.position);
    // }
  }

  void SelectTool::on_pointer_move() {
    // if ((m_element && Editor::scene().selection.has(m_element->id)) || InputManager::keys.alt) {
    //   if (!Editor::scene().selection.empty()) {
    //     vec2 delta = InputManager::pointer.scene.delta;
    //     // TODO: Snapping

    //     m_dragging_occurred = true;
    //     for (auto& [id, entity] : Editor::scene().selection) {
    //       entity->transform()->translate(delta - entity->transform()->position().delta());
    //     }
    //   }
    // } else if (m_selection_rect.active()) {
    //   m_selection_rect.size(InputManager::pointer.scene.delta);
    //   Editor::scene().selection.temp_select(Editor::scene().entities_in(m_selection_rect.transform()->bounding_box(), false));
    // }
  }

  void SelectTool::on_pointer_up() {
    // TODO: abort
    // Editor::scene().selection.sync();

    // if (m_selection_rect.active()) {
    //   m_selection_rect.reset();
    // }

    // if (m_dragging_occurred && !Editor::scene().selection.empty()) {
    //   for (auto& [id, entity] : Editor::scene().selection) {
    //     entity->transform()->position().apply();
    //   }
    // } else if (m_element && Editor::scene().selection.has(m_element->id) && !m_is_element_added_to_selection) {
    //   if (InputManager::keys.shift) {
    //     Editor::scene().selection.deselect(m_element->id);
    //   } else {
    //     if (InputManager::pointer.button == InputManager::PointerButton::Left) {
    //       Editor::scene().selection.clear();
    //     }
    //     Editor::scene().selection.select(m_element);
    //   }
    // }
  }

  // void SelectTool::tessellate_overlays_outline(const vec4& color, const RenderingOptions& options, Geometry& geo) const {
  //   m_selection_rect.tessellate_outline(color, options, geo);
  // }

  // void SelectTool::render_overlays(const RenderingOptions& options) const {
  //   m_selection_rect.render(options);
  // }

}
