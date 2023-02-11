#pragma once

#include "../tool.h"
#include "../../../utils/console.h"

class PanTool: public Tool {
public:
  virtual void on_pointer_move() override;
private:
  PanTool();
private:
  friend class ToolState;
};
