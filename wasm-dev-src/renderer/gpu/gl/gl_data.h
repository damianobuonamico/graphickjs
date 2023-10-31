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

namespace Graphick::Renderer::GPU::GL {

  inline void glClearErrors() {
    while (glGetError() != GL_NO_ERROR);
  }

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

  struct GLTexture {
    GLuint gl_texture;
    ivec2 size;
    TextureFormat format;

    GLTexture(GLuint gl_texture, ivec2 size, TextureFormat format)
      : gl_texture(gl_texture), size(size), format(format) {}
    ~GLTexture() { glCall(glDeleteTextures(1, &gl_texture)); }
  };

  template <typename T>
  struct ShaderPair {
    T vertex;
    T fragment;
  };

  struct GLUniform {
    GLint location;

    bool operator==(const GLUniform& other) const {
      return location == other.location;
    }
  };

  struct GLProgramParameters {
    // Mapping from texture unit number to uniform location.
    std::vector<GLUniform> textures;
  };

  struct GLShader {
    GLuint gl_shader;
  };

  struct GLProgram {
    GLuint gl_program;
    ShaderPair<GLShader> shaders;
    GLProgramParameters parameters;
  };

  struct GLVertexArray {
    GLuint gl_vertex_array;

    GLVertexArray(GLuint gl_vertex_array)
      : gl_vertex_array(gl_vertex_array) {}
    ~GLVertexArray() { glCall(glDeleteVertexArrays(1, &gl_vertex_array)); }
  };

  struct GLBuffer {
    GLuint gl_buffer;
    BufferUploadMode mode;

    GLBuffer(GLuint gl_buffer, BufferUploadMode mode)
      : gl_buffer(gl_buffer), mode(mode) {}
    ~GLBuffer() { glCall(glDeleteBuffers(1, &gl_buffer)); }
  };

  struct GLFramebuffer {
    GLuint gl_framebuffer;
    std::unique_ptr<GLTexture> texture;

    GLFramebuffer(GLuint gl_framebuffer, std::unique_ptr<GLTexture> texture)
      : gl_framebuffer(gl_framebuffer), texture(std::move(texture)) {}
    ~GLFramebuffer() { glCall(glDeleteFramebuffers(1, &gl_framebuffer)); }
  };

  struct GLTextureParameter {
    GLUniform uniform;
    GLuint texture_unit;
  };

  struct GLStorageBuffer {
    GLint location;
  };

  struct GLVertexAttr {
    GLuint attr;

    inline void configure_float(GLint size, GLuint gl_type, GLboolean normalized, GLsizei stride, size_t offset, GLuint divisor) {
      glCall(glVertexAttribPointer(attr, size, gl_type, normalized, stride, (const void*)offset));
      glCall(glVertexAttribDivisor(attr, divisor));
      glCall(glEnableVertexAttribArray(attr));
    }

    inline void configure_int(GLint size, GLuint gl_type, GLsizei stride, size_t offset, GLuint divisor) {
      glCall(glVertexAttribIPointer(attr, size, gl_type, stride, (const void*)offset));
      glCall(glVertexAttribDivisor(attr, divisor));
      glCall(glEnableVertexAttribArray(attr));
    }
  };

  using GLRenderTarget = const GLFramebuffer*;

  struct GLRenderState {
    GLRenderTarget target;
    GLProgram program;
    const GLVertexArray& vertex_array;
    Primitive primitive;
    std::vector<TextureBinding<GLTextureParameter, const GLTexture&>> textures;
    std::vector<std::pair<GLStorageBuffer, const GLBuffer&>> storage_buffers;
    std::vector<UniformBinding<GLUniform>> uniforms;
    rect viewport;
    RenderOptions options;
  };

}
