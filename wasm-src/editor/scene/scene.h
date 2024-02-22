/**
 * @file scene.h
 * @brief This file contains the definition of the scene class.
 */

#pragma once

#include "viewport.h"
#include "selection.h"
#include "history/history.h"

#include "../input/tool_state.h"

#include "../../io/encode/encode.h"

#include "../../lib/entt/entt.hpp"

namespace Graphick::Renderer::Geometry {
  class Path;
}

namespace Graphick::Editor {

  class Entity;

  /**
   * @brief A scene is a collection of entities and their components.
   *
   * @class Scene
   */
  class Scene {
  public:
    const uuid id;                  /* The unique identifier of the scene. */

    Viewport viewport;              /* Manages the viewport of the scene. */
    Selection selection;            /* Manages the selection of entities. */
    History history;                /* Manages the history of the scene. */

    Input::ToolState tool_state;    /* Manages the tool state of the scene. */
  public:
    /**
     * @brief Default constructor, copy constructor and move constructor.
    */
    Scene();
    Scene(const Scene& other);
    Scene(Scene&& other) noexcept;

    /**
     * @brief Deleted copy and move assignment operators.
     */
    Scene& operator=(const Scene& other) = delete;
    Scene& operator=(Scene&& other) = delete;

    /**
     * @brief Default destructor.
     */
    ~Scene();

    /**
     * @brief Returns a view of all the entities with the specified components.
     *
     * @return A view of all the entities with the specified components.
     */
    template <typename... C>
    inline auto get_all_entities_with() { return m_registry.view<C...>(); }

    /**
     * @brief Checks if an entity exists.
     *
     * @return true if the entity exists, false otherwise.
     */
    bool has_entity(const uuid id) const;

    /**
     * @brief Returns the entity with the specified unique identifier.
     *
     * @return The entity with the specified unique identifier.
     */
    Entity get_entity(const uuid id);

    /**
     * @brief Creates a new entity.
     *
     * @param tag The tag of the entity, if empty a default tag will be generated.
     * @param generate_tag If true, a default tag will be generated.
     * @return The new entity.
     */
    Entity create_entity(const std::string& tag = "", const bool generate_tag = true);

    /**
     * @brief Creates a new element entity.
     *
     * This method automatically adds all the required components of an element entity.
     *
     * @return The new element.
     */
    Entity create_element();

    /**
     * @brief Creates a new element entity from a path.
     *
     * This method automatically adds all the required components of an element entity.
     *
     * @param path The underlying path of the element.
     * @return The new element.
     */
    Entity create_element(const Renderer::Geometry::Path& path);

    /**
     * @brief Deletes an entity.
     *
     * @param entity The entity to delete.
     */
    void delete_entity(Entity entity);

    /**
     * @brief Deletes an entity.
     *
     * @param id The unique identifier of the entity to delete.
     */
    void delete_entity(uuid id);

    /**
     * @brief Returns the uuid of the entity at the specified position if any, uuid::null otherwise.
     *
     * @param position The position to check.
     * @param deep_search If true, individual vertices and other handles will be checked.
     * @param threshold The threshold to use when hit testing.
     * @return The entity at the specified position.
     */
    uuid entity_at(const vec2 position, bool deep_search = false, float threshold = 0.0f);

    /**
     * @brief Returns the entities in the specified rectangle.
     *
     * @param rect The rectangle to check.
     * @param deep_search If true, individual vertices and other handles will be checked.
     * @return The entities in the specified rectangle.
     */
    std::unordered_map<uuid, Selection::SelectionEntry> entities_in(const Math::rect& rect, bool deep_search = false);
  private:
    /**
     * @brief Renders the scene.
     *
     * This method should only be called by the editor.
     */
    void render() const;

    /**
     * @brief Adds a new entity to the scene from encoded data.
     *
     * This method should only be called by the history manager.
     *
     * @param id The unique identifier of the entity.
     * @param encoded_data The encoded data of the entity.
     */
    void add(const uuid id, const io::EncodedData& encoded_data);

    /**
     * @brief Removes an entity from the scene.
     *
     * This method should only be called by the history manager.
     */
    void remove(const uuid id);
  private:
    entt::registry m_registry;                            /* The main entt registry of the scene. */

    std::unordered_map<uuid, entt::entity> m_entities;    /* A map of entities by their unique identifier. */
    std::vector<entt::entity> m_order;                    /* The z-order of the entities. */

    size_t m_entity_tag_number = 0;                       /* The number of unnamed entities created, used for generating tags. */
  private:
    friend class Editor;
    friend class Entity;
    friend struct Action;
  };

}
