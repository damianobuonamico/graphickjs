#pragma once

class Editor {
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
  static Editor* s_instance;
};