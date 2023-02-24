#pragma once

#include "viewport.h"
#include "scene/scene.h"
#include "../utils/console.h"

class Editor {
public:
  static Viewport viewport;
  static Scene scene;
public:
  Editor(const Editor&) = delete;
  Editor(Editor&&) = delete;

  static inline Editor* get() { return s_instance; }

  static void init();
  static void shutdown();

#ifdef EMSCRIPTEN
  static void render();
#else
  static void render(bool is_main_loop = false);
#endif
private:
  Editor() {}
  ~Editor() = default;

  void render_frame(double time);
private:
  friend int render_callback(double time, void* user_data);
private:
  static Editor* s_instance;
};