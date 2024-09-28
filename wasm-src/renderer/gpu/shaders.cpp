/**
 * @file renderer/gpu/shaders.cpp
 * @brief Contains the GPU shaders and vertex arrays implementations.
 *
 * @todo query max number of textures
 */

#include "shaders.h"

#include "device.h"

#include <sstream>

namespace graphick::renderer::GPU {

/* -- Programs -- */

TileProgram::TileProgram() :
  program(
    Device::create_program("tile", {{"MAX_TEXTURES", (std::stringstream() << (Device::max_texture_image_units() - 2)).str()}})
  ),
  vp_uniform(Device::get_uniform(program, "u_view_projection")),
  samples_uniform(Device::get_uniform(program, "u_samples")),
  bands_texture_uniform(Device::get_texture_uniform(program, "u_bands_texture")),
  curves_texture_uniform(Device::get_texture_uniform(program, "u_curves_texture")),
  textures_uniform(Device::get_textures_uniform(program, "u_textures", Device::max_texture_image_units() - 2)) { }

FillProgram::FillProgram() :
  program(
    Device::create_program("fill", {{"MAX_TEXTURES", (std::stringstream() << (Device::max_texture_image_units() - 2)).str()}})
  ),
  vp_uniform(Device::get_uniform(program, "u_view_projection")),
  textures_uniform(Device::get_textures_uniform(program, "u_textures", Device::max_texture_image_units() - 2)) { }

LineProgram::LineProgram() :
  program(Device::create_program("line")),
  vp_uniform(Device::get_uniform(program, "u_view_projection")),
  zoom_uniform(Device::get_uniform(program, "u_zoom")) { }

RectProgram::RectProgram() :
  program(Device::create_program("rect")), vp_uniform(Device::get_uniform(program, "u_view_projection")) { }

CircleProgram::CircleProgram() :
  program(Device::create_program("circle")),
  vp_uniform(Device::get_uniform(program, "u_view_projection")),
  zoom_uniform(Device::get_uniform(program, "u_zoom")) { }

ImageProgram::ImageProgram() :
  program(Device::create_program("image")),
  vp_uniform(Device::get_uniform(program, "u_view_projection")),
  image_texture(Device::get_texture_uniform(program, "u_texture")) { }

/* -- VertexArrays -- */

TileVertexArray::TileVertexArray(const TileProgram& program, const Buffer& vertex_buffer, const Buffer& index_buffer) {
  VertexAttribute position_attr = Device::get_vertex_attribute(program.program, "a_position");
  VertexAttribute color_attr = Device::get_vertex_attribute(program.program, "a_color");
  VertexAttribute tex_coord_attr = Device::get_vertex_attribute(program.program, "a_tex_coord");
  VertexAttribute tex_coord_curves_attr = Device::get_vertex_attribute(program.program, "a_tex_coord_curves");
  VertexAttribute first_attr = Device::get_vertex_attribute(program.program, "a_attr_1");
  VertexAttribute second_attr = Device::get_vertex_attribute(program.program, "a_attr_2");
  VertexAttribute third_attr = Device::get_vertex_attribute(program.program, "a_attr_3");

  VertexAttrDescriptor position_desc = {VertexAttrClass::Float, VertexAttrType::F32, 2, 32, 0, 0, 0};
  VertexAttrDescriptor color_desc = {VertexAttrClass::Int, VertexAttrType::U8, 4, 32, 8, 0, 0};
  VertexAttrDescriptor tex_coords_desc = {VertexAttrClass::Float, VertexAttrType::F16, 2, 32, 12, 0, 0};
  VertexAttrDescriptor tex_coords_curves_desc = {VertexAttrClass::Float, VertexAttrType::F16, 2, 32, 16, 0, 0};
  VertexAttrDescriptor first_desc = {VertexAttrClass::Int, VertexAttrType::U32, 1, 32, 20, 0, 0};
  VertexAttrDescriptor second_desc = {VertexAttrClass::Int, VertexAttrType::U32, 1, 32, 24, 0, 0};
  VertexAttrDescriptor third_desc = {VertexAttrClass::Int, VertexAttrType::U32, 1, 32, 28, 0, 0};

  index_buffer.bind(vertex_array);

  vertex_buffer.bind(vertex_array);
  vertex_array.configure_attribute(position_attr, position_desc);
  vertex_array.configure_attribute(color_attr, color_desc);
  vertex_array.configure_attribute(tex_coord_attr, tex_coords_desc);
  vertex_array.configure_attribute(tex_coord_curves_attr, tex_coords_curves_desc);
  vertex_array.configure_attribute(first_attr, first_desc);
  vertex_array.configure_attribute(second_attr, second_desc);
  vertex_array.configure_attribute(third_attr, third_desc);
}

FillVertexArray::FillVertexArray(const FillProgram& program, const Buffer& vertex_buffer, const Buffer& index_buffer) {
  VertexAttribute position_attr = Device::get_vertex_attribute(program.program, "a_position");
  VertexAttribute color_attr = Device::get_vertex_attribute(program.program, "a_color");
  VertexAttribute tex_coord_attr = Device::get_vertex_attribute(program.program, "a_tex_coord");
  VertexAttribute first_attr = Device::get_vertex_attribute(program.program, "a_attr_1");
  VertexAttribute second_attr = Device::get_vertex_attribute(program.program, "a_attr_2");

  VertexAttrDescriptor position_desc = {VertexAttrClass::Float, VertexAttrType::F32, 2, 24, 0, 0, 0};
  VertexAttrDescriptor color_desc = {VertexAttrClass::Int, VertexAttrType::U8, 4, 24, 8, 0, 0};
  VertexAttrDescriptor tex_coords_desc = {VertexAttrClass::Float, VertexAttrType::F16, 2, 24, 12, 0, 0};
  VertexAttrDescriptor first_desc = {VertexAttrClass::Int, VertexAttrType::U32, 1, 24, 16, 0, 0};
  VertexAttrDescriptor second_desc = {VertexAttrClass::Int, VertexAttrType::U32, 1, 24, 20, 0, 0};

  index_buffer.bind(vertex_array);

  vertex_buffer.bind(vertex_array);
  vertex_array.configure_attribute(position_attr, position_desc);
  vertex_array.configure_attribute(color_attr, color_desc);
  vertex_array.configure_attribute(tex_coord_attr, tex_coords_desc);
  vertex_array.configure_attribute(first_attr, first_desc);
  vertex_array.configure_attribute(second_attr, second_desc);
}

LineVertexArray::LineVertexArray(const LineProgram& program, const Buffer& instance_buffer, const Buffer& vertex_buffer) {
  VertexAttribute position_attr = Device::get_vertex_attribute(program.program, "a_position");
  VertexAttribute instance_from_attr = Device::get_vertex_attribute(program.program, "a_instance_from");
  VertexAttribute instance_to_attr = Device::get_vertex_attribute(program.program, "a_instance_to");
  VertexAttribute instance_width_attr = Device::get_vertex_attribute(program.program, "a_instance_width");
  VertexAttribute instance_color_attr = Device::get_vertex_attribute(program.program, "a_instance_color");

  VertexAttrDescriptor position_desc = {VertexAttrClass::Int, VertexAttrType::U8, 2, 2, 0, 0, 0};
  VertexAttrDescriptor instance_from_desc = {VertexAttrClass::Float, VertexAttrType::F32, 2, 24, 0, 1, 1};
  VertexAttrDescriptor instance_to_desc = {VertexAttrClass::Float, VertexAttrType::F32, 2, 24, 8, 1, 1};
  VertexAttrDescriptor instance_width_desc = {VertexAttrClass::Float, VertexAttrType::F32, 1, 24, 16, 1, 1};
  VertexAttrDescriptor instance_color_desc = {VertexAttrClass::Int, VertexAttrType::U8, 4, 24, 20, 1, 1};

  vertex_buffer.bind(vertex_array);
  vertex_array.configure_attribute(position_attr, position_desc);

  instance_buffer.bind(vertex_array);
  vertex_array.configure_attribute(instance_from_attr, instance_from_desc);
  vertex_array.configure_attribute(instance_to_attr, instance_to_desc);
  vertex_array.configure_attribute(instance_width_attr, instance_width_desc);
  vertex_array.configure_attribute(instance_color_attr, instance_color_desc);
}

RectVertexArray::RectVertexArray(const RectProgram& program, const Buffer& instance_buffer, const Buffer& vertex_buffer) {
  VertexAttribute position_attr = Device::get_vertex_attribute(program.program, "a_position");
  VertexAttribute instance_position_attr = Device::get_vertex_attribute(program.program, "a_instance_position");
  VertexAttribute instance_size_attr = Device::get_vertex_attribute(program.program, "a_instance_size");
  VertexAttribute instance_color_attr = Device::get_vertex_attribute(program.program, "a_instance_color");

  VertexAttrDescriptor position_desc = {VertexAttrClass::Int, VertexAttrType::U8, 2, 2, 0, 0, 0};
  VertexAttrDescriptor instance_position_desc = {VertexAttrClass::Float, VertexAttrType::F32, 2, 20, 0, 1, 1};
  VertexAttrDescriptor instance_size_desc = {VertexAttrClass::Float, VertexAttrType::F32, 2, 20, 8, 1, 1};
  VertexAttrDescriptor instance_color_desc = {VertexAttrClass::Int, VertexAttrType::U8, 4, 20, 16, 1, 1};

  vertex_buffer.bind(vertex_array);
  vertex_array.configure_attribute(position_attr, position_desc);

  instance_buffer.bind(vertex_array);
  vertex_array.configure_attribute(instance_position_attr, instance_position_desc);
  vertex_array.configure_attribute(instance_size_attr, instance_size_desc);
  vertex_array.configure_attribute(instance_color_attr, instance_color_desc);
}

CircleVertexArray::CircleVertexArray(const CircleProgram& program, const Buffer& instance_buffer, const Buffer& vertex_buffer) {
  VertexAttribute position_attr = Device::get_vertex_attribute(program.program, "a_position");
  VertexAttribute instance_position_attr = Device::get_vertex_attribute(program.program, "a_instance_position");
  VertexAttribute instance_radius_attr = Device::get_vertex_attribute(program.program, "a_instance_radius");
  VertexAttribute instance_color_attr = Device::get_vertex_attribute(program.program, "a_instance_color");

  VertexAttrDescriptor position_desc = {VertexAttrClass::Int, VertexAttrType::U8, 2, 2, 0, 0, 0};
  VertexAttrDescriptor instance_position_desc = {VertexAttrClass::Float, VertexAttrType::F32, 2, 16, 0, 1, 1};
  VertexAttrDescriptor instance_radius_desc = {VertexAttrClass::Float, VertexAttrType::F32, 1, 16, 8, 1, 1};
  VertexAttrDescriptor instance_color_desc = {VertexAttrClass::Int, VertexAttrType::U8, 4, 16, 12, 1, 1};

  vertex_buffer.bind(vertex_array);
  vertex_array.configure_attribute(position_attr, position_desc);

  instance_buffer.bind(vertex_array);
  vertex_array.configure_attribute(instance_position_attr, instance_position_desc);
  vertex_array.configure_attribute(instance_radius_attr, instance_radius_desc);
  vertex_array.configure_attribute(instance_color_attr, instance_color_desc);
}

ImageVertexArray::ImageVertexArray(const ImageProgram& program, const Buffer& instance_buffer, const Buffer& vertex_buffer) {
  VertexAttribute position_attr = Device::get_vertex_attribute(program.program, "a_position");
  VertexAttribute instance_position_attr = Device::get_vertex_attribute(program.program, "a_instance_position");
  VertexAttribute instance_size_attr = Device::get_vertex_attribute(program.program, "a_instance_size");

  VertexAttrDescriptor position_desc = {VertexAttrClass::Int, VertexAttrType::U8, 2, 2, 0, 0, 0};
  VertexAttrDescriptor instance_position_desc = {VertexAttrClass::Float, VertexAttrType::F32, 2, 16, 0, 1, 1};
  VertexAttrDescriptor instance_size_desc = {VertexAttrClass::Float, VertexAttrType::F32, 2, 16, 8, 1, 1};

  vertex_buffer.bind(vertex_array);
  vertex_array.configure_attribute(position_attr, position_desc);

  instance_buffer.bind(vertex_array);
  vertex_array.configure_attribute(instance_position_attr, instance_position_desc);
  vertex_array.configure_attribute(instance_size_attr, instance_size_desc);
}

}  // namespace graphick::renderer::GPU
