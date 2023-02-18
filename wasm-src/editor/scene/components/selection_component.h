#pragma once

#include "../component.h"

#include <unordered_map>

class SelectionComponent: public Component {
public:
  SelectionComponent(Entity* entity): Component(entity) {};
  SelectionComponent(const SelectionComponent&) = default;
  SelectionComponent(SelectionComponent&&) = default;

  ~SelectionComponent() = default;

  inline size_t size() const { return m_selected.size() + m_temp_selected.size(); }
  inline bool empty() const { return size() < 1; }
  inline bool has(UUID id) const {
    return m_selected.find(id) != m_selected.end() ||
      m_temp_selected.find(id) != m_temp_selected.end();
  }
  bool full() const;
  std::vector<Entity*> entities();

  void clear(bool deselect = true);
  void select(Entity* entity);
  void deselect(UUID id);
  void temp_select(std::vector<Entity*> entities);
  void sync();
  void all();
private:
  std::unordered_map<UUID, Entity*> m_selected;
  std::unordered_map<UUID, Entity*> m_temp_selected;
};
