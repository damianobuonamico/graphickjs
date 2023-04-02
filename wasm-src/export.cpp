#include "common.h"
#include "editor/editor.h"
#include "editor/settings.h"
#include "renderer/renderer.h"
#include "editor/input/input_manager.h"

#include <emscripten/bind.h>

using namespace emscripten;

extern "C" {
  bool EMSCRIPTEN_KEEPALIVE on_pointer_event(
    int target, int event, int type, int button,
    float x, float y, float pressure, double time_stamp,
    bool alt, bool ctrl, bool shift
  ) {
    return InputManager::on_pointer_event(
      (InputManager::PointerTarget)target, (InputManager::PointerEvent)event, (InputManager::PointerType)type, (InputManager::PointerButton)button,
      x, y, pressure, time_stamp, alt, ctrl, shift
    );
  }

  bool EMSCRIPTEN_KEEPALIVE on_keyboard_event(
    int event, int key,
    bool repeat, bool alt, bool ctrl, bool shift
  ) {
    return InputManager::on_keyboard_event(
      (InputManager::KeyboardEvent)event, (KeyboardKey)key,
      repeat, alt, ctrl, shift
    );
  }

  bool EMSCRIPTEN_KEEPALIVE on_resize_event(int width, int height, int offset_x, int offset_y) {
    return InputManager::on_resize_event(width, height, offset_x, offset_y);
  }

  bool EMSCRIPTEN_KEEPALIVE on_wheel_event(int target, float delta_x, float delta_y) {
    return InputManager::on_wheel_event((InputManager::PointerTarget)target, delta_x, delta_y);
  }

  bool EMSCRIPTEN_KEEPALIVE on_clipboard_event(int event) {
    return InputManager::on_clipboard_event((InputManager::ClipboardEvent)event);
  }

  void EMSCRIPTEN_KEEPALIVE set_tool(int tool) {
    if (tool < 0 || tool >= static_cast<int>(Tool::ToolType::None)) return;

    InputManager::set_tool((Tool::ToolType)tool);
  }

  void EMSCRIPTEN_KEEPALIVE set_upsample(bool upsample) {
    Settings::upsample_before_fitting = upsample;
  }
}

EMSCRIPTEN_BINDINGS(Renderer) {
  function("_init", Editor::init);
  function("_shutdown", Editor::shutdown);
}