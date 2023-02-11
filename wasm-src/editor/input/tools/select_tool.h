#pragma once

#include "../tool.h"
#include "../../scene/entity.h"

class SelectTool: public Tool {
public:
  virtual void on_pointer_down() override;
  virtual void on_pointer_move() override;
  virtual void on_pointer_up(bool abort = false) override;
private:
  SelectTool();
private:
  bool m_dragging_occurred = false;
  bool m_is_element_added_to_selection = false;
  Entity* m_element = nullptr;
private:
  friend class ToolState;
};
