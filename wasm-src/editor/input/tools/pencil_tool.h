#pragma once

#include "../tool.h"
#include "../../../utils/console.h"
#include "../../scene/entities/freehand_entity.h"
#include "../../../math/models/path_point.h"

class PencilTool: public Tool {
public:
  virtual void on_pointer_down() override;
  virtual void on_pointer_move() override;
  virtual void on_pointer_up(bool abort = false) override;
private:
  PencilTool();
private:
  std::shared_ptr<FreehandEntity> m_entity = nullptr;
private:
  friend class ToolState;
};
