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
  static double flattening_tolerance;  // The pixel accuracy of the path flattening algorithm.
  static double stroking_tolerance;    // The accuracy of the path stroking algorithm.
  static double culling_threshold;     // The threshold in pixels for culling paths.

  static double ui_handle_size;        // The size of the UI handles.
  static double ui_line_width;         // The width of the UI lines.

  static vec4 ui_primary_color;        // The primary color of the UI.
  static vec4 ui_primary_transparent;  // The primary color of the UI with transparency.
};

}  // namespace graphick::renderer
