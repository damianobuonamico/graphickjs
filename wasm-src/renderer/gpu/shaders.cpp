/**
 * @file shaders.cpp
 * @brief Contains the GPU shaders and vertex arrays implementations.
 *
 * @todo remove temp using declarations
 */

#include "shaders.h"

namespace graphick::renderer::GPU {

  /* -- Programs -- */

  PathProgram::PathProgram() :
    program(Device::create_program("path")),
    vp_uniform(Device::get_uniform(program, "uViewProjection").value()),
    viewport_size_uniform(Device::get_uniform(program, "uViewportSize").value()),
    min_samples_uniform(Device::get_uniform(program, "uMinSamples").value()),
    max_samples_uniform(Device::get_uniform(program, "uMaxSamples").value()),
    curves_texture(Device::get_texture_parameter(program, "uCurvesTexture").value()),
    bands_texture(Device::get_texture_parameter(program, "uBandsTexture").value()) {}

  LineProgram::LineProgram() :
    program(Device::create_program("line")),
    vp_uniform(Device::get_uniform(program, "uViewProjection").value()),
    zoom_uniform(Device::get_uniform(program, "uZoom").value()) {}

  RectProgram::RectProgram() :
    program(Device::create_program("rect")),
    vp_uniform(Device::get_uniform(program, "uViewProjection").value()) {}

  CircleProgram::CircleProgram() :
    program(Device::create_program("circle")),
    vp_uniform(Device::get_uniform(program, "uViewProjection").value()),
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
    VertexAttr instance_curves_data_attr = Device::get_vertex_attr(program.program, "aInstanceCurvesData").value();
    VertexAttr instance_bands_data_attr = Device::get_vertex_attr(program.program, "aInstanceBandsData").value();

    VertexAttrDescriptor position_attr_desc = {
      2, VertexAttrClass::Float, VertexAttrType::F32,
      8, 0, 0, 0
    };

    VertexAttrDescriptor instance_first_attr_desc = {
      4, VertexAttrClass::Float, VertexAttrType::F32,
      52, 0, 1, 1
    };

    VertexAttrDescriptor instance_second_attr_desc = {
      2, VertexAttrClass::Float, VertexAttrType::F32,
      52, 16, 1, 1
    };

    VertexAttrDescriptor instance_position_desc = {
      2, VertexAttrClass::Float, VertexAttrType::F32,
      52, 24, 1, 1
    };

    VertexAttrDescriptor instance_size_desc = {
      2, VertexAttrClass::Float, VertexAttrType::F32,
      52, 32, 1, 1
    };

    VertexAttrDescriptor instance_color_attr_desc = {
      4, VertexAttrClass::Int, VertexAttrType::U8,
      52, 40, 1, 1
    };

    VertexAttrDescriptor instance_curves_data_attr_desc = {
      1, VertexAttrClass::Int, VertexAttrType::U32,
      52, 44, 1, 1
    };

    VertexAttrDescriptor instance_bands_data_attr_desc = {
      2, VertexAttrClass::Int, VertexAttrType::U32,
      52, 48, 1, 1
    };

    Device::bind_buffer(*vertex_array, vertex_buffer, BufferTarget::Vertex);
    Device::configure_vertex_attr(*vertex_array, position_attr, position_attr_desc);

    Device::bind_buffer(*vertex_array, instance_buffer, BufferTarget::Vertex);
    Device::configure_vertex_attr(*vertex_array, instance_first_attr, instance_first_attr_desc);
    Device::configure_vertex_attr(*vertex_array, instance_second_attr, instance_second_attr_desc);
    Device::configure_vertex_attr(*vertex_array, instance_position_attr, instance_position_desc);
    Device::configure_vertex_attr(*vertex_array, instance_size_attr, instance_size_desc);
    Device::configure_vertex_attr(*vertex_array, instance_color_attr, instance_color_attr_desc);
    Device::configure_vertex_attr(*vertex_array, instance_curves_data_attr, instance_curves_data_attr_desc);
    Device::configure_vertex_attr(*vertex_array, instance_bands_data_attr, instance_bands_data_attr_desc);
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
    VertexAttr instance_width_attr = Device::get_vertex_attr(program.program, "aInstanceWidth").value();
    VertexAttr instance_color_attr = Device::get_vertex_attr(program.program, "aInstanceColor").value();

    VertexAttrDescriptor position_desc = {
      2, VertexAttrClass::Float, VertexAttrType::F32,
      8, 0, 0, 0
    };

    VertexAttrDescriptor instance_from_desc = {
      2, VertexAttrClass::Float, VertexAttrType::F32,
      24, 0, 1, 1
    };

    VertexAttrDescriptor instance_to_desc = {
      2, VertexAttrClass::Float, VertexAttrType::F32,
      24, 8, 1, 1
    };

    VertexAttrDescriptor instance_width_desc = {
      1, VertexAttrClass::Float, VertexAttrType::F32,
      24, 16, 1, 1
    };

    VertexAttrDescriptor instance_color_attr_desc = {
      4, VertexAttrClass::Int, VertexAttrType::U8,
      24, 20, 1, 1
    };

    Device::bind_buffer(*vertex_array, vertex_buffer, BufferTarget::Vertex);
    Device::configure_vertex_attr(*vertex_array, position_attr, position_desc);

    Device::bind_buffer(*vertex_array, instance_buffer, BufferTarget::Vertex);
    Device::configure_vertex_attr(*vertex_array, instance_from_attr, instance_from_desc);
    Device::configure_vertex_attr(*vertex_array, instance_to_attr, instance_to_desc);
    Device::configure_vertex_attr(*vertex_array, instance_width_attr, instance_width_desc);
    Device::configure_vertex_attr(*vertex_array, instance_color_attr, instance_color_attr_desc);
  }

  RectVertexArray::RectVertexArray(
   const RectProgram& program,
   const Buffer& instance_buffer,
   const Buffer& vertex_buffer
  )
    : vertex_array(Device::create_vertex_array())
  {
    VertexAttr position_attr = Device::get_vertex_attr(program.program, "aPosition").value();
    VertexAttr instance_position_attr = Device::get_vertex_attr(program.program, "aInstancePosition").value();
    VertexAttr instance_size_attr = Device::get_vertex_attr(program.program, "aInstanceSize").value();
    VertexAttr instance_color_attr = Device::get_vertex_attr(program.program, "aInstanceColor").value();

    VertexAttrDescriptor position_desc = {
      2, VertexAttrClass::Float, VertexAttrType::F32,
      8, 0, 0, 0
    };

    VertexAttrDescriptor instance_position_desc = {
      2, VertexAttrClass::Float, VertexAttrType::F32,
      20, 0, 1, 1
    };

    VertexAttrDescriptor instance_size_desc = {
      2, VertexAttrClass::Float, VertexAttrType::F32,
      20, 8, 1, 1
    };

    VertexAttrDescriptor instance_color_attr_desc = {
      4, VertexAttrClass::Int, VertexAttrType::U8,
      20, 16, 1, 1
    };

    Device::bind_buffer(*vertex_array, vertex_buffer, BufferTarget::Vertex);
    Device::configure_vertex_attr(*vertex_array, position_attr, position_desc);

    Device::bind_buffer(*vertex_array, instance_buffer, BufferTarget::Vertex);
    Device::configure_vertex_attr(*vertex_array, instance_position_attr, instance_position_desc);
    Device::configure_vertex_attr(*vertex_array, instance_size_attr, instance_size_desc);
    Device::configure_vertex_attr(*vertex_array, instance_color_attr, instance_color_attr_desc);
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
    VertexAttr instance_radius_attr = Device::get_vertex_attr(program.program, "aInstanceRadius").value();
    VertexAttr instance_color_attr = Device::get_vertex_attr(program.program, "aInstanceColor").value();

    VertexAttrDescriptor position_desc = {
      2, VertexAttrClass::Float, VertexAttrType::F32,
      8, 0, 0, 0
    };

    VertexAttrDescriptor instance_position_desc = {
      2, VertexAttrClass::Float, VertexAttrType::F32,
      16, 0, 1, 1
    };

    VertexAttrDescriptor instance_radius_desc = {
      1, VertexAttrClass::Float, VertexAttrType::F32,
      16, 8, 1, 1
    };

    VertexAttrDescriptor instance_color_attr_desc = {
      4, VertexAttrClass::Int, VertexAttrType::U8,
      16, 12, 1, 1
    };

    Device::bind_buffer(*vertex_array, vertex_buffer, BufferTarget::Vertex);
    Device::configure_vertex_attr(*vertex_array, position_attr, position_desc);

    Device::bind_buffer(*vertex_array, instance_buffer, BufferTarget::Vertex);
    Device::configure_vertex_attr(*vertex_array, instance_position_attr, instance_position_desc);
    Device::configure_vertex_attr(*vertex_array, instance_radius_attr, instance_radius_desc);
    Device::configure_vertex_attr(*vertex_array, instance_color_attr, instance_color_attr_desc);
  }

}
