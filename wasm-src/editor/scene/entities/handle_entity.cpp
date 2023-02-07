#include "handle_entity.h"

void HandleEntity::render(float zoom) const {
  // TODO: Implement square geometry and refactor
  Geometry geo;
  const vec4 color = { 1.0f, 0.0f, 0.0f, 1.0f };

  geo.vertices.push_back({ m_position, color });

  const float radius = m_radius / zoom;
  const float step = 0.1f;
  for (float angle = 0.0f; angle < MATH_TWO_PI; angle += step) {
    geo.vertices.push_back({ m_position + radius * vec2(cos(angle), sin(angle)), color });
  }

  for (uint32_t i = 0; i < geo.vertices.size() - 1; i++) {
    geo.indices.push_back(0);
    geo.indices.push_back(i);
    geo.indices.push_back(i + 1);
  }

  geo.indices.push_back(0);
  geo.indices.push_back((uint32_t)geo.vertices.size() - 1);
  geo.indices.push_back(1);

  Renderer::get()->draw(geo);
}

Entity* HandleEntity::entity_at(const vec2& position, bool lower_level, float threshold) {
  if (type == Type::Circle) {
    if (is_point_in_circle(position, m_position, m_radius + threshold)) {
      return this;
    }

    return nullptr;
  }

  // TODO: Replace with bounding box
  if (is_point_in_box(position, Box{ m_position - m_radius, m_position + m_radius })) {
    return this;
  }

  return nullptr;
}
