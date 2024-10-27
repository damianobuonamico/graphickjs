/**
 * @file renderer/gpu/device.h
 * @brief Contains the main GPU device.
 *
 * Currently the only backend is OpenGL 3.0+, so everything falls back to that.
 *
 * @note If later on we decide to add more backends, effort must be made to extract all platform agnostic structs out of the
 * gpu/gl directory.
 */

#pragma once

#if defined(GK_GLES3) || defined(GK_GL3)
#include "opengl/gl_device.h"
#else
#include "opengl/gl_device.h"
#endif

namespace graphick::renderer::GPU {

/**
 * @brief The device is the main entry point for the GPU rendering. It is responsible for creating and managing the GPU resources.
 *
 * @class Device
 */
using Device = GL::GLDevice;

}  // namespace graphick::renderer::GPU
