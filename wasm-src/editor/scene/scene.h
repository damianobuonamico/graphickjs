/**
 * @file scene.h
 * @brief This file contains the definition of the Scene class.
 */

#pragma once

#include "cache.h"
#include "history/history.h"
#include "selection.h"
#include "viewport.h"

#include "../input/tool_state.h"

#include "../../io/encode/encode.h"

#include "../../lib/entt/entt.hpp"

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

 private:
  /**
   * @brief The type of hit test to perform.
   */
  enum class HitTestType {
    All,         // Both fill, stroke and outline.
    EntityOnly,  // Only the fill and stroke, no outline.
    OutlineOnly  // Only the outline (used for selected entities priority).
  };

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

  /**
   * @brief Check if the entity is at the specified position.
   *
   * @param entity The entity to check.
   * @param position The position to check.
   * @param deep_search If true, individual vertices and other handles will be checked.
   * @param threshold The threshold to use when hit testing.
   * @param fill If true, the fill of the entity will be hit tested.
   * @param stroke If true, the stroke of the entity will be hit tested.
   * @return true if the entity is at the specified position, false otherwise.
   */
  bool is_entity_at(const Entity entity,
                    const vec2 position,
                    const bool deep_search,
                    const float threshold,
                    const HitTestType hit_test_type) const;

 private:
  entt::registry m_registry;          // The main entt registry of the scene.

  std::unordered_map<uuid, entt::entity>
      m_entities;                     // A map of entities by their unique identifier.
  std::vector<entt::entity> m_order;  // The z-order of the entities.

  size_t m_entity_tag_number =
      0;                  // The number of unnamed entities created, used for generating tags.

  mutable Cache m_cache;  // The cache of the scene.
 private:
  friend class Editor;
  friend class Entity;
  friend struct Action;
};

}  // namespace graphick::editor
