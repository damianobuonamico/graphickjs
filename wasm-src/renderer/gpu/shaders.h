#pragma once

#include "device.h"

namespace Graphick::Render::GPU {

  struct FillProgram {
    Program program;
    Uniform view_projection_uniform;
    Uniform framebuffer_size_uniform;
    Uniform tile_size_uniform;

    FillProgram();
  };

  struct LineProgram {
    Program program;
    Uniform view_projection_uniform;
    Uniform color_uniform;

    LineProgram();
  };

  struct Programs {
    FillProgram fill_program;
    LineProgram line_program;

    Programs();
  };

  struct FillVertexArray {
    std::shared_ptr<VertexArray> vertex_array;

    FillVertexArray(
      const FillProgram& fill_program,
      const Buffer& vertex_buffer,
      const Buffer& quad_vertex_positions_buffer,
      const Buffer& quad_vertex_indices_buffer
    );
  };

  struct LineVertexArray {
    std::shared_ptr<VertexArray> vertex_array;

    LineVertexArray(
      const LineProgram& line_program,
      const Buffer& vertex_positions_buffer,
      const Buffer& vertex_indices_buffer
    );
  };

}
