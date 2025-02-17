/**
 * @file hover_state.h
 * @brief Contains the declaration of the HoverState class, which stores what is currently under
 * the pointer.
 */

#pragma once

#include "../../math/vec2.h"

#include "../../utils/uuid.h"

#include <memory>
#include <optional>

namespace graphick::editor {
class Entity;
}

namespace graphick::editor::input {

/**
 * @brief The HoverState class represents the state of the pointer hover.
 *
 * This class provides methods for setting and getting the hovered object.
 */
class HoverState {
 public:
  /**
   * @brief The HoverType enum represents the type of the hovered object.
   */
  enum class HoverType { None = 0, Entity, Element, Segment, Vertex, Handle };

 public:
  /**
   * @brief Default constructor.
   */
  HoverState() = default;

  /**
   * @brief Deleted copy and move constructors.
   */
  HoverState(const HoverState &) = delete;
  HoverState(HoverState &&) = delete;

  /**
   * @brief Destructor.
   */
  ~HoverState();

  /**
   * @brief Returns the type of the hovered object.
   *
   * @return The type of the hovered object.
   */
  inline HoverType type() const
  {
    return m_type;
  }

  /**
   * @brief Returns the UUID of the hovered entity.
   *
   * @return The UUID of the hovered entity.
   */
  inline uuid entity_id() const
  {
    return m_entity;
  }

  /**
   * @brief Returns the hovered entity.
   *
   * @return The hovered entity, std::nullopt if no entity is hovered.
   */
  std::optional<Entity> entity() const;

  /**
   * @brief Returns the index of the hovered segment.
   *
   * @return The index of the hovered segment, std::nullopt if no segment is hovered.
   */
  std::optional<size_t> segment() const;

  /**
   * @brief Returns the index of the hovered vertex.
   *
   * @return The index of the hovered vertex, std::nullopt if no vertex is hovered.
   */
  std::optional<size_t> vertex() const;

  /**
   * @brief Returns the index of the hovered handle.
   *
   * @return The index of the hovered handle, std::nullopt if no handle is hovered.
   */
  std::optional<size_t> handle() const;

 private:
  /**
   * @brief Sets the hovered entity and calculates the hovered object of the given entity.
   *
   * This method should be called by the InputManager.
   *
   * @param entity The UUID of the entity to set as hovered.
   * @param position The position of the pointer.
   * @param deep_search Whether to perform a deep search (i.e. include children of the entity).
   * @param threshold The threshold for the hover.
   * @param zoom The current zoom level.
   */
  void set_hovered(const uuid entity,
                   const vec2 position,
                   const bool deep_search,
                   float threshold,
                   const double zoom);

  /**
   * @brief Resets the hover state.
   */
  void reset();

 private:
  HoverType m_type = HoverType::None;  // The type of the hovered object.

  uuid m_entity = uuid::null;          // The UUID of the hovered entity.
  int64_t m_segment = -1;  // The index of the hovered segment, -1 if no segment is hovered.
  int64_t m_vertex = -1;   // The index of the hovered vertex, -1 if no vertex is hovered.
  int64_t m_handle = -1;   // The index of the hovered handle, -1 if no handle is hovered.
 private:
  friend class InputManager;
};

}  // namespace graphick::editor::input
