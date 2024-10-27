/**
 * @file selection.h
 * @brief Contains the selection manager of a scene.
 */

#pragma once

#include "../../math/rect.h"

#include "../../utils/uuid.h"

#include <unordered_map>
#include <unordered_set>

namespace graphick::editor {

class Scene;

/**
 * @brief The selection manager of a scene.
 *
 * Each scene has a selection manager that keeps track of the selected entities and children (e.g.
 * vertices of an element).
 */
class Selection {
 public:
  /**
   * @brief The entry of a selection.
   */
  struct SelectionEntry {
    /**
     * @brief The type of the selection entry.
     */
    enum class Type {
      Entity = 0,  // An entity selection entry does not have any children.
      Element      // An element selection entry has children (e.g. vertices).
    };

    std::unordered_set<uint32_t> indices;  // The indices of the children.
    Type type;                             // The type of the selection entry.

    /**
     * @brief Constructs a selection entry.
     *
     * @param type The type of the selection entry.
     */
    SelectionEntry(const Type type = Type::Entity) : type(type) {}

    /**
     * @brief Constructs a selection entry.
     *
     * @param type The type of the selection entry.
     * @param indices The indices of the children.
     */
    SelectionEntry(std::unordered_set<uint32_t> indices, const Type type = Type::Element)
        : indices(indices), type(type)
    {
    }

    /**
     * @brief Checks if the selection entry is empty.
     *
     * An enitity selection entry cannot be empty.
     *
     * @return true if the selection entry is empty, false otherwise.
     */
    inline bool empty() const
    {
      return type == Type::Element && indices.empty();
    }

    /**
     * @brief Checks if the selection entry is full.
     *
     * An entity selection entry is always full. If all the children of an element are selected,
     * the element's selection is treated as an entity selection.
     *
     * @return true if the selection entry is full, false otherwise.
     */
    inline bool full() const
    {
      return type == Type::Entity;
    }
  };

 public:
  /**
   * @brief Constructs a selection manager.
   *
   * @param scene The scene that the selection manager belongs to.
   */
  Selection(Scene *scene);

  /**
   * @brief Returns the selected entities and children.
   *
   * @return The selected entities and children.
   */
  inline const std::unordered_map<uuid, SelectionEntry> &selected() const
  {
    return m_selected;
  }

  /**
   * @brief Returns the temporary selected entities and children.
   *
   * @return The temporary selected entities and children.
   */
  inline const std::unordered_map<uuid, SelectionEntry> &temp_selected() const
  {
    return m_temp_selected;
  }

  /**
   * @brief Returns the number of selected entities.
   *
   * Temporarily selected entities are not counted.
   *
   * @return The number of selected entities.
   */
  inline size_t size() const
  {
    return m_selected.size();
  }

  /**
   * @brief Checks if the selection manager is empty.
   *
   * Temporarily selected entities are not counted.
   *
   * @return true if the selection manager is empty, false otherwise.
   */
  inline bool empty() const
  {
    return m_selected.empty();
  }

  /**
   * @brief Returns the selection entry of an entity.
   *
   * @param id The ID of the entity.
   * @return The selection entry of the entity.
   */
  inline const SelectionEntry &get(const uuid id) const
  {
    return m_selected.at(id);
  }

  /**
   * @brief Calculates the bounding rectangle of the selected entities.
   *
   * @return The bounding rectangle of the selected entities.
   */
  rect bounding_rect() const;

  /**
   * @brief Checks if an entity is selected.
   *
   * @param id The ID of the entity.
   * @param include_temp Whether to include temporarily selected entities, default is false.
   * @return true if the entity is selected, false otherwise.
   */
  bool has(const uuid id, bool include_temp = false) const;

  /**
   * @brief Checks if a child of an element is selected.
   *
   * @param element_id The ID of the element the child belongs to.
   * @param child_index The index of the child within the element.
   * @param include_temp Whether to include temporarily selected entities, default is false.
   * @return true if the child is selected, false otherwise.
   */
  bool has_child(const uuid element_id,
                 const uint32_t child_index,
                 bool include_temp = false) const;

  /**
   * @brief Clears the selection.
   *
   * This method also clears the temporarily selected elements.
   */
  void clear();

  /**
   * @brief Selects the entity with the given ID.
   *
   * @param id The ID of the entity to select.
   */
  void select(const uuid id);

  /**
   * @brief Selects the ith child of the element with the given ID.
   *
   * @param id The ID of the element the child belongs to.
   * @param child_index The index of the child within the element.
   */
  void select_child(const uuid element_id, const uint32_t child_index);

  /**
   * @brief Deselects the entity with the given ID.
   *
   * @param id The ID of the entity to deselect.
   */
  void deselect(const uuid id);

  /**
   * @brief Deselects the ith child of the element with the given ID.
   *
   * @param id The ID of the element the child belongs to.
   * @param child_index The index of the child within the element.
   */
  void deselect_child(const uuid element_id, const uint32_t child_index);

  /**
   * @brief Adds all of the provided selection entries to the temporarily selected ones.
   *
   * @param entities A std::unordered_map<uuid, SelectionEntry> of selection entries.
   */
  void temp_select(const std::unordered_map<uuid, SelectionEntry> &entities);

  /**
   * @brief Selects all of the temporarily selected entities.
   */
  void sync();

 private:
  std::unordered_map<uuid, SelectionEntry> m_selected;       // The selected entities.
  std::unordered_map<uuid, SelectionEntry> m_temp_selected;  // The temporarily selected entities.

  Scene *m_scene;  // A pointer to the scene the selection manager belongs to.
};

}  // namespace graphick::editor
