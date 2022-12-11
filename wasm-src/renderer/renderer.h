#pragma once

#include "shader_manager.h"

class Renderer {
public:
  Renderer() {};
  
  static void init();
  static void resize(const int width, const int height);

  static void begin_frame(const float* position, const float zoom);
  static void end_frame();
private:
  static ShaderManager s_shaders;
};
