#include "scene.h"

#include "entities/element_entity.h"

void Scene::load() {
  m_children.insert({ {}, std::make_shared<ElementEntity>(vec2{ 0.0f }) });
}

void Scene::render(float zoom) const {
  for (const auto& [id, entity] : m_children) {
    entity->render(zoom);
  }
}

Entity* Scene::entity_at(const vec2& position, bool lower_level, float threshold) {
  for (const auto& [id, entity] : m_children) {
    Entity* hovered = entity->entity_at(position, lower_level, threshold);
    if (hovered) {
      return hovered;
    }
  }

  return nullptr;
}
