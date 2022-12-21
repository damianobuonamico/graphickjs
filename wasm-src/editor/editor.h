#pragma once

#include "viewport.h"
#include "scene/scene.h"
#include "../values/ordered_map.h"
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

  static void render();
private:
  Editor() {}
  ~Editor() = default;

  void render_frame(double time);
private:
  friend int render_callback(double time, void* user_data);
private:
  static Editor* s_instance;
};