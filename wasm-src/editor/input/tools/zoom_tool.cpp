/**
 * @file zoom_tool.cpp
 * @brief Contains the implementation of the ZoomTool class.
 */

#include "zoom_tool.h"

#include "../input_manager.h"

#include "../../editor.h"

namespace graphick::editor::input {

ZoomTool::ZoomTool() : Tool(ToolType::Zoom, CategoryImmediate | CategoryView) {}

void ZoomTool::on_pointer_move()
{
  Scene& scene = Editor::scene();

  const float delta = std::abs(InputManager::pointer.client.movement.x) >
                              std::abs(InputManager::pointer.client.movement.y) ?
                          InputManager::pointer.client.movement.x :
                          -InputManager::pointer.client.movement.y;

  scene.viewport.zoom_to(scene.viewport.zoom() *
                             (1.0f + (delta * Settings::Input::zoom_step) / 50.0f),
                         InputManager::pointer.client.origin);
}

}  // namespace graphick::editor::input
