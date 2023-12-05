#pragma once

#include "device.h"

namespace Graphick::Renderer::GPU {

  struct DefaultProgram {
    Program program;
    Uniform view_projection_uniform;
    Uniform color_uniform;
    TextureParameter texture;

    DefaultProgram();
  };

  struct OpaqueTileProgram {
    Program program;
    Uniform offset_uniform;
    Uniform framebuffer_size_uniform;
    Uniform tile_size_uniform;

    OpaqueTileProgram();
  };

  struct MaskedTileProgram {
    Program program;
    Uniform offset_uniform;
    Uniform framebuffer_size_uniform;
    Uniform tile_size_uniform;
    Uniform masks_texture_size_uniform;
    Uniform cover_table_texture_size_uniform;
    TextureParameter masks_texture;
    TextureParameter cover_table_texture;

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

  struct SquareProgram {
    Program program;
    Uniform view_projection_uniform;
    Uniform color_uniform;
    Uniform size_uniform;

    SquareProgram();
  };

  struct CircleProgram {
    Program program;
    Uniform view_projection_uniform;
    Uniform color_uniform;
    Uniform radius_uniform;
    Uniform zoom_uniform;

    CircleProgram();
  };

  struct GPUPathProgram {
    Program program;
    Uniform view_projection_uniform;
    Uniform color_uniform;
    Uniform paths_texture_size_uniform;
    TextureParameter paths_texture;

    GPUPathProgram();
  };

  struct Programs {
    DefaultProgram default_program;
    OpaqueTileProgram opaque_tile_program;
    MaskedTileProgram masked_tile_program;
    LineProgram line_program;
    SquareProgram square_program;
    CircleProgram circle_program;
    GPUPathProgram gpu_path_program;

    Programs();
  };

  struct DefaultVertexArray {
    std::shared_ptr<VertexArray> vertex_array;

    DefaultVertexArray(
      const DefaultProgram& default_program,
      const Buffer& vertex_buffer
    );
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

  struct SquareVertexArray {
    std::shared_ptr<VertexArray> vertex_array;

    SquareVertexArray(
      const SquareProgram& square_program,
      const Buffer& instance_buffer,
      const Buffer& vertex_positions_buffer,
      const Buffer& vertex_indices_buffer
    );
  };

  struct CircleVertexArray {
    std::shared_ptr<VertexArray> vertex_array;

    CircleVertexArray(
      const CircleProgram& circle_program,
      const Buffer& instance_buffer,
      const Buffer& vertex_positions_buffer,
      const Buffer& vertex_indices_buffer
    );
  };

  struct GPUPathVertexArray {
    std::shared_ptr<VertexArray> vertex_array;

    GPUPathVertexArray(
      const GPUPathProgram& gpu_path_program,
      const Buffer& instance_buffer,
      const Buffer& quad_vertex_positions_buffer,
      const Buffer& quad_vertex_indices_buffer
    );
  };

}
