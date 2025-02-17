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
};

}  // namespace graphick::editor
