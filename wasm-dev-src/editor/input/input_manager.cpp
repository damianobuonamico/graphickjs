#include "input_manager.h"

// #include "../../math/vector.h"
#include "../editor.h"

#include "../../math/ivec2.h"
#include "../../math/math.h"

#include "../../utils/console.h"

namespace Graphick::Editor::Input {

  InputManager* InputManager::s_instance = nullptr;
  InputManager::Pointer InputManager::pointer{};
  InputManager::KeysState InputManager::keys{};
  // HoverState InputManager::hover{};

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
    float x, float y, float pressure, double time_stamp,
    bool alt, bool ctrl, bool shift
  ) {
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
    KeyboardEvent event, KeyboardKey key,
    bool repeat, bool alt, bool ctrl, bool shift
  ) {
    InputManager* instance = get();

    instance->set_keys_state(alt, ctrl, shift);

    if (key == KeyboardKey::Escape) {
      instance->m_abort = true;
    } else if (key == KeyboardKey::Space) {
      keys.space_state_changed = keys.space == (event == KeyboardEvent::Up);
      keys.space = event == KeyboardEvent::Down;
    }

    if (!pointer.down && (keys.ctrl_state_changed || keys.space_state_changed)) {
      Editor::scene().tool_state.recalculate_active();
    }

    switch (event) {
    case KeyboardEvent::Down:
      return instance->on_key_down(key);
    case KeyboardEvent::Up:
      return instance->on_key_up(key);
    }

    return false;
  }

  bool InputManager::on_resize_event(int width, int height, float dpr, int offset_x, int offset_y) {
    return get()->on_resize(width, height, dpr, offset_x, offset_y);
  }

  bool InputManager::on_wheel_event(PointerTarget target, float delta_x, float delta_y) {
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

  void InputManager::set_tool(Tool::ToolType tool) {
    Editor::scene().tool_state.set_current(tool);
  }

  void InputManager::set_keys_state(bool alt, bool ctrl, bool shift) {
    keys.alt_state_changed = keys.alt != alt;
    keys.alt = alt;

    keys.ctrl_state_changed = keys.ctrl != ctrl;
    keys.ctrl = ctrl;

    keys.shift_state_changed = keys.shift != shift;
    keys.shift = shift;
  }

  bool InputManager::on_pointer_down(PointerTarget target, PointerButton button, float x, float y) {
    pointer.target = target;

    if (target != PointerTarget::Canvas) return false;

    vec2 current_position = { x, y };

    pointer.client.movement = { 0.0f, 0.0f };
    pointer.client.position = current_position;
    pointer.client.delta = { 0.0f, 0.0f };
    pointer.client.origin = current_position;

    pointer.scene.movement = { 0.0f, 0.0f };
    pointer.scene.position = Editor::scene().viewport.client_to_scene(current_position);
    pointer.scene.delta = { 0.0f, 0.0f };
    pointer.scene.origin = pointer.scene.position;

    pointer.down = true;
    pointer.button = button;

    m_abort = false;

    if (pointer.button == PointerButton::Middle) {
      Editor::scene().tool_state.set_active(keys.ctrl_state_changed ? Tool::ToolType::Zoom : Tool::ToolType::Pan);
    }

    // CommandHistory::end_batch();

    Editor::scene().tool_state.on_pointer_down();

    Editor::render();

    return false;
  }

  bool InputManager::on_pointer_move(PointerTarget target, float x, float y) {
    if (pointer.target != PointerTarget::Canvas && target != PointerTarget::Canvas) return false;

    vec2 current_position = { x, y };

    pointer.client.movement = current_position - pointer.client.position;
    pointer.client.position = current_position;
    pointer.client.delta = current_position - pointer.client.origin;

    pointer.scene.movement = pointer.client.movement / Editor::scene().viewport.zoom();
    pointer.scene.position = Editor::scene().viewport.client_to_scene(current_position);
    pointer.scene.delta = pointer.scene.position - pointer.scene.origin;

    // if (!Editor::scene().tool_state.active().is_in_category(Tool::CategoryImmediate)) {
    //   hover.set(
    //     Editor::scene().entity_at(
    //       pointer.scene.position,
    //       Editor::scene().tool_state.active().is_in_category(Tool::CategoryDirect),
    //       5.0f / Editor::scene().viewport.zoom()
    //     )
    //   );
    // }

    if (!m_moving && pointer.down) {
      if (
        Editor::scene().tool_state.active().is_in_category(Tool::CategoryImmediate) ||
        length(pointer.client.delta) > INPUT_MOVEMENT_THRESHOLD * INPUT_MOVEMENT_THRESHOLD_MULTIPLIER[(int)pointer.type]
        ) {
        m_moving = true;
      } else {
        return false;
      }
    }

    if (m_moving && !m_abort) {
      Editor::scene().tool_state.on_pointer_move();
      Editor::render();
    } else if (!pointer.down) {
      Editor::scene().tool_state.on_pointer_hover();
      Editor::render();
    }

    return false;
  }

  bool InputManager::on_pointer_up() {
    if (!pointer.down) return false;

    pointer.target = PointerTarget::Other;
    pointer.down = false;

    m_moving = false;

    Editor::scene().tool_state.on_pointer_up();

    // CommandHistory::end_batch();

    if (pointer.button == PointerButton::Middle) {
      Editor::scene().tool_state.set_active(Editor::scene().tool_state.current().type());
    } else {
      Editor::scene().tool_state.recalculate_active();
    }

    Editor::render();

    return false;
  }

  bool InputManager::on_pointer_enter() {
    pointer.inside = true;
    return false;
  }

  bool InputManager::on_pointer_leave() {
    pointer.inside = false;
    return false;
  }

  bool InputManager::on_key_down(KeyboardKey key) {
    if ((key == KeyboardKey::Z || (int)key == 90 /* TEMP: GLFW */) && keys.ctrl) {
      // if (keys.shift) {
      //   CommandHistory::redo();
      // } else {
      //   CommandHistory::undo();
      // }

      Editor::render();
    }

    return false;
  }
  bool InputManager::on_key_up(KeyboardKey key) {
    return false;
  }

  bool InputManager::on_resize(int width, int height, float dpr, int offset_x, int offset_y) {
    ivec2 size = { width, height };
    ivec2 offset = { offset_x, offset_y };

    Editor::resize(size, offset, dpr);
    Editor::render();

    return false;
  }
  bool InputManager::on_wheel(PointerTarget target, float delta_x, float delta_y) {
    if (!keys.ctrl) return false;

    // TODO: Move in scene specific input code
    Editor::scene().viewport.zoom_to(Math::map(-delta_y, -100.0f, 100.0f, 1.0f - ZOOM_STEP / 10.0f, 1.0f + ZOOM_STEP / 10.0f) * Editor::scene().viewport.zoom(), pointer.client.position);
    Editor::render();

    return true;
  }
  bool InputManager::on_clipboard_copy() {
    return false;
  }
  bool InputManager::on_clipboard_paste() {
    return false;
  }
  bool InputManager::on_clipboard_cut() {
    return false;
  }

}
