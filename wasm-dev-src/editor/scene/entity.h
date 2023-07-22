#pragma once

#include "scene.h"
#include "components.h"

#include "../../utils/console.h"
#include "../../lib/entt/entt.hpp"

namespace Graphick::Editor {

  class Entity {
  public:
    Entity() = default;
    Entity(entt::entity handle, Scene* scene) : m_handle(handle), m_scene(scene) {}
    Entity(const Entity& other) = default;

    template <typename T, typename... Args>
    T& add_component(Args&&... args) {
      GK_ASSERT(!has_component<T>(), "Entity already has component!");
      return m_scene->m_registry.emplace<T>(m_handle, std::forward<Args>(args)...);
    }

    template <typename T, typename... Args>
    T& add_or_replace_component(Args&&... args) {
      return m_scene->m_registry.emplace_or_replace<T>(m_handle, std::forward<Args>(args)...);
    }

    template<typename T>
    T& get_component() {
      GK_ASSERT(has_component<T>(), "Entity does not have component!");
      return m_scene->m_registry.get<T>(m_handle);
    }

    template<typename T>
    const T& get_component() const {
      GK_ASSERT(has_component<T>(), "Entity does not have component!");
      return m_scene->m_registry.get<T>(m_handle);
    }

    template<typename T>
    bool has_component() const {
      return m_scene->m_registry.all_of<T>(m_handle);
    }

    template<typename T>
    void remove_component() {
      GK_ASSERT(has_component<T>(), "Entity does not have component!");
      m_scene->m_registry.remove<T>(m_handle);
    }

    operator bool() const { return m_handle != entt::null; }
    operator entt::entity() const { return m_handle; }
    operator uint32_t() const { return (uint32_t)m_handle; }

    uuid id() const { return get_component<IDComponent>().id; }
    const std::string& tag() const { return get_component<TagComponent>().tag; }

    bool operator==(const Entity& other) const {
      return m_handle == other.m_handle && m_scene == other.m_scene;
    }

    bool operator!=(const Entity& other) const {
      return !(*this == other);
    }
  private:
    entt::entity m_handle;
    Scene* m_scene;
  };

}
