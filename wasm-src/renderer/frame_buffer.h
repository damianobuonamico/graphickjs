#pragma once

#include "../math/vec2.h"
#include "shader_manager.h"

class FrameBuffer {
public:
  FrameBuffer(ShaderManager& shaders, const vec2 size);

  void init(bool use_msaa, uint16_t msaa_samples);
  void resize(const vec2 size);
  void bind() const;
  void unbind() const;
  void render() const;
private:
  ShaderManager& m_shaders;
  vec2 m_size;

  GLuint m_frame_buffer_rb = 0;
  GLuint m_frame_buffer_cb = 0;

  GLuint m_frame_buffer_object = 0;
  GLuint m_render_buffer_object = 0;
  GLuint m_texture_object = 0;
  GLuint m_vertex_array_object = 0;
  GLuint m_vertex_buffer_object = 0;

  GLuint m_post_processing_frame_buffer_object = 0;
  GLuint m_post_processing_texture_object = 0;

  uint16_t m_msaa_samples = 0;
  bool m_use_msaa = false;
  bool m_initialized = false;
};
