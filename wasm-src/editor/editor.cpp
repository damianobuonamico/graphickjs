#include "editor.h"

#include "scene/scene.h"
#include "input/input_manager.h"

#include "../renderer/renderer.h"

#include "../history/command_history.h"
#include "../history/history.h"

#include "../utils/resource_manager.h"
#include "../utils/debugger.h"
#include "../utils/console.h"

#ifdef EMSCRIPTEN
#include <emscripten/html5.h>
#endif

namespace Graphick::Editor {

  int render_callback(double time, void* user_data) {
    Editor::get()->render_frame(time);
    return 0;
  }

  Editor* Editor::s_instance = nullptr;

  // TODO: fix all todos (one day)
  void Editor::init() {
    // TODO: Editor reinitialization
    if (s_instance != nullptr) {
      console::error("Editor already initialized, call shutdown() before reinitializing!");
      return;
    }

    s_instance = new Editor();

    Input::InputManager::init();
    Utils::ResourceManager::init();
    Renderer::Renderer::init();
    History::CommandHistory::init();
    History::History::init();
    // FontManager::init();

    GK_DEBUGGER_INIT();

    s_instance->m_scenes.emplace_back();

    // scene().load();
  }

  void Editor::prepare_refresh() {
    Renderer::Renderer::shutdown();
  }

  void Editor::refresh() {
    Renderer::Renderer::init();
  }

  void Editor::shutdown() {
    if (s_instance == nullptr) {
      console::error("Renderer already shutdown, call init() before shutting down!");
      return;
    }

    GK_DEBUGGER_SHUTDOWN();

    // FontManager::shutdown();
    History::History::shutdown();
    History::CommandHistory::shutdown();
    Renderer::Renderer::shutdown();
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
    // TODO: add emscripten loop or something similar
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
