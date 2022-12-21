#include "pencil_tool.h"

#include "../../editor.h"
#include "../input_manager.h"

PencilTool::PencilTool(): Tool(ToolType::Pencil, CategoryImmediate) {}

void PencilTool::on_pointer_down() {
  m_entity = std::make_shared<FreehandEntity>(InputManager::pointer.scene.position, InputManager::pointer.pressure, InputManager::pointer.time);

  Editor::scene.add_entity(m_entity);
}

void PencilTool::on_pointer_move() {
  if (!m_entity) return;

  m_entity->add_point(InputManager::pointer.scene.delta, InputManager::pointer.pressure, InputManager::pointer.time);
}

void PencilTool::on_pointer_up() {
  m_entity = nullptr;
}
