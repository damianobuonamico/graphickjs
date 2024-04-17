/**
 * @file zoom_tool.cpp
 * @brief Contains the implementation of the ZoomTool class.
 */

#include "zoom_tool.h"

#include "../input_manager.h"

#include "../../editor.h"

#include "../../../utils/defines.h"

namespace graphick::editor::input {

  ZoomTool::ZoomTool() : Tool(ToolType::Zoom, CategoryImmediate) {}

  void ZoomTool::on_pointer_move() {
    Scene& scene = Editor::scene();

    const float delta =
      std::abs(InputManager::pointer.client.movement.x) > std::abs(InputManager::pointer.client.movement.y) ?
      InputManager::pointer.client.movement.x : -InputManager::pointer.client.movement.y;

    scene.viewport.zoom_to(
      scene.viewport.zoom() * (1.0f + (delta * ZOOM_STEP) / 50.0f),
      InputManager::pointer.client.origin
    );
  }

}
