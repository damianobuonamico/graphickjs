/**
 * @file editor.cpp
 * @brief Contains the main Graphick Editor class implementation.
 *
 * @todo fix all todos (one day)
 * @todo editor reinitialization
 */

#include "editor.h"

#include "input/input_manager.h"
#include "scene/entity.h"
#include "scene/scene.h"

#include "../io/json/json.h"
#include "../io/resource_manager.h"

#include "../renderer/renderer.h"

#include "../utils/console.h"

#ifdef EMSCRIPTEN
#  include <emscripten/html5.h>
#endif

namespace graphick::editor {

#ifdef EMSCRIPTEN
bool render_callback(const double time, void* user_data)
{
  Editor::get()->render_frame(time);
  return true;
}

EM_JS(void, msgbus_send, (const int msg_id), { window.msgbus.send(msg_id); });
#else
bool render_callback(const double time, void* user_data)
{
  return Editor::get()->render_frame(time);
}
#endif

Editor* Editor::s_instance = nullptr;

void Editor::init()
{
  if (s_instance != nullptr) {
    console::error("Editor already initialized, call shutdown() before reinitializing!");
    return;
  }

  s_instance = new Editor();

  input::InputManager::init();
  io::ResourceManager::init();
  renderer::Renderer::init();

  s_instance->m_scenes.emplace_back();

#ifdef EMSCRIPTEN
  emscripten_request_animation_frame_loop(render_callback, nullptr);
#endif
}

void Editor::prepare_refresh()
{
  renderer::Renderer::shutdown();
}

void Editor::refresh()
{
  renderer::Renderer::init();
}

void Editor::shutdown()
{
  if (s_instance == nullptr) {
    console::error("Renderer already shutdown, call init() before shutting down!");
    return;
  }

  renderer::Renderer::shutdown();
  io::ResourceManager::shutdown();
  input::InputManager::shutdown();

  delete s_instance;
  s_instance = nullptr;
}

Scene& Editor::scene()
{
  if (get()->m_scenes.empty()) {
    get()->m_scenes.emplace_back();
  }

  return get()->m_scenes[0];
}

void Editor::resize(const ivec2 size, const ivec2 offset, float dpr)
{
  if (size.x >= 900) {
    modify_ui_data("{\"components\":{\"background\":[0.0,0.0,0.0,1.0]}}");
  }

  for (auto& scene : get()->m_scenes) {
    scene.viewport.resize(size, offset, dpr);
  }
}

#ifndef EMSCRIPTEN
bool Editor::render_loop(const double time)
{
  return render_callback(time, nullptr);
}
#endif

void Editor::request_render(const RenderRequestOptions options)
{
  if (!get()->m_render_request.has_value()) {
    get()->m_render_request = options;
    return;
  }

  get()->m_render_request->update(options);
}

std::string Editor::ui_data()
{
  const Scene& scene = get()->scene();

  io::json::JSON data = io::json::JSON::object();

  if (scene.selection.size()) {

  } else {
    ArtboardComponent background = scene.get_background().get_component<ArtboardComponent>();

    data["background"] = background.color();
  }

  return data.dump();
}

void Editor::modify_ui_data(const std::string& data)
{
  io::json::JSON json = io::json::JSON::parse(data);

  if (!json.has("components")) {
    return;
  }

  io::json::JSON& components = json["components"];

  if (components.has("background")) {
    const vec4 color = components["background"].to_vec4();

    get()->scene().get_background().get_component<ArtboardComponent>().color(color);
  }

  console::log(data);

  request_render({false, false});
}

bool Editor::render_frame(const double time)
{
  if (!m_render_request.has_value())
    return false;

  if (m_render_request->frame_rate < 60 &&
      time - m_last_render_time < 1000.0 / m_render_request->frame_rate)
  {
    return false;
  }

  scene().render(m_render_request->ignore_cache);

  if (m_render_request->update_ui) {
#ifdef EMSCRIPTEN
    msgbus_send(0);
#endif
  }

  m_render_request.reset();

  return true;
}

}  // namespace graphick::editor
