/**
 * @file tool_state.cpp
 * @brief Contains the implementation of the ToolState class.
 *
 * @todo the manipulator should be available in all non-direct tools, not just the select tool.
 * @todo implement the rect and ellipse tools.
 */

#include "tool_state.h"

#include "input_manager.h"

#include "../../renderer/renderer.h"

#include "../settings.h"

#include "tools/direct_select_tool.h"
#include "tools/pan_tool.h"
#include "tools/pen_tool.h"
#include "tools/pencil_tool.h"
#include "tools/select_tool.h"
#include "tools/zoom_tool.h"

#ifdef EMSCRIPTEN
#  include <emscripten.h>

#  ifndef __INTELLISENSE__
EM_JS(void, update_tool_ui, (int type), { window._set_tool(type); });
#  else
void update_tool_ui(int type) {}
#  endif

#else
void update_tool_ui(int type) {}
#endif

namespace graphick::editor::input {

ToolState::ToolState()
    : m_current(Tool::ToolType::Select),
      m_active(m_current),
      m_last_tool(m_current),
      m_tools{new PanTool(),
              new ZoomTool(),
              new SelectTool(),
              new DirectSelectTool(),
              new PenTool(),
              new PencilTool()}
{
}

ToolState::~ToolState()
{
  for (auto tool : m_tools) {
    delete tool;
  }
}

PenTool* ToolState::pen() const
{
  if (m_active == Tool::ToolType::Pen || m_current == Tool::ToolType::Pen) {
    return static_cast<editor::input::PenTool*>(m_tools[static_cast<int>(Tool::ToolType::Pen)]);
  }

  return nullptr;
}

void ToolState::reset_tool()
{
  current().reset();
  active().reset();
}

void ToolState::set_current(Tool::ToolType tool)
{
  if (tool == Tool::ToolType::None)
    return;

  reset_tool();

  m_current = tool;

  recalculate_active();
}

void ToolState::set_active(Tool::ToolType tool)
{
  if (tool == Tool::ToolType::None)
    return;

  m_last_tool = m_active;
  m_active = tool;

  update_tool_ui(static_cast<int>(tool));
}

void ToolState::on_pointer_down(const float zoom)
{
  Tool& tool = active();

  if (!tool.is_in_category(Tool::CategoryDirect) &&
      !tool.is_in_category(Tool::CategoryImmediate) && manipulator.on_pointer_down(5.0f / zoom))
    return;

  if (m_active == Tool::ToolType::DirectSelect && m_current == Tool::ToolType::Pen) {
    if (InputManager::hover.type() == HoverState::HoverType::Vertex) {
      if (InputManager::hover.entity_id() != static_cast<PenTool*>(&current())->pen_element()) {
        reset_tool();
      }
    } else if (InputManager::hover.type() != HoverState::HoverType::Handle) {
      reset_tool();
    }
  } else if (m_last_tool != m_active && m_active != Tool::ToolType::Pan &&
             m_active != Tool::ToolType::Zoom)
  {
    m_tools[(int)m_last_tool]->reset();
  }

  active().on_pointer_down();
  manipulator.update();
}

void ToolState::on_pointer_move()
{
  if (manipulator.in_use()) {
    manipulator.on_pointer_move();
    return;
  }

  active().on_pointer_move();
  manipulator.update();
}

void ToolState::on_pointer_up()
{
  if (manipulator.in_use()) {
    manipulator.on_pointer_up();
    return;
  }

  active().on_pointer_up();
  manipulator.update();
}

void ToolState::on_pointer_hover()
{
  active().on_pointer_hover();
}

void ToolState::on_key(const bool down, const KeyboardKey key)
{
  Tool& tool = active();

  if (!tool.is_in_category(Tool::CategoryDirect) &&
      !tool.is_in_category(Tool::CategoryImmediate) && manipulator.on_key(down, key))
    return;

  tool.on_key(down, key);
}

void ToolState::recalculate_active()
{
  if (InputManager::keys.space) {
    if (InputManager::keys.ctrl) {
      set_active(Tool::ToolType::Zoom);
    } else {
      set_active(Tool::ToolType::Pan);
    }
  } else if (InputManager::keys.ctrl) {
    if (m_tools[(int)m_current]->is_in_category(Tool::CategoryDirect)) {
      if (m_current == Tool::ToolType::DirectSelect) {
        set_active(Tool::ToolType::Select);
      } else {
        set_active(Tool::ToolType::DirectSelect);
      }
    } else {
      if (m_current == Tool::ToolType::Select) {
        set_active(Tool::ToolType::DirectSelect);
      } else {
        set_active(Tool::ToolType::Select);
      }
    }
  } else {
    set_active(m_current);
  }
}

void ToolState::render_overlays(const float zoom) const
{
  Tool& tool = active();

  tool.render_overlays();

  if (tool.is_in_category(Tool::CategoryDirect) || tool.is_in_category(Tool::CategoryImmediate) ||
      !manipulator.active())
    return;

  mat2x3 transform = manipulator.transform();

  renderer::Outline outline{nullptr, false, Settings::Renderer::ui_primary_color};
  renderer::Renderer::draw(
      manipulator.path(), transform, renderer::DrawingOptions{nullptr, nullptr, &outline});

  const vec2* handles = manipulator.handles();

  for (int i = 0; i < Manipulator::RN; i++) {
    // TODO: replace with layer color
    renderer::Renderer::ui_square(transform * handles[i], 5.0f / zoom, vec4(0.0f, 1.0f, 0.0f, 1.0f));
    renderer::Renderer::ui_square(transform * handles[i], 3.0f / zoom, vec4::identity());
  }
}

}  // namespace graphick::editor::input
