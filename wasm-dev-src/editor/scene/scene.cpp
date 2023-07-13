#include "scene.h"

#include "entity.h"

#include "../../renderer/renderer.h"

#include "../../utils/console.h"

namespace Graphick::Editor {

  Scene::Scene() : selection(this) {}

  Scene::Scene(const Scene& other) : m_entities(other.m_entities), selection(this) {
    m_registry.assign(other.m_registry.data(), other.m_registry.data() + other.m_registry.size(), other.m_registry.released());
  }

  Scene::Scene(Scene&& other) noexcept :
    m_registry(std::move(other.m_registry)),
    m_entities(std::move(other.m_entities)),
    selection(this) {}

  Scene::~Scene() {}

  Entity Scene::create_entity(const std::string& tag) {
    return create_entity(uuid(), tag);
  }

  Entity Scene::create_entity(uuid id, const std::string& tag) {
    Entity entity = { m_registry.create(), this };

    entity.add_component<IDComponent>(id);
    entity.add_component<TagComponent>(tag.empty() ? "Entity" : tag);

    m_entities[id] = entity;
    return entity;
  }

  void Scene::destroy_entity(Entity entity) {
    m_entities.erase(entity.id());
    m_registry.destroy(entity);
  }

  bool Scene::has_entity(const uuid id) const {
    return m_entities.find(id) != m_entities.end();
  }

  Entity Scene::get_entity(const uuid id) {
    return { m_entities.at(id), this };
  }

  uuid Scene::entity_at(const vec2 position, bool lower_level, float threshold) {
    for (const auto& [id, entity] : m_entities) {
      if (m_registry.all_of<PathComponent, TransformComponent>(entity)) {
        const auto& path = m_registry.get<PathComponent>(entity).path;
        const auto& transform = m_registry.get<TransformComponent>(entity).get_matrix().Inverse();
        auto pos = transform.Map(position.x, position.y);
        if (path.is_inside({ (float)pos.X, (float)pos.Y }, lower_level, threshold)) return id;
      }
    }

    return { 0 };
  }

  void Scene::render() const {
    OPTICK_EVENT();

    Renderer::Renderer::begin_frame({
      viewport.size(),
      viewport.dpr(),
      viewport.position(),
      viewport.zoom(),
      vec4{1.0f, 1.0f, 1.0f, 1.0f}
      });

    auto view = m_registry.view<PathComponent, TransformComponent>();
    for (auto entity : view) {
      Renderer::Renderer::draw(view.get<PathComponent>(entity).path, view.get<TransformComponent>(entity).get_matrix());
      // Renderer::Renderer::draw_outline(view.get<PathComponent>(entity).path);
    }

    // for (auto it = m_children.rbegin(); it != m_children.rend(); it++) {

    Renderer::Renderer::end_frame();

    // for (auto entity : view) {
    //   Renderer::Renderer::draw_outline(view.get<PathComponent>(entity).path);
    // }

    for (auto id : selection.selected()) {
      if (has_entity(id)) {
        const auto& path = m_registry.get<PathComponent>(m_entities.at(id)).path;
        const auto& transform = m_registry.get<TransformComponent>(m_entities.at(id));
        Renderer::Renderer::draw_outline(path, transform.get_matrix());
      }
    }
    // Renderer::Renderer::render_frame({
    //   viewport.size(),
    //   viewport.dpr(),
    //   viewport.position(),
    //   viewport.zoom(),
    //   vec4{1.0f, 1.0f, 1.0f, 1.0f}
    //   });
  }

}
