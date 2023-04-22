#include "frame_buffer.h"

#ifdef EMSCRIPTEN
#include <GLES3/gl32.h>
#endif

FrameBuffer::FrameBuffer(ShaderManager& shaders, const vec2 size)
  : m_shaders(shaders), m_size({ 1.0f, 1.0f }) {}

void FrameBuffer::init(bool use_msaa, uint16_t msaa_samples) {
  m_use_msaa = use_msaa;
  m_msaa_samples = use_msaa ? msaa_samples : 1;

  // Create target texture object
  glGenTextures(1, &m_texture_object);
  glBindTexture(GL_TEXTURE_2D, m_texture_object);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)m_size.x, (GLsizei)m_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

  // First buffer is for storing the drawing, second is for processing the antialiased graphics
  glGenFramebuffers(1, &m_frame_buffer_rb);
  glGenFramebuffers(1, &m_frame_buffer_cb);

  // Renderbuffer for storing multisampling data
  glGenRenderbuffers(1, &m_render_buffer_object);
  glBindRenderbuffer(GL_RENDERBUFFER, m_render_buffer_object);
  glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_msaa_samples, GL_RGBA8, (GLsizei)m_size.x, (GLsizei)m_size.y);

  glBindFramebuffer(GL_FRAMEBUFFER, m_frame_buffer_rb);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_render_buffer_object);

  glBindFramebuffer(GL_FRAMEBUFFER, m_frame_buffer_cb);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture_object, 0);

  glBindTexture(GL_TEXTURE_2D, NULL);
  glBindFramebuffer(GL_FRAMEBUFFER, NULL);

  m_initialized = true;
}

void FrameBuffer::resize(const vec2 size) {
  if (!m_initialized) return;

  m_size = size;

  glBindTexture(GL_TEXTURE_2D, m_texture_object);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)m_size.x, (GLsizei)m_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

  glBindRenderbuffer(GL_RENDERBUFFER, m_render_buffer_object);
  glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_msaa_samples, GL_RGBA8, (GLsizei)size.x, (GLsizei)size.y);
}

void FrameBuffer::bind() const {
  if (!m_initialized) return;

  glBindFramebuffer(GL_FRAMEBUFFER, m_frame_buffer_rb);
}

void FrameBuffer::unbind() const {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::render() const {
  if (!m_initialized) return;

  // Blit the drawing into the color buffer, which adds antialiasing
  glBindFramebuffer(GL_READ_FRAMEBUFFER, m_frame_buffer_rb);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_frame_buffer_cb);
  glBlitFramebuffer(0, 0, (GLsizei)m_size.x, (GLsizei)m_size.y, 0, 0, (GLsizei)m_size.x, (GLsizei)m_size.y, GL_COLOR_BUFFER_BIT, GL_LINEAR);

  // Blit the color buffer into the default framebuffer, which is the screen
  glBindFramebuffer(GL_READ_FRAMEBUFFER, m_frame_buffer_rb);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glBlitFramebuffer(0, 0, (GLsizei)m_size.x, (GLsizei)m_size.y, 0, 0, (GLsizei)m_size.x, (GLsizei)m_size.y, GL_COLOR_BUFFER_BIT, GL_LINEAR);
}
