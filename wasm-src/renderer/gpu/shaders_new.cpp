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
    mvp_uniform(Device::get_uniform(program, "uMVP").value()) {}

  /* -- VertexArrays -- */

  PathVertexArray::PathVertexArray(
    const PathProgram& program,
    const Buffer& instance_buffer,
    const Buffer& vertex_buffer
  ) :
    vertex_array(Device::create_vertex_array())
  {
    VertexAttr position_attr = Device::get_vertex_attr(program.program, "aPosition").value();
    VertexAttr instance_size_attr = Device::get_vertex_attr(program.program, "aInstanceSize").value();
    VertexAttr instance_uniforms_index_attr = Device::get_vertex_attr(program.program, "aInstanceUniformsIndex").value();

    VertexAttrDescriptor position_attr_desc = {
      2, VertexAttrClass::Float, VertexAttrType::F32,
      8, 0, 0, 0
    };

    VertexAttrDescriptor instance_size_attr_desc = {
      2, VertexAttrClass::Float, VertexAttrType::F32,
      12, 0, 1, 1
    };

    VertexAttrDescriptor instance_uniforms_index_attr_desc = {
      1, VertexAttrClass::Int, VertexAttrType::I32,
      12, 8, 1, 1
    };

    Device::bind_buffer(*vertex_array, vertex_buffer, BufferTarget::Vertex);
    Device::configure_vertex_attr(*vertex_array, position_attr, position_attr_desc);

    Device::bind_buffer(*vertex_array, instance_buffer, BufferTarget::Vertex);
    Device::configure_vertex_attr(*vertex_array, instance_size_attr, instance_size_attr_desc);
    Device::configure_vertex_attr(*vertex_array, instance_uniforms_index_attr, instance_uniforms_index_attr_desc);
  }

}
