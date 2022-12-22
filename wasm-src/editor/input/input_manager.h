#pragma once

#include "../../math/vec2.h"
#include "keys.h"
#include "tool_state.h"

#include <string>

class InputManager {
public:
  enum class PointerEvent {
    Down = 0,
    Move = 1,
    Up = 2,
    Enter = 3,
    Leave = 4
  };
  enum class PointerButton {
    Touch = -1,
    Left = 0,
    Middle = 1,
    Right = 2,
    Eraser = 5
  };
  enum class PointerTarget {
    Other = 0,
    Canvas = 1
  };
  enum class PointerType {
    Mouse = 0,
    Pen = 1,
    Touch = 2
  };

  enum class KeyboardEvent {
    Down = 0,
    Up = 1
  };

  enum class ClipboardEvent {
    Copy = 0,
    Paste = 1,
    Cut = 2
  };
public:
  struct PointerCoord {
    vec2 position{};
    vec2 movement{};
    vec2 delta{};
    vec2 origin{};
  };
  struct KeysState {
    bool ctrl = false;
    bool alt = false;
    bool shift = false;
    bool space = false;
    bool ctrl_state_changed = false;
    bool alt_state_changed = false;
    bool shift_state_changed = false;
    bool space_state_changed = false;
  };
  struct Pointer {
    PointerTarget target = PointerTarget::Other;


    PointerCoord client{};
    PointerCoord scene{};

    bool down = false;
    bool inside = true;
    float pressure = 1.0f;
    float time = 0.0f;

    PointerButton button = PointerButton::Left;
  };

  static KeysState keys;
  static Pointer pointer;
public:
  InputManager(const InputManager&) = delete;
  InputManager(InputManager&&) = delete;

  static inline InputManager* get() { return s_instance; }

  static void init();
  static void shutdown();

  static bool on_pointer_event(
    PointerTarget target, PointerEvent event, PointerType type, PointerButton button,
    int x, int y, float pressure, float time_stamp,
    bool alt, bool ctrl, bool shift
  );
  static bool on_keyboard_event(
    KeyboardEvent event, KeyboardKey key,
    bool repeat, bool alt, bool ctrl, bool shift
  );
  static bool on_resize_event(int x, int y, int offset_x, int offset_y);
  static bool on_wheel_event(PointerTarget target, int delta_x, int delta_y);
  static bool on_clipboard_event(ClipboardEvent event);

  static void set_tool(Tool::ToolType type);
private:
  InputManager() = default;
  ~InputManager() = default;

  void set_keys_state(bool alt, bool ctrl, bool shift);

  bool on_pointer_down(PointerTarget target, PointerButton button, int x, int y);
  bool on_pointer_move(PointerTarget target, int x, int y);
  bool on_pointer_up();
  bool on_pointer_enter();
  bool on_pointer_leave();

  bool on_key_down(KeyboardKey key);
  bool on_key_up(KeyboardKey key);

  bool on_resize(int x, int y, int offset_x, int offset_y);
  bool on_wheel(PointerTarget target, float delta_x, float delta_y);

  bool on_clipboard_copy();
  bool on_clipboard_paste();
  bool on_clipboard_cut();
private:
  PointerType m_pointer_type = PointerType::Mouse;

  bool m_moving = false;
  bool m_abort = false;

  ToolState m_tool_state;
private:
  static InputManager* s_instance;
};