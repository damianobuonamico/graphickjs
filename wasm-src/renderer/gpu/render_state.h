/**
 * @file renderer/gpu/render_state.h
 * @brief Contains the render state definitions.
 */

#pragma once

#if defined(GK_GLES3) || defined(GK_GL3)
#include "opengl/gl_data.h"
#else
#include "opengl/gl_data.h"
#endif

#include "../../math/rect.h"

namespace graphick::renderer::GPU {

  /**
   * @brief The program object.
   *
   * @struct Program
   */
  using Program = GL::GLProgram;

  /**
   * @brief The uniform object.
   *
   * @struct Uniform
   */
  using Uniform = GL::GLUniform;

  /**
   * @brief The texture uniform object.
   *
   * @struct TextureUniform
   */
  using TextureUniform = GL::GLTextureUniform;

  /**
   * @brief The vertex array object.
   *
   * @struct VertexArray
   */
  using VertexArray = GL::GLVertexArray;

  /**
   * @brief The vertex attribute object.
   *
   * @struct VertexAttribute
   */
  using VertexAttribute = GL::GLVertexAttribute;

  /**
   * @brief The texture object.
   *
   * @struct Texture
   */
  using Texture = GL::GLTexture;

  /**
   * @brief The framebuffer object.
   *
   * @struct Framebuffer
   */
  using Framebuffer = GL::GLFramebuffer;

  /**
   * @brief The buffer object.
   *
   * @struct Buffer
   */
  using Buffer = GL::GLBuffer;

  /**
   * @brief A uniform binding is used to bind a uniform to a value.
   */
  using UniformBinding = std::pair<Uniform, UniformData>;

  /**
   * @brief A texture binding is used to bind a texture to a texture unit and uniform.
   */
  using TextureBinding = std::pair<TextureUniform, const Texture&>;

  /**
   * @brief The render state.
   *
   * The device will keep track of the current render state and update it as needed.
   *
   * @struct RenderState
   */
  struct RenderState {
    Program program;                         /* The current program. */
    VertexArray* vertex_array;               /* The current vertex array. */
    Primitive primitive;                     /* The current primitive. */
    irect viewport;                          /* The current viewport. */

    std::vector<UniformBinding> uniforms;    /* The uniform bindings. */
    std::vector<TextureBinding> textures;    /* The texture bindings. */

    ClearOps clear_ops;                      /* The clear operations. */

    std::optional<BlendState> blend;         /* The blend state, if std::nullopt, blending is disabled. */
    std::optional<DepthState> depth;         /* The depth state, if std::nullopt, the depth test is disabled. */
    std::optional<StencilState> stencil;     /* The stencil state, if std::nullopt, the stencil test is disabled. */

    RenderState() = default;

    RenderState(
      Program program, VertexArray* vertex_array,
      Primitive primitive, irect viewport,
      std::vector<UniformBinding> uniforms = {},
      std::vector<TextureBinding> textures = {}
    ) :
      program(program), vertex_array(vertex_array),
      primitive(primitive), viewport(viewport),
      uniforms(uniforms), textures(textures) {}

    RenderState(
      Program program, VertexArray* vertex_array,
      Primitive primitive, irect viewport,
      std::vector<UniformBinding> uniforms,
      std::vector<TextureBinding> textures,
      ClearOps clear_ops,
      std::optional<BlendState> blend = std::nullopt,
      std::optional<DepthState> depth = std::nullopt,
      std::optional<StencilState> stencil = std::nullopt
    ) :
      program(program), vertex_array(vertex_array),
      primitive(primitive), viewport(viewport),
      uniforms(uniforms), textures(textures),
      clear_ops(clear_ops), blend(blend), depth(depth), stencil(stencil) {}
  };

}