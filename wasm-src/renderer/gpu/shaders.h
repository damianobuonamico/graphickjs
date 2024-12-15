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

  TextureUniform bands_texture_uniform;   // The bands texture uniform (usampler2D), separate from
                                          // the sampler2D array.
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
 * @brief Line shader program.
 */
struct LineProgram {
  Program program;       // The shader program.
  Uniform vp_uniform;    // The view projection uniform.
  Uniform zoom_uniform;  // The zoom uniform.

  LineProgram();
};

/**
 * @brief Rect shader program.
 */
struct RectProgram {
  Program program;     // The shader program.
  Uniform vp_uniform;  // The view projection uniform.

  RectProgram();
};

/**
 * @brief Circle shader program.
 */
struct CircleProgram {
  Program program;       // The shader program.
  Uniform vp_uniform;    // The view projection uniform.
  Uniform zoom_uniform;  // The zoom uniform.

  CircleProgram();
};

/**
 * @brief Image shader program.
 */
struct ImageProgram {
  Program program;               // The shader program.
  Uniform vp_uniform;            // The view projection uniform.
  TextureUniform image_texture;  // The image texture.

  ImageProgram();
};

#ifdef GK_DEBUG

/**
 * @brief Debug rect shader program.
 */
struct DebugRectProgram {
  Program program;     // The shader program.
  Uniform vp_uniform;  // The view projection uniform.
  TextureUniform texture;  // The image texture.

  DebugRectProgram();
};

#endif

/**
 * @brief Groups all of the available shaders together.
 */
struct Programs {
  // TileProgram tile_program;             // The tile shader program.
  // FillProgram fill_program;             // The fill shader program.
  // LineProgram line_program;             // The line shader program.
  // RectProgram rect_program;             // The square shader program.
  // CircleProgram circle_program;         // The circle shader program.
  // ImageProgram image_program;           // The image shader program.

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
 * @brief Vertex array to use with LineProgram.
 */
struct LineVertexArray {
  VertexArray vertex_array;  // The vertex array.

  LineVertexArray(const LineProgram& program,
                  const Buffer& instance_buffer,
                  const Buffer& vertex_buffer);
};

/**
 * @brief Vertex array to use with RectProgram.
 */
struct RectVertexArray {
  VertexArray vertex_array;  // The vertex array.

  RectVertexArray(const RectProgram& program,
                  const Buffer& instance_buffer,
                  const Buffer& vertex_buffer);
};

/**
 * @brief Vertex array to use with CircleProgram.
 */
struct CircleVertexArray {
  VertexArray vertex_array;  // The vertex array.

  CircleVertexArray(const CircleProgram& program,
                    const Buffer& instance_buffer,
                    const Buffer& vertex_buffer);
};

/**
 * @brief Vertex array to use with ImageProgram.
 */
struct ImageVertexArray {
  VertexArray vertex_array;  // The vertex array.

  ImageVertexArray(const ImageProgram& program,
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
  // std::unique_ptr<TileVertexArray> tile_vertex_array;      // The tile shader vertex array.
  // std::unique_ptr<FillVertexArray> fill_vertex_array;      // The fill shader vertex array.
  // std::unique_ptr<LineVertexArray> line_vertex_array;      // The line shader vertex array.
  // std::unique_ptr<RectVertexArray> rect_vertex_array;      // The square shader vertex array.
  // std::unique_ptr<CircleVertexArray> circle_vertex_array;  // The circle shader vertex array.
  // std::unique_ptr<ImageVertexArray> image_vertex_array;    // The image shader vertex array.

  VertexArrays() = default;

  // VertexArrays(std::unique_ptr<TileVertexArray> tile_vertex_array,
  //              std::unique_ptr<FillVertexArray> fill_vertex_array,
  //              std::unique_ptr<LineVertexArray> line_vertex_array,
  //              std::unique_ptr<RectVertexArray> rect_vertex_array,
  //              std::unique_ptr<CircleVertexArray> circle_vertex_array,
  //              std::unique_ptr<ImageVertexArray> image_vertex_array)
  //     : tile_vertex_array(std::move(tile_vertex_array)),
  //       fill_vertex_array(std::move(fill_vertex_array)),
  //       line_vertex_array(std::move(line_vertex_array)),
  //       rect_vertex_array(std::move(rect_vertex_array)),
  //       circle_vertex_array(std::move(circle_vertex_array)),
  //       image_vertex_array(std::move(image_vertex_array))
  // {
  // }

#ifdef GK_DEBUG

  std::unique_ptr<DebugRectVertexArray> debug_rect_vertex_array;  // The debug rects.

  VertexArrays(std::unique_ptr<DebugRectVertexArray> debug_rect_vertex_array)
      : debug_rect_vertex_array(std::move(debug_rect_vertex_array))
  {
  }

#endif
};

}  // namespace graphick::renderer::GPU
