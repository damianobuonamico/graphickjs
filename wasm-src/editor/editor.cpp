#include "editor.h"

#include "input/input_manager.h"
#include "../renderer/renderer.h"
#include "../history/command_history.h"
#include "../utils/console.h"

#include <functional>
#include <emscripten/html5.h>

int render_callback(double time, void* user_data) {
  Editor::get()->render_frame(time);
  return 0;
}

Editor* Editor::s_instance = nullptr;
Viewport Editor::viewport{};

void Editor::init() {
  // TODO: Editor reinitialization
  assert(!s_instance);
  s_instance = new Editor();

  InputManager::init();
  Renderer::init();
  CommandHistory::init();

  get()->m_scene.load();
}

void Editor::shutdown() {
  CommandHistory::shutdown();
  Renderer::shutdown();
  InputManager::shutdown();

  delete s_instance;
}


void Editor::render() {
  emscripten_request_animation_frame(render_callback, nullptr);
}

void Editor::render_frame(double time) {
  Renderer::begin_frame(viewport.position(), viewport.zoom());

  m_scene.render();

  Renderer::end_frame();
}
