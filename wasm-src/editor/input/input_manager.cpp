/**
 * @file input_manager.cpp
 * @brief Contains the implementation of the InputManager class.
 *
 * @todo InputManager reinitialization
 * @todo implement shortcuts and emscripten/glfw key codes mapping
 * @todo move zoom and scroll normalization to the viewport class
 * @todo clipboard events
 */

#include "input_manager.h"

#include "../editor.h"
#include "../scene/entity.h"
#include "../settings.h"

#include "../../math/math.h"
#include "../../math/scalar.h"
#include "../../math/vec2.h"

#include "../../utils/console.h"

namespace graphick::editor::input {

InputManager* InputManager::s_instance = nullptr;
InputManager::Pointer InputManager::pointer{};
InputManager::KeysState InputManager::keys{};
HoverState InputManager::hover{};

void InputManager::init()
{
  assert(!s_instance);
  s_instance = new InputManager();
}

void InputManager::shutdown()
{
  delete s_instance;
  s_instance = nullptr;
}

bool InputManager::on_pointer_event(PointerTarget target,
                                    PointerEvent event,
                                    PointerType type,
                                    PointerButton button,
                                    float x,
                                    float y,
                                    float pressure,
                                    double time_stamp,
                                    bool alt,
                                    bool ctrl,
                                    bool shift)
{
  get()->set_keys_state(alt, ctrl, shift);
  pointer.type = type;
  pointer.pressure = pressure;
  pointer.time = time_stamp;

  switch (event) {
    case PointerEvent::Down:
      return get()->on_pointer_down(target, button, x, y);
    case PointerEvent::Move:
      return get()->on_pointer_move(target, x, y);
    case PointerEvent::Up:
      return get()->on_pointer_up();
    case PointerEvent::Enter:
      return get()->on_pointer_enter();
    case PointerEvent::Leave:
      return get()->on_pointer_leave();
  }

  return false;
}

bool InputManager::on_keyboard_event(
    KeyboardEvent event, KeyboardKey key, bool repeat, bool alt, bool ctrl, bool shift)
{
  InputManager* instance = get();

  instance->set_keys_state(alt, ctrl, shift);

  if (key == KeyboardKey::Escape) {
    instance->m_abort = true;
  } else if (key == KeyboardKey::Space) {
    keys.space_state_changed = keys.space == (event == KeyboardEvent::Up);
    keys.space = event == KeyboardEvent::Down;
  }

  if (key != KeyboardKey::Space) {
    keys.space_state_changed = false;
  }

  bool should_render = false;

  if (!pointer.down && (keys.ctrl_state_changed || keys.space_state_changed)) {
    Editor::scene().tool_state.recalculate_active();
    should_render = true;
  }

  if (instance->m_moving && !instance->m_abort) {
    Editor::scene().tool_state.on_key(event == KeyboardEvent::Down, key);
    should_render = true;
  }

  if (should_render && (keys.ctrl_state_changed || keys.space_state_changed))
    Editor::request_render();

  switch (event) {
    case KeyboardEvent::Down:
      return instance->on_key_down(key);
    case KeyboardEvent::Up:
      return instance->on_key_up(key);
  }

  return false;
}

bool InputManager::on_resize_event(int width, int height, float dpr, int offset_x, int offset_y)
{
  return get()->on_resize(width, height, dpr, offset_x, offset_y);
}

bool InputManager::on_wheel_event(PointerTarget target, float delta_x, float delta_y, bool ctrl)
{
  return get()->on_wheel(target, delta_x, delta_y, ctrl);
}

bool InputManager::on_clipboard_event(ClipboardEvent event)
{
  switch (event) {
    case ClipboardEvent::Copy:
      return get()->on_clipboard_copy();
    case ClipboardEvent::Paste:
      return get()->on_clipboard_paste();
    case ClipboardEvent::Cut:
      return get()->on_clipboard_cut();
  }

  return false;
}

bool InputManager::on_touch_pinch(PointerTarget target,
                                  float delta,
                                  float center_x,
                                  float center_y)
{
  return get()->on_pinch(target, delta, center_x, center_y);
}

bool InputManager::on_touch_drag(PointerTarget target, float delta_x, float delta_y)
{
  return get()->on_drag(target, delta_x, delta_y);
}

const Tool& InputManager::tool()
{
  return Editor::scene().tool_state.active();
}

void InputManager::set_tool(Tool::ToolType tool)
{
  Editor::scene().tool_state.set_current(tool);
}

void InputManager::set_keys_state(bool alt, bool ctrl, bool shift)
{
  keys.alt_state_changed = keys.alt != alt;
  keys.alt = alt;

  keys.ctrl_state_changed = keys.ctrl != ctrl;
  keys.ctrl = ctrl;

  keys.shift_state_changed = keys.shift != shift;
  keys.shift = shift;
}

void InputManager::recalculate_hover()
{
  Scene& scene = Editor::scene();

  if (!scene.tool_state.active().is_in_category(Tool::CategoryImmediate)) {
    float threshold = Settings::Input::movement_threshold_multiplier[(int)pointer.type] *
                      Settings::Input::hit_threshold;

    hover.set_hovered(
        scene.entity_at(pointer.scene.position,
                        scene.tool_state.active().is_in_category(Tool::CategoryDirect),
                        threshold),
        pointer.scene.position,
        scene.tool_state.active().is_in_category(Tool::CategoryDirect),
        threshold,
        scene.viewport.zoom());
  }
}

bool InputManager::on_pointer_down(PointerTarget target, PointerButton button, float x, float y)
{
  pointer.target = target;

  if (target != PointerTarget::Canvas)
    return false;

  Scene& scene = Editor::scene();

  vec2 current_position = {x, y};

  pointer.client.movement = {0.0f, 0.0f};
  pointer.client.position = current_position;
  pointer.client.delta = {0.0f, 0.0f};
  pointer.client.origin = current_position;

  pointer.scene.movement = {0.0f, 0.0f};
  pointer.scene.position = scene.viewport.client_to_scene(current_position);
  pointer.scene.delta = {0.0f, 0.0f};
  pointer.scene.origin = pointer.scene.position;

  pointer.down = true;
  pointer.button = button;

  m_abort = false;

  recalculate_hover();

  if (pointer.button == PointerButton::Middle) {
    scene.tool_state.set_active(keys.ctrl_state_changed ? Tool::ToolType::Zoom :
                                                          Tool::ToolType::Pan);
  }

  Editor::scene().history.end_batch();

  scene.tool_state.on_pointer_down(scene.viewport.zoom());

  Editor::request_render();

  return false;
}

bool InputManager::on_pointer_move(PointerTarget target, float x, float y)
{
  if (pointer.target != PointerTarget::Canvas && target != PointerTarget::Canvas)
    return false;

  Scene& scene = Editor::scene();

  vec2 current_position = {x, y};

  pointer.client.movement = current_position - pointer.client.position;
  pointer.client.position = current_position;
  pointer.client.delta = current_position - pointer.client.origin;

  pointer.scene.movement = pointer.client.movement / scene.viewport.zoom();
  pointer.scene.position = scene.viewport.client_to_scene(current_position);
  pointer.scene.delta = pointer.scene.position - pointer.scene.origin;

  recalculate_hover();

  if (!m_moving && pointer.down) {
    if (scene.tool_state.active().is_in_category(Tool::CategoryImmediate) ||
        length(pointer.client.delta) >
            Settings::Input::movement_threshold *
                Settings::Input::movement_threshold_multiplier[(int)pointer.type])
    {
      m_moving = true;
    } else {
      return false;
    }
  }

  if (m_moving && !m_abort) {
    scene.tool_state.on_pointer_move();
  } else if (!pointer.down) {
    scene.tool_state.on_pointer_hover();
  }

  if (pointer.down) {
    Editor::request_render();
  }

  return false;
}

bool InputManager::on_pointer_up()
{
  if (!pointer.down)
    return false;

  Scene& scene = Editor::scene();

  pointer.target = PointerTarget::Other;
  pointer.down = false;

  m_moving = false;

  scene.tool_state.on_pointer_up();
  scene.history.end_batch();

  bool complete_redraw = scene.tool_state.active().is_in_category(Tool::CategoryView);

  if (pointer.button == PointerButton::Middle) {
    scene.tool_state.set_active(scene.tool_state.current().type());
  } else {
    scene.tool_state.recalculate_active();
  }

  Editor::request_render({complete_redraw});

  return false;
}

bool InputManager::on_pointer_enter()
{
  pointer.inside = true;
  return false;
}

bool InputManager::on_pointer_leave()
{
  pointer.inside = false;
  return false;
}

bool InputManager::on_key_down(KeyboardKey key)
{
  Scene& scene = Editor::scene();

  // TODO: prevent default after shortcut
  if ((key == KeyboardKey::Z) && keys.ctrl) {
    if (keys.shift) {
      scene.history.redo();
    } else {
      scene.history.undo();
    }

    Editor::request_render();
  } else if ((int)key == 259 /* TEMP: GLFW */) {
    if (!scene.selection.empty()) {
      scene.delete_entity(scene.selection.selected().begin()->first);
      scene.history.end_batch();
    }
  } else if ((key == KeyboardKey::G) && keys.ctrl) {
    scene.group_selected();
  }

  // TODO: call when successfully performed
  scene.tool_state.force_update();

  return false;
}

bool InputManager::on_key_up(KeyboardKey key)
{
  return false;
}

bool InputManager::on_resize(int width, int height, float dpr, int offset_x, int offset_y)
{
  ivec2 size = {width, height};
  ivec2 offset = {offset_x, offset_y};

  Editor::resize(size, offset, dpr);
  Editor::request_render();

  return false;
}

bool InputManager::on_wheel(PointerTarget target, float delta_x, float delta_y, bool ctrl)
{
  keys.ctrl_state_changed = keys.ctrl != ctrl;
  keys.ctrl = ctrl;

  Scene& scene = Editor::scene();

  if (keys.ctrl) {
    scene.viewport.zoom_to(math::map(-delta_y,
                                     -1.0f,
                                     1.0f,
                                     1.0f - Settings::Input::zoom_step,
                                     1.0f + Settings::Input::zoom_step) *
                               scene.viewport.zoom(),
                           pointer.client.position);
  } else {
    scene.viewport.move(math::round(Settings::Input::pan_step * vec2{-delta_x, -delta_y}) /
                        scene.viewport.zoom());
  }

  Editor::request_render();

  return true;
}

bool InputManager::on_clipboard_copy()
{
  return false;
}

bool InputManager::on_clipboard_paste()
{
  return false;
}

bool InputManager::on_clipboard_cut()
{
  return false;
}

bool InputManager::on_pinch(PointerTarget target, float delta, float center_x, float center_y)
{
  if (target == PointerTarget::Other)
    return false;

  Scene& scene = Editor::scene();

  scene.viewport.zoom_to(scene.viewport.zoom() * delta, vec2{center_x, center_y});
  Editor::request_render();

  return true;
}

bool InputManager::on_drag(PointerTarget target, float delta_x, float delta_y)
{
  if (target == PointerTarget::Other)
    return false;

  Scene& scene = Editor::scene();

  scene.viewport.move(math::round(vec2{delta_x, delta_y}) / scene.viewport.zoom());
  Editor::request_render();

  return true;
}

}  // namespace graphick::editor::input
