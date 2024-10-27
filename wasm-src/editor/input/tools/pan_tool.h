/**
 * @file pan_tool.h
 * @brief Contains the declaration of the PanTool class.
 */

#pragma once

#include "../tool.h"

#include "../../../math/vec2.h"

namespace graphick::editor::input {

/**
 * @brief The PanTool class represents a tool used for panning the viewport.
 *
 * @class PanTool
 */
class PanTool : public Tool {
public:
  virtual void on_pointer_down() override;

  virtual void on_pointer_move() override;
private:
  /**
   * @brief Default constructor.
   */
  PanTool();
private:
  vec2 m_start_position;  // The screen space position before panning.
private:
  friend class ToolState;
};

}  // namespace graphick::editor::input
