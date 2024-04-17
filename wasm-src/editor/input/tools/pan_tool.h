/**
 * @file pan_tool.h
 * @brief Contains the declaration of the PanTool class.
 */

#pragma once

#include "../tool.h"

namespace graphick::editor::input {

  /**
   * @brief The PanTool class represents a tool used for panning the viewport.
   *
   * @class PanTool
   */
  class PanTool : public Tool {
  public:
    virtual void on_pointer_move() override;
  private:
    /**
     * @brief Default constructor.
     */
    PanTool();
  private:
    friend class ToolState;
  };

}
