#include "pan_tool.h"

#include "../../editor.h"
#include "../input_manager.h"

PanTool::PanTool(): Tool(ToolType::Pan, CategoryImmediate) {}

void PanTool::on_pointer_move() {
  Editor::scene().viewport.move(InputManager::pointer.scene.movement);
}
