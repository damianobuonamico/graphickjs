/**
 * @file shaders.h
 * @brief Contains the GPU shaders and vertex arrays definitions.
 *
 * @todo remove unnecessary shaders and merge similar ones
 */

#pragma once

#include "device.h"

namespace Graphick::Renderer::GPU {

  /**
   * @brief Default shader program.
   *
   * @struct DefaultProgram
   */
  struct DefaultProgram {
    Program program;                    /* The shader program. */
    Uniform view_projection_uniform;    /* The view projection uniform. */
    Uniform color_uniform;              /* The color uniform. */
    TextureParameter texture;           /* The texture uniform. */

    DefaultProgram();
  };

  /**
   * @brief Opaque tile shader program.
   *
   * @struct OpaqueTileProgram
   */
  struct OpaqueTileProgram {
    Program program;                     /* The shader program. */
    Uniform offset_uniform;              /* The offset uniform. */
    Uniform framebuffer_size_uniform;    /* The framebuffer size uniform. */
    Uniform tile_size_uniform;           /* The tile size uniform. */

    OpaqueTileProgram();
  };

  /**
   * @brief Masked tile shader program.
   *
   * @struct MaskedTileProgram
   */
  struct MaskedTileProgram {
    Program program;                             /* The shader program. */
    Uniform offset_uniform;                      /* The offset uniform. */
    Uniform framebuffer_size_uniform;            /* The framebuffer size uniform. */
    Uniform tile_size_uniform;                   /* The tile size uniform. */
    Uniform masks_texture_size_uniform;          /* The masks texture size uniform. */
    Uniform cover_table_texture_size_uniform;    /* The cover table texture size uniform. */
    TextureParameter masks_texture;              /* The masks texture uniform. */
    TextureParameter cover_table_texture;        /* The cover table texture uniform. */

    MaskedTileProgram();
  };

  /**
   * @brief Line shader program.
   *
   * @struct LineProgram
   */
  struct LineProgram {
    Program program;                    /* The shader program. */
    Uniform view_projection_uniform;    /* The view projection uniform. */
    Uniform color_uniform;              /* The color uniform. */
    Uniform line_width_uniform;         /* The line width uniform. */
    Uniform zoom_uniform;               /* The zoom uniform. */

    LineProgram();
  };

  /**
   * @brief Square shader program.
   *
   * @struct SquareProgram
   */
  struct SquareProgram {
    Program program;                    /* The shader program. */
    Uniform view_projection_uniform;    /* The view projection uniform. */
    Uniform color_uniform;              /* The color uniform. */
    Uniform size_uniform;               /* The size uniform. */

    SquareProgram();
  };

  /**
   * @brief Circle shader program.
   *
   * @struct CircleProgram
   */
  struct CircleProgram {
    Program program;                    /* The shader program. */
    Uniform view_projection_uniform;    /* The view projection uniform. */
    Uniform color_uniform;              /* The color uniform. */
    Uniform radius_uniform;             /* The radius uniform. */
    Uniform zoom_uniform;               /* The zoom uniform. */

    CircleProgram();
  };

  /**
   * @brief Groups all of the available shaders together.
   *
   * @struct Programs
   */
  struct Programs {
    DefaultProgram default_program;           /* The default shader program. */
    OpaqueTileProgram opaque_tile_program;    /* The opaque tile shader program. */
    MaskedTileProgram masked_tile_program;    /* The masked tile shader program. */
    LineProgram line_program;                 /* The line shader program. */
    SquareProgram square_program;             /* The square shader program. */
    CircleProgram circle_program;             /* The circle shader program. */

    Programs();
  };

  /**
   * @brief Vertex array.
   *
   * @struct VertexArray
   */
  struct DefaultVertexArray {
    std::shared_ptr<VertexArray> vertex_array;    /* The vertex array. */

    DefaultVertexArray(
      const DefaultProgram& default_program,
      const Buffer& vertex_buffer
    );
  };

  /**
   * @brief Vertex array.
   *
   * @struct OpaqueTileVertexArray
   */
  struct OpaqueTileVertexArray {
    std::shared_ptr<VertexArray> vertex_array;    /* The vertex array. */

    OpaqueTileVertexArray(
      const OpaqueTileProgram& opaque_tile_program,
      const Buffer& vertex_buffer,
      const Buffer& quad_vertex_positions_buffer,
      const Buffer& quad_vertex_indices_buffer
    );
  };

  /**
   * @brief Vertex array.
   *
   * @struct MaskedTileVertexArray
   */
  struct MaskedTileVertexArray {
    std::shared_ptr<VertexArray> vertex_array;    /* The vertex array. */

    MaskedTileVertexArray(
      const MaskedTileProgram& masked_tile_program,
      const Buffer& vertex_buffer,
      const Buffer& quad_vertex_positions_buffer,
      const Buffer& quad_vertex_indices_buffer
    );
  };

  /**
   * @brief Vertex array.
   */
  struct LineVertexArray {
    std::shared_ptr<VertexArray> vertex_array;    /* The vertex array. */

    LineVertexArray(
      const LineProgram& line_program,
      const Buffer& instance_buffer,
      const Buffer& vertex_positions_buffer,
      const Buffer& vertex_indices_buffer
    );
  };

  /**
   * @brief Vertex array.
   *
   * @struct SquareVertexArray
   */
  struct SquareVertexArray {
    std::shared_ptr<VertexArray> vertex_array;    /* The vertex array. */

    SquareVertexArray(
      const SquareProgram& square_program,
      const Buffer& instance_buffer,
      const Buffer& vertex_positions_buffer,
      const Buffer& vertex_indices_buffer
    );
  };

  /**
   * @brief Vertex array.
   *
   * @struct CircleVertexArray
   */
  struct CircleVertexArray {
    std::shared_ptr<VertexArray> vertex_array;    /* The vertex array. */

    CircleVertexArray(
      const CircleProgram& circle_program,
      const Buffer& instance_buffer,
      const Buffer& vertex_positions_buffer,
      const Buffer& vertex_indices_buffer
    );
  };

}
