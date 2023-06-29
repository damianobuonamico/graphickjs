#include "pan_tool.h"

#include "../input_manager.h"

#include "../../editor.h"

namespace Graphick::Editor::Input {

  PanTool::PanTool() : Tool(ToolType::Pan, CategoryImmediate) {}

  void PanTool::on_pointer_move() {
    Editor::scene().viewport.move(InputManager::pointer.scene.movement);
  }

}
