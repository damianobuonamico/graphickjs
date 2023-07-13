#pragma once

#include "viewport.h"
#include "selection.h"

#include "../input/tool_state.h"

#include "../../utils/uuid.h"

#include "../../lib/entt/entt.hpp"

namespace Graphick::Renderer::Geometry {

  class Path;

}

namespace Graphick::Editor {

  class Entity;

  class Scene {
  public:
    const uuid id;

    Viewport viewport;
    Selection selection;

    Input::ToolState tool_state;
  public:
    Scene();
    Scene(const Scene& other);
    Scene(Scene&& other) noexcept;

    Scene& operator=(const Scene& other) = delete;
    Scene& operator=(Scene&& other) = delete;

    ~Scene();

    Entity create_entity(const std::string& tag = "");
    Entity create_entity(const uuid id, const std::string& tag = "");
    void destroy_entity(Entity entity);

    template <typename... C>
    inline auto get_all_entities_with() { return m_registry.view<C...>(); }

    bool has_entity(const uuid id) const;
    Entity get_entity(const uuid id);

    uuid entity_at(const vec2 position, bool lower_level = false, float threshold = 0.0f);

    Entity create_element(const std::string& tag = "");
    Entity create_element(Renderer::Geometry::Path& path, const std::string& tag = "");
  private:
    void render() const;
  private:
    entt::registry m_registry;

    std::unordered_map<uuid, entt::entity> m_entities;
  private:
    friend class Editor;
    friend class Entity;
  };

}
