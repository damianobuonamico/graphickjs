#include "tool_state.h"

#include "input_manager.h"

#ifdef EMSCRIPTEN
#include <emscripten.h>

#ifndef __INTELLISENSE__
EM_JS(void, update_tool_ui, (int type), {
  window._set_tool(type);
  });
#else 
void update_tool_ui(int type) {}
#endif

#else
void update_tool_ui(int type) {}
#endif

ToolState::ToolState()
  :m_active(Tool::ToolType::Pencil), m_current(Tool::ToolType::Pencil), m_last_tool(m_current) {}

ToolState::~ToolState() {
  for (auto tool : m_tools) {
    delete tool;
  }
}

void ToolState::set_current(Tool::ToolType tool) {
  if (m_active == Tool::ToolType::None) return;

  m_current = tool;

  recalculate_active();
}

void ToolState::set_active(Tool::ToolType tool) {
  if (m_active == Tool::ToolType::None) return;

  m_last_tool = m_active;
  m_active = tool;

  update_tool_ui(static_cast<int>(tool));
}

void ToolState::on_pointer_down() {
  active().on_pointer_down();
}

void ToolState::on_pointer_move() {
  active().on_pointer_move();
}

void ToolState::on_pointer_up() {
  active().on_pointer_up();

}

void ToolState::on_pointer_hover() {
  active().on_pointer_hover();
}

void ToolState::recalculate_active() {
  if (InputManager::keys.space) {
    if (InputManager::keys.ctrl) {
      set_active(Tool::ToolType::Zoom);
    } else {
      set_active(Tool::ToolType::Pan);
    }
  } else {
    set_active(m_current);
  }
}