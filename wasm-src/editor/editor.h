#pragma once

#include "scene/scene.h"
#include "../utils/console.h"

#include <vector>

class Editor {
public:
  Editor(const Editor&) = delete;
  Editor(Editor&&) = delete;

  static inline Editor* get() { return s_instance; }
  static inline Scene& scene() { return s_instance->m_scenes[0]; }

  static void init();
  static void shutdown();

#ifdef EMSCRIPTEN
  static void render();
#else
  static void render(bool is_main_loop = false);
#endif

  static void save();
  static void load(const char* data);
private:
  Editor() {}
  ~Editor() = default;

  void render_frame(double time);
private:
  std::vector<Scene> m_scenes;
private:
  friend int render_callback(double time, void* user_data);
private:
  static Editor* s_instance;
};