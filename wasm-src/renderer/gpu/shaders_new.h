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
    Program program;    /* The shader program. */
    Uniform mvp_uniform;  // TEMP

    PathProgram();
  };

  /**
   * @brief Groups all of the available shaders together.
   *
   * @struct Programs
   */
  struct Programs {
    PathProgram path_program;   /* The path shader program. */
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

}
