/**
 * @file defines.h
 * @brief The file contains the definition of the global constants.
 */

#pragma once

#if !defined(GK_CONF_DIST) && !defined(EMSCRIPTEN)
#  define GK_DEBUG 1
#endif

#define GK_SMALL_BUFFER_SIZE 16384
#define GK_BUFFER_SIZE 131072
#define GK_LARGE_BUFFER_SIZE 524288
#define GK_CURVES_TEXTURE_SIZE 256
#define GK_GRADIENTS_TEXTURE_WIDTH 64
#define GK_GRADIENTS_TEXTURE_HEIGHT 64
