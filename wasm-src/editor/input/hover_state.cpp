#include "hover_state.h"

Entity* HoverState::element() const {
  if (!m_entity) {
    return nullptr;
  }

  Entity* entity = m_entity;

  while (entity->parent && !entity->is_in_category(Entity::CategorySelectable)) {
    entity = entity->parent;
  }

  if (entity->is_in_category(Entity::CategorySelectable)) {
    return entity;
  }

  return nullptr;
}
