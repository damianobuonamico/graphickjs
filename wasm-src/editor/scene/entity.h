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

namespace graphick::editor {

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
    inline T add_component(Args&&... args) {
      if (has_component<T>()) remove_component<T>();

      using Data = typename T::Data;

      Data data = Data(std::forward<Args>(args)...);
      io::EncodedData encoded_data{};

      m_scene->history.add(
        id(),
        Action::Target::Component,
        std::move(T(this, &data).encode(encoded_data))
      );

      return get_component<T>();
    }

    /**
     * @brief Gets a component from the entity.
     *
     * @return The component.
     */
    template <typename T>
    inline T get_component() {
      GK_ASSERT(has_component<T>(), "Entity does not have component!");
      return T{ this, &m_scene->m_registry.get<typename T::Data>(m_handle) };
    }
    template<>
    inline TransformComponent get_component() {
      GK_ASSERT(has_component<TransformComponent>(), "Entity does not have component!");

      if (has_component<PathComponent>()) {
        return TransformComponent{ this, &m_scene->m_registry.get<TransformComponent::Data>(m_handle), &m_scene->m_registry.get<PathComponent::Data>(m_handle) };
      } else {
        return TransformComponent{ this, &m_scene->m_registry.get<TransformComponent::Data>(m_handle) };
      }
    }

    /**
     * @brief Gets a component from the entity.
     *
     * @return The component.
     */
    template <typename T>
    inline const T get_component() const {
      GK_ASSERT(has_component<T>(), "Entity does not have component!");
      return T{ this, &m_scene->m_registry.get<typename T::Data>(m_handle) };
    }
    template<>
    inline const TransformComponent get_component() const {
      GK_ASSERT(has_component<TransformComponent>(), "Entity does not have component!");

      if (has_component<PathComponent>()) {
        return TransformComponent{ this, &m_scene->m_registry.get<TransformComponent::Data>(m_handle), &m_scene->m_registry.get<PathComponent::Data>(m_handle) };
      } else {
        return TransformComponent{ this, &m_scene->m_registry.get<TransformComponent::Data>(m_handle) };
      }
    }

    /**
     * @brief Checks if the entity has a component.
     *
     * @return true if the entity has the component, false otherwise.
     */
    template <typename T>
    inline bool has_component() const {
      return m_scene->m_registry.all_of<typename T::Data>(m_handle);
    }

    /**
     * @brief Checks if the entity has all of the specified components.
     *
     * @return true if the entity has all of the specified components, false otherwise.
     */
    template <typename... T>
    inline bool has_components() const {
      return (has_component<T>() && ...);
    }

    /**
     * @brief Removes a component from the entity.
     */
    template <typename T>
    inline void remove_component() {
      if (!has_component<T>()) return;

      io::EncodedData encoded_data{};

      m_scene->history.remove(
        id(),
        Action::Target::Component,
        get_component<T>().encode(encoded_data)
      );
    }

    /**
     * @brief Bool conversion operator.
     *
     * @return true if the entity is valid, false otherwise.
     */
    inline operator bool() const { return m_handle != entt::null; }

    /**
     * @brief Entity handle conversion operator.
     *
     * @return The entity handle.
     */
    inline operator entt::entity() const { return m_handle; }

    /**
     * @brief uint32_t conversion operator.
     *
     * @return The entity handle as a uint32_t.
     */
    inline operator uint32_t() const { return static_cast<uint32_t>(m_handle); }

    /**
     * @brief Gets the entity's ID.
     *
     * @return The entity's ID.
     */
    inline uuid id() const { return m_scene->m_registry.get<IDComponent::Data>(m_handle).id; }

    /**
     * @brief Gets the entity's tag.
     *
     * @return The entity's tag.
     */
    inline const std::string tag() const {
      if (has_component<TagComponent>()) {
        return m_scene->m_registry.get<TagComponent::Data>(m_handle).tag;
      } else {
        return "Entity " + std::to_string(static_cast<uint32_t>(m_handle));
      }
    }

    /**
     * @brief Gets the scene the entity belongs to.
     *
     * @return The scene the entity belongs to.
     */
    inline Scene* scene() const { return m_scene; }

    /**
     * @brief Equality operator.
     */
    inline bool operator==(const Entity& other) const {
      return m_handle == other.m_handle && m_scene == other.m_scene;
    }

    /**
     * @brief Inequality operator.
     */
    inline bool operator!=(const Entity& other) const {
      return !(*this == other);
    }

    /**
     * @brief Checks if the entity is in the specified category.
     *
     * @param category The category to check.
     * @return true if the entity is in the category, false otherwise.
     */
    inline bool is_in_category(CategoryComponent::Category category) const {
      if (!has_component<CategoryComponent>()) return false;
      return get_component<CategoryComponent>().is_in_category(category);
    }

    /**
     * @brief Checks if the entity is an element.
     *
     * An element is an entity that has a PathComponent.
     * All entities have a TransformComponent.
     *
     * @return true if the entity is an element, false otherwise.
     */
    inline bool is_element() const {
      return has_components<PathComponent>();
    }

    /**
     * @brief Encodes the entity in binary format.
     *
     * @return The encoded data.
     */
    io::EncodedData encode() const;

    /**
     * @brief Duplicates the entity in binary format.
     *
     * @return A pair containing the new UUID and the encoded data.
     */
    std::pair<uuid, io::EncodedData> duplicate() const;
  private:
    /**
     * @brief Adds a component to the entity.
     *
     * This method should only be called internally.
     *
     * @param args The component constructor arguments.
     * @return The added component.
     */
    template <typename T, typename... Args>
    inline T add(Args&&... args) {
      return T{ this, &m_scene->m_registry.emplace<typename T::Data>(m_handle, std::forward<Args>(args)...) };
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
    template <typename T>
    void remove() {
      m_scene->m_registry.remove<typename T::Data>(m_handle);
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
