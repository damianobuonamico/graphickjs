/**
 * @file editor.cpp
 * @brief Contains the main Graphick Editor class implementation.
 *
 * @todo fix all todos (one day)
 * @todo editor reinitialization
 * @todo add emscripten main loop or something similar
 */

#include "editor.h"

#include "input/input_manager.h"
#include "scene/scene.h"

#include "../renderer/renderer.h"

#include "../utils/console.h"
#include "../utils/resource_manager.h"

#ifdef EMSCRIPTEN
#include <emscripten/html5.h>
#endif

namespace graphick::editor {

#ifdef EMSCRIPTEN
int render_callback(const double time, void* user_data) {
  Editor::get()->render_frame(time);
  return 1;
}
#else
bool render_callback(const double time, void* user_data) { return Editor::get()->render_frame(time); }
#endif

Editor* Editor::s_instance = nullptr;

void Editor::init() {
  if (s_instance != nullptr) {
    console::error("Editor already initialized, call shutdown() before reinitializing!");
    return;
  }

  s_instance = new Editor();

  input::InputManager::init();
  utils::ResourceManager::init();
  renderer::Renderer::init();

  s_instance->m_scenes.emplace_back();

#ifdef EMSCRIPTEN
  emscripten_request_animation_frame_loop(render_callback, nullptr);
#endif
}

void Editor::prepare_refresh() { renderer::Renderer::shutdown(); }

void Editor::refresh() { renderer::Renderer::init(); }

void Editor::shutdown() {
  if (s_instance == nullptr) {
    console::error("Renderer already shutdown, call init() before shutting down!");
    return;
  }

  renderer::Renderer::shutdown();
  utils::ResourceManager::shutdown();
  input::InputManager::shutdown();

  delete s_instance;
  s_instance = nullptr;
}

Scene& Editor::scene() {
  if (get()->m_scenes.empty()) {
    get()->m_scenes.emplace_back();
  }

  return get()->m_scenes[0];
}

void Editor::resize(const ivec2 size, const ivec2 offset, float dpr) {
  for (auto& scene : get()->m_scenes) {
    scene.viewport.resize(size, offset, dpr);
  }
}

#ifndef EMSCRIPTEN
bool Editor::render_loop(const double time) { return render_callback(time, nullptr); }
#endif

void Editor::request_render(const RenderRequestOptions options) {
  if (!get()->m_render_request.has_value()) {
    get()->m_render_request = options;
    return;
  }

  get()->m_render_request->update(options);
}

bool Editor::render_frame(const double time) {
  if (!m_render_request.has_value()) return false;

  if (m_render_request->frame_rate < 60 && time - m_last_render_time < 1000.0 / m_render_request->frame_rate) {
    return false;
  }

  scene().render(m_render_request->ignore_cache);

  m_render_request.reset();

  return true;
}

}  // namespace graphick::editor
