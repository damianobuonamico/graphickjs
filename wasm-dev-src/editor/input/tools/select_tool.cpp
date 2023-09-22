#include "select_tool.h"

#include "../input_manager.h"

#include "../../editor.h"
#include "../../scene/entity.h"

#include "../../../renderer/renderer.h"

#include "../../../utils/console.h"

namespace Graphick::Editor::Input {

  SelectTool::SelectTool() : Tool(ToolType::Select, CategoryNone) {}

  void SelectTool::on_pointer_down() {
    m_is_element_added_to_selection = false;
    m_dragging_occurred = false;
    m_entity = InputManager::hover.entity().has_value() ? InputManager::hover.entity()->id() : uuid{ 0 };

    if (!InputManager::keys.shift && (m_entity == uuid{ 0 } || !Editor::scene().selection.has(m_entity))) {
      Editor::scene().selection.clear();
    }

    if (m_entity != uuid{ 0 }) {
      if (!Editor::scene().selection.has(m_entity)) {
        Editor::scene().selection.select(m_entity);
        m_is_element_added_to_selection = true;
      }

      if (InputManager::keys.alt) {
        // std::vector<Entity*> entities = Editor::scene().selection.entities();
        // Editor::scene().selection.clear();

        // TODO: Duplication
        // for (Entity* entity : entities) {
        //   Entity* duplicate = Editor::scene().duplicate(entity);
        //   if (duplicate) Editor::scene().selection.select(duplicate);
        // }
      }
    } else {
      m_selection_rect.set(InputManager::pointer.scene.position);
    }
  }

  void SelectTool::on_pointer_move() {
    if ((m_entity != uuid{ 0 } && Editor::scene().selection.has(m_entity)) || InputManager::keys.alt) {
      if (!Editor::scene().selection.empty()) {
        vec2 delta = InputManager::pointer.scene.delta;
        // TODO: Snapping

        m_dragging_occurred = true;
        for (auto& [id, _] : Editor::scene().selection.selected()) {
          Entity entity = Editor::scene().get_entity(id);
          if (!entity.has_component<TransformComponent>()) continue;

          entity.get_component<TransformComponent>().position.set_delta(delta);
        }
        //     for (auto& [id, entity] : Editor::scene().selection) {
        //       entity->transform()->translate(delta - entity->transform()->position().delta());
      }
    } else if (m_selection_rect.active()) {
      OPTICK_EVENT();

      m_selection_rect.size(InputManager::pointer.scene.delta);
      Editor::scene().selection.temp_select(Editor::scene().entities_in(m_selection_rect.bounding_rect()));
    }
  }

  void SelectTool::on_pointer_up() {
    // TODO: abort
    Editor::scene().selection.sync();

    if (m_selection_rect.active()) {
      m_selection_rect.reset();
    }

    if (m_dragging_occurred && !Editor::scene().selection.empty()) {
      for (auto& [id, _] : Editor::scene().selection.selected()) {
        Entity entity = Editor::scene().get_entity(id);
        if (!entity.has_component<TransformComponent>()) continue;

        entity.get_component<TransformComponent>().position.apply();
      }
      // for (auto& [id, entity] : Editor::scene().selection) {
      //   entity->transform()->position().apply();
      // }
    } else if (m_entity != uuid{ 0 } && Editor::scene().selection.has(m_entity) && !m_is_element_added_to_selection) {
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

    Renderer::Renderer::draw_outline(m_selection_rect.path(), m_selection_rect.position());
  }

}
