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

  Geometry lines_geometry{ GL_LINES };
  InstancedGeometry vertex_geometry{};
  InstancedGeometry handle_geometry{};

  vertex_geometry.push_quad(vec2{ 0.0f }, 4.0f / zoom, vec4(0.22f, 0.76f, 0.95f, 1.0f));
  vertex_geometry.push_quad(vec2{ 0.0f }, 2.5f / zoom, vec4(1.0f, 1.0f, 1.0f, 1.0f));
  handle_geometry.push_circle(vec2{ 0.0f }, 2.5f / zoom, vec4(0.22f, 0.76f, 0.95f, 1.0f), 10);

  for (const auto& [id, entity] : selection) {
    const ElementEntity* element = dynamic_cast<const ElementEntity*>(entity);
    if (element != nullptr) {
      vec2 position = element->transform().position().get();
      vertex_geometry.reserve_instances(element->vertex_count());
      handle_geometry.reserve_instances(element->vertex_count());

      for (const auto& [id, vertex] : *element) {
        vec2 vertex_position = position + vertex->transform().position().get();
        vertex_geometry.push_instance(vertex_position);

        HandleEntity* left = vertex->left();
        HandleEntity* right = vertex->right();

        uint32_t vertex_index = lines_geometry.offset();

        if (left || right) {
          lines_geometry.push_vertex({ vertex_position, vec4(0.22f, 0.76f, 0.95f, 1.0f) });
        }

        if (left) {
          vec2 handle_position = vertex_position + left->transform().position().get();
          handle_geometry.push_instance(handle_position);

          lines_geometry.push_indices({ vertex_index, lines_geometry.offset() });
          lines_geometry.push_vertex({ handle_position, vec4(0.22f, 0.76f, 0.95f, 1.0f) });
        }
        if (right) {
          vec2 handle_position = vertex_position + right->transform().position().get();
          handle_geometry.push_instance(handle_position);

          lines_geometry.push_indices({ vertex_index, lines_geometry.offset() });
          lines_geometry.push_vertex({ handle_position, vec4(0.22f, 0.76f, 0.95f, 1.0f) });
        }
      }
    }
  }

  Renderer::draw(lines_geometry);
  Renderer::draw(vertex_geometry);
  Renderer::draw(handle_geometry);
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
