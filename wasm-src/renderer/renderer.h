#pragma once

#include "shader_manager.h"
#include "../math/vec2.h"
#include "geometry.h"

class Renderer {
public:
  Renderer(const Renderer&) = delete;
  Renderer(Renderer&&) = delete;

  inline static Renderer* get() { return s_instance; }

  static void init();
  static void shutdown();

  void resize(const int width, const int height);

  void begin_frame(const float* position, const float zoom);
  void end_frame();

  void draw(const Geometry& geometry);
private:
  Renderer() = default;
  ~Renderer() = default;

  void begin_batch();
  void end_batch();
  void flush();
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

  ShaderManager m_shaders;
  RendererData m_data;
  vec2 m_size;
private:
  static Renderer* s_instance;
};
