#include "scene.h"

#include "entity.h"

#include "../input/input_manager.h"

#include "../../renderer/renderer.h"

// TEMP
#include "../../math/math.h"

#include "../../utils/misc.h"
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

  uuid Scene::entity_at(const vec2 position, bool deep_search, float threshold) {
    for (auto it = m_order.rbegin(); it != m_order.rend(); it++) {
      if (m_registry.all_of<PathComponent, TransformComponent>(*it)) {
        const auto& path = m_registry.get<PathComponent>(*it).path;
        const vec2 translation = m_registry.get<TransformComponent>(*it).position.get();
        // auto pos = transform.Map(position.x, position.y);
        // if (path.is_inside({ (float)pos.X, (float)pos.Y }, deep_search, threshold)) return id;
        if (path.is_inside(position - translation, deep_search, threshold)) {
          return m_registry.get<IDComponent>(*it).id;
        }
      }
    }

    return { 0 };
  }

  std::unordered_map<uuid, Selection::SelectionEntry> Scene::entities_in(const Math::rect& rect, bool deep_search) {
    std::unordered_map<uuid, Selection::SelectionEntry> entities;
    std::unordered_set<uuid> vertices;

    auto view = get_all_entities_with<IDComponent, PathComponent, TransformComponent>();

    for (entt::entity entity : view) {
      auto components = view.get<IDComponent, PathComponent, TransformComponent>(entity);

      const uuid id = std::get<0>(components).id;
      const Renderer::Geometry::Path& path = std::get<1>(components).path;
      const vec2 position = std::get<2>(components).position.get();

      if (deep_search) {
        vertices.clear();

        if (path.intersects(rect - position, vertices)) {
          entities.insert({ id, Selection::SelectionElementEntry{ vertices } });
        }
      } else {
        if (path.intersects(rect - position)) {
          entities.insert({ id, Selection::SelectionElementEntry{ { 0 } } });
        }
      }
    }

    return entities;
  }

  Entity Scene::create_element(const std::string& tag) {
    Renderer::Geometry::Path path;
    return create_element(path, tag);
  }

  Entity Scene::create_element(Renderer::Geometry::Path& path, const std::string& tag) {
    Entity entity = create_entity(tag);

    entity.add_component<CategoryComponent>(CategoryComponent::Selectable);
    entity.add_component<PathComponent>(path);
    entity.add_component<TransformComponent>();

    return entity;
  }

  void Scene::render() const {
    GK_AVERAGE("render");
    OPTICK_EVENT();

    Renderer::Renderer::begin_frame({
      Math::round(IVEC2_TO_VEC2(viewport.size()) * viewport.dpr()),
      viewport.position(),
      viewport.zoom() * viewport.dpr(),
      vec4{1.0f, 1.0f, 1.0f, 1.0f}
      });

    float z_index = 0.0f;
    size_t z_far = m_order.size();

    {
      OPTICK_EVENT("Render Entities");

      auto& selected = selection.selected();
      auto& temp_selected = selection.temp_selected();

      for (auto it = m_order.rbegin(); it != m_order.rend(); it++) {
        if (!m_registry.all_of<IDComponent, PathComponent, TransformComponent>(*it)) continue;

        auto components = m_registry.get<IDComponent, PathComponent, TransformComponent>(*it);

        const uuid id = std::get<0>(components).id;
        const Renderer::Geometry::Path& path = std::get<1>(components).path;
        const vec2 position = std::get<2>(components).position.get();

        if (!has_entity(id)) return;

        if (m_registry.all_of<FillComponent>(*it)) {
          Renderer::Renderer::draw(path, (z_far - z_index++) / z_far, position, m_registry.get<FillComponent>(*it).color);
        } else {
          Renderer::Renderer::draw(path, (z_far - z_index++) / z_far, position);
        }

        if (selected.find(id) != selected.end() || temp_selected.find(id) != temp_selected.end()) {
          Renderer::Renderer::draw_outline(id, path, position);
        }
      }
    }

    {
      OPTICK_EVENT("Render Overlays");

      tool_state.active().render_overlays();
    }

    Renderer::Renderer::end_frame();
  }

}
