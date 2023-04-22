#pragma once

#include "frame_buffer.h"
#include "geometry/geometry.h"
#include "texture.h"
#include "../math/vec2.h"

struct RenderingOptions {
  float zoom;
  float facet_angle;
  Box viewport;
};

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

    const bool wireframe = false;

    const uint32_t max_vertex_buffer_size = (uint32_t)std::pow(2, 20);
    const uint32_t max_vertex_count = max_vertex_buffer_size / sizeof(Vertex);
    const uint32_t max_index_count = max_vertex_count * 2;

    const GLenum buffer_type = GL_STREAM_DRAW;

    int max_uniforms = 1024;
    float z_far = 1000.0f;
  };

  struct RendererStats {
    uint32_t batched_draw_calls = 0;
    uint32_t instanced_draw_calls = 0;
    uint32_t image_draw_calls = 0;
    uint32_t entities = 0;
    uint32_t vertex_count = 0;
    uint32_t index_count = 0;
    uint32_t instance_count = 0;

    void reset();
  };
public:
  Renderer(const Renderer&) = delete;
  Renderer(Renderer&&) = delete;

  static inline Renderer* get() { return s_instance; }
  static inline const RendererStats& stats() { return s_instance->m_stats; }

  static void init();
  static void shutdown();

  static void resize(const vec2& size, float dpr);

  static void begin_frame(const vec2& position, float zoom, float z_far = 1000.0f);
  static void end_frame();

  static void push_overlay_layer(const vec2& position);

  static void draw(const Geometry& geometry);
  // static void draw(const InstancedGeometry& geometry);
  static void draw(const Texture& texture);

  static void push_frame_buffer(vec2 size);
  static void pop_frame_buffer();
private:
  Renderer();
  ~Renderer() = default;

  void init_batch_renderer();
  void init_instance_renderer(bool create_vertex_array = true);

  void refresh_batch_renderer();
  void refresh_stencil_renderer();

  void bind_batch_renderer();
  void bind_instance_renderer();

  void set_viewport(const vec2& position, float zoom);

  bool can_batch(const Geometry& geometry);
  void begin_batch();
  void add_to_batch(const Geometry& geometry);
  void end_batch();
  void flush();

  // void draw_instanced(const InstancedGeometry& geometry);
private:
  enum class RenderCall {
    Batch = 0,
    Instance,
    Image,
    None
  };

  struct BatchData {


  };

  struct BatchRendererData {
    // rgb: color, a: z-index
    std::vector<vec4> uniforms;

    uint32_t vertex_count = 0;
    uint32_t index_count = 0;

    GLenum primitive = 0;

    GLuint vertex_array_object = 0;
    GLuint vertex_buffer_object = 0;
    GLuint index_buffer_object = 0;

    GLVertex* vertex_buffer = nullptr;
    GLVertex* vertex_buffer_ptr = nullptr;
    uint32_t* index_buffer = nullptr;
    uint32_t* index_buffer_ptr = nullptr;
  };

  struct BatchedRendererData {
    GLuint vertex_array_object = 0;
    GLuint vertex_buffer_object = 0;
    GLuint index_buffer_object = 0;

    uint32_t vertex_count = 0;
    uint32_t index_count = 0;
    GLenum primitive = 0;

    Vertex* vertex_buffer = nullptr;
    Vertex* vertex_buffer_ptr = nullptr;
    uint32_t* index_buffer = nullptr;
    uint32_t* index_buffer_ptr = nullptr;

    uint32_t max_vertex_buffer_size = (uint32_t)std::pow(2, 22);
    uint32_t max_vertex_count = max_vertex_buffer_size / sizeof(GLVertex);
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
  RendererStats m_stats;

  BatchedRendererData m_data;
  InstancedRendererData m_instanced_data;
  BatchRendererData m_batch_data;

  ShaderManager m_shaders;
  FrameBuffer m_frame_buffer;
  FrameBuffer* m_current_frame_buffer = nullptr;
  RenderCall m_last_call = RenderCall::None;

  vec2 m_size;
  float m_dpr;
private:
  static Renderer* s_instance;
};
