#pragma once

#include "frame_buffer.h"
#include "geometry/geometry.h"
#include "../math/vec2.h"

class Renderer {
  enum class Antialiasing {
    None = 0,
    Hardware,
    FXAA,
    MSAA
  };

  struct RendererSettings {
    const Antialiasing antialiasing = Antialiasing::Hardware;
    const uint16_t msaa_samples = 4;
  };
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
  static void draw(const InstancedGeometry& geometry);
private:
  Renderer();
  ~Renderer() = default;

  void init_batch_renderer();
  void init_instance_renderer();

  void bind_batch_renderer();
  void bind_instance_renderer();

  void set_viewport(const vec2& position, float zoom);

  void begin_batch();
  void add_to_batch(const Geometry& geometry);
  void end_batch();
  void flush();

  void draw_instanced(const InstancedGeometry& geometry);
private:
  enum class RenderCall {
    Batch = 0,
    Instance,
    None
  };

  struct BatchedRendererData {
    GLuint vertex_array_object = 0;
    GLuint vertex_buffer_object = 0;
    GLuint index_buffer_object = 0;

    uint32_t vertex_count = 0;
    uint32_t index_count = 0;

    Vertex* vertex_buffer = nullptr;
    Vertex* vertex_buffer_ptr = nullptr;
    uint32_t* index_buffer = nullptr;
    uint32_t* index_buffer_ptr = nullptr;

    uint32_t max_vertex_buffer_size = (uint32_t)std::pow(2, 18);
    uint32_t max_vertex_count = max_vertex_buffer_size / sizeof(Vertex);
    uint32_t max_index_count = max_vertex_count * 2;
  };

  struct InstancedRendererData {
    GLuint vertex_array_object = 0;
    GLuint vertex_buffer_object = 0;
    GLuint index_buffer_object = 0;
    GLuint instance_buffer_object = 0;

    uint32_t instances;

    uint32_t max_vertex_count = 100;
    uint32_t max_index_count = max_vertex_count * 2;
    uint32_t max_instance_count = 100;

    uint32_t last_allocation_usage = 0;
  };
private:
  RendererSettings m_settings;

  BatchedRendererData m_data;
  InstancedRendererData m_instanced_data;

  ShaderManager m_shaders;
  FrameBuffer m_frame_buffer;
  RenderCall m_last_call = RenderCall::None;

  vec2 m_size;
private:
  static Renderer* s_instance;
};
