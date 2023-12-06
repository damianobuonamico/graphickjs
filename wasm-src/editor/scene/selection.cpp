#include "selection.h"

#include "scene.h"
#include "entity.h"

#include "../../math/vector.h"

#include "../../utils/console.h"

namespace Graphick::Editor {

  Selection::Selection(Scene* scene) : m_scene(scene) {}

  // TODO: optimize selection parsing and rendering
  rect Selection::bounding_rect() const {
    GK_TOTAL("Selection::bounding_rect");

    rect selection_rect;

    for (auto& [id, _] : m_selected) {
      Entity entity = m_scene->get_entity(id);
      rect entity_rect = entity.get_component<TransformComponent>().bounding_rect();

      Math::min(selection_rect.min, entity_rect.min, selection_rect.min);
      Math::max(selection_rect.max, entity_rect.max, selection_rect.max);
    }

    return selection_rect;
  }

  bool Selection::has(const uuid id, bool include_temp) const {
    return m_selected.find(id) != m_selected.end() || (include_temp && m_temp_selected.find(id) != m_temp_selected.end());
  }

  bool Selection::has_vertex(const uuid id, const uuid element_id, bool include_temp) const {
    auto it = m_selected.find(element_id);
    if (it == m_selected.end()) {
      it = m_temp_selected.find(element_id);

      if (it == m_temp_selected.end()) return false;
    }

    if (it->second.type != SelectionEntry::Type::Element) return false;

    SelectionElementEntry& entry = (SelectionElementEntry&)it->second;
    if (entry.full()) return true;

    return entry.vertices.find(id) != entry.vertices.end();
  }

  void Selection::clear() {
    m_selected.clear();
    m_temp_selected.clear();
  }

  void Selection::select(const uuid id) {
    if (!m_scene->has_entity(id)) return;

    Entity entity = m_scene->get_entity(id);
    if (!entity.is_in_category(CategoryComponent::Selectable)) return;

    if (entity.is_element()) {
      m_selected[id] = SelectionElementEntry{ { 0 } };
    } else {
      m_selected[id] = SelectionEntry{};
    }
  }

  // Rename api to select_child, ...
  void Selection::select_vertex(const uuid id, const uuid element_id) {
    auto it = m_selected.find(element_id);
    if (it == m_selected.end() || it->second.type != SelectionEntry::Type::Element) {
      m_selected[element_id] = SelectionElementEntry{ { id } };
      return;
    }

    SelectionElementEntry& entry = (SelectionElementEntry&)it->second;

    if (!entry.full()) {
      entry.vertices.insert(id);
    }
  }

  void Selection::deselect(const uuid id) {
    m_selected.erase(id);
  }

  void Selection::deselect_vertex(const uuid id, const uuid element_id) {
    auto it = m_selected.find(element_id);
    if (it == m_selected.end() || it->second.type != SelectionEntry::Type::Element) return;

    SelectionElementEntry& entry = (SelectionElementEntry&)it->second;

    if (entry.full()) {
      Entity element = m_scene->get_entity(element_id);
      entry.vertices.clear();

#if 0
      for (uuid vertex_id : element.get_component<PathComponent>().path.vertices_ids()) {
        if (vertex_id == id) continue;
        entry.vertices.insert(vertex_id);
      }
#endif
    } else {
      entry.vertices.erase(id);
    }

    if (entry.vertices.empty()) {
      m_selected.erase(element_id);
    }
  }

  void Selection::temp_select(const std::unordered_map<uuid, SelectionEntry>& entities) {
    m_temp_selected = entities;
  }

  void Selection::sync() {
    for (auto& [id, entry] : m_temp_selected) {
      if (entry.type == SelectionEntry::Type::Element) {
        SelectionElementEntry& element_entry = (SelectionElementEntry&)entry;
        auto it = m_selected.find(id);

        if (it != m_selected.end() && it->second.type == SelectionEntry::Type::Element) {
          if (((SelectionElementEntry&)it->second).full()) continue;

          for (uuid vertex_id : element_entry.vertices) {
            ((SelectionElementEntry&)it->second).vertices.insert(vertex_id);
          }
        } else {
          m_selected[id] = element_entry;
        }
      } else {
        m_selected[id] = entry;
      }
    }

    m_temp_selected.clear();
  }

}
