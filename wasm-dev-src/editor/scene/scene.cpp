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
    m_order.push_back(entity);

    return entity;
  }

  void Scene::destroy_entity(Entity entity) {
    // TODO: implement z-ordering
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
    for (auto it = m_order.rbegin(); it != m_order.rend(); it++) {
      if (m_registry.all_of<PathComponent, TransformComponent>(*it)) {
        const auto& path = m_registry.get<PathComponent>(*it).path;
        // const auto& transform = m_registry.get<TransformComponent>(entity).get_matrix().Inverse();
        // auto pos = transform.Map(position.x, position.y);
        // if (path.is_inside({ (float)pos.X, (float)pos.Y }, lower_level, threshold)) return id;
        if (path.is_inside(position, lower_level, threshold)) {
          return m_registry.get<IDComponent>(*it).id;
        }
      }
    }

    return { 0 };
  }

  Entity Scene::create_element(const std::string& tag) {
    Renderer::Geometry::Path path;
    return create_element(path, tag);
  }

  Entity Scene::create_element(Renderer::Geometry::Path& path, const std::string& tag) {
    Entity entity = create_entity(tag);

    entity.add_component<PathComponent>(path);
    entity.add_component<TransformComponent>();

    return entity;
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

    {
      OPTICK_EVENT("Render Entities");

      for (auto it = m_order.rbegin(); it != m_order.rend(); it++) {
        if (!m_registry.all_of<PathComponent, TransformComponent>(*it)) continue;

        if (m_registry.all_of<FillComponent>(*it)) {
          Renderer::Renderer::draw(m_registry.get<PathComponent>(*it).path, m_registry.get<FillComponent>(*it).color);
        } else {
          Renderer::Renderer::draw(m_registry.get<PathComponent>(*it).path);
        }
      }
    }

    {
      OPTICK_EVENT("Render Outlines");

      for (auto id : selection.selected()) {
        if (!has_entity(id)) continue;

        entt::entity entity = m_entities.at(id);

        if (!m_registry.all_of<PathComponent, TransformComponent>(entity)) continue;

        const auto& path = m_registry.get<PathComponent>(entity).path;
        const auto& transform = m_registry.get<TransformComponent>(entity);

        Renderer::Renderer::draw_outline(path);
      }
    }

    Renderer::Renderer::end_frame();
  }

}
