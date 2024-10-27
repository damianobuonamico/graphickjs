/**
 * @file editor.h
 * @brief Contains the main Graphick Editor class definition.
 */

#pragma once

#include "scene/scene.h"

#include "../math/vec2.h"

#include <optional>
#include <vector>

namespace graphick::editor {

/**
 * @brief Includes options to customize the render request.
 *
 * @struct RenderRequestOptions
 */
struct RenderRequestOptions {
  bool ignore_cache = false;  // Whether to ignore the cache and redraw everything.
  int frame_rate = 60;        // The frame rate to render at, if >= 60 it will render at the screen's refresh rate.

  RenderRequestOptions() = default;

  RenderRequestOptions(bool ignore_cache) : ignore_cache(ignore_cache) { }

  /**
   * @brief Updates the options with the latest request.
   *
   * @param options The options to update with.
   */
  inline void update(const RenderRequestOptions& options) {
    ignore_cache |= options.ignore_cache;
    frame_rate = options.frame_rate;
  }
};

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

#ifndef EMSCRIPTEN
  /**
   * @brief Callback for the render loop.
   *
   * This function should only be called by int main().
   *
   * @param time The current timestamp.
   * @return Whether the frame was rendered (and buffers should be swapped).
   */
  static bool render_loop(const double time);
#endif

  /**
   * @brief Creates a render request with the specified options.
   *
   * @param options The options to use, leave empty for default.
   */
  static void request_render(const RenderRequestOptions options = {});
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
   * @brief Renders a new frame if needed.
   *
   * This function gets called by the main loop at the screen's refresh rate.
   *
   * @param time The current timestamp.
   * @return Whether the frame was rendered.
   */
  bool render_frame(const double time);
private:
  std::vector<Scene> m_scenes;                           // The scenes managed by the editor.
  std::optional<RenderRequestOptions> m_render_request;  // The current render request.

  double m_last_render_time = 0.0;                       // The last render time.
private:
#ifdef EMSCRIPTEN
  friend int render_callback(const double time, void* user_data);
#else
  friend bool render_callback(const double time, void* user_data);
#endif
private:
  static Editor* s_instance;  // The editor's singleton instance.
};

}  // namespace graphick::editor
