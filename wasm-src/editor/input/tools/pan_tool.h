#pragma once

#include "../tool.h"

namespace Graphick::Editor::Input {

  class PanTool : public Tool {
  public:
    virtual void on_pointer_move() override;
  private:
    PanTool();
  private:
    friend class ToolState;
  };

}
