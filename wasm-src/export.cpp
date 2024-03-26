/**
 * @file export.cpp
 * @brief The file exports methods to the WebAssembly runtime.
 */

#include "editor/editor.h"
#include "editor/scene/entity.h"
#include "editor/input/input_manager.h"
#include "io/svg/svg.h"

#include <stdio.h>

#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

#ifdef __INTELLISENSE__
#define EMSCRIPTEN_KEEPALIVE
#endif

#include <emscripten/bind.h>

using namespace emscripten;

extern "C" {
  bool EMSCRIPTEN_KEEPALIVE on_pointer_event(
    int target, int event, int type, int button,
    float x, float y, float pressure, double time_stamp,
    bool alt, bool ctrl, bool shift
  ) {
    return Graphick::Editor::Input::InputManager::on_pointer_event(
      (Graphick::Editor::Input::InputManager::PointerTarget)target, (Graphick::Editor::Input::InputManager::PointerEvent)event, (Graphick::Editor::Input::InputManager::PointerType)type, (Graphick::Editor::Input::InputManager::PointerButton)button,
      x, y, pressure, time_stamp, alt, ctrl, shift
    );
    return true;
  }

  bool EMSCRIPTEN_KEEPALIVE on_keyboard_event(
    int event, int key,
    bool repeat, bool alt, bool ctrl, bool shift
  ) {
    return Graphick::Editor::Input::InputManager::on_keyboard_event(
      (Graphick::Editor::Input::InputManager::KeyboardEvent)event, (Graphick::Editor::Input::KeyboardKey)key,
      repeat, alt, ctrl, shift
    );
  }

  bool EMSCRIPTEN_KEEPALIVE on_resize_event(int width, int height, float dpr, int offset_x, int offset_y) {
    return Graphick::Editor::Input::InputManager::on_resize_event(width, height, dpr, offset_x, offset_y);
  }

  bool EMSCRIPTEN_KEEPALIVE on_wheel_event(int target, float delta_x, float delta_y, bool ctrl) {
    return Graphick::Editor::Input::InputManager::on_wheel_event((Graphick::Editor::Input::InputManager::PointerTarget)target, delta_x, delta_y, ctrl);
  }

  bool EMSCRIPTEN_KEEPALIVE on_clipboard_event(int event) {
    return Graphick::Editor::Input::InputManager::on_clipboard_event((Graphick::Editor::Input::InputManager::ClipboardEvent)event);
  }

  bool EMSCRIPTEN_KEEPALIVE on_touch_pinch(int target, float delta, float center_x, float center_y) {
    return Graphick::Editor::Input::InputManager::on_touch_pinch((Graphick::Editor::Input::InputManager::PointerTarget)target, delta, center_x, center_y);
  }

  bool EMSCRIPTEN_KEEPALIVE on_touch_drag(int target, float delta_x, float delta_y) {
    return Graphick::Editor::Input::InputManager::on_touch_drag((Graphick::Editor::Input::InputManager::PointerTarget)target, delta_x, delta_y);
  }

  void EMSCRIPTEN_KEEPALIVE set_tool(int tool) {
    if (tool < 0 || tool >= static_cast<int>(Graphick::Editor::Input::Tool::ToolType::None)) return;

    Graphick::Editor::Input::InputManager::set_tool((Graphick::Editor::Input::Tool::ToolType)tool);
  }

  void EMSCRIPTEN_KEEPALIVE load(const char* data) {
    // Graphick::Editor::Editor::load(data);
  }

  void EMSCRIPTEN_KEEPALIVE load_font(const unsigned char* buffer, long buffer_size) {
    // FontManager::load_font(buffer, buffer_size);
  }

  void EMSCRIPTEN_KEEPALIVE load_svg(const char* svg) {
    Graphick::io::svg::parse_svg(svg);
  }

  void EMSCRIPTEN_KEEPALIVE init() {
    Graphick::Editor::Editor::init();

    Graphick::Renderer::Geometry::Path path;

    path.move_to({ 0.0f, 0.0f });
    path.quadratic_to({ 100.0f, 100.0f }, { 200.0f, 000.0f });
    path.quadratic_to({ 100.0f, -100.0f }, { 0.0f, 0.0f });
    path.close();

    Graphick::Editor::Entity test_entity = Graphick::Editor::Editor::scene().create_element(path);

    test_entity.add_component<Graphick::Editor::FillComponent>(Graphick::vec4{ 0.8f, 0.3f, 0.3f, 1.0f });
    test_entity.add_component<Graphick::Editor::StrokeComponent>(Graphick::vec4{ 0.93f, 0.64f, 0.74f, 1.0f });
  }

  void EMSCRIPTEN_KEEPALIVE prepare_refresh() {
    Graphick::Editor::Editor::prepare_refresh();
  }

  void EMSCRIPTEN_KEEPALIVE refresh() {
    Graphick::Editor::Editor::refresh();
  }

  void EMSCRIPTEN_KEEPALIVE shutdown() {
    Graphick::Editor::Editor::shutdown();
  }

  void EMSCRIPTEN_KEEPALIVE save() {
    // Graphick::Editor::Editor::save();
  }
}
