#include "selection_component.h"
#include "../entity.h"
#include "../../editor.h"
#include "../entities/element_entity.h"

bool SelectionComponent::full() const {
  ElementEntity* element = dynamic_cast<ElementEntity*>(parent);
  if (!element) {
    return false;
  }

  return size() == element->vertex_count();
}

std::vector<Entity*> SelectionComponent::entities() {
  std::vector<Entity*> entities;
  if (!parent) return entities;

  entities.reserve(size());

  for (auto& [id, entity] : m_selected) {
    entities.push_back(entity);
  }

  for (auto& [id, entity] : m_temp_selected) {
    entities.push_back(entity);
  }

  return entities;
}

void SelectionComponent::clear(bool deselect) {
  m_selected.clear();
  m_temp_selected.clear();

  if (deselect && parent) {
    Editor::scene().selection.deselect(parent->id, false);
  }
}

void SelectionComponent::select(Entity* entity) {
  m_selected.insert({ entity->id, entity });

  if (parent && !Editor::scene().selection.has(parent->id)) {
    Editor::scene().selection.select(parent, false);
  }
}

void SelectionComponent::deselect(UUID id) {
  m_selected.erase(id);

  if (parent && m_selected.empty()) {
    Editor::scene().selection.deselect(parent->id, false);
  }
}

void SelectionComponent::temp_select(std::vector<Entity*> entities) {
  m_temp_selected.clear();

  for (Entity* entity : entities) {
    m_temp_selected.insert({ entity->id, entity });
  }
}

void SelectionComponent::sync() {
  for (auto& [id, entity] : m_temp_selected) {
    select(entity);
  }

  m_temp_selected.clear();
}

void SelectionComponent::all() {
  ElementEntity* element = dynamic_cast<ElementEntity*>(parent);
  if (!element) {
    return;
  }

  for (const auto& [id, entity] : *element) {
    select(entity.get());
  }
}

void SelectionComponent::temp_all() {
  ElementEntity* element = dynamic_cast<ElementEntity*>(parent);
  if (!element) {
    return;
  }

  for (const auto& [id, entity] : *element) {
    m_temp_selected.insert({ entity->id, entity.get() });
  }
}
