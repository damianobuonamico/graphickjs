/**
 * @file pan_tool.cpp
 * @brief Contains the implementation of the PanTool class.
 */

#include "pan_tool.h"

#include "../input_manager.h"

#include "../../editor.h"

namespace graphick::editor::input {

  PanTool::PanTool() : Tool(ToolType::Pan, CategoryImmediate) {}

  void PanTool::on_pointer_move() {
    Editor::scene().viewport.move(InputManager::pointer.scene.movement);
  }

}
