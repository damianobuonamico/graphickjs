/**
 * @file export.cpp
 * @brief The file exports methods to the WebAssembly runtime.
 */

#include "editor/editor.h"
#include "editor/input/input_manager.h"
#include "editor/scene/entity.h"
#include "io/resource_manager.h"
#include "io/svg/svg.h"

#include <stdio.h>

#ifdef EMSCRIPTEN
#  include <emscripten.h>
#  include <emscripten/bind.h>

using namespace emscripten;

#endif

#ifdef __INTELLISENSE__
#  define EMSCRIPTEN_KEEPALIVE
#endif

using namespace graphick;

extern "C" {
bool EMSCRIPTEN_KEEPALIVE on_pointer_event(int target,
                                           int event,
                                           int type,
                                           int button,
                                           float x,
                                           float y,
                                           float pressure,
                                           double time_stamp,
                                           bool alt,
                                           bool ctrl,
                                           bool shift)
{
  return editor::input::InputManager::on_pointer_event(
      (editor::input::InputManager::PointerTarget)target,
      (editor::input::InputManager::PointerEvent)event,
      (editor::input::InputManager::PointerType)type,
      (editor::input::InputManager::PointerButton)button,
      x,
      y,
      pressure,
      time_stamp,
      alt,
      ctrl,
      shift);
  return true;
}

bool EMSCRIPTEN_KEEPALIVE
on_keyboard_event(int event, int key, bool repeat, bool alt, bool ctrl, bool shift)
{
  return editor::input::InputManager::on_keyboard_event(
      (editor::input::InputManager::KeyboardEvent)event,
      (editor::input::KeyboardKey)key,
      repeat,
      alt,
      ctrl,
      shift);
}

bool EMSCRIPTEN_KEEPALIVE
on_resize_event(int width, int height, float dpr, int offset_x, int offset_y)
{
  return editor::input::InputManager::on_resize_event(width, height, dpr, offset_x, offset_y);
}

bool EMSCRIPTEN_KEEPALIVE on_wheel_event(int target, float delta_x, float delta_y, bool ctrl)
{
  return editor::input::InputManager::on_wheel_event(
      (editor::input::InputManager::PointerTarget)target, delta_x, delta_y, ctrl);
}

bool EMSCRIPTEN_KEEPALIVE on_clipboard_event(int event)
{
  return editor::input::InputManager::on_clipboard_event(
      (editor::input::InputManager::ClipboardEvent)event);
}

bool EMSCRIPTEN_KEEPALIVE on_touch_pinch(int target, float delta, float center_x, float center_y)
{
  return editor::input::InputManager::on_touch_pinch(
      (editor::input::InputManager::PointerTarget)target, delta, center_x, center_y);
}

bool EMSCRIPTEN_KEEPALIVE on_touch_drag(int target, float delta_x, float delta_y)
{
  return editor::input::InputManager::on_touch_drag(
      (editor::input::InputManager::PointerTarget)target, delta_x, delta_y);
}

void EMSCRIPTEN_KEEPALIVE set_tool(int tool)
{
  if (tool < 0 || tool >= static_cast<int>(editor::input::Tool::ToolType::None))
    return;

  editor::input::InputManager::set_tool((editor::input::Tool::ToolType)tool);
}

void EMSCRIPTEN_KEEPALIVE load(const char* data)
{
  // editor::Editor::load(data);
}

void EMSCRIPTEN_KEEPALIVE load_font(const unsigned char* buffer, long buffer_size)
{
  // FontManager::load_font(buffer, buffer_size);
}

void EMSCRIPTEN_KEEPALIVE load_svg(const char* svg)
{
  io::svg::parse_svg(svg);
}

void EMSCRIPTEN_KEEPALIVE load_image(const uint8_t* data, size_t buffer_size)
{
  uuid image_id = io::ResourceManager::load_image(data, buffer_size);
  editor::Editor::scene().create_image(image_id);
}

void EMSCRIPTEN_KEEPALIVE init()
{
  editor::Editor::init();

  geom::path path;

  path.move_to({0.0f, 0.0f});
  path.quadratic_to({100.0f, 100.0f}, {200.0f, 000.0f});
  path.quadratic_to({100.0f, -100.0f}, {0.0f, 0.0f});
  path.close();

  editor::Entity test_entity = editor::Editor::scene().create_element(path);

  test_entity.add_component<editor::FillComponent>(vec4{0.8f, 0.3f, 0.3f, 1.0f});
  test_entity.add_component<editor::StrokeComponent>(vec4{0.93f, 0.64f, 0.74f, 1.0f});
}

void EMSCRIPTEN_KEEPALIVE prepare_refresh()
{
  editor::Editor::prepare_refresh();
}

void EMSCRIPTEN_KEEPALIVE refresh()
{
  editor::Editor::refresh();
}

void EMSCRIPTEN_KEEPALIVE shutdown()
{
  editor::Editor::shutdown();
}

void EMSCRIPTEN_KEEPALIVE save()
{
  // editor::Editor::save();
}
}
