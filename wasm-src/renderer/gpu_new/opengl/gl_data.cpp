/**
 * @file renderer/gpu/opengl/gl_data.cpp
 * @brief The file contains the implementation of the OpenGL GPU data.
 */

#include "gl_data.h"

#include "opengl.h"

namespace graphick::renderer::GPU::GL {

  /**
   * @brief Converts the texture format to the OpenGL internal format.
   *
   * @param format The texture format.
   * @return The OpenGL internal format.
   */
  static constexpr GLint gl_internal_format(TextureFormat format) {
    switch (format) {
    case TextureFormat::R8: return GL_R8;
    case TextureFormat::R16UI: return GL_R16UI;
    case TextureFormat::R32F: return GL_R32F;
    case TextureFormat::R16F: return GL_R16F;
    case TextureFormat::RGBA8: return GL_RGBA8;
    case TextureFormat::RGBA8UI: return GL_RGBA8UI;
    case TextureFormat::RGBA16F: return GL_RGBA16F;
    default:
    case TextureFormat::RGBA32F: return GL_RGBA32F;
    }
  }

  /**
   * @brief Converts the texture format to the OpenGL format.
   *
   * @param format The texture format.
   * @return The OpenGL format.
   */
  static constexpr GLuint gl_format(TextureFormat format) {
    switch (format) {
    case TextureFormat::R8:
    case TextureFormat::R32F: return GL_RED;
    case TextureFormat::R16F: return GL_RED;
    case TextureFormat::R16UI: return GL_RED_INTEGER;
    case TextureFormat::RGBA8: return GL_RGBA;
    case TextureFormat::RGBA8UI: return GL_RGBA_INTEGER;
    case TextureFormat::RGBA16F:
    default:
    case TextureFormat::RGBA32F: return GL_RGBA;
    }
  }

  /**
   * @brief Converts the texture format to the OpenGL type.
   *
   * @param format The texture format.
   * @return The OpenGL type.
   */
  static constexpr GLuint gl_type(TextureFormat format) {
    switch (format) {
    case TextureFormat::R8:
    case TextureFormat::RGBA8:
    case TextureFormat::RGBA8UI: return GL_UNSIGNED_BYTE;
    case TextureFormat::R16UI: return GL_UNSIGNED_SHORT;
    case TextureFormat::R16F:
    case TextureFormat::RGBA16F: return GL_HALF_FLOAT;
    default:
    case TextureFormat::R32F: return GL_FLOAT;
    case TextureFormat::RGBA32F: return GL_FLOAT;
    }
  }

  /**
   * @brief Converts the vertex attribute type to the OpenGL type.
   *
   * @param format The vertex attribute type.
   * @return The OpenGL type.
   */
  static constexpr GLuint gl_type(VertexAttrType format) {
    switch (format) {
    case VertexAttrType::F32: return GL_FLOAT;
    case VertexAttrType::I8: return GL_BYTE;
    case VertexAttrType::I16: return GL_SHORT;
    case VertexAttrType::I32: return GL_INT;
    case VertexAttrType::U8: return GL_UNSIGNED_BYTE;
    case VertexAttrType::U32: return GL_UNSIGNED_INT;
    default:
    case VertexAttrType::U16: return GL_UNSIGNED_SHORT;
    }
  }

  /**
   * @brief Converts the buffer target to the OpenGL target.
   *
   * @param target The buffer target.
   * @return The OpenGL target.
   */
  static constexpr GLenum gl_target(BufferTarget target) {
    switch (target) {
    case BufferTarget::Index: return GL_ELEMENT_ARRAY_BUFFER;
    default:
    case BufferTarget::Vertex: return GL_ARRAY_BUFFER;
    }
  }

  /**
   * @brief Converts the buffer upload mode to the OpenGL usage.
   *
   * @param usage The buffer upload mode.
   * @return The OpenGL usage.
   */
  static constexpr GLenum gl_usage(BufferUploadMode usage) {
    switch (usage) {
    case BufferUploadMode::Static: return GL_STATIC_DRAW;
    case BufferUploadMode::Dynamic: return GL_DYNAMIC_DRAW;
    default:
    case BufferUploadMode::Stream: return GL_STREAM_DRAW;
    }
  }

  /* -- GLVertexArray -- */

  GLVertexArray::GLVertexArray() {
    glCall(glGenVertexArrays(1, &gl_vertex_array));
  }

  GLVertexArray::~GLVertexArray() {
    glCall(glDeleteVertexArrays(1, &gl_vertex_array));
  }

  void GLVertexArray::bind() const {
    glCall(glBindVertexArray(gl_vertex_array));
  }

  void GLVertexArray::unbind() const {
    glCall(glBindVertexArray(0));
  }

  void GLVertexArray::configure_attribute(const GLVertexAttribute attr, const VertexAttrDescriptor& desc) const {
    bind();

    GLuint attr_type = gl_type(desc.attr_type);

    if (desc.attr_class == VertexAttrClass::Int) {
      glCall(glVertexAttribIPointer(attr.attribute, (GLint)desc.size, attr_type, (GLsizei)desc.stride, (const void*)desc.offset));
    } else {
      bool normalized = desc.attr_class == VertexAttrClass::FloatNorm;
      glCall(glVertexAttribPointer(attr.attribute, (GLint)desc.size, attr_type, normalized, (GLsizei)desc.stride, (const void*)desc.offset));
    }

    glCall(glVertexAttribDivisor(attr.attribute, desc.divisor));
    glCall(glEnableVertexAttribArray(attr.attribute));

    unbind();
  }

  /* -- GLTexture -- */

  GLTexture::GLTexture(const TextureFormat format, const ivec2 size, const int sampling_flags, const void* data)
    : format(format), size(size), sampling_flags(sampling_flags)
  {
    glCall(glGenTextures(1, &gl_texture));
    bind(0);
    glTexImage2D(
      GL_TEXTURE_2D, 0,
      gl_internal_format(format),
      size.x, size.y, 0,
      gl_format(format), gl_type(format),
      data
    );

    set_sampling_flags(sampling_flags);
  }

  GLTexture::~GLTexture() {
    glCall(glDeleteTextures(1, &gl_texture));
  }

  void GLTexture::bind(GLuint unit) const {
    glCall(glActiveTexture(GL_TEXTURE0 + unit));
    glCall(glBindTexture(GL_TEXTURE_2D, gl_texture));
  }

  void GLTexture::unbind(GLuint unit) const {
    glCall(glActiveTexture(GL_TEXTURE0 + unit));
    glCall(glBindTexture(GL_TEXTURE_2D, 0));
  }

  void GLTexture::set_sampling_flags(const int flags) {
    bind(0);

    glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, flags & TextureSamplingFlagNearestMin ? GL_NEAREST : GL_LINEAR));
    glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, flags & TextureSamplingFlagNearestMag ? GL_NEAREST : GL_LINEAR));
    glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, flags & TextureSamplingFlagRepeatU ? GL_REPEAT : GL_CLAMP_TO_EDGE));
    glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, flags & TextureSamplingFlagRepeatV ? GL_REPEAT : GL_CLAMP_TO_EDGE));
  }

  /* -- GLBuffer -- */

  GLBuffer::GLBuffer(const BufferTarget target, const BufferUploadMode mode, const size_t size, const void* data)
    : target(target), mode(mode), size(size)
  {
    glCall(glGenBuffers(1, &gl_buffer));

    GLenum buffer_target = gl_target(target);
    GLenum buffer_usage = gl_usage(mode);

    glCall(glBindBuffer(buffer_target, gl_buffer));
    glCall(glBufferData(buffer_target, size, data, buffer_usage));
  }

  GLBuffer::~GLBuffer() {
    glCall(glDeleteBuffers(1, &gl_buffer));
  }

  void GLBuffer::bind() const {
    glCall(glBindBuffer(gl_target(target), gl_buffer));
  }

  void GLBuffer::bind(const GLVertexArray& vertex_array) const {
    vertex_array.bind();
    bind();
    vertex_array.unbind();
  }

  void GLBuffer::unbind() const {
    glCall(glBindBuffer(gl_target(target), 0));
  }

}
