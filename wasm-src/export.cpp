/**
 * @file export.cpp
 * @brief The file exports methods to the WebAssembly runtime.
 */

#include "editor/editor.h"
#include "editor/input/input_manager.h"
#include "editor/scene/entity.h"
#include "io/svg/svg.h"
#include "utils/resource_manager.h"

#include <stdio.h>

#ifdef EMSCRIPTEN
#  include <emscripten.h>
#  include <emscripten/bind.h>

using namespace emscripten;

#endif

#ifdef __INTELLISENSE__
#  define EMSCRIPTEN_KEEPALIVE
#endif

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
  return graphick::editor::input::InputManager::on_pointer_event(
      (graphick::editor::input::InputManager::PointerTarget)target,
      (graphick::editor::input::InputManager::PointerEvent)event,
      (graphick::editor::input::InputManager::PointerType)type,
      (graphick::editor::input::InputManager::PointerButton)button,
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
  return graphick::editor::input::InputManager::on_keyboard_event(
      (graphick::editor::input::InputManager::KeyboardEvent)event,
      (graphick::editor::input::KeyboardKey)key,
      repeat,
      alt,
      ctrl,
      shift);
}

bool EMSCRIPTEN_KEEPALIVE
on_resize_event(int width, int height, float dpr, int offset_x, int offset_y)
{
  return graphick::editor::input::InputManager::on_resize_event(
      width, height, dpr, offset_x, offset_y);
}

bool EMSCRIPTEN_KEEPALIVE on_wheel_event(int target, float delta_x, float delta_y, bool ctrl)
{
  return graphick::editor::input::InputManager::on_wheel_event(
      (graphick::editor::input::InputManager::PointerTarget)target, delta_x, delta_y, ctrl);
}

bool EMSCRIPTEN_KEEPALIVE on_clipboard_event(int event)
{
  return graphick::editor::input::InputManager::on_clipboard_event(
      (graphick::editor::input::InputManager::ClipboardEvent)event);
}

bool EMSCRIPTEN_KEEPALIVE on_touch_pinch(int target, float delta, float center_x, float center_y)
{
  return graphick::editor::input::InputManager::on_touch_pinch(
      (graphick::editor::input::InputManager::PointerTarget)target, delta, center_x, center_y);
}

bool EMSCRIPTEN_KEEPALIVE on_touch_drag(int target, float delta_x, float delta_y)
{
  return graphick::editor::input::InputManager::on_touch_drag(
      (graphick::editor::input::InputManager::PointerTarget)target, delta_x, delta_y);
}

void EMSCRIPTEN_KEEPALIVE set_tool(int tool)
{
  if (tool < 0 || tool >= static_cast<int>(graphick::editor::input::Tool::ToolType::None))
    return;

  graphick::editor::input::InputManager::set_tool((graphick::editor::input::Tool::ToolType)tool);
}

void EMSCRIPTEN_KEEPALIVE load(const char* data)
{
  // graphick::editor::Editor::load(data);
}

void EMSCRIPTEN_KEEPALIVE load_font(const unsigned char* buffer, long buffer_size)
{
  // FontManager::load_font(buffer, buffer_size);
}

void EMSCRIPTEN_KEEPALIVE load_svg(const char* svg)
{
  graphick::io::svg::parse_svg(svg);
}

void EMSCRIPTEN_KEEPALIVE load_image(const uint8_t* data, size_t buffer_size)
{
  graphick::utils::uuid image_id = graphick::utils::ResourceManager::load_image(data, buffer_size);
  graphick::editor::Editor::scene().create_image(image_id);
}

void EMSCRIPTEN_KEEPALIVE init()
{
  graphick::editor::Editor::init();

  graphick::geom::path path;

  path.move_to({0.0f, 0.0f});
  path.quadratic_to({100.0f, 100.0f}, {200.0f, 000.0f});
  path.quadratic_to({100.0f, -100.0f}, {0.0f, 0.0f});
  path.close();

  graphick::editor::Entity test_entity = graphick::editor::Editor::scene().create_element(path);

  test_entity.add_component<graphick::editor::FillComponent>(
      graphick::vec4{0.8f, 0.3f, 0.3f, 1.0f});
  test_entity.add_component<graphick::editor::StrokeComponent>(
      graphick::vec4{0.93f, 0.64f, 0.74f, 1.0f});
}

void EMSCRIPTEN_KEEPALIVE prepare_refresh()
{
  graphick::editor::Editor::prepare_refresh();
}

void EMSCRIPTEN_KEEPALIVE refresh()
{
  graphick::editor::Editor::refresh();
}

void EMSCRIPTEN_KEEPALIVE shutdown()
{
  graphick::editor::Editor::shutdown();
}

void EMSCRIPTEN_KEEPALIVE save()
{
  // graphick::editor::Editor::save();
}
}
