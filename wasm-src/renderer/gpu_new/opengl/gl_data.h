/**
 * @file renderer/gpu/opengl/gl_data.h
 * @brief The file contains the definition of the OpenGL GPU data.
 */

#pragma once

#include "../gpu_data.h"

#include <vector>

namespace graphick::renderer::GPU::GL {

  /**
   * @brief OpenGL specific types.
   */
  typedef unsigned int GLuint;
  typedef int GLint;

  /**
   * @brief The OpenGL uniform object.
   *
   * @struct GLUniform
   */
  struct GLUniform {
    GLint location;

    bool operator==(const GLUniform& other) const {
      return location == other.location;
    }
  };

  /**
   * @brief The OpenGL texture uniform object.
   *
   * @struct GLTextureUniform
   */
  struct GLTextureUniform {
    GLUniform uniform;      /* The uniform. */
    GLuint unit;            /* The texture unit. */
  };

  /**
   * @brief The OpenGL program object.
   *
   * @struct GLProgram
   */
  struct GLProgram {
    GLuint gl_program;                  /* The OpenGL underlying program. */
    GLuint vertex;                      /* The vertex shader. */
    GLuint fragment;                    /* The fragment shader. */

    std::vector<GLUniform> textures;    /* Mapping from texture unit number to uniform location. */
  };

  /**
   * @brief The OpenGL vertex attribute.
   *
   * @struct GLVertexAttribute
   */
  struct GLVertexAttribute {
    GLuint attribute;    /* The underlying vertex attribute. */
  };

  /**
   * @brief The OpenGL vertex array object.
   *
   * @struct GLVertexArray
   */
  struct GLVertexArray {
    GLuint gl_vertex_array;    /* The OpenGL underlying vertex array. */

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
   *
   * @struct GLTexture
   */
  struct GLTexture {
    TextureFormat format;    /* The texture format. */
    GLuint gl_texture;       /* The OpenGL underlying texture. */
    ivec2 size;              /* The size of the texture. */

    int sampling_flags;      /* The texture sampling flags. */

    GLTexture(const TextureFormat format, const ivec2 size, const int sampling_flags = TextureSamplingFlagNone, const void* data = nullptr);
    ~GLTexture();

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
  };

  /**
   * @brief The OpenGL buffer object.
   *
   * @struct GLBuffer
   */
  struct GLBuffer {
    BufferUploadMode mode;    /* The buffer upload mode. */
    BufferTarget target;      /* The buffer target. */
    GLuint gl_buffer;         /* The OpenGL underlying buffer. */
    size_t size;              /* The size of the buffer in bytes. */

    GLBuffer(const BufferTarget target, const BufferUploadMode mode, const size_t size, const void* data = nullptr);
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
  };

  /**
   * @brief The OpenGL texture object.
   */
  struct GLState {
    ivec2 viewport_size;    /* The size of the viewport. */
  };

}
