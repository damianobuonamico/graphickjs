#include "handle_entity.h"

void HandleEntity::render(float zoom) const {
  // TODO: Implement square geometry and refactor
  Geometry geo;
  const vec4 color = { 1.0f, 0.0f, 0.0f, 1.0f };
  vec2 position = transform().position().get();

  if (parent && parent->parent) position += parent->parent->transform().position().get();

  geo.push_circle(position, m_radius / zoom, color);

  // geo.vertices.push_back({ position, color });

  // const float radius = m_radius / zoom;
  // const float step = 0.1f;
  // for (float angle = 0.0f; angle < MATH_TWO_PI; angle += step) {
  //   geo.vertices.push_back({ position + radius * vec2(cos(angle), sin(angle)), color });
  // }

  // for (uint32_t i = 0; i < geo.vertices.size() - 1; i++) {
  //   geo.indices.push_back(0);
  //   geo.indices.push_back(i);
  //   geo.indices.push_back(i + 1);
  // }

  // geo.indices.push_back(0);
  // geo.indices.push_back((uint32_t)geo.vertices.size() - 1);
  // geo.indices.push_back(1);

  Renderer::draw(geo);
}

Entity* HandleEntity::entity_at(const vec2& position, bool lower_level, float threshold) {
  const vec2 handle_position = m_transform.position().get();

  if (type == Type::Circle) {
    if (is_point_in_circle(position, handle_position, m_radius + threshold)) {
      return this;
    }

    return nullptr;
  }

  if (is_point_in_box(position, m_transform.bounding_box())) {
    return this;
  }

  return nullptr;
}
