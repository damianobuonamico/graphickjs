/**
 * @file shaders.cpp
 * @brief Contains the GPU shaders and vertex arrays implementations.
 *
 * @todo remove temp using declarations
 */

#include "shaders_new.h"

namespace Graphick::renderer::GPU {

  //TEMP
  using Device = Graphick::Renderer::GPU::Device;

  /* -- Programs -- */

  PathProgram::PathProgram() :
    program(Device::create_program("path")),
    vp_uniform(Device::get_uniform(program, "uViewProjection").value()),
    curves_texture(Device::get_texture_parameter(program, "uCurvesTexture").value()) {}

  LineProgram::LineProgram() :
    program(Device::create_program("line")),
    vp_uniform(Device::get_uniform(program, "uViewProjection").value()),
    color_uniform(Device::get_uniform(program, "uColor").value()),
    line_width_uniform(Device::get_uniform(program, "uLineWidth").value()),
    zoom_uniform(Device::get_uniform(program, "uZoom").value()) {}

  SquareProgram::SquareProgram() :
    program(Device::create_program("square")),
    vp_uniform(Device::get_uniform(program, "uViewProjection").value()),
    color_uniform(Device::get_uniform(program, "uColor").value()),
    size_uniform(Device::get_uniform(program, "uSize").value()) {}

  CircleProgram::CircleProgram() :
    program(Device::create_program("circle")),
    vp_uniform(Device::get_uniform(program, "uViewProjection").value()),
    color_uniform(Device::get_uniform(program, "uColor").value()),
    radius_uniform(Device::get_uniform(program, "uRadius").value()),
    zoom_uniform(Device::get_uniform(program, "uZoom").value()) {}

  /* -- VertexArrays -- */

  PathVertexArray::PathVertexArray(
    const PathProgram& program,
    const Buffer& instance_buffer,
    const Buffer& vertex_buffer
  ) :
    vertex_array(Device::create_vertex_array())
  {
    VertexAttr position_attr = Device::get_vertex_attr(program.program, "aPosition").value();
    VertexAttr instance_first_attr = Device::get_vertex_attr(program.program, "aInstanceAttrib1").value();
    VertexAttr instance_second_attr = Device::get_vertex_attr(program.program, "aInstanceAttrib2").value();
    VertexAttr instance_position_attr = Device::get_vertex_attr(program.program, "aInstancePosition").value();
    VertexAttr instance_size_attr = Device::get_vertex_attr(program.program, "aInstanceSize").value();
    VertexAttr instance_color_attr = Device::get_vertex_attr(program.program, "aInstanceColor").value();

    VertexAttrDescriptor position_attr_desc = {
      2, VertexAttrClass::Float, VertexAttrType::F32,
      8, 0, 0, 0
    };

    VertexAttrDescriptor instance_first_attr_desc = {
      4, VertexAttrClass::Float, VertexAttrType::F32,
      44, 0, 1, 1
    };

    VertexAttrDescriptor instance_second_attr_desc = {
      2, VertexAttrClass::Float, VertexAttrType::F32,
      44, 16, 1, 1
    };

    VertexAttrDescriptor instance_position_desc = {
      2, VertexAttrClass::Float, VertexAttrType::F32,
      44, 24, 1, 1
    };

    VertexAttrDescriptor instance_size_desc = {
      2, VertexAttrClass::Float, VertexAttrType::F32,
      44, 32, 1, 1
    };

    VertexAttrDescriptor instance_color_attr_desc = {
      4, VertexAttrClass::Int, VertexAttrType::U8,
      44, 40, 1, 1
    };

    Device::bind_buffer(*vertex_array, vertex_buffer, BufferTarget::Vertex);
    Device::configure_vertex_attr(*vertex_array, position_attr, position_attr_desc);

    Device::bind_buffer(*vertex_array, instance_buffer, BufferTarget::Vertex);
    Device::configure_vertex_attr(*vertex_array, instance_first_attr, instance_first_attr_desc);
    Device::configure_vertex_attr(*vertex_array, instance_second_attr, instance_second_attr_desc);
    Device::configure_vertex_attr(*vertex_array, instance_position_attr, instance_position_desc);
    Device::configure_vertex_attr(*vertex_array, instance_size_attr, instance_size_desc);
    Device::configure_vertex_attr(*vertex_array, instance_color_attr, instance_color_attr_desc);
  }

  LineVertexArray::LineVertexArray(
    const LineProgram& program,
    const Buffer& instance_buffer,
    const Buffer& vertex_buffer
  )
    : vertex_array(Device::create_vertex_array())
  {
    VertexAttr position_attr = Device::get_vertex_attr(program.program, "aPosition").value();
    VertexAttr instance_from_attr = Device::get_vertex_attr(program.program, "aInstanceFrom").value();
    VertexAttr instance_to_attr = Device::get_vertex_attr(program.program, "aInstanceTo").value();

    VertexAttrDescriptor position_desc = {
      2, VertexAttrClass::Float, VertexAttrType::F32,
      8, 0, 0, 0
    };

    VertexAttrDescriptor instance_from_desc = {
      2, VertexAttrClass::Float, VertexAttrType::F32,
      16, 0, 1, 1
    };

    VertexAttrDescriptor instance_to_desc = {
      2, VertexAttrClass::Float, VertexAttrType::F32,
      16, 8, 1, 1
    };

    Device::bind_buffer(*vertex_array, vertex_buffer, BufferTarget::Vertex);
    Device::configure_vertex_attr(*vertex_array, position_attr, position_desc);

    Device::bind_buffer(*vertex_array, instance_buffer, BufferTarget::Vertex);
    Device::configure_vertex_attr(*vertex_array, instance_from_attr, instance_from_desc);
    Device::configure_vertex_attr(*vertex_array, instance_to_attr, instance_to_desc);
  }

  SquareVertexArray::SquareVertexArray(
   const SquareProgram& program,
   const Buffer& instance_buffer,
   const Buffer& vertex_buffer
  )
    : vertex_array(Device::create_vertex_array())
  {
    VertexAttr position_attr = Device::get_vertex_attr(program.program, "aPosition").value();
    VertexAttr instance_position_attr = Device::get_vertex_attr(program.program, "aInstancePosition").value();

    VertexAttrDescriptor position_desc = {
      2, VertexAttrClass::Float, VertexAttrType::F32,
      8, 0, 0, 0
    };

    VertexAttrDescriptor instance_position_desc = {
      2, VertexAttrClass::Float, VertexAttrType::F32,
      8, 0, 1, 1
    };

    Device::bind_buffer(*vertex_array, vertex_buffer, BufferTarget::Vertex);
    Device::configure_vertex_attr(*vertex_array, position_attr, position_desc);

    Device::bind_buffer(*vertex_array, instance_buffer, BufferTarget::Vertex);
    Device::configure_vertex_attr(*vertex_array, instance_position_attr, instance_position_desc);
  }

  CircleVertexArray::CircleVertexArray(
   const CircleProgram& program,
   const Buffer& instance_buffer,
   const Buffer& vertex_buffer
  )
    : vertex_array(Device::create_vertex_array())
  {
    VertexAttr position_attr = Device::get_vertex_attr(program.program, "aPosition").value();
    VertexAttr instance_position_attr = Device::get_vertex_attr(program.program, "aInstancePosition").value();

    VertexAttrDescriptor position_desc = {
      2, VertexAttrClass::Float, VertexAttrType::F32,
      8, 0, 0, 0
    };

    VertexAttrDescriptor instance_position_desc = {
      2, VertexAttrClass::Float, VertexAttrType::F32,
      8, 0, 1, 1
    };

    Device::bind_buffer(*vertex_array, vertex_buffer, BufferTarget::Vertex);
    Device::configure_vertex_attr(*vertex_array, position_attr, position_desc);

    Device::bind_buffer(*vertex_array, instance_buffer, BufferTarget::Vertex);
    Device::configure_vertex_attr(*vertex_array, instance_position_attr, instance_position_desc);
  }

}
