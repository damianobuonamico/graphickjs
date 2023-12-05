#pragma once

#include "../tool.h"

namespace Graphick::Editor::Input {

  class ZoomTool : public Tool {
  public:
    virtual void on_pointer_move() override;
  private:
    ZoomTool();
  private:
    friend class ToolState;
  };

}
