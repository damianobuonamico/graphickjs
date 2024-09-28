/**
 * @file renderer/gpu/opengl/gl_data.cpp
 * @brief The file contains the implementation of the OpenGL GPU data.
 */

#include "gl_data.h"

#include "opengl.h"

namespace graphick::renderer::GPU::GL {

/* -- Static methods -- */

/**
 * @brief Converts the texture format to the OpenGL internal format.
 *
 * @param format The texture format.
 * @return The OpenGL internal format.
 */
static constexpr GLint gl_internal_format(TextureFormat format) {
  switch (format) {
  case TextureFormat::R8:
    return GL_R8;
  case TextureFormat::R16UI:
    return GL_R16UI;
  case TextureFormat::R32F:
    return GL_R32F;
  case TextureFormat::R16F:
    return GL_R16F;
  case TextureFormat::RGBA8:
    return GL_RGBA8;
  case TextureFormat::RGBA8UI:
    return GL_RGBA8UI;
  case TextureFormat::RGBA16F:
    return GL_RGBA16F;
  default:
  case TextureFormat::RGBA32F:
    return GL_RGBA32F;
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
  case TextureFormat::R32F:
    return GL_RED;
  case TextureFormat::R16F:
    return GL_RED;
  case TextureFormat::R16UI:
    return GL_RED_INTEGER;
  case TextureFormat::RGBA8:
    return GL_RGBA;
  case TextureFormat::RGBA8UI:
    return GL_RGBA_INTEGER;
  case TextureFormat::RGBA16F:
  default:
  case TextureFormat::RGBA32F:
    return GL_RGBA;
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
  case TextureFormat::RGBA8UI:
    return GL_UNSIGNED_BYTE;
  case TextureFormat::R16UI:
    return GL_UNSIGNED_SHORT;
  case TextureFormat::R16F:
  case TextureFormat::RGBA16F:
    return GL_HALF_FLOAT;
  default:
  case TextureFormat::R32F:
    return GL_FLOAT;
  case TextureFormat::RGBA32F:
    return GL_FLOAT;
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
  case VertexAttrType::F16:
    return GL_HALF_FLOAT;
  case VertexAttrType::F32:
    return GL_FLOAT;
  case VertexAttrType::I8:
    return GL_BYTE;
  case VertexAttrType::I16:
    return GL_SHORT;
  case VertexAttrType::I32:
    return GL_INT;
  case VertexAttrType::U8:
    return GL_UNSIGNED_BYTE;
  case VertexAttrType::U32:
    return GL_UNSIGNED_INT;
  default:
  case VertexAttrType::U16:
    return GL_UNSIGNED_SHORT;
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
  case BufferTarget::Index:
    return GL_ELEMENT_ARRAY_BUFFER;
  default:
  case BufferTarget::Vertex:
    return GL_ARRAY_BUFFER;
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
  case BufferUploadMode::Static:
    return GL_STATIC_DRAW;
  case BufferUploadMode::Dynamic:
    return GL_DYNAMIC_DRAW;
  default:
  case BufferUploadMode::Stream:
    return GL_STREAM_DRAW;
  }
}

/* -- Methods -- */

GLenum gl_primitive(Primitive primitive) {
  switch (primitive) {
  case Primitive::Triangles:
    return GL_TRIANGLES;
  default:
  case Primitive::Lines:
    return GL_LINES;
  }
}

GLenum gl_blend_factor(BlendFactor factor) {
  switch (factor) {
  case BlendFactor::Zero:
    return GL_ZERO;
  case BlendFactor::One:
    return GL_ONE;
  case BlendFactor::SrcAlpha:
    return GL_SRC_ALPHA;
  case BlendFactor::OneMinusSrcAlpha:
    return GL_ONE_MINUS_SRC_ALPHA;
  case BlendFactor::DestAlpha:
    return GL_DST_ALPHA;
  case BlendFactor::OneMinusDestAlpha:
    return GL_ONE_MINUS_DST_ALPHA;
  default:
  case BlendFactor::DestColor:
    return GL_DST_COLOR;
  }
}

GLenum gl_blend_op(BlendOp op) {
  switch (op) {
  case BlendOp::Add:
    return GL_FUNC_ADD;
  case BlendOp::Subtract:
    return GL_FUNC_SUBTRACT;
  case BlendOp::ReverseSubtract:
    return GL_FUNC_REVERSE_SUBTRACT;
  case BlendOp::Min:
    return GL_MIN;
  default:
  case BlendOp::Max:
    return GL_MAX;
  }
}

GLenum gl_depth_func(DepthFunc func) {
  switch (func) {
  case DepthFunc::Always:
    return GL_ALWAYS;
  case DepthFunc::Less:
    return GL_LESS;
  default:
  case DepthFunc::Lequal:
    return GL_LEQUAL;
  }
}

GLenum gl_stencil_func(StencilFunc func) {
  switch (func) {
  case StencilFunc::Always:
    return GL_ALWAYS;
  case StencilFunc::Nequal:
    return GL_NOTEQUAL;
  default:
  case StencilFunc::Equal:
    return GL_EQUAL;
  }
}

/* -- GLUniform -- */

void GLUniform::set(const UniformData& data) const {
  // TODO: make better!
  if (std::holds_alternative<int>(data)) {
    glCall(glUniform1i(location, std::get<int>(data)));
  } else if (std::holds_alternative<uint32_t>(data)) {
    glCall(glUniform1ui(location, std::get<uint32_t>(data)));
  } else if (std::holds_alternative<ivec2>(data)) {
    ivec2 vec = std::get<ivec2>(data);
    glCall(glUniform2i(location, (GLint)vec.x, (GLint)vec.y));
  } else if (std::holds_alternative<float>(data)) {
    glCall(glUniform1f(location, std::get<float>(data)));
  } else if (std::holds_alternative<vec2>(data)) {
    vec2 vec = std::get<vec2>(data);
    glCall(glUniform2f(location, vec.x, vec.y));
  } else if (std::holds_alternative<vec4>(data)) {
    vec4 vec = std::get<vec4>(data);
    glCall(glUniform4f(location, vec.x, vec.y, vec.z, vec.w));
  } else if (std::holds_alternative<mat4>(data)) {
    mat4 mat = std::get<mat4>(data);
    glCall(glUniformMatrix4fv(location, 1, GL_TRUE, &mat[0].x));
  } else if (std::holds_alternative<std::vector<int>>(data)) {
    std::vector<int> ints = std::get<std::vector<int>>(data);
    glCall(glUniform1iv(location, (GLsizei)ints.size(), &ints[0]));
  } else if (std::holds_alternative<std::vector<vec4>>(data)) {
    std::vector<vec4> vecs = std::get<std::vector<vec4>>(data);
    glCall(glUniform4fv(location, (GLsizei)vecs.size(), &vecs[0].x));
  }
}

/* -- GLProgram -- */

void GLProgram::use() const { glCall(glUseProgram(gl_program)); }

/* -- GLVertexArray -- */

GLVertexArray::GLVertexArray() { glCall(glGenVertexArrays(1, &gl_vertex_array)); }

GLVertexArray::~GLVertexArray() { glCall(glDeleteVertexArrays(1, &gl_vertex_array)); }

void GLVertexArray::bind() const { glCall(glBindVertexArray(gl_vertex_array)); }

void GLVertexArray::unbind() const { glCall(glBindVertexArray(0)); }

void GLVertexArray::configure_attribute(const GLVertexAttribute attr, const VertexAttrDescriptor& desc) const {
  bind();

  GLuint attr_type = gl_type(desc.attr_type);

  if (desc.attr_class == VertexAttrClass::Int) {
    glCall(glVertexAttribIPointer(attr.attribute, (GLint)desc.size, attr_type, (GLsizei)desc.stride, (const void*)desc.offset));
  } else {
    bool normalized = desc.attr_class == VertexAttrClass::FloatNorm;
    glCall(glVertexAttribPointer(
      attr.attribute,
      (GLint)desc.size,
      attr_type,
      normalized,
      (GLsizei)desc.stride,
      (const void*)desc.offset
    ));
  }

  glCall(glVertexAttribDivisor(attr.attribute, desc.divisor));
  glCall(glEnableVertexAttribArray(attr.attribute));

  unbind();
}

/* -- GLTexture -- */

GLTexture::GLTexture(const TextureFormat format, const ivec2 size, const int sampling_flags, const void* data) :
  format(format), size(size), sampling_flags(sampling_flags) {
  glCall(glGenTextures(1, &gl_texture));
  bind(0);
  glTexImage2D(GL_TEXTURE_2D, 0, gl_internal_format(format), size.x, size.y, 0, gl_format(format), gl_type(format), data);

  set_sampling_flags(sampling_flags);
}

GLTexture::~GLTexture() { glCall(glDeleteTextures(1, &gl_texture)); }

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

void GLTexture::upload(const void* data, const irect region) const {
  bind(0);

  GLenum format = gl_format(this->format);
  GLenum type = gl_type(this->format);

  ivec2 origin = region.min;
  ivec2 size = region.size();

  glCall(glTexSubImage2D(GL_TEXTURE_2D, 0, (GLint)origin.x, (GLint)origin.y, (GLsizei)size.x, (GLsizei)size.y, format, type, data)
  );
}

// TODO: try to implement offset
void GLTexture::upload(const void* data, const size_t byte_size, const size_t offset) const {
  bind(0);

  GLenum format = gl_format(this->format);
  GLenum type = gl_type(this->format);

  ivec2 origin = ivec2::zero();
  // ivec2 size = ivec2(this->size.x, static_cast<int>(byte_size) / this->size.x + 1);
  ivec2 size = ivec2(this->size.x, this->size.y);

  glCall(glTexSubImage2D(GL_TEXTURE_2D, 0, (GLint)origin.x, (GLint)origin.y, (GLsizei)size.x, (GLsizei)size.y, format, type, data)
  );
}

/* -- GLFramebuffer -- */

GLFramebuffer::GLFramebuffer(const ivec2 size, const bool has_depth) :
  texture(TextureFormat::RGBA8, size, TextureSamplingFlagNone) {
  glCall(glGenFramebuffers(1, &gl_framebuffer));
  bind();
  glCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.gl_texture, 0));

  if (has_depth) {
    glCall(glGenRenderbuffers(1, &gl_renderbuffer));
    glCall(glBindRenderbuffer(GL_RENDERBUFFER, gl_renderbuffer));
    glCall(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, size.x, size.y));
    glCall(glBindRenderbuffer(GL_RENDERBUFFER, 0));

    glCall(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, gl_renderbuffer));
  }

  complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;

  unbind();
}

GLFramebuffer::~GLFramebuffer() {
  unbind();

  if (gl_renderbuffer) {
    glCall(glDeleteRenderbuffers(1, &gl_renderbuffer));
  }

  glCall(glDeleteFramebuffers(1, &gl_framebuffer));
}

GLFramebuffer::GLFramebuffer(GLFramebuffer&& other) noexcept :
  texture(std::move(other.texture)),
  gl_framebuffer(other.gl_framebuffer),
  gl_renderbuffer(other.gl_renderbuffer),
  has_depth(other.has_depth),
  complete(other.complete) { }

GLFramebuffer& GLFramebuffer::operator=(GLFramebuffer&& other) noexcept {
  texture = std::move(other.texture);
  gl_framebuffer = other.gl_framebuffer;
  gl_renderbuffer = other.gl_renderbuffer;
  has_depth = other.has_depth;
  complete = other.complete;

  other.gl_framebuffer = 0;
  other.gl_renderbuffer = 0;
  other.complete = false;

  return *this;
}

void GLFramebuffer::bind() const { glCall(glBindFramebuffer(GL_FRAMEBUFFER, gl_framebuffer)); }

void GLFramebuffer::unbind() const { glCall(glBindFramebuffer(GL_FRAMEBUFFER, 0)); }

/* -- GLBuffer -- */

GLBuffer::GLBuffer(const BufferTarget target, const BufferUploadMode mode, const size_t size, const void* data) :
  target(target), mode(mode), size(size) {
  glCall(glGenBuffers(1, &gl_buffer));

  GLenum buffer_target = gl_target(target);
  GLenum buffer_usage = gl_usage(mode);

  glCall(glBindBuffer(buffer_target, gl_buffer));
  glCall(glBufferData(buffer_target, size, data, buffer_usage));
}

GLBuffer::~GLBuffer() { glCall(glDeleteBuffers(1, &gl_buffer)); }

void GLBuffer::bind() const { glCall(glBindBuffer(gl_target(target), gl_buffer)); }

void GLBuffer::bind(const GLVertexArray& vertex_array) const {
  vertex_array.bind();
  bind();
  vertex_array.unbind();
}

void GLBuffer::unbind() const { glCall(glBindBuffer(gl_target(target), 0)); }

void GLBuffer::upload(const void* data, const size_t size, const size_t offset) const {
  bind();
  glCall(glBufferSubData(gl_target(target), (GLintptr)offset, (GLsizeiptr)size, data));
}

}
