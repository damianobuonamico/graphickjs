/**
 * @file renderer/gpu/shaders.cpp
 * @brief Contains the GPU shaders and vertex arrays implementations.
 */

#include "shaders.h"

#include <sstream>

namespace graphick::renderer::GPU {

  /* -- Programs -- */

  PathProgram::PathProgram() :
    program(Device::create_program("path")),
    vp_uniform(Device::get_uniform(program, "uViewProjection")),
    viewport_size_uniform(Device::get_uniform(program, "uViewportSize")),
    min_samples_uniform(Device::get_uniform(program, "uMinSamples")),
    max_samples_uniform(Device::get_uniform(program, "uMaxSamples")),
    curves_texture(Device::get_texture_uniform(program, "uCurvesTexture")),
    bands_texture(Device::get_texture_uniform(program, "uBandsTexture")) {}

  BoundarySpanProgram::BoundarySpanProgram() :
    program(Device::create_program("boundary_span", { { "MAX_MODELS", (std::stringstream() << (Device::max_vertex_uniform_vectors() - 6)).str() } })),
    vp_uniform(Device::get_uniform(program, "u_view_projection")),
    viewport_size_uniform(Device::get_uniform(program, "u_viewport_size")),
    max_samples_uniform(Device::get_uniform(program, "u_max_samples")),
    models_uniform(Device::get_uniform(program, "u_models")),
    curves_texture(Device::get_texture_uniform(program, "u_curves_texture")) {}

  FilledSpanProgram::FilledSpanProgram() :
    program(Device::create_program("filled_span", { { "MAX_MODELS", (std::stringstream() << (Device::max_vertex_uniform_vectors() - 6)).str() } })),
    vp_uniform(Device::get_uniform(program, "u_view_projection")),
    models_uniform(Device::get_uniform(program, "u_models")) {}

  LineProgram::LineProgram() :
    program(Device::create_program("line")),
    vp_uniform(Device::get_uniform(program, "uViewProjection")),
    zoom_uniform(Device::get_uniform(program, "uZoom")) {}

  RectProgram::RectProgram() :
    program(Device::create_program("rect")),
    vp_uniform(Device::get_uniform(program, "uViewProjection")) {}

  CircleProgram::CircleProgram() :
    program(Device::create_program("circle")),
    vp_uniform(Device::get_uniform(program, "uViewProjection")),
    zoom_uniform(Device::get_uniform(program, "uZoom")) {}

  /* -- VertexArrays -- */

  PathVertexArray::PathVertexArray(
    const PathProgram& program,
    const Buffer& instance_buffer,
    const Buffer& vertex_buffer
  ) {
    VertexAttribute position_attr = Device::get_vertex_attribute(program.program, "aPosition");
    VertexAttribute instance_first_attr = Device::get_vertex_attribute(program.program, "aInstanceAttrib1");
    VertexAttribute instance_second_attr = Device::get_vertex_attribute(program.program, "aInstanceAttrib2");
    VertexAttribute instance_position_attr = Device::get_vertex_attribute(program.program, "aInstancePosition");
    VertexAttribute instance_size_attr = Device::get_vertex_attribute(program.program, "aInstanceSize");
    VertexAttribute instance_color_attr = Device::get_vertex_attribute(program.program, "aInstanceColor");
    VertexAttribute instance_curves_data_attr = Device::get_vertex_attribute(program.program, "aInstanceCurvesData");
    VertexAttribute instance_bands_data_attr = Device::get_vertex_attribute(program.program, "aInstanceBandsData");

    VertexAttrDescriptor position_desc = {
      VertexAttrClass::Int, VertexAttrType::U8,
      2, 2, 0, 0, 0
    };

    VertexAttrDescriptor instance_first_desc = {
      VertexAttrClass::Float, VertexAttrType::F32,
      4, 52, 0, 1, 1
    };

    VertexAttrDescriptor instance_second_desc = {
      VertexAttrClass::Float, VertexAttrType::F32,
      2, 52, 16, 1, 1
    };

    VertexAttrDescriptor instance_position_desc = {
      VertexAttrClass::Float, VertexAttrType::F32,
      2, 52, 24, 1, 1
    };

    VertexAttrDescriptor instance_size_desc = {
      VertexAttrClass::Float, VertexAttrType::F32,
      2, 52, 32, 1, 1
    };

    VertexAttrDescriptor instance_color_desc = {
      VertexAttrClass::Int, VertexAttrType::U8,
      4, 52, 40, 1, 1
    };

    VertexAttrDescriptor instance_curves_data_desc = {
      VertexAttrClass::Int, VertexAttrType::U32,
      1, 52, 44, 1, 1
    };

    VertexAttrDescriptor instance_bands_data_desc = {
      VertexAttrClass::Int, VertexAttrType::U32,
      2, 52, 48, 1, 1
    };

    vertex_buffer.bind(vertex_array);
    vertex_array.configure_attribute(position_attr, position_desc);

    instance_buffer.bind(vertex_array);
    vertex_array.configure_attribute(instance_first_attr, instance_first_desc);
    vertex_array.configure_attribute(instance_second_attr, instance_second_desc);
    vertex_array.configure_attribute(instance_position_attr, instance_position_desc);
    vertex_array.configure_attribute(instance_size_attr, instance_size_desc);
    vertex_array.configure_attribute(instance_color_attr, instance_color_desc);
    vertex_array.configure_attribute(instance_curves_data_attr, instance_curves_data_desc);
    vertex_array.configure_attribute(instance_bands_data_attr, instance_bands_data_desc);
  }

  BoundarySpanVertexArray::BoundarySpanVertexArray(
    const BoundarySpanProgram& program,
    const Buffer& instance_buffer,
    const Buffer& vertex_buffer
  ) {
    VertexAttribute position_attr = Device::get_vertex_attribute(program.program, "a_position");
    VertexAttribute instance_position_attr = Device::get_vertex_attribute(program.program, "a_instance_position");
    VertexAttribute instance_size_attr = Device::get_vertex_attribute(program.program, "a_instance_size");
    VertexAttribute instance_color_attr = Device::get_vertex_attribute(program.program, "a_instance_color");
    VertexAttribute instance_first_attr = Device::get_vertex_attribute(program.program, "a_instance_attr_1");
    VertexAttribute instance_second_attr = Device::get_vertex_attribute(program.program, "a_instance_attr_2");
    VertexAttribute instance_third_attr = Device::get_vertex_attribute(program.program, "a_instance_attr_3");

    VertexAttrDescriptor position_desc = {
      VertexAttrClass::Int, VertexAttrType::U8,
      2, 2, 0, 0, 0
    };

    VertexAttrDescriptor instance_position_desc = {
      VertexAttrClass::Float, VertexAttrType::F32,
      2, 32, 0, 1, 1
    };

    VertexAttrDescriptor instance_size_desc = {
      VertexAttrClass::Float, VertexAttrType::F32,
      2, 32, 8, 1, 1
    };

    VertexAttrDescriptor instance_color_desc = {
      VertexAttrClass::Int, VertexAttrType::U8,
      4, 32, 16, 1, 1
    };

    VertexAttrDescriptor instance_first_desc = {
      VertexAttrClass::Int, VertexAttrType::U32,
      1, 32, 20, 1, 1
    };

    VertexAttrDescriptor instance_second_desc = {
      VertexAttrClass::Int, VertexAttrType::U32,
      1, 32, 24, 1, 1
    };

    VertexAttrDescriptor instance_third_desc = {
      VertexAttrClass::Int, VertexAttrType::U32,
      1, 32, 28, 1, 1
    };

    vertex_buffer.bind(vertex_array);
    vertex_array.configure_attribute(position_attr, position_desc);

    instance_buffer.bind(vertex_array);
    vertex_array.configure_attribute(instance_position_attr, instance_position_desc);
    vertex_array.configure_attribute(instance_size_attr, instance_size_desc);
    vertex_array.configure_attribute(instance_color_attr, instance_color_desc);
    vertex_array.configure_attribute(instance_first_attr, instance_first_desc);
    vertex_array.configure_attribute(instance_second_attr, instance_second_desc);
    vertex_array.configure_attribute(instance_third_attr, instance_third_desc);
  }

  FilledSpanVertexArray::FilledSpanVertexArray(
    const FilledSpanProgram& program,
    const Buffer& instance_buffer,
    const Buffer& vertex_buffer
  ) {
    VertexAttribute position_attr = Device::get_vertex_attribute(program.program, "a_position");
    VertexAttribute instance_position_attr = Device::get_vertex_attribute(program.program, "a_instance_position");
    VertexAttribute instance_size_attr = Device::get_vertex_attribute(program.program, "a_instance_size");
    VertexAttribute instance_color_attr = Device::get_vertex_attribute(program.program, "a_instance_color");
    VertexAttribute instance_first_attr = Device::get_vertex_attribute(program.program, "a_instance_attr_1");

    VertexAttrDescriptor position_desc = {
      VertexAttrClass::Int, VertexAttrType::U8,
      2, 2, 0, 0, 0
    };

    VertexAttrDescriptor instance_position_desc = {
      VertexAttrClass::Float, VertexAttrType::F32,
      2, 24, 0, 1, 1
    };

    VertexAttrDescriptor instance_size_desc = {
      VertexAttrClass::Float, VertexAttrType::F32,
      2, 24, 8, 1, 1
    };

    VertexAttrDescriptor instance_color_desc = {
      VertexAttrClass::Int, VertexAttrType::U8,
      4, 24, 16, 1, 1
    };

    VertexAttrDescriptor instance_first_desc = {
      VertexAttrClass::Int, VertexAttrType::U32,
      1, 24, 20, 1, 1
    };

    vertex_buffer.bind(vertex_array);
    vertex_array.configure_attribute(position_attr, position_desc);

    instance_buffer.bind(vertex_array);
    vertex_array.configure_attribute(instance_position_attr, instance_position_desc);
    vertex_array.configure_attribute(instance_size_attr, instance_size_desc);
    vertex_array.configure_attribute(instance_color_attr, instance_color_desc);
    vertex_array.configure_attribute(instance_first_attr, instance_first_desc);
  }

  LineVertexArray::LineVertexArray(
    const LineProgram& program,
    const Buffer& instance_buffer,
    const Buffer& vertex_buffer
  ) {
    VertexAttribute position_attr = Device::get_vertex_attribute(program.program, "aPosition");
    VertexAttribute instance_from_attr = Device::get_vertex_attribute(program.program, "aInstanceFrom");
    VertexAttribute instance_to_attr = Device::get_vertex_attribute(program.program, "aInstanceTo");
    VertexAttribute instance_width_attr = Device::get_vertex_attribute(program.program, "aInstanceWidth");
    VertexAttribute instance_color_attr = Device::get_vertex_attribute(program.program, "aInstanceColor");

    VertexAttrDescriptor position_desc = {
      VertexAttrClass::Int, VertexAttrType::U8,
      2, 2, 0, 0, 0
    };

    VertexAttrDescriptor instance_from_desc = {
      VertexAttrClass::Float, VertexAttrType::F32,
      2, 24, 0, 1, 1
    };

    VertexAttrDescriptor instance_to_desc = {
      VertexAttrClass::Float, VertexAttrType::F32,
      2, 24, 8, 1, 1
    };

    VertexAttrDescriptor instance_width_desc = {
      VertexAttrClass::Float, VertexAttrType::F32,
      1, 24, 16, 1, 1
    };

    VertexAttrDescriptor instance_color_desc = {
      VertexAttrClass::Int, VertexAttrType::U8,
      4, 24, 20, 1, 1
    };

    vertex_buffer.bind(vertex_array);
    vertex_array.configure_attribute(position_attr, position_desc);

    instance_buffer.bind(vertex_array);
    vertex_array.configure_attribute(instance_from_attr, instance_from_desc);
    vertex_array.configure_attribute(instance_to_attr, instance_to_desc);
    vertex_array.configure_attribute(instance_width_attr, instance_width_desc);
    vertex_array.configure_attribute(instance_color_attr, instance_color_desc);
  }

  RectVertexArray::RectVertexArray(
   const RectProgram& program,
   const Buffer& instance_buffer,
   const Buffer& vertex_buffer
  ) {
    VertexAttribute position_attr = Device::get_vertex_attribute(program.program, "aPosition");
    VertexAttribute instance_position_attr = Device::get_vertex_attribute(program.program, "aInstancePosition");
    VertexAttribute instance_size_attr = Device::get_vertex_attribute(program.program, "aInstanceSize");
    VertexAttribute instance_color_attr = Device::get_vertex_attribute(program.program, "aInstanceColor");

    VertexAttrDescriptor position_desc = {
      VertexAttrClass::Int, VertexAttrType::U8,
      2, 2, 0, 0, 0
    };

    VertexAttrDescriptor instance_position_desc = {
      VertexAttrClass::Float, VertexAttrType::F32,
      2, 20, 0, 1, 1
    };

    VertexAttrDescriptor instance_size_desc = {
      VertexAttrClass::Float, VertexAttrType::F32,
      2, 20, 8, 1, 1
    };

    VertexAttrDescriptor instance_color_desc = {
      VertexAttrClass::Int, VertexAttrType::U8,
      4, 20, 16, 1, 1
    };

    vertex_buffer.bind(vertex_array);
    vertex_array.configure_attribute(position_attr, position_desc);

    instance_buffer.bind(vertex_array);
    vertex_array.configure_attribute(instance_position_attr, instance_position_desc);
    vertex_array.configure_attribute(instance_size_attr, instance_size_desc);
    vertex_array.configure_attribute(instance_color_attr, instance_color_desc);
  }

  CircleVertexArray::CircleVertexArray(
   const CircleProgram& program,
   const Buffer& instance_buffer,
   const Buffer& vertex_buffer
  ) {
    VertexAttribute position_attr = Device::get_vertex_attribute(program.program, "aPosition");
    VertexAttribute instance_position_attr = Device::get_vertex_attribute(program.program, "aInstancePosition");
    VertexAttribute instance_radius_attr = Device::get_vertex_attribute(program.program, "aInstanceRadius");
    VertexAttribute instance_color_attr = Device::get_vertex_attribute(program.program, "aInstanceColor");

    VertexAttrDescriptor position_desc = {
      VertexAttrClass::Int, VertexAttrType::U8,
      2, 2, 0, 0, 0
    };

    VertexAttrDescriptor instance_position_desc = {
      VertexAttrClass::Float, VertexAttrType::F32,
      2, 16, 0, 1, 1
    };

    VertexAttrDescriptor instance_radius_desc = {
      VertexAttrClass::Float, VertexAttrType::F32,
      1, 16, 8, 1, 1
    };

    VertexAttrDescriptor instance_color_desc = {
      VertexAttrClass::Int, VertexAttrType::U8,
      4, 16, 12, 1, 1
    };

    vertex_buffer.bind(vertex_array);
    vertex_array.configure_attribute(position_attr, position_desc);

    instance_buffer.bind(vertex_array);
    vertex_array.configure_attribute(instance_position_attr, instance_position_desc);
    vertex_array.configure_attribute(instance_radius_attr, instance_radius_desc);
    vertex_array.configure_attribute(instance_color_attr, instance_color_desc);
  }

}
