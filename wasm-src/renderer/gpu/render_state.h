/**
 * @file renderer/gpu/render_state.h
 * @brief Contains the render state definitions.
 */

#pragma once

#if defined(GK_GLES3) || defined(GK_GL3)
#  include "opengl/gl_data.h"
#else
#  include "opengl/gl_data.h"
#endif

#include "../../math/rect.h"

namespace graphick::renderer::GPU {

/**
 * @brief The program object.
 */
using Program = GL::GLProgram;

/**
 * @brief The uniform object.
 */
using Uniform = GL::GLUniform;

/**
 * @brief The texture uniform object.
 */
using TextureUniform = GL::GLTextureUniform;

/**
 * @brief The array of textures uniform object.
 */
using TexturesUniform = GL::GLTexturesUniform;

/**
 * @brief The vertex array object.
 */
using VertexArray = GL::GLVertexArray;

/**
 * @brief The vertex attribute object.
 */
using VertexAttribute = GL::GLVertexAttribute;

/**
 * @brief The texture object.
 */
using Texture = GL::GLTexture;

/**
 * @brief The framebuffer object.
 */
using Framebuffer = GL::GLFramebuffer;

/**
 * @brief The buffer object.
 */
using Buffer = GL::GLBuffer;

/**
 * @brief A uniform binding is used to bind a uniform to a value.
 */
using UniformBinding = std::pair<Uniform, UniformData>;

/**
 * @brief A texture binding is used to bind a texture to a texture unit and uniform.
 */
using TextureBinding = std::pair<TextureUniform, const Texture *>;

/**
 * @brief A texture array binding is used to bind an array of textures to texture units and a
 * uniform.
 */
using TextureArrayBinding = std::pair<TexturesUniform, const std::vector<const Texture *>>;

/**
 * @brief The render state.
 *
 * The device will keep track of the current render state and update it as needed.
 */
struct RenderState {
  Program program;                                  // The current program.
  VertexArray *vertex_array;                        // The current vertex array.
  Primitive primitive;                              // The current primitive.
  irect viewport;                                   // The current viewport.

  std::vector<UniformBinding> uniforms;             // The uniform bindings.
  std::vector<TextureBinding> textures;             // The texture bindings.
  std::vector<TextureArrayBinding> texture_arrays;  // The texture bindings.

  ClearOps clear_ops;                               // The clear operations.

  std::optional<BlendState> blend;  // The blend state, if std::nullopt, blending is disabled.
  std::optional<DepthState>
      depth;    // The depth state, if std::nullopt, the depth test is disabled.
  std::optional<StencilState>
      stencil;  // The stencil state, if std::nullopt, the stencil test is disabled.

  RenderState() = default;

  RenderState(Program program,
              VertexArray *vertex_array,
              Primitive primitive,
              irect viewport,
              std::vector<UniformBinding> uniforms = {},
              std::vector<TextureBinding> textures = {},
              std::vector<TextureArrayBinding> texture_arrays = {})
      : program(program),
        vertex_array(vertex_array),
        primitive(primitive),
        viewport(viewport),
        uniforms(uniforms),
        textures(textures)
  {
  }

  RenderState(Program program,
              VertexArray *vertex_array,
              Primitive primitive,
              irect viewport,
              std::vector<UniformBinding> uniforms,
              std::vector<TextureBinding> textures,
              ClearOps clear_ops,
              std::optional<BlendState> blend = std::nullopt,
              std::optional<DepthState> depth = std::nullopt,
              std::optional<StencilState> stencil = std::nullopt)
      : program(program),
        vertex_array(vertex_array),
        primitive(primitive),
        viewport(viewport),
        uniforms(uniforms),
        textures(textures),
        clear_ops(clear_ops),
        blend(blend),
        depth(depth),
        stencil(stencil)
  {
  }

  RenderState &operator=(const RenderState &other)
  {
    program = other.program;
    vertex_array = other.vertex_array;
    primitive = other.primitive;
    viewport = other.viewport;
    clear_ops = other.clear_ops;
    blend = other.blend;
    depth = other.depth;
    stencil = other.stencil;

    uniforms.clear();
    textures.clear();
    texture_arrays.clear();

    return *this;
  }

  RenderState &default_blend()
  {
    blend = BlendState{BlendFactor::One,
                       BlendFactor::OneMinusSrcAlpha,
                       BlendFactor::One,
                       BlendFactor::OneMinusSrcAlpha,
                       BlendOp::Add};

    return *this;
  }

  RenderState &no_blend()
  {
    blend = std::nullopt;

    return *this;
  }

  RenderState &default_depth()
  {
    depth = DepthState{DepthFunc::Less, true};

    return *this;
  }

  RenderState &no_depth_write()
  {
    depth = DepthState{DepthFunc::Less, false};

    return *this;
  }

  RenderState &no_depth()
  {
    depth = std::nullopt;

    return *this;
  }

  RenderState &add_stencil()
  {
    stencil = StencilState{StencilFunc::Always, 1, 0xFF, true};

    return *this;
  }

  RenderState &subtract_stencil()
  {
    stencil = StencilState{StencilFunc::Always, 0, 0xFF, true};

    return *this;
  }

  RenderState &no_stencil()
  {
    stencil = std::nullopt;

    return *this;
  }

  RenderState &keep_stencil()
  {
    stencil = StencilState{StencilFunc::Nequal, 1, 0xFF, false};

    return *this;
  }
};

}  // namespace graphick::renderer::GPU
