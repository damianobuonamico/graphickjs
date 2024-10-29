/**
 * @file renderer/renderer_settings.cpp
 * @brief This file contains the default settings of the renderer.
 */

#include "renderer_settings.h"

namespace graphick::renderer {

double RendererSettings::flattening_tolerance = 0.25;

double RendererSettings::ui_handle_size = 5.0;
double RendererSettings::ui_line_width = 1.0;

vec4 RendererSettings::ui_primary_color = vec4(0.22f, 0.76f, 0.95f, 1.0f);

}  // namespace graphick::renderer
