#pragma once

#include "../tool.h"
// #include "../../scene/entities/freehand_entity.h"

namespace Graphick::Editor::Input {

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
