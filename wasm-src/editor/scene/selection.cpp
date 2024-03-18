/**
 * @file selection.cpp
 * @brief This file contains the implementation of the Selection class.
 * 
 * @todo optimize selection parsing and rendering
 */

#include "selection.h"

#include "scene.h"
#include "entity.h"

#include "../../math/vector.h"

#include "../../utils/console.h"

namespace Graphick::Editor {

  Selection::Selection(Scene* scene) : m_scene(scene) {}

  rect Selection::bounding_rect() const {
    GK_TOTAL("Selection::bounding_rect");

    rect selection_rect;

    for (auto& [id, _] : m_selected) {
      const Entity entity = m_scene->get_entity(id);
      const rect entity_rect = entity.get_component<TransformComponent>().bounding_rect();

      Math::min(selection_rect.min, entity_rect.min, selection_rect.min);
      Math::max(selection_rect.max, entity_rect.max, selection_rect.max);
    }

    return selection_rect;
  }

  bool Selection::has(const uuid id, bool include_temp) const {
    return m_selected.find(id) != m_selected.end() || (include_temp && m_temp_selected.find(id) != m_temp_selected.end());
  }

  bool Selection::has_child(const uuid element_id, const size_t child_index, bool include_temp) const {
    auto it = m_selected.find(element_id);

    if (it == m_selected.end()) {
      it = m_temp_selected.find(element_id);

      if (it == m_temp_selected.end()) {
        return false;
      }
    }

    if (it->second.full()) {
      return true;
    }

    return it->second.indices.find(child_index) != it->second.indices.end();
  }

  void Selection::clear() {
    m_selected.clear();
    m_temp_selected.clear();
  }

  void Selection::select(const uuid id) {
    if (!m_scene->has_entity(id)) return;

    const Entity entity = m_scene->get_entity(id);

    if (entity.is_in_category(CategoryComponent::Category::Selectable)) {
      m_selected[id] = SelectionEntry{};
    }
  }

  void Selection::select_child(const uuid element_id, const size_t child_index) {
    auto it = m_selected.find(element_id);

    if (it == m_selected.end()) {
      m_selected[element_id] = SelectionEntry{ { child_index } };
      return;
    }

    if (!it->second.full()) {
      it->second.indices.insert(child_index);
    }
  }

  void Selection::deselect(const uuid id) {
    m_selected.erase(id);
  }

  void Selection::deselect_child(const uuid element_id, const size_t child_index) {
    auto it = m_selected.find(element_id);

    if (it == m_selected.end()) return;

    if (it->second.full()) {
      Entity element = m_scene->get_entity(element_id);

      it->second.indices.clear();
      it->second.type = SelectionEntry::Type::Element;

      for (size_t i : element.get_component<PathComponent>().data().vertex_indices()) {
        if (i == child_index) continue;

        it->second.indices.insert(i);
      }
    } else {
      it->second.indices.erase(child_index);
    }

    if (it->second.indices.empty()) {
      m_selected.erase(element_id);
    }
  }

  void Selection::temp_select(const std::unordered_map<uuid, SelectionEntry>& entities) {
    m_temp_selected = entities;
  }

  void Selection::sync() {
    for (auto& [id, entry] : m_temp_selected) {
      if (entry.type == SelectionEntry::Type::Element) {
        auto it = m_selected.find(id);

        if (it != m_selected.end() && it->second.type == SelectionEntry::Type::Element) {
          if (it->second.full()) continue;

          for (size_t child_index : entry.indices) {
            it->second.indices.insert(child_index);
          }
        } else {
          m_selected[id] = entry;
        }
      } else {
        m_selected[id] = entry;
      }
    }

    m_temp_selected.clear();
  }

}
