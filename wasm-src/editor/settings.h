/**
 * @file editor/settings.h
 * @brief This file contains the definition of the Settings struct.
 */

#pragma once

#include "../renderer/renderer_settings.h"

namespace graphick::editor {

/**
 * @brief Represents the global settings of the editor.
 *
 * The default settings are stored in `settings.cpp`.
 */
struct Settings {
  using Renderer = renderer::RendererSettings;

  struct Input {
    inline static float movement_threshold = 4.0f;  // The threshold for input movement.
    inline static float movement_threshold_multiplier[3] = {
        1.0f, 2.0f, 2.0f};  // The multiplier for different input devices (mouse, pen, touch).

    inline static float zoom_min = 0.02f;   // The minimum zoom level.
    inline static float zoom_max = 640.0f;  // The maximum zoom level.
    inline static float zoom_step = 0.25f;  // The zoom step.
    inline static float pan_step = 36.0f;   // The pan step.
  };
};

}  // namespace graphick::editor
