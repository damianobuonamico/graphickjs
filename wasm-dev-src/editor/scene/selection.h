#pragma once

#include "../../utils/uuid.h"

#include <unordered_set>

namespace Graphick::Editor {

  class Scene;

  class Selection {
  public:
    Selection(Scene* scene);

    inline const std::unordered_set<uuid>& selected() const { return m_selected; }
    inline const std::unordered_set<uuid>& temp_selected() const { return m_temp_selected; }

    inline size_t size() const { return m_selected.size(); }
    inline bool empty() const { return size() < 1; }
    inline bool has(const uuid id) const {
      return m_selected.find(id) != m_selected.end() ||
        m_temp_selected.find(id) != m_temp_selected.end();
    }

    void clear();
    void select(const uuid id);
    void deselect(const uuid id);
    void temp_select(const std::vector<uuid>& entities);
    void sync();
  // TEMP
  public:
    std::unordered_set<uuid> m_selected_vertices;
  private:
    std::unordered_set<uuid> m_selected;
    std::unordered_set<uuid> m_temp_selected;


    Scene* m_scene;
  };

}
