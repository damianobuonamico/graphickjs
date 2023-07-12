#include "shaders.h"

namespace Graphick::Renderer::GPU {

  OpaqueTileProgram::OpaqueTileProgram() :
    program(Device::create_program("opaque_tile")),
    view_uniform(Device::get_uniform(program, "uViewMatrix").value()),
    projection_uniform(Device::get_uniform(program, "uProjectionMatrix").value()),
    framebuffer_size_uniform(Device::get_uniform(program, "uFramebufferSize").value()),
    tile_size_uniform(Device::get_uniform(program, "uTileSize").value()) {}

  MaskedTileProgram::MaskedTileProgram() :
    program(Device::create_program("masked_tile")),
    view_uniform(Device::get_uniform(program, "uViewMatrix").value()),
    projection_uniform(Device::get_uniform(program, "uProjectionMatrix").value()),
    framebuffer_size_uniform(Device::get_uniform(program, "uFramebufferSize").value()),
    tile_size_uniform(Device::get_uniform(program, "uTileSize").value()),
    masks_texture_uniform(Device::get_uniform(program, "uMasksTexture").value()) {}

  LineProgram::LineProgram() :
    program(Device::create_program("line")),
    view_projection_uniform(Device::get_uniform(program, "uViewProjection").value()),
    color_uniform(Device::get_uniform(program, "uColor").value()) {}

  QuadProgram::QuadProgram() :
    program(Device::create_program("quad")),
    frame_texture_uniform(Device::get_uniform(program, "uTexture").value()) {}

  Programs::Programs() :
    opaque_tile_program(),
    masked_tile_program(),
    line_program(),
    quad_program() {}

  OpaqueTileVertexArray::OpaqueTileVertexArray(
    const OpaqueTileProgram& opaque_tile_program,
    const Buffer& vertex_buffer,
    const Buffer& quad_vertex_positions_buffer,
    const Buffer& quad_vertex_indices_buffer
  )
    : vertex_array(Device::create_vertex_array())
  {
    VertexAttr position_attr = Device::get_vertex_attr(opaque_tile_program.program, "aPosition").value();
    VertexAttr color_attr = Device::get_vertex_attr(opaque_tile_program.program, "aColor").value();
    VertexAttr index_attr = Device::get_vertex_attr(opaque_tile_program.program, "aIndex").value();

    VertexAttrDescriptor position_desc = {
      2,
      VertexAttrClass::Int,
      VertexAttrType::U16,
      4,
      0,
      0,
      0
    };

    VertexAttrDescriptor color_desc = {
      4,
      VertexAttrClass::Float,
      VertexAttrType::F32,
      20,
      0,
      1,
      1
    };

    VertexAttrDescriptor index_desc = {
      1,
      VertexAttrClass::Int,
      VertexAttrType::I32,
      20,
      16,
      1,
      1
    };

    Device::bind_buffer(*vertex_array, quad_vertex_positions_buffer, BufferTarget::Vertex);
    Device::configure_vertex_attr(*vertex_array, position_attr, position_desc);

    Device::bind_buffer(*vertex_array, vertex_buffer, BufferTarget::Vertex);
    Device::configure_vertex_attr(*vertex_array, color_attr, color_desc);
    Device::configure_vertex_attr(*vertex_array, index_attr, index_desc);

    Device::bind_buffer(*vertex_array, quad_vertex_indices_buffer, BufferTarget::Index);
  }

  MaskedTileVertexArray::MaskedTileVertexArray(
    const MaskedTileProgram& masked_tile_program,
    const Buffer& vertex_buffer,
    const Buffer& quad_vertex_positions_buffer,
    const Buffer& quad_vertex_indices_buffer
  )
    : vertex_array(Device::create_vertex_array())
  {
    VertexAttr position_attr = Device::get_vertex_attr(masked_tile_program.program, "aPosition").value();
    VertexAttr color_attr = Device::get_vertex_attr(masked_tile_program.program, "aColor").value();
    VertexAttr index_attr = Device::get_vertex_attr(masked_tile_program.program, "aIndex").value();
    VertexAttr mask_index_attr = Device::get_vertex_attr(masked_tile_program.program, "aMaskIndex").value();

    VertexAttrDescriptor position_desc = {
      2,
      VertexAttrClass::Int,
      VertexAttrType::U16,
      4,
      0,
      0,
      0
    };

    VertexAttrDescriptor color_desc = {
      4,
      VertexAttrClass::Float,
      VertexAttrType::F32,
      24,
      0,
      1,
      1
    };

    VertexAttrDescriptor index_desc = {
      1,
      VertexAttrClass::Int,
      VertexAttrType::I32,
      24,
      16,
      1,
      1
    };

    VertexAttrDescriptor mask_index_desc = {
      1,
      VertexAttrClass::Int,
      VertexAttrType::I32,
      24,
      20,
      1,
      1
    };

    Device::bind_buffer(*vertex_array, quad_vertex_positions_buffer, BufferTarget::Vertex);
    Device::configure_vertex_attr(*vertex_array, position_attr, position_desc);

    Device::bind_buffer(*vertex_array, vertex_buffer, BufferTarget::Vertex);
    Device::configure_vertex_attr(*vertex_array, color_attr, color_desc);
    Device::configure_vertex_attr(*vertex_array, index_attr, index_desc);
    Device::configure_vertex_attr(*vertex_array, mask_index_attr, mask_index_desc);

    Device::bind_buffer(*vertex_array, quad_vertex_indices_buffer, BufferTarget::Index);
  }

  LineVertexArray::LineVertexArray(
    const LineProgram& line_program,
    const Buffer& vertex_positions_buffer,
    const Buffer& vertex_indices_buffer
  )
    : vertex_array(Device::create_vertex_array())
  {
    VertexAttr position_attr = Device::get_vertex_attr(line_program.program, "aPosition").value();

    VertexAttrDescriptor position_desc = {
      2,
      VertexAttrClass::Float,
      VertexAttrType::F32,
      8,
      0,
      0,
      0
    };

    Device::bind_buffer(*vertex_array, vertex_positions_buffer, BufferTarget::Vertex);
    Device::configure_vertex_attr(*vertex_array, position_attr, position_desc);

    Device::bind_buffer(*vertex_array, vertex_indices_buffer, BufferTarget::Index);
  }

  QuadVertexArray::QuadVertexArray(
    const QuadProgram& quad_program,
    const Buffer& vertex_positions_buffer,
    const Buffer& vertex_indices_buffer
  )
    : vertex_array(Device::create_vertex_array())
  {
    VertexAttr position_attr = Device::get_vertex_attr(quad_program.program, "position").value();
    VertexAttr tex_coord_attr = Device::get_vertex_attr(quad_program.program, "texcoord").value();

    VertexAttrDescriptor position_desc = {
      3,
      VertexAttrClass::Float,
      VertexAttrType::F32,
      20,
      0,
      0,
      0
    };

    VertexAttrDescriptor tex_coord_desc = {
      2,
      VertexAttrClass::Float,
      VertexAttrType::F32,
      20,
      12,
      0,
      0
    };

    Device::bind_buffer(*vertex_array, vertex_positions_buffer, BufferTarget::Vertex);
    Device::configure_vertex_attr(*vertex_array, position_attr, position_desc);
    Device::configure_vertex_attr(*vertex_array, tex_coord_attr, tex_coord_desc);

    Device::bind_buffer(*vertex_array, vertex_indices_buffer, BufferTarget::Index);
  }

}
