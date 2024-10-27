/**
 * @file zoom_tool.h
 * @brief Contains the declaration of the ZoomTool class.
 */

#pragma once

#include "../tool.h"

namespace graphick::editor::input {

/**
 * @brief The ZoomTool class represents a tool used for zooming the viewport.
 *
 * @class ZoomTool
 */
class ZoomTool : public Tool {
public:
  virtual void on_pointer_move() override;
private:
  /**
   * @brief Default constructor.
   */
  ZoomTool();
private:
  friend class ToolState;
};

}  // namespace graphick::editor::input
