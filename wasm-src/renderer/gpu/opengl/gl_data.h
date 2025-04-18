/**
 * @file renderer/gpu/opengl/gl_data.h
 * @brief The file contains the definition of the OpenGL GPU data.
 */

#pragma once

#include "../gpu_data.h"

#include "../../../math/rect.h"

#include <vector>

namespace graphick::renderer::GPU::GL {

/**
 * @brief OpenGL specific types.
 */
typedef unsigned int GLuint;
typedef int GLint;
typedef unsigned int GLenum;

/**
 * @brief Converts the primitive to the OpenGL primitive.
 *
 * @param primitive The primitive.
 * @return The OpenGL primitive.
 */
GLenum gl_primitive(Primitive primitive);

/**
 * @brief Converts the blend factor to the OpenGL blend factor.
 *
 * @param factor The blend factor.
 * @return The OpenGL blend factor.
 */
GLenum gl_blend_factor(BlendFactor factor);

/**
 * @brief Converts the blend operation to the OpenGL blend operation.
 *
 * @param op The blend operation.
 * @return The OpenGL blend operation.
 */
GLenum gl_blend_op(BlendOp op);

/**
 * @brief Converts the depth function to the OpenGL depth function.
 *
 * @param func The depth function.
 * @return The OpenGL depth function.
 */
GLenum gl_depth_func(DepthFunc func);

/**
 * @brief Converts the stencil function to the OpenGL stencil function.
 *
 * @param func The stencil function.
 * @return The OpenGL stencil function.
 */
GLenum gl_stencil_func(StencilFunc func);

/**
 * @brief The OpenGL uniform object.
 */
struct GLUniform {
  GLint location;

  bool operator==(const GLUniform& other) const
  {
    return location == other.location;
  }

  /**
   * @brief Sets the data to the uniform.
   *
   * @param data The data to set.
   */
  void set(const UniformData& data) const;
};

/**
 * @brief The OpenGL texture uniform object.
 */
struct GLTextureUniform {
  GLUniform uniform;  // The uniform.
  GLuint unit;        // The texture unit.
};

/**
 * @brief The OpenGL array of textures uniform object.
 */
struct GLTexturesUniform {
  GLUniform uniform;          // The uniform.
  std::vector<GLuint> units;  // The texture units.
};

/**
 * @brief The OpenGL program object.
 */
struct GLProgram {
  GLuint gl_program;                // The OpenGL underlying program.
  GLuint vertex;                    // The vertex shader.
  GLuint fragment;                  // The fragment shader.

  std::vector<GLUniform> textures;  // Mapping from texture unit number to uniform location.

  /**
   * @brief Binds the program to the pipeline.
   */
  void use() const;
};

/**
 * @brief The OpenGL vertex attribute.
 */
struct GLVertexAttribute {
  GLuint attribute;  // The underlying vertex attribute.
};

/**
 * @brief The OpenGL vertex array object.
 */
struct GLVertexArray {
  GLuint gl_vertex_array;  // The OpenGL underlying vertex array.

  GLVertexArray();
  ~GLVertexArray();

  /**
   * @brief Binds the vertex array.
   */
  void bind() const;

  /**
   * @brief Unbinds the vertex array.
   */
  void unbind() const;

  /**
   * @brief Configures the given vertex attribute.
   *
   * @param attr The attribute to configure.
   * @param desc The attribute descriptor.
   */
  void configure_attribute(const GLVertexAttribute attr, const VertexAttrDescriptor& desc) const;
};

/**
 * @brief The OpenGL texture object.
 */
struct GLTexture {
  TextureFormat format;  // The texture format.
  GLuint gl_texture;     // The OpenGL underlying texture.
  ivec2 size;            // The size of the texture.

  int sampling_flags;    // The texture sampling flags.

  GLTexture(const TextureFormat format,
            const ivec2 size,
            const int sampling_flags = TextureSamplingFlagNone,
            const void* data = nullptr,
            const bool mipmaps = false);
  ~GLTexture();

  GLTexture(const GLTexture&) = delete;
  GLTexture& operator=(const GLTexture&) = delete;

  GLTexture(GLTexture&& other) noexcept
      : format(other.format),
        gl_texture(other.gl_texture),
        size(other.size),
        sampling_flags(other.sampling_flags)
  {
    other.gl_texture = std::numeric_limits<GLuint>::max();
  }

  GLTexture& operator=(GLTexture&& other) noexcept
  {
    this->~GLTexture();

    format = other.format;
    gl_texture = other.gl_texture;
    size = other.size;
    sampling_flags = other.sampling_flags;

    other.gl_texture = std::numeric_limits<GLuint>::max();

    return *this;
  }

  /**
   * @brief Binds the texture.
   *
   * @param unit The texture unit to bind the texture to.
   */
  void bind(GLuint unit) const;

  /**
   * @brief Unbinds the texture.
   *
   * @param unit The texture unit to unbind the texture from.
   */
  void unbind(GLuint unit) const;

  /**
   * @brief Sets the texture sampling flags.
   *
   * @param sampling_flags The sampling flags to set.
   */
  void set_sampling_flags(const int flags);

  /**
   * @brief Uploads the data to the texture.
   *
   * @param data The data to upload.
   * @param region The region to upload the data to.
   */
  void upload(const void* data, const irect region) const;

  /**
   * @brief Uploads the data to the texture, treating it as a 1D buffer.
   *
   * @param data The data to upload.
   * @param count The number of pixels to upload.
   * @param offset The offset to upload the data to.
   */
  void upload(const void* data, const size_t count, const size_t offset = 0) const;
};

/**
 * @brief The OpenGL framebuffer object.
 */
struct GLFramebuffer {
  GLTexture texture;       // The texture to render to.
  GLuint gl_framebuffer;   // The OpenGL underlying framebuffer.
  GLuint gl_renderbuffer;  // The OpenGL underlying renderbuffer.

  bool has_depth;          // Whether the framebuffer has a depth buffer.
  bool complete;           // Whether the framebuffer was successfully created.

  GLFramebuffer(const ivec2 size, const bool has_depth);
  ~GLFramebuffer();

  GLFramebuffer(const GLFramebuffer&) = delete;
  GLFramebuffer& operator=(const GLFramebuffer&) = delete;

  GLFramebuffer(GLFramebuffer&& other) noexcept;
  GLFramebuffer& operator=(GLFramebuffer&& other) noexcept;

  /**
   * @brief Returns the size of the framebuffer.
   *
   * @return The size of the framebuffer.
   */
  inline ivec2 size() const
  {
    return texture.size;
  }

  /**
   * @brief Binds the framebuffer.
   */
  void bind() const;

  /**
   * @brief Unbinds the framebuffer.
   */
  void unbind() const;
};

/**
 * @brief The OpenGL double framebuffer object.
 */
struct GLDoubleFramebuffer {
  GLTexture front_texture;      // The front framebuffer texture.
  GLTexture back_texture;       // The back framebuffer texture.

  GLuint gl_front_framebuffer;  // The OpenGL underlying front framebuffer.
  GLuint gl_back_framebuffer;   // The OpenGL underlying back framebuffer.
  GLuint gl_renderbuffer;       // The OpenGL underlying shared renderbuffer.

  bool has_depth;               // Whether the framebuffer has a depth buffer.
  bool complete;                // Whether the framebuffer was successfully created.

  GLDoubleFramebuffer(const ivec2 size, const bool has_depth = true);
  ~GLDoubleFramebuffer();

  GLDoubleFramebuffer(const GLDoubleFramebuffer&) = delete;
  GLDoubleFramebuffer& operator=(const GLDoubleFramebuffer&) = delete;

  GLDoubleFramebuffer(GLDoubleFramebuffer&& other) noexcept;
  GLDoubleFramebuffer& operator=(GLDoubleFramebuffer&& other) noexcept;

  /**
   * @brief Returns the size of the framebuffer.
   *
   * @return The size of the framebuffer.
   */
  inline ivec2 size() const
  {
    return front_texture.size;
  }

  /**
   * @brief Swaps the front and back framebuffers.
   */
  void swap();

  /**
   * @brief Blits the back framebuffer to the front framebuffer.
   */
  void blit_back_to_front() const;

  /**
   * @brief Blits the front framebuffer to the default one.
   */
  void blit() const;

  /**
   * @brief Binds the front framebuffer.
   */
  void bind() const;

  /**
   * @brief Unbinds the framebuffers.
   */
  void unbind() const;
};

/**
 * @brief The OpenGL buffer object.
 */
struct GLBuffer {
  BufferUploadMode mode;  // The buffer upload mode.
  BufferTarget target;    // The buffer target.
  GLuint gl_buffer;       // The OpenGL underlying buffer.
  size_t size;            // The size of the buffer in bytes.

  GLBuffer(const BufferTarget target,
           const BufferUploadMode mode,
           const size_t size,
           const void* data = nullptr);
  ~GLBuffer();

  /**
   * @brief Binds the buffer.
   */
  void bind() const;

  /**
   * @brief Binds the buffer
   *
   * @param vertex_array The vertex array to bind the buffer to.
   */
  void bind(const GLVertexArray& vertex_array) const;

  /**
   * @brief Unbinds the buffer.
   */
  void unbind() const;

  /**
   * @brief Uploads the data to the buffer.
   *
   * @param data The data to upload.
   * @param size The size of the data in bytes.
   * @param offset The offset to upload the data to.
   */
  void upload(const void* data, const size_t size, const size_t offset = 0) const;
};

}  // namespace graphick::renderer::GPU::GL
