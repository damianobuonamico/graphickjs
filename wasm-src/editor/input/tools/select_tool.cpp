/**
 * @file select_tool.cpp
 * @brief Contains the implementation of the SelectTool class.
 */

#include "select_tool.h"

#include "../input_manager.h"

#include "../../editor.h"
#include "../../scene/entity.h"

#include "../../../renderer/renderer.h"

#include "../../../utils/console.h"

namespace graphick::editor::input {

  SelectTool::SelectTool() : Tool(ToolType::Select, CategoryNone) {}

  void SelectTool::on_pointer_down() {
    m_is_entity_added_to_selection = false;
    m_dragging_occurred = false;
    m_entity = InputManager::hover.entity().has_value() ? InputManager::hover.entity()->id() : uuid::null;

    Scene& scene = Editor::scene();

    if (!InputManager::keys.shift && (m_entity == uuid::null || !scene.selection.has(m_entity))) {
      scene.selection.clear();
    }

    if (m_entity != uuid::null) {
      if (!scene.selection.has(m_entity)) {
        scene.selection.select(m_entity);
        m_is_entity_added_to_selection = true;
      }

      if (InputManager::keys.alt) {
        std::vector<uuid> duplicated;

        for (const auto& [id, _] : scene.selection.selected()) {
          duplicated.push_back(scene.duplicate_entity(id).id());
        }

        scene.selection.clear();

        for (uuid id : duplicated) {
          scene.selection.select(id);
        }
      }
    } else {
      m_selection_rect.set(InputManager::pointer.scene.position);
    }
  }

  void SelectTool::on_pointer_move() {
    m_dragging_occurred = true;

    if ((m_entity != uuid::null && Editor::scene().selection.has(m_entity)) || InputManager::keys.alt) {
      if (!Editor::scene().selection.empty()) {
        const vec2 movement = InputManager::pointer.scene.movement;

        for (auto& [id, _] : Editor::scene().selection.selected()) {
          Entity entity = Editor::scene().get_entity(id);

          entity.get_component<TransformComponent>().translate(movement);
        }
      }
    } else if (m_selection_rect.active()) {
      OPTICK_EVENT();

      m_selection_rect.size(InputManager::pointer.scene.delta);
      Editor::scene().selection.temp_select(Editor::scene().entities_in(m_selection_rect.bounding_rect()));
    }
  }

  void SelectTool::on_pointer_up() {
    Editor::scene().selection.sync();

    if (m_selection_rect.active()) {
      m_selection_rect.reset();
    }

    if (m_dragging_occurred) {
      return;
    }

    if (m_entity != uuid::null && Editor::scene().selection.has(m_entity) && !m_is_entity_added_to_selection) {
      if (InputManager::keys.shift) {
        Editor::scene().selection.deselect(m_entity);
      } else {
        if (InputManager::pointer.button == InputManager::PointerButton::Left) {
          Editor::scene().selection.clear();
        }
        Editor::scene().selection.select(m_entity);
      }
    }
  }

  void SelectTool::render_overlays() const {
    if (!m_selection_rect.active()) return;

    renderer::Renderer::draw_outline(m_selection_rect.path(), m_selection_rect.transform());
    renderer::Renderer::draw_rect(m_selection_rect.bounding_rect(), vec4(0.22f, 0.76f, 0.95f, 0.25f));
  }

}
