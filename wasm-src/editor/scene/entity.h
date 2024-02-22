/**
 * @file entity.h
 * @brief Entity class definition
 *
 * @todo merge old and new components if one is already present when adding a new one
 */

#pragma once

#include "scene.h"
#include "components.h"

#include "../../utils/assert.h"

#include <vector>

namespace Graphick::Editor {

  /**
   * @brief The Entity class represents an entity in the scene.
   *
   * An entity is a collection of components that define its properties and behavior.
   *
   * @class Entity
   */
  class Entity {
  public:
    /**
     * @brief Default constructor
     */
    Entity() = default;

    /**
     * @brief Constructs an entity with a handle and a scene pointer.
     *
     * @param handle The entity handle.
     * @param scene The scene the entity belongs to.
     */
    Entity(entt::entity handle, Scene* scene) : m_handle(handle), m_scene(scene) {}

    /**
     * @brief Constructs an entity with a handle, a scene pointer, and encoded data.
     *
     * @param handle The entity handle.
     * @param scene The scene the entity belongs to.
     * @param encoded_data The encoded data to get the entity's components from.
     */
    Entity(entt::entity handle, Scene* scene, const io::EncodedData& encoded_data);

    /**
     * @brief Copy constructor
     */
    Entity(const Entity& other) = default;


    /**
     * @brief Adds a component to the entity.
     *
     * If the entity already has the component, it will be replaced.
     *
     * @param args The component constructor arguments.
     * @return The added component.
     */
    template <typename T, typename... Args>
    T& add_component(Args&&... args) {
      if (has_component<T>()) remove_component<T>();

      m_scene->history.add(
        id(),
        Action::Target::Component,
        T(std::forward<Args>(args)...).encode(io::EncodedData())
      );

      return get_component<T>();
    }

    /**
     * @brief Gets a component from the entity.
     *
     * @return The component.
     */
    template<typename T>
    T& get_component() {
      GK_ASSERT(has_component<T>(), "Entity does not have component!");
      return m_scene->m_registry.get<T>(m_handle);
    }

    /**
     * @brief Gets a component from the entity.
     *
     * @return The component.
     */
    template<typename T>
    const T& get_component() const {
      GK_ASSERT(has_component<T>(), "Entity does not have component!");
      return m_scene->m_registry.get<T>(m_handle);
    }

    /**
     * @brief Checks if the entity has a component.
     *
     * @return true if the entity has the component, false otherwise.
     */
    template<typename T>
    bool has_component() const {
      return m_scene->m_registry.all_of<T>(m_handle);
    }

    /**
     * @brief Checks if the entity has all of the specified components.
     *
     * @return true if the entity has all of the specified components, false otherwise.
     */
    template<typename... T>
    bool has_components() const {
      return (has_component<T>() && ...);
    }

    /**
     * @brief Removes a component from the entity.
     */
    template<typename T>
    void remove_component() {
      if (!has_component<T>()) return;

      m_scene->history.remove(
        id(),
        Action::Target::Component,
        get_component<T>().encode(io::EncodedData())
      );
    }

    /**
     * @brief Bool conversion operator.
     *
     * @return true if the entity is valid, false otherwise.
     */
    operator bool() const { return m_handle != entt::null; }

    /**
     * @brief Entity handle conversion operator.
     *
     * @return The entity handle.
     */
    operator entt::entity() const { return m_handle; }

    /**
     * @brief uint32_t conversion operator.
     *
     * @return The entity handle as a uint32_t.
     */
    operator uint32_t() const { return (uint32_t)m_handle; }

    /**
     * @brief Gets the entity's ID.
     *
     * @return The entity's ID.
     */
    uuid id() const { return get_component<IDComponent>().id; }

    /**
     * @brief Gets the entity's tag.
     *
     * @return The entity's tag.
     */
    const std::string& tag() const { return get_component<TagComponent>().tag; }

    /**
     * @brief Equality operator.
     */
    bool operator==(const Entity& other) const {
      return m_handle == other.m_handle && m_scene == other.m_scene;
    }

    /**
     * @brief Inequality operator.
     */
    bool operator!=(const Entity& other) const {
      return !(*this == other);
    }

    /**
     * @brief Checks if the entity is in the specified category.
     *
     * @param category The category to check.
     * @return true if the entity is in the category, false otherwise.
     */
    bool is_in_category(CategoryComponent::Category category) const {
      if (!has_component<CategoryComponent>()) return false;
      return get_component<CategoryComponent>().category & category;
    }

    /**
     * @brief Checks if the entity is an element.
     *
     * An element is an entity that has a PathComponent and a TransformComponent.
     *
     * @return true if the entity is an element, false otherwise.
     */
    bool is_element() const {
      return has_components<PathComponent, TransformComponent>();
    }

    /**
     * @brief Encodes the entity in binary format.
     *
     * @return The encoded data.
     */
    io::EncodedData encode() const;
  private:
    /**
     * @brief Adds a component to the entity.
     *
     * This method should only be called internally.
     *
     * @param args The component constructor arguments.
     * @return The added component.
     */
    template<typename T, typename... Args>
    T& add(Args&&... args) {
      return m_scene->m_registry.emplace<T>(m_handle, std::forward<Args>(args)...);
    }

    /**
     * @brief Adds a component to the entity from encoded data.
     *
     * This method should only be called by the history manager.
     *
     * @param encoded_data The encoded data of the component.
     * @param full_entity If true, default components will be added if they are missing.
     */
    void add(const io::EncodedData& encoded_data, const bool full_entity = false);

    /**
     * @brief Removes a component from the entity.
     *
     * This method should only be called internally.
     */
    template<typename T>
    void remove() {
      m_scene->m_registry.remove<T>(m_handle);
    }

    /**
     * @brief Removes a component from the entity.
     *
     * This method should only be called by the history manager.
     *
     * @param encoded_data The encoded data of the component.
     */
    void remove(const io::EncodedData& encoded_data);

    /**
     * @brief Modifies a component of the entity.
     *
     * This method should only be called internally.
     *
     * @param encoded_data A diff of the modified component's data.
     */
    void modify(const io::EncodedData& encoded_data);
  private:
    entt::entity m_handle;    /* The entt entity handle. */
    Scene* m_scene;           /* A pointer to the scene this entity belongs to. */
  private:
    friend struct Action;
  };

}
