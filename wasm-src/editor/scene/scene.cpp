#include "scene.h"

#include "entities/element_entity.h"
#include "../../utils/debugger.h"
#include "../settings.h"
#include "../input/input_manager.h"

void Scene::load() {
  // std::shared_ptr<ElementEntity> element1 = std::make_shared<ElementEntity>(vec2{ 0.0f });
  std::shared_ptr<ElementEntity> element2 = std::make_shared<ElementEntity>(vec2{ 140.0f, 80.0f });

  // m_children.insert({ element1->id, element1 });
  m_children.insert({ element2->id, element2 });
}

void Scene::render(float zoom, const Box& viewport) const {
  RenderingOptions options = { zoom, std::acosf(2.0f * std::pow(1.0f - Settings::tessellation_error / std::fmaxf(zoom, 0.26f), 2.0f) - 1.0f), viewport };

  for (const auto& [id, entity] : m_children) {
    entity->render(options);
  }

  render_selection(options);

  DEBUGGER_UPDATE();
}

void Scene::render_selection(const RenderingOptions& options) const {
  const vec4 outline_color{ 0.22f, 0.76f, 0.95f, 1.0f };
  const vec4 white{ 1.0f, 1.0f, 1.0f, 1.0f };

  Geometry outline_geometry{ GL_LINES };

  const Tool& tool = InputManager::tool();
  bool is_direct_tool = tool.is_in_category(Tool::CategoryDirect);

  tool.render_overlays(options);
  tool.tessellate_overlays_outline(outline_color, options, outline_geometry);

  Entity* hovered = InputManager::hover.element();

  if (!tool.is_in_category(Tool::CategoryImmediate) && hovered && !selection.has(hovered->id)) {
    hovered->tessellate_outline(outline_color, options, outline_geometry);
  }

  if (selection.empty()) {
    Renderer::draw(outline_geometry);
    return;
  }

  InstancedGeometry selected_vertex_geometry{};
  InstancedGeometry vertex_geometry{};
  InstancedGeometry handle_geometry{};

  if (is_direct_tool) {
    selected_vertex_geometry.push_quad(vec2{ 0.0f }, 3.5f / options.zoom, outline_color);
    vertex_geometry.push_quad(vec2{ 0.0f }, 3.5f / options.zoom, outline_color);
    vertex_geometry.push_quad(vec2{ 0.0f }, 2.0f / options.zoom, white);
    handle_geometry.push_circle(vec2{ 0.0f }, 2.5f / options.zoom, outline_color, 10);
  }

  for (const auto& [id, entity] : selection) {
    entity->tessellate_outline(outline_color, options, outline_geometry);

    const ElementEntity* element = dynamic_cast<const ElementEntity*>(entity);
    if (element != nullptr) {
      if (!is_direct_tool) continue;

      vec2 position = element->transform()->position().get();
      vertex_geometry.reserve_instances(element->vertex_count());
      handle_geometry.reserve_instances(element->vertex_count());

      for (const auto& [id, vertex] : *element) {
        vec2 vertex_position = position + vertex->transform()->position().get();
        if (element->selection()->has(id)) {
          selected_vertex_geometry.push_instance(vertex_position);
        } else {
          vertex_geometry.push_instance(vertex_position);
        }

        HandleEntity* left = vertex->left();
        HandleEntity* right = vertex->right();

        uint32_t vertex_index = outline_geometry.offset();

        if (left || right) {
          outline_geometry.push_vertex({ vertex_position, outline_color });
        }

        if (left) {
          vec2 handle_position = vertex_position + left->transform()->position().get();
          handle_geometry.push_instance(handle_position);

          outline_geometry.push_indices({ vertex_index, outline_geometry.offset() });
          outline_geometry.push_vertex({ handle_position, outline_color });
        }
        if (right) {
          vec2 handle_position = vertex_position + right->transform()->position().get();
          handle_geometry.push_instance(handle_position);

          outline_geometry.push_indices({ vertex_index, outline_geometry.offset() });
          outline_geometry.push_vertex({ handle_position, outline_color });
        }
      }
    }
  }

  Renderer::draw(outline_geometry);

  if (is_direct_tool) {
    Renderer::draw(vertex_geometry);
    Renderer::draw(selected_vertex_geometry);
    Renderer::draw(handle_geometry);
  }
}

Entity* Scene::entity_at(const vec2& position, bool lower_level, float threshold) {
  for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
    Entity* hovered = it->second->entity_at(position, lower_level, threshold);
    if (hovered) {
      return hovered;
    }
  }

  return nullptr;
}

std::vector<Entity*> Scene::entities_in(const Box& box, bool lower_level) {
  std::vector<Entity*> entities;

  for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
    it->second->entities_in(box, entities, lower_level);
  }

  return entities;
}

Entity* Scene::duplicate(const Entity* entity) {
  if (!entity) return nullptr;

  Entity* duplicate = entity->duplicate();
  if (!duplicate) return nullptr;

  std::shared_ptr<Entity> shared_duplicate{duplicate};
  m_children.insert({ shared_duplicate->id, shared_duplicate });

  return shared_duplicate.get();
}
