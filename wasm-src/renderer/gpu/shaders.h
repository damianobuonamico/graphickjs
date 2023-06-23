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

  struct MaskProgram {
    Program program;
    Uniform view_projection_uniform;
    Uniform framebuffer_size_uniform;
    Uniform tile_size_uniform;

    MaskProgram();
  };

  struct TileProgram {
    Program program;
    // Uniform view_projection_uniform;
    Uniform view_uniform;
    Uniform projection_uniform;
    Uniform framebuffer_size_uniform;
    Uniform tile_size_uniform;
    Uniform segments_texture_uniform;
    // Uniform mask_texture_uniform;

    TileProgram();
  };

  struct SpanProgram {
    Program program;
    // Uniform view_projection_uniform;
    Uniform view_uniform;
    Uniform projection_uniform;
    Uniform framebuffer_size_uniform;
    Uniform tile_size_uniform;
    // Uniform mask_texture_uniform;

    SpanProgram();
  };

  struct LineProgram {
    Program program;
    Uniform view_projection_uniform;
    Uniform color_uniform;

    LineProgram();
  };

  struct Programs {
    FillProgram fill_program;
    MaskProgram mask_program;
    SpanProgram span_program;
    TileProgram tile_program;
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

  struct MaskVertexArray {
    std::shared_ptr<VertexArray> vertex_array;

    MaskVertexArray(
      const MaskProgram& mask_program,
      const Buffer& vertex_buffer,
      const Buffer& quad_vertex_positions_buffer,
      const Buffer& quad_vertex_indices_buffer
    );
  };

  struct TileVertexArray {
    std::shared_ptr<VertexArray> vertex_array;

    TileVertexArray(
      const TileProgram& tile_program,
      const Buffer& vertex_buffer,
      const Buffer& quad_vertex_positions_buffer,
      const Buffer& quad_vertex_indices_buffer
    );
  };

  struct SpanVertexArray {
    std::shared_ptr<VertexArray> vertex_array;

    SpanVertexArray(
      const SpanProgram& span_program,
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
