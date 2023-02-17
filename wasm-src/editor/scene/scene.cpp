#include "scene.h"

#include "entities/element_entity.h"

void Scene::load() {
  m_children.insert({ {}, std::make_shared<ElementEntity>(vec2{ 0.0f }) });
}

void Scene::render(float zoom) const {
  for (const auto& [id, entity] : m_children) {
    entity->render(zoom);
  }

  render_selection(zoom);
}

void Scene::render_selection(float zoom) const {
  if (selection.empty()) return;

  InstancedGeometry vertex_geometry{};

  vertex_geometry.push_quad(vec2{ 0.0f }, 5.0f / zoom, vec4(0.35f, 0.35f, 1.0f, 1.0f));

  for (const auto& [id, entity] : selection) {
    const ElementEntity* element = dynamic_cast<const ElementEntity*>(entity);
    if (element != nullptr) {
      vec2 position = element->transform().position().get();
      for (const auto& [id, vertex] : *element) {
        vertex_geometry.push_instance(position + vertex->transform().position().get());
      }
    }
  }

  Renderer::draw(vertex_geometry);
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
