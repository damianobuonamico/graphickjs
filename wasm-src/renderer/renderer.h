#pragma once

#include "shader_manager.h"
#include "../math/vec2.h"
#include "geometry/geometry.h"

class Renderer {
public:
  Renderer(const Renderer&) = delete;
  Renderer(Renderer&&) = delete;

  static inline Renderer* get() { return s_instance; }

  static void init();
  static void shutdown();

  static void resize(const vec2& size);

  static void begin_frame(const vec2& position, float zoom);
  static void end_frame();

  static void draw(const Geometry& geometry);
private:
  Renderer() = default;
  ~Renderer() = default;

  void set_viewport(const vec2& position, float zoom);

  void begin_batch();
  void add_to_batch(const Geometry& geometry);
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
