/**
 * @file editor.h
 * @brief Contains the main Graphick Editor class definition.
 */

#pragma once

#include "scene/scene.h"

#include "../math/vec2.h"

#include <vector>

namespace Graphick::Editor {

  /**
   * @brief The main Graphick Editor singleton.
   *
   * This class is responsible for managing and rendering the scenes.
   *
   * @class Editor
   */
  class Editor {
  public:
    /**
     * @brief Deleted copy and move constructors.
     */
    Editor(const Editor&) = delete;
    Editor(Editor&&) = delete;

    /**
     * @brief Initializes the whole editor.
     */
    static void init();

    /**
     * @brief Prepares the editor for a refresh.
     */
    static void prepare_refresh();

    /**
     * @brief Refreshes the editor.
     *
     * A refresh is a reinitialization of the editor's renderer.
     */
    static void refresh();

    /**
     * @brief Shuts down the whole editor.
     */
    static void shutdown();

    /**
     * @brief Returns the currently active scene.
     *
     * @return The current scene.
     */
    static Scene& scene();

    /**
     * @brief Resizes the editor.
     *
     * This method dispatches the resize event to all of the scenes (and canvases) in the editor.
     *
     * @param size The new size of the editor.
     * @param offset The new offset of the editor.
     * @param dpr The new device pixel ratio of the editor.
     */
    static void resize(const ivec2 size, const ivec2 offset, float dpr);

#ifdef EMSCRIPTEN
    /**
     * @brief Renders the editor's canvases.
     */
    static void render();
#else
    /**
     * @brief Renders the editor's canvases.
     *
     * @param is_main_loop Whether the render is the main loop or not.
     */
    static void render(bool is_main_loop = false);
#endif
  private:
    /**
     * @brief Default constructor and destructor.
     */
    Editor() = default;
    ~Editor() = default;

    /**
     * @brief Returns the editor's instance.
     *
     * @return The editor's instance.
     */
    static inline Editor* get() { return s_instance; }

    /**
     * @brief Renders a new frame.
     *
     * @param time The current timestamp.
     */
    void render_frame(double time);
  private:
    std::vector<Scene> m_scenes;    /* The scenes managed by the editor. */
  private:
    friend int render_callback(double time, void* user_data);
  private:
    static Editor* s_instance;      /* The editor's singleton instance. */
  };

}
