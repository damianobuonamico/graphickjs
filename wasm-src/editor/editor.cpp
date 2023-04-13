#include "editor.h"

#include "input/input_manager.h"
#include "font_manager.h"
#include "../renderer/renderer.h"
#include "../history/command_history.h"
#include "../utils/console.h"
#include "../utils/debugger.h"
#include "../utils/json.h"

#include <functional>
#include <sstream>

#ifdef EMSCRIPTEN
#include <emscripten/html5.h>
#endif

int render_callback(double time, void* user_data) {
  Editor::get()->render_frame(time);
  return 0;
}

Editor* Editor::s_instance = nullptr;

void Editor::init() {
  // TODO: Editor reinitialization
  assert(!s_instance);
  s_instance = new Editor();

  DEBUGGER_INIT();
  InputManager::init();
  Renderer::init();
  CommandHistory::init();
  FontManager::init();

  s_instance->m_scenes.emplace_back();

  scene().load();
}

void Editor::shutdown() {
  FontManager::shutdown();
  CommandHistory::shutdown();
  Renderer::shutdown();
  InputManager::shutdown();
  DEBUGGER_SHUTDOWN();

  delete s_instance;
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

#ifdef EMSCRIPTEN
EM_JS(void, save_print, (const char* data, int len_data), {
  window._save(UTF8ToString(data, len_data));
  });
#endif



void Editor::save() {
  JSON data = JSON::object();
  JSON files = JSON::array();

  for (Scene& scene : s_instance->m_scenes) {
    files.append(scene.json());
  }

  data["version"] = "0.1.0";
  data["files"] = files;

  std::string dump = data.dump();

#ifdef EMSCRIPTEN
  save_print(dump.c_str(), dump.length());
#endif
}

void Editor::load(const char* data) {
  JSON json = JSON::load(data);

  if (json.has("files")) {
    JSON& files = json["files"];

    for (JSON& file : files.array_range()) {
      // TODO: support multiple scenes
      scene().load(file);
    }
  }
}

void Editor::render_frame(double time) {
  Renderer::begin_frame(scene().viewport.position(), scene().viewport.zoom());

  scene().render();

  Renderer::end_frame();
}
