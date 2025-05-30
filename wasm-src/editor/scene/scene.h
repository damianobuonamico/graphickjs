/**
 * @file scene.h
 * @brief This file contains the definition of the Scene class.
 */

#pragma once

#include "cache.h"
#include "hierarchy.h"
#include "history/history.h"
#include "selection.h"
#include "viewport.h"

#include "../input/tool_state.h"

#include "../../io/encode/encode.h"
#include "../../lib/entt/entt.hpp"
#include "../../math/mat2x3.h"

#include <functional>

namespace graphick::editor {

class Entity;

/**
 * @brief A scene is a collection of entities and their components.
 */
class Scene {
 public:
  const uuid id;                // The unique identifier of the scene.

  Viewport viewport;            // Manages the viewport of the scene.
  Selection selection;          // Manages the selection of entities.
  History history;              // Manages the history of the scene.

  input::ToolState tool_state;  // Manages the tool state of the scene.
 public:
  struct ForEachOptions {
    bool reverse = false;
    bool callback_on_layers = false;
    bool callback_on_groups = false;
    bool layers_in_hierarchy = false;
    bool break_and_return = false;

    ForEachOptions() noexcept {}

    ForEachOptions(const bool reverse,
                   const bool callback_on_layers,
                   const bool callback_on_groups,
                   const bool layers_in_hierarchy,
                   const bool break_and_return) noexcept
        : reverse(reverse),
          callback_on_layers(callback_on_layers),
          callback_on_groups(callback_on_groups),
          layers_in_hierarchy(layers_in_hierarchy),
          break_and_return(break_and_return)
    {
    }
  };

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
  template<typename... C>
  inline auto get_all_entities_with()
  {
    return m_registry.view<C...>();
  }

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
   * @brief Returns the entity with the specified unique identifier.
   *
   * @return The entity with the specified unique identifier.
   */
  const Entity get_entity(const uuid id) const;

  /**
   * @brief Returns the active layer.
   *
   * @return The active layer.
   */
  Entity get_active_layer();

  /**
   * @brief Returns the active layer.
   *
   * @return The active layer.
   */
  const Entity get_active_layer() const;

  /**
   * @brief Returns the background entity.
   *
   * @return The background entity.
   */
  Entity get_background();

  /**
   * @brief Returns the background entity.
   *
   * @return The background entity.
   */
  const Entity get_background() const;

  /**
   * @brief Returns the hierarchy of the specified entity.
   *
   * @param entity_id The unique identifier of the entity to get the hierarchy of.
   * @param layers_in_hierarchy If true, layers will be included in the hierarchy.
   * @return The hierarchy of the specified entity.
   */
  Hierarchy get_hierarchy(const uuid entity_id, const bool layers_in_hierarchy = true) const;

  /**
   * @brief Iterates over all the entities in the scene.
   *
   * @param entity_callback The callback to call for each entity.
   */
  void for_each(std::function<void(const Entity, const Hierarchy&)> entity_callback,
                const ForEachOptions& options = {}) const;

  /**
   * @brief Iterates over all the entities in the scene
   *
   * @param entity_callback The callback to call for each entity.
   */
  void for_each(std::function<void(Entity, const Hierarchy&)> entity_callback,
                const ForEachOptions& options = {});

  /**
   * @brief Adds a new layer to the scene.
   *
   * This method automatically adds all the required components of a group entity.
   *
   * @return The new layer.
   */
  Entity create_layer();

  /**
   * @brief Creates a new group entity.
   *
   * This method automatically adds all the required components of a group entity.
   *
   * @param entities The entities to add to the group.
   * @return The new group.
   */
  Entity create_group(const std::vector<entt::entity>& entities = {});

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
  Entity create_element(const geom::path& path);

  /**
   * @brief Creates a new image entity.
   *
   * This method automatically adds all the required components of an image entity.
   *
   * @param image_id The UUID of the image.
   * @return The new image.
   */
  Entity create_image(const uuid image_id);

  /**
   * @brief Creates a new text entity.
   *
   * This method automatically adds all the required components of a text entity.
   *
   * @param text The text of the entity.
   * @param font_id The UUID of the font.
   * @return The new text.
   */
  Entity create_text(const std::string& text, const uuid font_id);

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
   * @brief Duplicates an entity.
   *
   * @param id The id of the entity to duplicate.
   * @return The duplicated entity.
   */
  Entity duplicate_entity(const uuid id);

  /**
   * @brief Returns the uuid of the entity at the specified position if any, uuid::null otherwise.
   *
   * The entities are traversed front to back, giving priority to the selected ones.
   *
   * @param position The position to check.
   * @param deep_search If true, individual vertices and other handles will be checked.
   * @param threshold The threshold to use when hit testing.
   * @return The entity at the specified position.
   */
  uuid entity_at(const vec2 position,
                 const bool deep_search = false,
                 const float threshold = 0.0f) const;

  /**
   * @brief Returns the entities in the specified rectangle.
   *
   * @param rect The rectangle to check.
   * @param deep_search If true, individual vertices and other handles will be checked.
   * @return The entities in the specified rectangle.
   */
  std::unordered_map<uuid, Selection::SelectionEntry> entities_in(const math::rect& rect,
                                                                  bool deep_search = false);

  /**
   * @brief Groups the selected entities.
   */
  void group_selected();

  /**
   * @brief Ungroups the selected entities.
   */
  void ungroup_selected();

 private:
  /**
   * @brief Renders the scene.
   *
   * This method should only be called by the editor.
   *
   * @param ignore_cache If true, the entire scene will be redrawn, ignoring the cache.
   */
  void render(const bool ignore_cache) const;

  /**
   * @brief Creates a new entity with the specified unique identifier.
   *
   * This method should only be called internally to create elements, images, text, ecc.
   *
   * @param id The unique identifier of the entity.
   * @param tag_type The display name of the entity type.
   */
  Entity create_entity(const uuid id, const std::string& tag_type);

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
  entt::registry m_registry;                          // The main entt registry of the scene.
  entt::entity m_background;                          // The background entity of the scene.

  std::unordered_map<uuid, entt::entity> m_entities;  // Entities by their unique identifier.
  std::vector<entt::entity> m_layers;                 // The layers of the scene.

  size_t m_active_layer = 0;                          // The index of the active layer.
  size_t m_layer_tag_number = 0;                      // Number of unnamed layers (for tags).
  size_t m_entity_tag_number = 0;                     // Number of unnamed entities (for tags).

  mutable Cache m_cache;                              // The cache of the scene.
 private:
  friend class Editor;
  friend class Entity;
  friend class History;
  friend struct Action;
};

}  // namespace graphick::editor
