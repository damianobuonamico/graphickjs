#include "shaders.h"

namespace Graphick::Render::GPU {

  FillProgram::FillProgram() :
    program(Device::create_program("fill")),
    view_projection_uniform(Device::get_uniform(program, "uViewProjection").value()),
    framebuffer_size_uniform(Device::get_uniform(program, "uFramebufferSize").value()),
    tile_size_uniform(Device::get_uniform(program, "uTileSize").value()) {}

  LineProgram::LineProgram() :
    program(Device::create_program("line")),
    view_projection_uniform(Device::get_uniform(program, "uViewProjection").value()),
    color_uniform(Device::get_uniform(program, "uColor").value()) {}

  Programs::Programs() :
    fill_program(),
    line_program() {}

  FillVertexArray::FillVertexArray(
    const FillProgram& fill_program,
    const Buffer& vertex_buffer,
    const Buffer& quad_vertex_positions_buffer,
    const Buffer& quad_vertex_indices_buffer
  )
    : vertex_array(Device::create_vertex_array())
  {
    VertexAttr position_attr = Device::get_vertex_attr(fill_program.program, "aPosition").value();
    VertexAttr index_attr = Device::get_vertex_attr(fill_program.program, "aIndex").value();
    VertexAttr color_attr = Device::get_vertex_attr(fill_program.program, "aColor").value();

    VertexAttrDescriptor position_desc = {
      2,
      VertexAttrClass::Int,
      VertexAttrType::U16,
      4,
      0,
      0,
      0
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

    VertexAttrDescriptor color_desc = {
      4,
      VertexAttrClass::Float,
      VertexAttrType::F32,
      20,
      0,
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

}
