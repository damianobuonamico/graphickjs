#pragma once

#include "viewport.h"
#include "../values/ordered_map.h"
#include "../utils/console.h"

class Editor {
public:
  static Viewport viewport;
public:
  Editor(const Editor&) = delete;
  Editor(Editor&&) = delete;

  static inline Editor* get() { return s_instance; }

  static void init();
  static void shutdown();
private:
  Editor() {}
  ~Editor() = default;
private:
private:
  static Editor* s_instance;
};