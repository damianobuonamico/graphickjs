#include "frame_buffer.h"

#ifdef EMSCRIPTEN
#include <GLES3/gl32.h>
#endif

FrameBuffer::FrameBuffer(ShaderManager& shaders, const vec2& size)
  : m_shaders(shaders), m_size({ 1.0f, 1.0f }) {}

void FrameBuffer::init(bool use_msaa, uint16_t msaa_samples) {
  m_use_msaa = use_msaa;
  m_msaa_samples = msaa_samples;

#ifdef EMSCRIPTEN
  glGenTextures(1, &m_texture_object);
  glBindTexture(GL_TEXTURE_2D, m_texture_object);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  // TODO: try RGB
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_size.x, m_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glBindTexture(GL_TEXTURE_2D, NULL);

  glGenFramebuffers(1, &m_frame_buffer_rb);
  glGenFramebuffers(1, &m_frame_buffer_cb);

  glGenRenderbuffers(1, &m_render_buffer_object);
  glBindRenderbuffer(GL_RENDERBUFFER, m_render_buffer_object);
  glRenderbufferStorageMultisample(GL_RENDERBUFFER, msaa_samples, GL_RGBA8, m_size.x, m_size.y);

  glBindFramebuffer(GL_FRAMEBUFFER, m_frame_buffer_rb);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_render_buffer_object);

  glBindFramebuffer(GL_FRAMEBUFFER, m_frame_buffer_cb);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture_object, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, NULL);
#else
  glGenFramebuffers(1, &m_frame_buffer_object);
  glBindFramebuffer(GL_FRAMEBUFFER, m_frame_buffer_object);

  glGenTextures(1, &m_texture_object);
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_texture_object);
  glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, msaa_samples, GL_RGB, m_size.x, m_size.y, GL_TRUE);
  glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_texture_object, 0);

  glGenRenderbuffers(1, &m_render_buffer_object);
  glBindRenderbuffer(GL_RENDERBUFFER, m_render_buffer_object);
  glRenderbufferStorageMultisample(GL_RENDERBUFFER, msaa_samples, GL_DEPTH24_STENCIL8, m_size.x, m_size.y);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_render_buffer_object);

  // TODO: Error checking everywhere, maybe spdlog or custom alternative
  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE) {
    console::log("Framebuffer error: ", status);
  }

  glGenFramebuffers(1, &m_post_processing_frame_buffer_object);
  glBindFramebuffer(GL_FRAMEBUFFER, m_post_processing_frame_buffer_object);

  glGenTextures(1, &m_post_processing_texture_object);
  glBindTexture(GL_TEXTURE_2D, m_post_processing_texture_object);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_size.x, m_size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_post_processing_texture_object, 0);

  // TODO: Error checking everywhere, maybe spdlog or custom alternative
  status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE) {
    console::log("Framebuffer error: ", status);
  }
#endif

  float vertices[] = {
    1.0f, -1.0f, 1.0f, 0.0f,
    -1.0f, -1.0f, 0.0f, 0.0f,
    -1.0f,  1.0f, 0.0f, 1.0f,
    1.0f,  1.0f, 1.0f, 1.0f,
    1.0f, -1.0f, 1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f, 1.0f
  };

  glGenVertexArrays(1, &m_vertex_array_object);
  glBindVertexArray(m_vertex_array_object);

  glGenBuffers(1, &m_vertex_buffer_object);
  glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_object);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);

  if (use_msaa) {
    m_shaders.use("msaa");
  } else {
    m_shaders.use("fxaa");
  }
  m_shaders.set_uniform("uScreenTexture", 0);
  m_shaders.set_attribute("aPosition", 2, GL_FLOAT, GL_FALSE, 2 * sizeof(vec2), (const void*)(0));
  m_shaders.set_attribute("aTexCoords", 2, GL_FLOAT, GL_FALSE, 2 * sizeof(vec2), (const void*)(sizeof(vec2)));

  m_initialized = true;
}

void FrameBuffer::resize(const vec2& size) {
  if (!m_initialized) return;

  m_size = size;

#ifdef EMSCRIPTEN
  glBindTexture(GL_TEXTURE_2D, m_texture_object);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_size.x, m_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

  glBindRenderbuffer(GL_RENDERBUFFER, m_render_buffer_object);
  glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_msaa_samples, GL_RGBA8, size.x, size.y);
#else
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_texture_object);
  glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_msaa_samples, GL_RGB, m_size.x, m_size.y, GL_TRUE);

  glBindRenderbuffer(GL_RENDERBUFFER, m_render_buffer_object);
  glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_msaa_samples, GL_DEPTH24_STENCIL8, size.x, size.y);

  glBindTexture(GL_TEXTURE_2D, m_post_processing_texture_object);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_size.x, m_size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
#endif
}

void FrameBuffer::bind() const {
  if (!m_initialized) return;

  glBindFramebuffer(GL_FRAMEBUFFER, m_frame_buffer_object);
}

void FrameBuffer::unbind() const {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::render() const {
  if (!m_initialized) return;

#ifdef EMSCRIPTEN
  glBindFramebuffer(GL_READ_FRAMEBUFFER, m_frame_buffer_rb);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_frame_buffer_cb);
  glBlitFramebuffer(0, 0, m_size.x, m_size.y, 0, 0, m_size.x, m_size.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glBindVertexArray(m_vertex_array_object);
  glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_object);
  glBindTexture(GL_TEXTURE_2D, m_texture_object);
  glActiveTexture(GL_TEXTURE0);
#else
  glBindFramebuffer(GL_READ_FRAMEBUFFER, m_frame_buffer_object);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_post_processing_frame_buffer_object);
  glBlitFramebuffer(0, 0, m_size.x, m_size.y, 0, 0, m_size.x, m_size.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glBindVertexArray(m_vertex_array_object);
  glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_object);
  glBindTexture(GL_TEXTURE_2D, m_post_processing_texture_object);
  glActiveTexture(GL_TEXTURE0);
#endif

  if (m_use_msaa) {
    m_shaders.use("msaa");
  } else {
    m_shaders.use("fxaa");
  }

  glDrawArrays(GL_TRIANGLES, 0, 6);
}
