/**
 * @file renderer/gpu/opengl/opengl.h
 * @brief This file includes the necessary OpenGL headers.
 */

#pragma once

#ifdef EMSCRIPTEN
#include <GLES3/gl32.h>
#else
#include <glad/glad.h>
#endif

#include "../../../utils/console.h"

#ifndef GK_CONF_DIST
/**
 * @brief Clears all OpenGL errors.
 */
static inline void glClearErrors() { while (glGetError() != GL_NO_ERROR); }

/**
 * @brief Logs an OpenGL error.
 *
 * @param function The function that caused the error.
 * @param line The line that caused the error.
 * @return True if there was no error, false otherwise.
 */
inline bool glLogCall(const char* function, int line) {
  while (GLenum error = glGetError()) {
    std::stringstream ss;
    ss << "OpenGL Error: " << error << " " << function << " " << line;
    graphick::console::error(ss.str());

    return false;
  }

  return true;
}

#ifdef _MSC_VER
#define glCall(x)                                                                                                                \
  glClearErrors();                                                                                                               \
  x;                                                                                                                             \
  if (!glLogCall(#x, __LINE__)) __debugbreak();
#else
#define glCall(x)                                                                                                                \
  glClearErrors();                                                                                                               \
  x;                                                                                                                             \
  glLogCall(#x, __LINE__)
#endif
#else
#define glCall(x) x
#endif
