/**
 * @file renderer/gpu/shaders.h
 * @brief Contains the GPU shaders and vertex arrays definitions.
 */

#pragma once

#include "../../utils/defines.h"

#include "render_state.h"

#include <memory>

namespace graphick::renderer::GPU {

/**
 * @brief The tile shader program.
 */
struct TileProgram {
  Program program;                        // The shader program.

  Uniform vp_uniform;                     // The view projection uniform.
  Uniform samples_uniform;                // The antialiasing samples uniform.

  TextureUniform curves_texture_uniform;  // The curves texture uniform (sampler2D), separate from
                                          // the non float array.
  TexturesUniform textures_uniform;       // The texture uniforms:
                                          //  - [0] is the gradient texture
                                          //  - [1...] are the image textures (or tiles textures).

  TileProgram();
};

/**
 * @brief Fill shader program.
 */
struct FillProgram {
  Program program;                   // The shader program.
  Uniform vp_uniform;                // The view projection uniform.

  TexturesUniform textures_uniform;  // The texture uniforms:
                                     //  - [0] is the gradient texture
                                     //  - [1...] are the image textures (or tiles textures).

  FillProgram();
};

/**
 * @brief Primitive shader program.
 */
struct PrimitiveProgram {
  Program program;       // The shader program.
  Uniform vp_uniform;    // The view projection uniform.
  Uniform zoom_uniform;  // The zoom uniform.

  PrimitiveProgram();
};

#ifdef GK_DEBUG

/**
 * @brief Debug rect shader program.
 */
struct DebugRectProgram {
  Program program;         // The shader program.
  Uniform vp_uniform;      // The view projection uniform.
  TextureUniform texture;  // The image texture.

  DebugRectProgram();
};

#endif

/**
 * @brief Groups all of the available shaders together.
 */
struct Programs {
  TileProgram tile_program;             // The tile shader program.
  FillProgram fill_program;             // The fill shader program.
  PrimitiveProgram primitive_program;   // The primitive shader program.

#ifdef GK_DEBUG
  DebugRectProgram debug_rect_program;  // The debug rect shader program.
#endif
};

/**
 * @brief Vertex array to use with TileProgram.
 */
struct TileVertexArray {
  VertexArray vertex_array;  // The vertex array.

  TileVertexArray(const TileProgram& program,
                  const Buffer& vertex_buffer,
                  const Buffer& index_buffer);
};

struct FillVertexArray {
  VertexArray vertex_array;  // The vertex array.

  FillVertexArray(const FillProgram& program,
                  const Buffer& vertex_buffer,
                  const Buffer& index_buffer);
};

/**
 * @brief Vertex array to use with PrimitiveProgram.
 */
struct PrimitiveVertexArray {
  VertexArray vertex_array;  // The vertex array.

  PrimitiveVertexArray(const PrimitiveProgram& program,
                       const Buffer& instance_buffer,
                       const Buffer& vertex_buffer);
};

#ifdef GK_DEBUG

/**
 * @brief Vertex array to use with DebugRectProgram.
 */
struct DebugRectVertexArray {
  VertexArray vertex_array;  // The vertex array.

  DebugRectVertexArray(const DebugRectProgram& program, const Buffer& vertex_buffer);
};

#endif

/**
 * @brief Groups all of the available vertex arrays together.
 */
struct VertexArrays {
  std::unique_ptr<TileVertexArray> tile_vertex_array;             // The tile shader vertex array.
  std::unique_ptr<FillVertexArray> fill_vertex_array;             // The fill shader vertex array.
  std::unique_ptr<PrimitiveVertexArray> primitive_vertex_array;   // The primitive vertex array.

#ifdef GK_DEBUG
  std::unique_ptr<DebugRectVertexArray> debug_rect_vertex_array;  // The debug rects.
#endif

  VertexArrays() = default;

  VertexArrays(
#ifdef GK_DEBUG
      std::unique_ptr<DebugRectVertexArray> debug_rect_vertex_array,
#endif
      std::unique_ptr<PrimitiveVertexArray> primitive_vertex_array,
      std::unique_ptr<TileVertexArray> tile_vertex_array,
      std::unique_ptr<FillVertexArray> fill_vertex_array)
      :
#ifdef GK_DEBUG
        debug_rect_vertex_array(std::move(debug_rect_vertex_array)),
#endif
        primitive_vertex_array(std::move(primitive_vertex_array)),
        tile_vertex_array(std::move(tile_vertex_array)),
        fill_vertex_array(std::move(fill_vertex_array))
  {
  }
};

}  // namespace graphick::renderer::GPU
