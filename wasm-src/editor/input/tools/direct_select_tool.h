/**
 * @file direct_select_tool.h
 * @brief Contains the declaration of the DirectSelectTool class.
 */

#pragma once

#include "common.h"

#include "../tool.h"

#include "../../../utils/uuid.h"

#include <memory>
#include <optional>

namespace graphick::editor::input {

/**
 * @brief The DirectSelectTool class represents a tool used for selecting and manipulating
 * entities.
 *
 * The DirectSelectTool is a multi-purpose tool that allows the user to select and manipulate
 * entities and their components (i.e. paths). The state of the direct select tool is managed
 * internally.
 */
class DirectSelectTool : public Tool {
 public:
  virtual void on_pointer_down() override;
  virtual void on_pointer_move() override;
  virtual void on_pointer_up() override;

  virtual void render_overlays(const vec4& color) const override;

 private:
  /**
   * @brief Default constructor.
   */
  DirectSelectTool();

  /**
   * @brief Translates the selected vertices and full entities.
   */
  void translate_selected();

  /**
   * @brief Creates a selection rectangle.
   */
  void on_none_pointer_down();

  /**
   * @brief Duplicates the selected entities.
   */
  void on_duplicate_pointer_down();

  /**
   * @brief Selects an entity.
   */
  void on_entity_pointer_down();

  /**
   * @brief Selects an element.
   */
  void on_element_pointer_down();

  /**
   * @brief Enters the bezier mode calculating its parameters for molding.
   */
  void on_segment_pointer_down();

  /**
   * @brief Selects a vertex.
   */
  void on_vertex_pointer_down();

  /**
   * @brief Enters the handle mode.
   */
  void on_handle_pointer_down();

  /**
   * @brief Temp selects the entities and vertices inside the selection rectangle.
   */
  void on_none_pointer_move();

  /**
   * @brief Molds or moves the bezier curve.
   *
   * A curve is molded if not already selected, otherwise it is moved.
   */
  void on_segment_pointer_move();

  /**
   * @brief Moves the handle updating its symmetric counterpart if present.
   */
  void on_handle_pointer_move();

  /**
   * @brief Selectes the temp selected entities and vertices.
   */
  void on_none_pointer_up();

  /**
   * @brief Manages the entity selction.
   */
  void on_entity_pointer_up();

  /**
   * @brief If no dragging occurred, selects the segment.
   */
  void on_segment_pointer_up();

  /**
   * @brief Manages the vertex selction.
   */
  void on_vertex_pointer_up();

  /**
   * @brief Collapses very close handles.
   */
  void on_handle_pointer_up();

 private:
  /**
   * @brief Enum class representing the different modes of the direct select tool.
   */
  enum class Mode { None = 0, Duplicate, Element, Vertex, Handle, Bezier, Entity };

 private:
  bool m_is_entity_added_to_selection =
      false;                                 // Whether the entity was just added to the selection.
  bool m_should_evaluate_selection = false;  // Whether the selection should be evaluated (i.e.
                                             // select the entity under the pointer).
  bool m_dragging_occurred = false;          // Whether a dragging occurred.

  Mode m_mode = Mode::None;                  // The current mode of the direct select tool.
  uuid m_entity = uuid::null;                // The UUID of the active entity.

  std::optional<size_t> m_segment = std::nullopt;  // The active segment.
  std::optional<size_t> m_vertex = std::nullopt;   // The active vertex.
  std::optional<size_t> m_handle = std::nullopt;   // The active handle.

  SelectionRect m_selection_rect;                  // The selection rectangle.
 private:
  friend class ToolState;
};

}  // namespace graphick::editor::input
