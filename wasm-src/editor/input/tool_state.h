#pragma once

#include "tool.h"
#include "tools/pan_tool.h"
#include "tools/zoom_tool.h"
#include "tools/select_tool.h"
#include "tools/direct_select_tool.h"
#include "tools/pencil_tool.h"

class ToolState {
public:
  ToolState();

  ToolState(const ToolState&) = delete;
  ToolState(ToolState&&) = delete;

  ~ToolState();

  inline Tool& current() const { return *m_tools[(int)m_current]; }
  inline Tool& active() const { return *m_tools[(int)m_active]; }

  void set_current(Tool::ToolType tool);
  void set_active(Tool::ToolType tool);

  void on_pointer_down();
  void on_pointer_move();
  void on_pointer_up();

  void on_pointer_hover();

  void recalculate_active();
private:
  Tool* m_tools[static_cast<int>(Tool::ToolType::None)] = {
    new PanTool(),
    new ZoomTool(),
    new SelectTool(),
    new DirectSelectTool(),
    new PencilTool(),
  };

  Tool::ToolType m_current;
  Tool::ToolType m_active;
  Tool::ToolType m_last_tool;
};
