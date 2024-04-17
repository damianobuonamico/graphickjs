/**
 * @file editor.cpp
 * @brief Contains the main Graphick Editor class implementation.
 *
 * @todo fix all todos (one day)
 * @todo editor reinitialization
 * @todo add emscripten main loop or something similar
 */

#include "editor.h"

#include "scene/scene.h"
#include "input/input_manager.h"

#include "../renderer/renderer_new.h"

#include "../utils/resource_manager.h"
#include "../utils/debugger.h"
#include "../utils/console.h"

#ifdef EMSCRIPTEN
#include <emscripten/html5.h>
#endif

namespace graphick::editor {

  int render_callback(double time, void* user_data) {
    Editor::get()->render_frame(time);
    return 0;
  }

  Editor* Editor::s_instance = nullptr;

  void Editor::init() {
    if (s_instance != nullptr) {
      console::error("Editor already initialized, call shutdown() before reinitializing!");
      return;
    }

    s_instance = new Editor();

    Input::InputManager::init();
    Utils::ResourceManager::init();
    renderer::Renderer::init();

    GK_DEBUGGER_INIT();

    s_instance->m_scenes.emplace_back();
  }

  void Editor::prepare_refresh() {
    renderer::Renderer::shutdown();
  }

  void Editor::refresh() {
    renderer::Renderer::init();
  }

  void Editor::shutdown() {
    if (s_instance == nullptr) {
      console::error("Renderer already shutdown, call init() before shutting down!");
      return;
    }

    GK_DEBUGGER_SHUTDOWN();

    renderer::Renderer::shutdown();
    Utils::ResourceManager::shutdown();
    Input::InputManager::shutdown();

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

  void Editor::render(
#ifdef EMSCRIPTEN
#else
    bool is_main_loop
#endif
  ) {
#ifdef EMSCRIPTEN
    emscripten_request_animation_frame(render_callback, nullptr);
#else
    if (is_main_loop) {
      render_callback(0, nullptr);
    }
#endif
  }

  void Editor::render_frame(double time) {
    OPTICK_EVENT();

    scene().render();
  }

}
