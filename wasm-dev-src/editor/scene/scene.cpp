#include "scene.h"

#include "entity.h"

#include "../../renderer/renderer.h"

#include "../../utils/console.h"

namespace Graphick::Editor {

  Scene::Scene() {}

  Scene::Scene(const Scene& other) : m_entities(other.m_entities) {
    m_registry.assign(other.m_registry.data(), other.m_registry.data() + other.m_registry.size(), other.m_registry.released());
  }

  Scene::Scene(Scene&& other) noexcept :
    m_registry(std::move(other.m_registry)),
    m_entities(std::move(other.m_entities)) {}

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

  void Scene::render() const {
    OPTICK_EVENT();

    Renderer::Renderer::begin_frame({
      viewport.size(),
      viewport.dpr(),
      viewport.position(),
      viewport.zoom(),
      vec4{1.0f, 1.0f, 1.0f, 1.0f}
      });

    auto view = m_registry.view<PathComponent>();
    for (auto entity : view) {
      // Renderer::Renderer::draw_outline(view.get<PathComponent>(entity).path);
      Renderer::Renderer::draw(view.get<PathComponent>(entity).path);
    }
    // for (auto it = m_children.rbegin(); it != m_children.rend(); it++) {

    Renderer::Renderer::end_frame();

    // Renderer::Renderer::render_frame({
    //   viewport.size(),
    //   viewport.dpr(),
    //   viewport.position(),
    //   viewport.zoom(),
    //   vec4{1.0f, 1.0f, 1.0f, 1.0f}
    //   });
  }

}
