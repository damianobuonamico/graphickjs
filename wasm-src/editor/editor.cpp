#include "editor.h"

#include "../input/input_manager.h"
#include "../renderer/renderer.h"
#include "../history/command_history.h"
#include "../history/change_vec2_command.h"

Editor* Editor::s_instance = nullptr;
Viewport Editor::viewport{};

void Editor::init() {
  // TODO: Editor reinitialization
  assert(!s_instance);
  s_instance = new Editor();

  InputManager::init();
  Renderer::init();
  CommandHistory::init();
}

void Editor::shutdown() {
  CommandHistory::shutdown();
  Renderer::shutdown();
  InputManager::shutdown();

  delete s_instance;
}