/**
 * @file pencil_tool.cpp
 * @brief Contains the implementation of the PencilTool class.
 */

#include "pencil_tool.h"

#include "../input_manager.h"
// #include "../wobble_smoother.h"

#include "../../editor.h"

namespace Graphick::Editor::Input {

  PencilTool::PencilTool() : Tool(ToolType::Pencil, CategoryImmediate) {}

  void PencilTool::on_pointer_down() {
    // float pressure = std::max(InputManager::pointer.pressure, 0.1f);

    // m_entity = std::make_shared<FreehandEntity>(InputManager::pointer.scene.position, pressure, InputManager::pointer.time);

    // WobbleSmoother::reset(
    //   { InputManager::pointer.type != InputManager::PointerType::Pen, 20.0f, 40.0f, 1.31f, 1.44f },
    //   vec2{ 0.0f }, pressure, InputManager::pointer.time
    // );

    // Editor::scene().add_entity(m_entity);
  }

  void PencilTool::on_pointer_move() {
    // if (!m_entity) return;

    // float pressure = 0.5f + (InputManager::pointer.pressure - 0.5f) * 0.8f;

    // vec3 smoothed_point = WobbleSmoother::update(InputManager::pointer.scene.delta, std::max(pressure, 0.1f), InputManager::pointer.time);
    // m_entity->add_point(InputManager::pointer.scene.delta, pressure, InputManager::pointer.time, smoothed_point);
  }

  void PencilTool::on_pointer_up() {
    // m_entity = nullptr;
  }

}
