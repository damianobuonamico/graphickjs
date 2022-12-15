#pragma once

#include "viewport.h"

class Editor {
public:
  static Viewport viewport;
public:
  Editor(const Editor&) = delete;
  Editor(Editor&&) = delete;

  inline static Editor* get() { return s_instance; }

  static void init();
  static void shutdown();
private:
  Editor() = default;
  ~Editor() = default;
private:
private:
  static Editor* s_instance;
};