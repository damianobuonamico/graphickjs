#include "vertex_entity.h"

void VertexEntity::render(float zoom) const {
  // TODO: Implement instanced rendering, offset by transform.position
  m_position.render(zoom);
}

Entity* VertexEntity::entity_at(const vec2& position, bool lower_level = false, float threshold = 0.0f) {
  if (m_position.entity_at(position, lower_level, threshold)) {
    return &m_position;
  }

  vec2 offset = position - transform().position().get();

  if (HandleEntity* handle = left(); handle && handle->entity_at(offset, lower_level, threshold)) {
    return handle;
  }

  if (HandleEntity* handle = right(); handle && handle->entity_at(offset, lower_level, threshold)) {
    return handle;
  }

  return nullptr;
}

void VertexEntity::entities_in(const Box& box, std::vector<Entity*>& entities, bool lower_level = false) {
  if (is_point_in_box(transform().position().get(), box)) {
    entities.push_back(this);
  }
}
