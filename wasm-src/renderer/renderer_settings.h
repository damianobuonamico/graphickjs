/**
 * @file renderer/renderer_settings.h
 * @brief Contains the definition of the renderer settings.
 */

#pragma once

#include "../math/vec4.h"

namespace graphick::renderer {

/**
 * @brief Represents the global settings of the renderer.
 *
 * The default settings are stored in `renderer_settings.cpp`.
 */
struct RendererSettings {
  inline static double flattening_tolerance = 0.25;  // Pixel accuracy of path flattening.
  inline static double stroking_tolerance = 1e-4;    // Accuracy of the path stroking algorithm.
  inline static double tile_size = 16.0;             // The target pixel size of the tiles.

  inline static double ui_handle_size = 5.0;         // The typical size of the UI handles.
  inline static double ui_line_width = 1.0;          // The width of the UI lines.

  inline static vec4 ui_primary_color = vec4(
      0.22f, 0.76f, 0.95f, 1.0f);                    // Primary color of the UI.
  inline static vec4 ui_primary_transparent = vec4(
      0.22f, 0.76f, 0.95f, 0.25f);                   // Translucent primary color of the UI.
};

}  // namespace graphick::renderer
