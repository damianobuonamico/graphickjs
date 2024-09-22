/**
 * @file renderer/gpu/shaders.h
 * @brief Contains the GPU shaders and vertex arrays definitions.
 */

#pragma once

#include "render_state.h"

#include <memory>

namespace graphick::renderer::GPU {

/**
 * @brief The tile shader program.
 *
 * @struct TileProgram
 */
struct TileProgram {
  Program program;                         // The shader program.

  Uniform vp_uniform;                      // The view projection uniform.
  Uniform samples_uniform;                 // The antialiasing samples uniform.

  TextureUniform bands_texture_uniform;    // The bands texture uniform (usampler2D), separate from the sampler2D array.
  TexturesUniform textures_uniform;        // The texture uniforms:
                                           //  - [0] is the curves texture
                                           //  - [1] is the gradient texture
                                           //  - [2...] are the image textures (or tiles textures).

  TileProgram();
};

/**
 * @brief The main shader program.
 *
 * @struct PathProgram
 */
struct PathProgram {
  Program program;               /* The shader program. */
  Uniform vp_uniform;            /* The view projection uniform. */
  Uniform viewport_size_uniform; /* The viewport size uniform. */
  Uniform samples_uniform;       /* The antialiasing samples uniform. */
  Uniform models_uniform;        /* The models uniform. */
  TextureUniform curves_texture; /* The curves texture. */
  TextureUniform bands_texture;  /* The bands texture. */

  PathProgram();
};

/**
 * @brief Boundary span shader program.
 *
 * @struct BoundarySpanProgram
 */
struct BoundarySpanProgram {
  Program program;               /* The shader program. */
  Uniform vp_uniform;            /* The view projection uniform. */
  Uniform viewport_size_uniform; /* The viewport size uniform. */
  Uniform max_samples_uniform;   /* The maximum antialiasing samples uniform. */
  Uniform models_uniform;        /* The models uniform. */
  TextureUniform curves_texture; /* The curves texture. */

  BoundarySpanProgram();
};

/**
 * @brief Filled span shader program.
 *
 * @struct FilledSpanProgram
 */
struct FilledSpanProgram {
  Program program;        /* The shader program. */
  Uniform vp_uniform;     /* The view projection uniform. */
  Uniform models_uniform; /* The models uniform. */

  FilledSpanProgram();
};

/**
 * @brief Line shader program.
 *
 * @struct LineProgram
 */
struct LineProgram {
  Program program;      /* The shader program. */
  Uniform vp_uniform;   /* The view projection uniform. */
  Uniform zoom_uniform; /* The zoom uniform. */

  LineProgram();
};

/**
 * @brief Rect shader program.
 *
 * @struct RectProgram
 */
struct RectProgram {
  Program program;    /* The shader program. */
  Uniform vp_uniform; /* The view projection uniform. */

  RectProgram();
};

/**
 * @brief Circle shader program.
 *
 * @struct CircleProgram
 */
struct CircleProgram {
  Program program;      /* The shader program. */
  Uniform vp_uniform;   /* The view projection uniform. */
  Uniform zoom_uniform; /* The zoom uniform. */

  CircleProgram();
};

/**
 * @brief Image shader program.
 *
 * @struct ImageProgram
 */
struct ImageProgram {
  Program program;              /* The shader program. */
  Uniform vp_uniform;           /* The view projection uniform. */
  TextureUniform image_texture; /* The image texture. */

  ImageProgram();
};

/**
 * @brief Groups all of the available shaders together.
 *
 * @struct Programs
 */
struct Programs {
  TileProgram tile_program;                  /* The tile shader program. */
  PathProgram path_program;                  /* The path shader program. */
  BoundarySpanProgram boundary_span_program; /* The boundary span shader program. */
  FilledSpanProgram filled_span_program;     /* The filled span shader program. */
  LineProgram line_program;                  /* The line shader program. */
  RectProgram rect_program;                  /* The square shader program. */
  CircleProgram circle_program;              /* The circle shader program. */
  ImageProgram image_program;                /* The image shader program. */
};

/**
 * @brief Vertex array to use with TileProgram.
 *
 * @struct TileVertexArray
 */
struct TileVertexArray {
  VertexArray vertex_array; /* The vertex array. */

  TileVertexArray(const TileProgram& program, const Buffer& vertex_buffer, const Buffer& index_buffer);
};

/**
 * @brief Vertex array to use with PathProgram.
 *
 * @struct PathVertexArray
 */
struct PathVertexArray {
  VertexArray vertex_array; /* The vertex array. */

  PathVertexArray(const PathProgram& program, const Buffer& instance_buffer, const Buffer& vertex_buffer);
};

/**
 * @brief Vertex array to use with BoundarySpanProgram.
 *
 * @struct BoundarySpanVertexArray
 */
struct BoundarySpanVertexArray {
  VertexArray vertex_array; /* The vertex array. */

  BoundarySpanVertexArray(const BoundarySpanProgram& program, const Buffer& instance_buffer, const Buffer& vertex_buffer);
};

/**
 * @brief Vertex array to use with FilledSpanProgram.
 *
 * @struct FilledSpanVertexArray
 */
struct FilledSpanVertexArray {
  VertexArray vertex_array; /* The vertex array. */

  FilledSpanVertexArray(const FilledSpanProgram& program, const Buffer& instance_buffer, const Buffer& vertex_buffer);
};

/**
 * @brief Vertex array to use with LineProgram.
 *
 * @struct LineVertexArray
 */
struct LineVertexArray {
  VertexArray vertex_array; /* The vertex array. */

  LineVertexArray(const LineProgram& program, const Buffer& instance_buffer, const Buffer& vertex_buffer);
};

/**
 * @brief Vertex array to use with RectProgram.
 *
 * @struct RectVertexArray
 */
struct RectVertexArray {
  VertexArray vertex_array; /* The vertex array. */

  RectVertexArray(const RectProgram& program, const Buffer& instance_buffer, const Buffer& vertex_buffer);
};

/**
 * @brief Vertex array to use with CircleProgram.
 *
 * @struct CircleVertexArray
 */
struct CircleVertexArray {
  VertexArray vertex_array; /* The vertex array. */

  CircleVertexArray(const CircleProgram& program, const Buffer& instance_buffer, const Buffer& vertex_buffer);
};

/**
 * @brief Vertex array to use with ImageProgram.
 *
 * @struct ImageVertexArray
 */
struct ImageVertexArray {
  VertexArray vertex_array; /* The vertex array. */

  ImageVertexArray(const ImageProgram& program, const Buffer& instance_buffer, const Buffer& vertex_buffer);
};

/**
 * @brief Groups all of the available vertex arrays together.
 *
 * @struct VertexArrays
 */
struct VertexArrays {
  std::unique_ptr<TileVertexArray> tile_vertex_array;                  /* The tile shader vertex array. */
  std::unique_ptr<PathVertexArray> path_vertex_array;                  /* The path shader vertex array. */
  std::unique_ptr<BoundarySpanVertexArray> boundary_span_vertex_array; /* The boundary span shader vertex array. */
  std::unique_ptr<FilledSpanVertexArray> filled_span_vertex_array;     /* The filled span shader vertex array. */
  std::unique_ptr<LineVertexArray> line_vertex_array;                  /* The line shader vertex array. */
  std::unique_ptr<RectVertexArray> rect_vertex_array;                  /* The square shader vertex array. */
  std::unique_ptr<CircleVertexArray> circle_vertex_array;              /* The circle shader vertex array. */
  std::unique_ptr<ImageVertexArray> image_vertex_array;                /* The image shader vertex array. */

  VertexArrays() = default;

  VertexArrays(
    std::unique_ptr<TileVertexArray> tile_vertex_array,
    std::unique_ptr<PathVertexArray> path_vertex_array,
    std::unique_ptr<BoundarySpanVertexArray> boundary_span_vertex_array,
    std::unique_ptr<FilledSpanVertexArray> filled_span_vertex_array,
    std::unique_ptr<LineVertexArray> line_vertex_array,
    std::unique_ptr<RectVertexArray> rect_vertex_array,
    std::unique_ptr<CircleVertexArray> circle_vertex_array,
    std::unique_ptr<ImageVertexArray> image_vertex_array
  ) :
    tile_vertex_array(std::move(tile_vertex_array)),
    path_vertex_array(std::move(path_vertex_array)),
    boundary_span_vertex_array(std::move(boundary_span_vertex_array)),
    filled_span_vertex_array(std::move(filled_span_vertex_array)),
    line_vertex_array(std::move(line_vertex_array)),
    rect_vertex_array(std::move(rect_vertex_array)),
    circle_vertex_array(std::move(circle_vertex_array)),
    image_vertex_array(std::move(image_vertex_array)) { }
};

}
