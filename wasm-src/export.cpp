#include "common.h"
#include "editor/editor.h"
#include "renderer/renderer.h"
#include "input/input_manager.h"

#include <emscripten/bind.h>

using namespace emscripten;

extern "C" {
  void EMSCRIPTEN_KEEPALIVE resize(const float width, const float height) {
    Renderer::get()->resize(width, height);
  }

  void EMSCRIPTEN_KEEPALIVE begin_frame(const float* position, const float zoom) {
    Renderer::get()->begin_frame(position, zoom);
  }

  void EMSCRIPTEN_KEEPALIVE end_frame() {
    Renderer::get()->end_frame();
  }

  void EMSCRIPTEN_KEEPALIVE draw(const float* vertices, const float vertices_length, const float* indices, const float indices_length) {
    Geometry geometry;

    for (int i = 0; i < vertices_length; i += 2) {
      geometry.vertices.push_back({ {vertices[i], vertices[i + 1]} });
    }

    for (int i = 0; i < indices_length; ++i) {
      geometry.indices.push_back(indices[i]);
    }

    Renderer::get()->draw(geometry);
  }

  bool EMSCRIPTEN_KEEPALIVE on_pointer_event(
    int target, int event, int type, int button,
    int x, int y, float pressure, float time_stamp,
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

  bool EMSCRIPTEN_KEEPALIVE on_resize_event(int x, int y, int offset_x, int offset_y) {
    return InputManager::on_resize_event(x, y, offset_x, offset_y);
  }

  bool EMSCRIPTEN_KEEPALIVE on_wheel_event(int target, int delta_x, int delta_y) {
    return InputManager::on_wheel_event((InputManager::PointerTarget)target, delta_x, delta_y);
  }

  bool EMSCRIPTEN_KEEPALIVE on_clipboard_event(int event) {
    return InputManager::on_clipboard_event((InputManager::ClipboardEvent)event);
  }
}

EMSCRIPTEN_BINDINGS(Renderer) {
  function("_init", Editor::init);
  function("_shutdown", Editor::shutdown);
}