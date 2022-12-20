#include "pan_tool.h"

#include "../../editor.h"
#include "../input_manager.h"

PanTool::PanTool(): Tool(ToolType::Pan, CategoryNone) {}

void PanTool::on_pointer_move() {
  Editor::viewport.move(InputManager::pointer.scene.movement);
}
