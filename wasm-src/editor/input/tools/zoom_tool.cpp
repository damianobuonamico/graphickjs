#include "zoom_tool.h"

#include "../../editor.h"
#include "../input_manager.h"
#include "../../../utils/defines.h"

#include <cmath>

ZoomTool::ZoomTool(): Tool(ToolType::Zoom, CategoryNone) {}

void ZoomTool::on_pointer_move() {
  float delta =
    std::abs(InputManager::pointer.client.movement.x) > std::abs(InputManager::pointer.client.movement.y) ?
    InputManager::pointer.client.movement.x : -InputManager::pointer.client.movement.y;

  Editor::viewport.zoom_to(Editor::viewport.zoom() * (1.0f + (delta * ZOOM_STEP) / 500.0f), InputManager::pointer.client.origin);
}
