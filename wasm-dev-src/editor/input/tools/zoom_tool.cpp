#include "zoom_tool.h"

#include "../input_manager.h"

#include "../../editor.h"

#include "../../../utils/defines.h"

namespace Graphick::Editor::Input {

  ZoomTool::ZoomTool() : Tool(ToolType::Zoom, CategoryImmediate) {}

  void ZoomTool::on_pointer_move() {
    float delta =
      std::abs(InputManager::pointer.client.movement.x) > std::abs(InputManager::pointer.client.movement.y) ?
      InputManager::pointer.client.movement.x : -InputManager::pointer.client.movement.y;

    Editor::scene().viewport.zoom_to(Editor::scene().viewport.zoom() * (1.0f + (delta * ZOOM_STEP) / 500.0f), InputManager::pointer.client.origin);
  }

}
