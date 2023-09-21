#pragma once

#include "../../utils/uuid.h"

#include <unordered_set>
#include <unordered_map>

namespace Graphick::Editor {

  class Scene;

  class Selection {
  public:
    // TODO: Join SelectionEntry and SelectionElementEntry
    struct SelectionEntry {
      enum class Type {
        Entity = 0,
        Element
      };

      std::unordered_set<uuid> vertices;
      Type type;

      SelectionEntry(const Type type = Type::Entity) : type(type) {}
      SelectionEntry(std::unordered_set<uuid> vertices, const Type type = Type::Entity) : vertices(vertices), type(type) {}
    };

    struct SelectionElementEntry : public SelectionEntry {

      SelectionElementEntry() : SelectionEntry(Type::Element) {}
      SelectionElementEntry(std::unordered_set<uuid> vertices) : SelectionEntry(vertices, Type::Element) {}

      inline bool full() const { return vertices.size() == 1 && *vertices.begin() == 0; }
    };
  public:
    Selection(Scene* scene);

    inline const std::unordered_map<uuid, SelectionEntry>& selected() const { return m_selected; }
    inline const std::unordered_map<uuid, SelectionEntry>& temp_selected() const { return m_temp_selected; }

    inline size_t size() const { return m_selected.size(); }
    inline bool empty() const { return size() < 1; }
    inline const SelectionEntry& get(const uuid id) const { return m_selected.at(id); }

    bool has(const uuid id, bool include_temp = false) const;
    bool has_vertex(const uuid id, const uuid element_id, bool include_temp = false) const;

    void clear();

    void select(const uuid id);
    void select_vertex(const uuid id, const uuid element_id);

    void deselect(const uuid id);
    void deselect_vertex(const uuid id, const uuid element_id);

    void temp_select(const std::unordered_map<uuid, SelectionEntry>& entities);

    void sync();
  private:
    std::unordered_map<uuid, SelectionEntry> m_selected;
    // std::unordered_set<uuid> m_selected_vertices;

    std::unordered_map<uuid, SelectionEntry> m_temp_selected;
    // std::unordered_set<uuid> m_temp_selected_vertices;

    Scene* m_scene;
  };

}
