#include "selection_state.h"

/* -- iterator -- */

SelectionState::iterator::iterator(
  std::unordered_map<UUID, Entity*>::iterator first_iterator,
  std::unordered_map<UUID, Entity*>::iterator first_end_iterator,
  std::unordered_map<UUID, Entity*>::iterator second_begin_iterator,
  std::unordered_map<UUID, Entity*>::iterator second_iterator
): m_first_iterator(first_iterator), m_first_end_iterator(first_end_iterator),
m_second_begin_iterator(second_begin_iterator), m_second_iterator(second_iterator) {}

SelectionState::iterator& SelectionState::iterator::operator++() {
  if (m_first_iterator != m_first_end_iterator) {
    ++m_first_iterator;
  } else {
    ++m_second_iterator;
  }

  return *this;
}

SelectionState::iterator SelectionState::iterator::operator++(int) {
  iterator tmp(*this);
  operator++();
  return tmp;
}

bool SelectionState::iterator::operator==(const iterator& other) const {
  return m_first_iterator == other.m_first_iterator && m_second_iterator == other.m_second_iterator;
}

bool SelectionState::iterator::operator!=(const iterator& other) const {
  return m_first_iterator != other.m_first_iterator || m_second_iterator != other.m_second_iterator;
}

SelectionState::iterator::reference SelectionState::iterator::operator*() {
  if (m_first_iterator != m_first_end_iterator) {
    return *m_first_iterator;
  } else {
    return *m_second_iterator;
  }
}

SelectionState::iterator::pointer SelectionState::iterator::operator->() {
  return &(operator*());
}

/* -- const_iterator -- */

SelectionState::const_iterator::const_iterator(
  std::unordered_map<UUID, Entity*>::const_iterator first_iterator,
  std::unordered_map<UUID, Entity*>::const_iterator first_end_iterator,
  std::unordered_map<UUID, Entity*>::const_iterator second_begin_iterator,
  std::unordered_map<UUID, Entity*>::const_iterator second_iterator
): m_first_iterator(first_iterator), m_first_end_iterator(first_end_iterator),
m_second_begin_iterator(second_begin_iterator), m_second_iterator(second_iterator) {}

SelectionState::const_iterator& SelectionState::const_iterator::operator++() {
  if (m_first_iterator != m_first_end_iterator) {
    ++m_first_iterator;
  } else {
    ++m_second_iterator;
  }

  return *this;
}

SelectionState::const_iterator SelectionState::const_iterator::operator++(int) {
  const_iterator tmp(*this);
  operator++();
  return tmp;
}

bool SelectionState::const_iterator::operator==(const const_iterator& other) const {
  return m_first_iterator == other.m_first_iterator && m_second_iterator == other.m_second_iterator;
}

bool SelectionState::const_iterator::operator!=(const const_iterator& other) const {
  return m_first_iterator != other.m_first_iterator || m_second_iterator != other.m_second_iterator;
}

SelectionState::const_iterator::reference SelectionState::const_iterator::operator*() {
  if (m_first_iterator != m_first_end_iterator) {
    return { m_first_iterator->first, m_first_iterator->second };
  } else {
    return *m_second_iterator;
  }
}

SelectionState::const_iterator::pointer SelectionState::const_iterator::operator->() {
  return { operator*() };
}

/* -- SelectionState -- */

std::vector<Entity*> SelectionState::entities() {
  std::vector<Entity*> entities;
  entities.reserve(size());

  for (auto& [id, entity] : *this) {
    entities.push_back(entity);
  }

  return entities;
}

Box SelectionState::bounding_box() const {
  Box box = { std::numeric_limits<vec2>::max(), std::numeric_limits<vec2>::min() };

  for (auto& [id, entity] : m_selected) {
    Box entity_box = entity->transform()->bounding_box();
    min(box.min, entity_box.min, box.min);
    max(box.max, entity_box.max, box.max);
  }

  return box;
}

void SelectionState::clear() {
  for (auto& [id, entity] : *this) {
    if (entity->is_in_category(Entity::CategorySelectableChildren)) {
      entity->selection()->clear(false);
    }
  }

  m_selected.clear();
  m_temp_selected.clear();
}

void SelectionState::select(Entity* entity, bool select_children) {
  if (!entity->is_in_category(Entity::CategorySelectable)) {
    return;
  }

  m_selected.insert({ entity->id, entity });

  if (select_children && entity->is_in_category(Entity::CategorySelectableChildren)) {
    entity->selection()->all();
  }
}

void SelectionState::deselect(UUID id, bool deselect_children) {
  if (deselect_children) {
    auto it = m_selected.find(id);
    if (it != m_selected.end()) {
      Entity* entity = it->second;

      if (entity->is_in_category(Entity::CategorySelectableChildren)) {
        entity->selection()->clear();
      }
    }
  }

  m_selected.erase(id);
}

void SelectionState::temp_select(std::vector<Entity*> entities) {
  m_temp_selected.clear();

  for (Entity* entity : entities) {
    if (entity->is_in_category(Entity::CategorySelectable)) {
      m_temp_selected.insert({ entity->id, entity });
    }
  }
}

void SelectionState::sync(bool sync_children) {
  if (sync_children) {
    for (auto& [id, entity] : m_temp_selected) {
      if (entity->is_in_category(Entity::CategorySelectableChildren)) {
        entity->selection()->sync();

        if (entity->selection()->size()) {
          select(entity, false);
        }
      }
    }
  } else {
    for (auto& [id, entity] : m_temp_selected) {
      select(entity);
    }
  }

  m_temp_selected.clear();
}
