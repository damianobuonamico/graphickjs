#pragma once

#include "scene/scene.h"

#include "../math/ivec2.h"

#include <vector>

namespace Graphick::Editor {

  class Editor {
  public:
    Editor(const Editor&) = delete;
    Editor(Editor&&) = delete;

    static void init();
    static void shutdown();

    static Scene& scene();

    static void resize(const ivec2 size, const ivec2 offset, float dpr);

#ifdef EMSCRIPTEN
    static void render();
#else
    static void render(bool is_main_loop = false);
#endif

  private:
    Editor() {}
    ~Editor() = default;

    static inline Editor* get() { return s_instance; }

    void render_frame(double time);
  private:
    std::vector<Scene> m_scenes;
  private:
    friend int render_callback(double time, void* user_data);
  private:
    static Editor* s_instance;
  };

}
