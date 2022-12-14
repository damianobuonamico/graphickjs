#pragma once

#include "shader_manager.h"
#include "../math/vec2.h"
#include "geometry.h"

class Renderer {
public:
  Renderer() {};

  static void init();
  static void resize(const int width, const int height);

  static void begin_frame(const float* position, const float zoom);
  static void end_frame();

  static void draw(const Geometry& geometry);
private:
  static void begin_batch();
  static void end_batch();
  static void flush();
private:
  struct RendererData {
    GLuint vertex_buffer_object = 0;
    GLuint index_buffer_object = 0;

    uint32_t vertex_count = 0;
    uint32_t index_count = 0;

    Vertex* vertex_buffer = nullptr;
    Vertex* vertex_buffer_ptr = nullptr;
    uint32_t* index_buffer = nullptr;
    uint32_t* index_buffer_ptr = nullptr;
  };

  static ShaderManager s_shaders;
  static RendererData s_data;
  static vec2 s_size;
};
