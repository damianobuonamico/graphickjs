#include "selection.h"

#include "scene.h"
#include "entity.h"

namespace Graphick::Editor {

  Selection::Selection(Scene* scene) : m_scene(scene) {}

  void Selection::clear() {
    m_selected.clear();
    m_temp_selected.clear();
  }

  void Selection::select(const uuid id) {
    if (!m_scene->has_entity(id)) return;

    Entity entity = m_scene->get_entity(id);
    // TODO: entity categories
    // if (!entity.is_in_category(Entity::CategorySelectable)) {
    //   return;
    // }

    m_selected.insert(id);
  }

  void Selection::deselect(const uuid id) {
    m_selected.erase(id);
  }

  void Selection::temp_select(const std::vector<uuid>& entities) {
    m_temp_selected.clear();

    for (uuid id : entities) {
      // TODO: entity categories
      if (m_scene->has_entity(id)) {
        m_temp_selected.insert(id);
      }
    }
  }

  void Selection::sync() {
    for (uuid id : m_temp_selected) {
      select(id);
    }

    m_temp_selected.clear();
  }

}
