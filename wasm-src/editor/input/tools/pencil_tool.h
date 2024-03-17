/**
 * @file pencil_tool.h
 * @brief Contains the declaration of the PencilTool class.
 *
 * @todo implement the PencilTool class.
 */

#pragma once

#include "../tool.h"

namespace Graphick::Editor::Input {

  /**
   * @brief The PencilTool class represents a tool used for drawing freehand entities.
   *
   * @class PencilTool
   */
  class PencilTool : public Tool {
  public:
    virtual void on_pointer_down() override;
    virtual void on_pointer_move() override;
    virtual void on_pointer_up() override;
  private:
    PencilTool();
  private:
    // std::shared_ptr<FreehandEntity> m_entity = nullptr;
  private:
    friend class ToolState;
  };

}
