/**
 * @file gl_data.h
 * @brief The file contains the definition of the OpenGL GPU data.
 */

#pragma once

#include "../gpu_data.h"

#include "../../../math/rect.h"
#include "../../../utils/console.h"

#ifdef EMSCRIPTEN
#include <GLES3/gl32.h>
#else
#include <glad/glad.h>
#endif

#include <vector>
#include <memory>
#include <optional>
#include <sstream>

namespace graphick::renderer::GPU::GL {

  /**
   * @brief Clears all OpenGL errors.
   */
  inline void glClearErrors() {
    while (glGetError() != GL_NO_ERROR);
  }

  /**
   * @brief Logs an OpenGL error.
   *
   * @param function The function that caused the error.
   * @param line The line that caused the error.
   * @return True if there was no error, false otherwise.
   */
  inline bool glLogCall(const char* function, int line) {
    while (GLenum error = glGetError()) {
      std::stringstream ss;
      ss << "OpenGL Error: " << error << " " << function << " " << line;
      console::error(ss.str());
      return false;
    }
    return true;
  }


#ifndef GK_CONF_DIST
#ifdef _MSC_VER
#define glCall(x) glClearErrors();\
	x;\
	if (!glLogCall(#x, __LINE__)) __debugbreak();
#else
#define glCall(x) glClearErrors();\
	x;\
  glLogCall(#x, __LINE__)
#endif
#else 
#define glCall(x) x
#endif

  /**
   * @brief The OpenGL texture object.
   *
   * @struct GLTexture
   */
  struct GLTexture {
    GLuint gl_texture;       /* The OpenGL underlying texture. */
    ivec2 size;              /* The size of the texture. */
    TextureFormat format;    /* The format of the texture. */

    GLTexture(GLuint gl_texture, ivec2 size, TextureFormat format)
      : gl_texture(gl_texture), size(size), format(format) {}
    ~GLTexture() { glCall(glDeleteTextures(1, &gl_texture)); }
  };

  /**
   * @brief Groups the vertex and fragment shaders together.
   *
   * @struct ShaderPair
   */
  template <typename T>
  struct ShaderPair {
    T vertex;      /* The vertex shader related data. */
    T fragment;    /* The fragment shader related data. */
  };

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
   * @brief The OpenGL program parameters.
   *
   * @struct GLProgramParameters
  */
  struct GLProgramParameters {
    std::vector<GLUniform> textures;    /* Mapping from texture unit number to uniform location. */
  };

  /**
   * @brief The OpenGL shader object.
   *
   * @struct GLShader
   */
  struct GLShader {
    GLuint gl_shader;    /* The OpenGL underlying shader. */
  };

  /**
   * @brief The OpenGL program object.
   *
   * @struct GLProgram
   */
  struct GLProgram {
    GLuint gl_program;                 /* The OpenGL underlying program. */
    ShaderPair<GLShader> shaders;      /* The shaders. */
    GLProgramParameters parameters;    /* The program parameters. */
  };

  /**
   * @brief The OpenGL vertex array object.
   *
   * @struct GLVertexArray
   */
  struct GLVertexArray {
    GLuint gl_vertex_array;    /* The OpenGL underlying vertex array. */

    GLVertexArray(GLuint gl_vertex_array)
      : gl_vertex_array(gl_vertex_array) {}
    ~GLVertexArray() { glCall(glDeleteVertexArrays(1, &gl_vertex_array)); }
  };

  /**
   * @brief The OpenGL buffer object.
   *
   * @struct GLBuffer
   */
  struct GLBuffer {
    GLuint gl_buffer;         /* The OpenGL underlying buffer. */
    BufferUploadMode mode;    /* The buffer upload mode. */

    GLBuffer(GLuint gl_buffer, BufferUploadMode mode)
      : gl_buffer(gl_buffer), mode(mode) {}
    ~GLBuffer() { glCall(glDeleteBuffers(1, &gl_buffer)); }
  };

  /**
   * @brief The OpenGL framebuffer object.
   *
   * @struct GLFramebuffer
   */
  struct GLFramebuffer {
    GLuint gl_framebuffer;                 /* The OpenGL underlying framebuffer. */
    std::unique_ptr<GLTexture> texture;    /* The texture attached to the framebuffer. */

    GLFramebuffer(GLuint gl_framebuffer, std::unique_ptr<GLTexture> texture)
      : gl_framebuffer(gl_framebuffer), texture(std::move(texture)) {}
    ~GLFramebuffer() { glCall(glDeleteFramebuffers(1, &gl_framebuffer)); }
  };

  /**
   * @brief The OpenGL renderbuffer object.
   *
   * @struct GLRenderbuffer
   */
  struct GLTextureParameter {
    GLUniform uniform;      /* The uniform. */
    GLuint texture_unit;    /* The texture unit. */
  };

  /**
   * @brief The OpenGL storage buffer object.
   *
   * @struct GLStorageBuffer
   */
  struct GLStorageBuffer {
    GLint location;    /* The location of the storage buffer. */
  };

  /**
   * @brief The OpenGL vertex attribute.
   *
   * @struct GLVertexAttr
   */
  struct GLVertexAttr {
    GLuint attr;    /* The underlying vertex attribute. */

    /**
     * @brief Configures the vertex attribute.
     *
     * @param size The size of the attribute.
     * @param gl_type The OpenGL type of the attribute.
     * @param normalized Whether the attribute is normalized.
     * @param stride The stride of the attribute.
     * @param offset The offset of the attribute.
     * @param divisor The divisor of the attribute.
     */
    inline void configure_float(GLint size, GLuint gl_type, GLboolean normalized, GLsizei stride, size_t offset, GLuint divisor) {
      glCall(glVertexAttribPointer(attr, size, gl_type, normalized, stride, (const void*)offset));
      glCall(glVertexAttribDivisor(attr, divisor));
      glCall(glEnableVertexAttribArray(attr));
    }

    /**
     * @brief Configures the vertex attribute.
     *
     * @param size The size of the attribute.
     * @param gl_type The OpenGL type of the attribute.
     * @param stride The stride of the attribute.
     * @param offset The offset of the attribute.
     * @param divisor The divisor of the attribute.
     */
    inline void configure_int(GLint size, GLuint gl_type, GLsizei stride, size_t offset, GLuint divisor) {
      glCall(glVertexAttribIPointer(attr, size, gl_type, stride, (const void*)offset));
      glCall(glVertexAttribDivisor(attr, divisor));
      glCall(glEnableVertexAttribArray(attr));
    }
  };

  using GLRenderTarget = const GLFramebuffer*;

  /**
   * @brief The OpenGL render state.
   *
   * @struct GLRenderState
   */
  struct GLRenderState {
    GLRenderTarget target;                                                         /* The render target. */
    GLProgram program;                                                             /* The program. */
    const GLVertexArray& vertex_array;                                             /* The vertex array. */
    Primitive primitive;                                                           /* The primitive. */
    std::vector<TextureBinding<GLTextureParameter, const GLTexture&>> textures;    /* The textures. */
    std::vector<std::pair<GLStorageBuffer, const GLBuffer&>> storage_buffers;      /* The storage buffers. */
    std::vector<UniformBinding<GLUniform>> uniforms;                               /* The uniforms. */
    rect viewport;                                                                 /* The viewport. */
    RenderOptions options;                                                         /* The render options. */
  };

}
