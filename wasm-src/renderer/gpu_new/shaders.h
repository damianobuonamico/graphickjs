/**
 * @file renderer/gpu/shaders.h
 * @brief Contains the GPU shaders and vertex arrays definitions.
 */

#pragma once

#include "device.h"

namespace graphick::renderer::GPU {

  /**
   * @brief The main shader program.
   *
   * @struct PathProgram
   */
  struct PathProgram {
    Program program;                  /* The shader program. */
    Uniform vp_uniform;               /* The view projection uniform. */
    Uniform viewport_size_uniform;    /* The viewport size uniform. */
    Uniform min_samples_uniform;      /* The minimum antialiasing samples uniform. */
    Uniform max_samples_uniform;      /* The maximum antialiasing samples uniform. */
    TextureUniform curves_texture;    /* The curves texture. */
    TextureUniform bands_texture;     /* The bands texture. */

    PathProgram();
  };

  /**
   * @brief Boundary span shader program.
   *
   * @struct BoundarySpanProgram
   */
  struct BoundarySpanProgram {
    Program program;                  /* The shader program. */
    Uniform vp_uniform;               /* The view projection uniform. */
    Uniform viewport_size_uniform;    /* The viewport size uniform. */
    Uniform max_samples_uniform;      /* The maximum antialiasing samples uniform. */
    Uniform models_uniform;           /* The models uniform. */
    TextureUniform curves_texture;    /* The curves texture. */

    BoundarySpanProgram();
  };

  /**
   * @brief Filled span shader program.
   *
   * @struct FilledSpanProgram
   */
  struct FilledSpanProgram {
    Program program;           /* The shader program. */
    Uniform vp_uniform;        /* The view projection uniform. */
    Uniform models_uniform;    /* The models uniform. */

    FilledSpanProgram();
  };

  /**
   * @brief Line shader program.
   *
   * @struct LineProgram
   */
  struct LineProgram {
    Program program;         /* The shader program. */
    Uniform vp_uniform;      /* The view projection uniform. */
    Uniform zoom_uniform;    /* The zoom uniform. */

    LineProgram();
  };

  /**
   * @brief Rect shader program.
   *
   * @struct RectProgram
   */
  struct RectProgram {
    Program program;       /* The shader program. */
    Uniform vp_uniform;    /* The view projection uniform. */

    RectProgram();
  };

  /**
   * @brief Circle shader program.
   *
   * @struct CircleProgram
   */
  struct CircleProgram {
    Program program;         /* The shader program. */
    Uniform vp_uniform;      /* The view projection uniform. */
    Uniform zoom_uniform;    /* The zoom uniform. */

    CircleProgram();
  };

  /**
   * @brief Groups all of the available shaders together.
   *
   * @struct Programs
   */
  struct Programs {
    PathProgram path_program;                     /* The path shader program. */
    BoundarySpanProgram boundary_span_program;    /* The boundary span shader program. */
    FilledSpanProgram filled_span_program;        /* The filled span shader program. */
    LineProgram line_program;                     /* The line shader program. */
    RectProgram rect_program;                     /* The square shader program. */
    CircleProgram circle_program;                 /* The circle shader program. */
  };

  /**
   * @brief Vertex array to use with PathProgram.
   *
   * @struct PathVertexArray
   */
  struct PathVertexArray {
    VertexArray vertex_array;    /* The vertex array. */

    PathVertexArray(
      const PathProgram& program,
      const Buffer& instance_buffer,
      const Buffer& vertex_buffer
    );
  };

  /**
   * @brief Vertex array to use with BoundarySpanProgram.
   *
   * @struct BoundarySpanVertexArray
   */
  struct BoundarySpanVertexArray {
    VertexArray vertex_array;    /* The vertex array. */

    BoundarySpanVertexArray(
      const BoundarySpanProgram& program,
      const Buffer& instance_buffer,
      const Buffer& vertex_buffer
    );
  };

  /**
   * @brief Vertex array to use with FilledSpanProgram.
   *
   * @struct FilledSpanVertexArray
   */
  struct FilledSpanVertexArray {
    VertexArray vertex_array;    /* The vertex array. */

    FilledSpanVertexArray(
      const FilledSpanProgram& program,
      const Buffer& instance_buffer,
      const Buffer& vertex_buffer
    );
  };

  /**
   * @brief Vertex array to use with LineProgram.
   *
   * @struct LineVertexArray
   */
  struct LineVertexArray {
    VertexArray vertex_array;    /* The vertex array. */

    LineVertexArray(
      const LineProgram& program,
      const Buffer& instance_buffer,
      const Buffer& vertex_buffer
    );
  };


  /**
   * @brief Vertex array to use with RectProgram.
   *
   * @struct RectVertexArray
   */
  struct RectVertexArray {
    VertexArray vertex_array;    /* The vertex array. */

    RectVertexArray(
      const RectProgram& program,
      const Buffer& instance_buffer,
      const Buffer& vertex_buffer
    );
  };

  /**
   * @brief Vertex array to use with CircleProgram.
   *
   * @struct CircleVertexArray
   */
  struct CircleVertexArray {
    VertexArray vertex_array;    /* The vertex array. */

    CircleVertexArray(
      const CircleProgram& program,
      const Buffer& instance_buffer,
      const Buffer& vertex_buffer
    );
  };

}
