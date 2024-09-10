/**
 * @file pan_tool.cpp
 * @brief Contains the implementation of the PanTool class.
 */

#include "pan_tool.h"

#include "../input_manager.h"

#include "../../editor.h"

namespace graphick::editor::input {

  PanTool::PanTool() : Tool(ToolType::Pan, CategoryImmediate | CategoryView) {}

  void PanTool::on_pointer_down() {
    m_start_position = Editor::scene().viewport.position();
  }

  void PanTool::on_pointer_move() {
    Editor::scene().viewport.move_to(m_start_position + InputManager::pointer.client.delta / Editor::scene().viewport.zoom());
  }

}
