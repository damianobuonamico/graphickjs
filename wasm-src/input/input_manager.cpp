#include "input_manager.h"

#include "../common.h"
#include "../editor/editor.h"

#include <assert.h>

InputManager* InputManager::s_instance = nullptr;
InputManager::Pointer InputManager::pointer{};

void InputManager::init() {
  // TODO: InputManager reinitialization
  assert(!s_instance);
  s_instance = new InputManager();
}

void InputManager::shutdown() {
  delete s_instance;
}

bool InputManager::on_pointer_event(
  PointerTarget target, PointerEvent event, PointerType type, PointerButton button,
  int x, int y, float pressure, float time_stamp,
  bool alt, bool ctrl, bool shift
) {
  get()->set_keys_state(alt, ctrl, shift);
  get()->m_pointer_type = type;
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
  KeyboardEvent event, KeyboardKey key,
  bool repeat, bool alt, bool ctrl, bool shift
) {
  get()->set_keys_state(alt, ctrl, shift);

  if (key == KeyboardKey::Escape) {
    get()->m_abort = true;
  }

  switch (event) {
  case KeyboardEvent::Down:
    return get()->on_key_down();
  case KeyboardEvent::Up:
    return get()->on_key_up();
  }

  return false;
}

bool InputManager::on_resize_event(int x, int y, int offset_x, int offset_y) {
  return get()->on_resize(x, y, offset_x, offset_y);
}

bool InputManager::on_wheel_event(PointerTarget target, int delta_x, int delta_y) {
  return get()->on_wheel(target, delta_x, delta_y);
}

bool InputManager::on_clipboard_event(ClipboardEvent event) {
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

void InputManager::set_keys_state(bool alt, bool ctrl, bool shift) {
  pointer.keys.alt_state_changed = pointer.keys.alt != alt;
  pointer.keys.alt = alt;

  pointer.keys.ctrl_state_changed = pointer.keys.ctrl != ctrl;
  pointer.keys.ctrl = ctrl;

  pointer.keys.shift_state_changed = pointer.keys.shift != shift;
  pointer.keys.shift = shift;
}

bool InputManager::on_pointer_down(PointerTarget target, PointerButton button, int x, int y) {
  console::log("PointerDown");
  pointer.target = target;

  if (target != PointerTarget::Canvas) return false;

  vec2 current_position = { (float)x, (float)y };

  pointer.client.movement = { 0.0f, 0.0f };
  pointer.client.position = current_position;
  pointer.client.delta = { 0.0f, 0.0f };
  pointer.client.origin = current_position;

  // TODO: client_to_scene
  pointer.scene.movement = { 0.0f, 0.0f };
  pointer.scene.position = current_position;
  pointer.scene.delta = { 0.0f, 0.0f };
  pointer.scene.origin = current_position;

  pointer.down = true;
  pointer.button = button;

  m_abort = false;

  return false;
}

bool InputManager::on_pointer_move(PointerTarget target, int x, int y) {
  console::log("PointerMove");
  if (pointer.target != PointerTarget::Canvas && target != PointerTarget::Canvas) return false;

  vec2 current_position = { (float)x, (float)y };

  pointer.client.movement = current_position - pointer.client.position;
  pointer.client.position = current_position;
  pointer.client.delta = current_position - pointer.client.origin;

  // TODO: client_to_scene
  pointer.scene.movement = current_position - pointer.client.position;
  pointer.scene.position = current_position;
  pointer.scene.delta = current_position - pointer.client.origin;

  if (!m_moving && pointer.down) {
    // TODO: check pointer delta
    if (true) {
      m_moving = true;
    } else {
      return false;
    }
  }

  return false;
}

bool InputManager::on_pointer_up() {
  console::log("PointerUp");
  if (!pointer.down) return false;

  pointer.target = PointerTarget::Other;
  pointer.down = false;

  m_moving = false;

  return false;
}

bool InputManager::on_pointer_enter() {
  console::log("PointerEnter");
  pointer.inside = true;
  return false;
}

bool InputManager::on_pointer_leave() {
  console::log("PointerLeave");
  pointer.inside = false;
  return false;
}

bool InputManager::on_key_down() {
  console::log("KeyDown");
  return false;
}
bool InputManager::on_key_up() {
  console::log("KeyUp");
  return false;
}

bool InputManager::on_resize(int x, int y, int offset_x, int offset_y) {
  console::log("Resize");
  vec2 size = vec2{ (float)x, (float)y };
  vec2 offset = vec2{ (float)offset_x, (float)offset_y };

  Editor::viewport.resize(size, offset);

  return false;
}
bool InputManager::on_wheel(PointerTarget target, int delta_x, int delta_y) {
  console::log("Wheel");
  Editor::viewport.zoom_to(1.0f, vec2{ 0.0f, 0.0f });

  return false;
}
bool InputManager::on_clipboard_copy() {
  console::log("ClipboardCopy");
  return false;
}
bool InputManager::on_clipboard_paste() {
  console::log("ClipboardPaste");
  return false;
}
bool InputManager::on_clipboard_cut() {
  console::log("ClipboardCut");
  return false;
}