/**
 * @file tool_state.h
 * @brief Contains the ToolState class, which is used to manage the state of the input tools in the Graphick editor.
 */

#pragma once

#include "tool.h"

#include "tools/common.h"

namespace graphick::editor::input {

class PenTool;

/**
 * @brief A class used to manage the state of the input tools in the Graphick editor.
 *
 * This class is used to manage the state of the input tools in the Graphick editor.
 * It is used to keep track of and update the current tool, the active tool, and the last tool.
 *
 * @class ToolState
 */
class ToolState {
public:
  Manipulator manipulator; /* The manipulator object */
public:
  /**
   * @brief Constructs a ToolState object.
   */
  ToolState();

  /**
   * @brief Copy constructor for ToolState.
   *
   * @param other The ToolState object to copy.
   */
  ToolState(const ToolState&) = delete;

  /**
   * @brief Move constructor for ToolState.
   *
   * @param other The ToolState object to move from.
   */
  ToolState(ToolState&&) = delete;

  /**
   * @brief Custom destructor for ToolState.
   */
  ~ToolState();

  /**
   * @brief Returns the current tool.
   *
   * @return A reference to the current tool.
   */
  inline Tool& current() const { return *m_tools[(int)m_current]; }

  /**
   * @brief Returns the active tool.
   *
   * @return A reference to the active tool.
   */
  inline Tool& active() const { return *m_tools[(int)m_active]; }

  /**
   * @brief Returns a pointer to the pen tool if in use.
   *
   * @return A pointer to the pen tool if it is either the current or active tool, otherwise nullptr.
   */
  PenTool* pen() const;

  /**
   * @brief Resets the current and active tools' state.
   */
  void reset_tool();

  /**
   * @brief Sets the current tool.
   *
   * @param tool The tool to set as the current tool.
   */
  void set_current(Tool::ToolType tool);

  /**
   * @brief Sets the active tool.
   *
   * @param tool The tool to set as the active tool.
   */
  void set_active(Tool::ToolType tool);

  /**
   * @brief Handles the pointer down event.
   *
   * This function dispatches the pointer down event to the active tool.
   *
   * @param zoom The current zoom level, used for thresholding.
   */
  void on_pointer_down(const float zoom);

  /**
   * @brief Handles the pointer move event.
   *
   * This function dispatches the pointer move event to the active tool.
   */
  void on_pointer_move();

  /**
   * @brief Handles the pointer up event.
   *
   * This function dispatches the pointer up event to the active tool.
   */
  void on_pointer_up();

  /**
   * @brief Handles the pointer hover event.
   *
   * This function dispatches the pointer hover event to the active tool.
   */
  void on_pointer_hover();

  /**
   * @brief Handles the key event.
   *
   * This function dispatches the key event to the active tool.
   *
   * @param down A boolean value indicating whether the key was pressed or released.
   * @param key The key that was pressed or released.
   */
  void on_key(const bool down, const KeyboardKey key);

  /**
   * @brief Recalculates the active tool.
   *
   * This function recalculates the active tool based on the current tool, the pointer state, the hover state, and the key state.
   */
  void recalculate_active();

  /**
   * @brief Renders the overlays for the current tool.
   *
   * This function queues to the renderer the overlays for the current tool and the manipulator (if active).
   */
  void render_overlays(const float zoom) const;
private:
  Tool* m_tools[static_cast<int>(Tool::ToolType::None)];  // An array of pointers to the tools.

  Tool::ToolType m_current;                               // The current tool.
  Tool::ToolType m_active;                                // The active tool.
  Tool::ToolType m_last_tool;                             // The last tool.
};

}  // namespace graphick::editor::input
