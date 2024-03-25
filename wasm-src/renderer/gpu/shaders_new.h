/**
 * @file shaders.h
 * @brief Contains the GPU shaders and vertex arrays definitions.
 *
 * @todo remove temp using declarations
 */

#pragma once

#include "device.h"

namespace Graphick::renderer::GPU {

  //TEMP
  using namespace Graphick::Renderer::GPU;

  /**
   * @brief The main shader program.
   *
   * @struct PathProgram
   */
  struct PathProgram {
    Program program;                    /* The shader program. */
    Uniform vp_uniform;                 /* The view projection uniform. */
    TextureParameter curves_texture;    /* The curves texture. */

    PathProgram();
  };

  /**
   * @brief Line shader program.
   *
   * @struct LineProgram
   */
  struct LineProgram {
    Program program;               /* The shader program. */
    Uniform vp_uniform;            /* The view projection uniform. */
    Uniform color_uniform;         /* The color uniform. */
    Uniform line_width_uniform;    /* The line width uniform. */
    Uniform zoom_uniform;          /* The zoom uniform. */

    LineProgram();
  };

  /**
   * @brief Square shader program.
   *
   * @struct SquareProgram
   */
  struct SquareProgram {
    Program program;          /* The shader program. */
    Uniform vp_uniform;       /* The view projection uniform. */
    Uniform color_uniform;    /* The color uniform. */
    Uniform size_uniform;     /* The size uniform. */

    SquareProgram();
  };

  /**
   * @brief Circle shader program.
   *
   * @struct CircleProgram
   */
  struct CircleProgram {
    Program program;           /* The shader program. */
    Uniform vp_uniform;        /* The view projection uniform. */
    Uniform color_uniform;     /* The color uniform. */
    Uniform radius_uniform;    /* The radius uniform. */
    Uniform zoom_uniform;      /* The zoom uniform. */

    CircleProgram();
  };

  /**
   * @brief Groups all of the available shaders together.
   *
   * @struct Programs
   */
  struct Programs {
    PathProgram path_program;        /* The path shader program. */
    LineProgram line_program;        /* The line shader program. */
    SquareProgram square_program;    /* The square shader program. */
    CircleProgram circle_program;    /* The circle shader program. */
  };

  /**
   * @brief Vertex array to use with PathProgram.
   *
   * @struct PathVertexArray
   */
  struct PathVertexArray {
    std::shared_ptr<VertexArray> vertex_array;    /* The vertex array. */

    PathVertexArray(
      const PathProgram& program,
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
    std::shared_ptr<VertexArray> vertex_array;    /* The vertex array. */

    LineVertexArray(
      const LineProgram& program,
      const Buffer& instance_buffer,
      const Buffer& vertex_buffer
    );
  };


  /**
   * @brief Vertex array to use with SquareProgram.
   *
   * @struct SquareVertexArray
   */
  struct SquareVertexArray {
    std::shared_ptr<VertexArray> vertex_array;    /* The vertex array. */

    SquareVertexArray(
      const SquareProgram& program,
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
    std::shared_ptr<VertexArray> vertex_array;    /* The vertex array. */

    CircleVertexArray(
      const CircleProgram& program,
      const Buffer& instance_buffer,
      const Buffer& vertex_buffer
    );
  };

}
