#pragma once

#include "device.h"

namespace Graphick::Renderer::GPU {

  struct OpaqueTileProgram {
    Program program;
    Uniform view_uniform;
    Uniform projection_uniform;
    Uniform framebuffer_size_uniform;
    Uniform tile_size_uniform;

    OpaqueTileProgram();
  };

  struct MaskedTileProgram {
    Program program;
    Uniform view_uniform;
    Uniform projection_uniform;
    Uniform framebuffer_size_uniform;
    Uniform tile_size_uniform;
    Uniform masks_texture_uniform;

    MaskedTileProgram();
  };

  struct LineProgram {
    Program program;
    Uniform view_projection_uniform;
    Uniform color_uniform;
    Uniform line_width_uniform;
    Uniform zoom_uniform;

    LineProgram();
  };

  struct Programs {
    OpaqueTileProgram opaque_tile_program;
    MaskedTileProgram masked_tile_program;
    LineProgram line_program;

    Programs();
  };

  struct OpaqueTileVertexArray {
    std::shared_ptr<VertexArray> vertex_array;

    OpaqueTileVertexArray(
      const OpaqueTileProgram& opaque_tile_program,
      const Buffer& vertex_buffer,
      const Buffer& quad_vertex_positions_buffer,
      const Buffer& quad_vertex_indices_buffer
    );
  };

  struct MaskedTileVertexArray {
    std::shared_ptr<VertexArray> vertex_array;

    MaskedTileVertexArray(
      const MaskedTileProgram& masked_tile_program,
      const Buffer& vertex_buffer,
      const Buffer& quad_vertex_positions_buffer,
      const Buffer& quad_vertex_indices_buffer
    );
  };


  struct LineVertexArray {
    std::shared_ptr<VertexArray> vertex_array;

    LineVertexArray(
      const LineProgram& line_program,
      const Buffer& instance_buffer,
      const Buffer& vertex_positions_buffer,
      const Buffer& vertex_indices_buffer
    );
  };

}
