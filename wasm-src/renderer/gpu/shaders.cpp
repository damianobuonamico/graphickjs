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

TileProgram::TileProgram()
    : program(Device::create_program(
          "tile",
          {{"MAX_TEXTURES",
            (std::stringstream() << (Device::max_texture_image_units() - 2)).str()}})),
      vp_uniform(Device::get_uniform(program, "u_view_projection")),
      samples_uniform(Device::get_uniform(program, "u_samples")),
      bands_texture_uniform(Device::get_texture_uniform(program, "u_bands_texture")),
      curves_texture_uniform(Device::get_texture_uniform(program, "u_curves_texture")),
      textures_uniform(Device::get_textures_uniform(
          program, "u_textures", Device::max_texture_image_units() - 2))
{
}

FillProgram::FillProgram()
    : program(Device::create_program(
          "fill",
          {{"MAX_TEXTURES",
            (std::stringstream() << (Device::max_texture_image_units() - 2)).str()}})),
      vp_uniform(Device::get_uniform(program, "u_view_projection")),
      textures_uniform(Device::get_textures_uniform(
          program, "u_textures", Device::max_texture_image_units() - 2))
{
}

PrimitiveProgram::PrimitiveProgram()
    : program(Device::create_program("primitive")),
      vp_uniform(Device::get_uniform(program, "u_view_projection")),
      zoom_uniform(Device::get_uniform(program, "u_zoom"))
{
}

#ifdef GK_DEBUG

DebugRectProgram::DebugRectProgram()
    : program(Device::create_program("debug_rect")),
      vp_uniform(Device::get_uniform(program, "u_view_projection")),
      texture(Device::get_texture_uniform(program, "u_texture"))
{
}

#endif

/* -- VertexArrays -- */

TileVertexArray::TileVertexArray(const TileProgram& program,
                                 const Buffer& vertex_buffer,
                                 const Buffer& index_buffer)
{
  VertexAttribute position_attr = Device::get_vertex_attribute(program.program, "a_position");
  VertexAttribute color_attr = Device::get_vertex_attribute(program.program, "a_color");
  VertexAttribute tex_coord_attr = Device::get_vertex_attribute(program.program, "a_tex_coord");
  VertexAttribute tex_coord_curves_attr = Device::get_vertex_attribute(program.program,
                                                                       "a_tex_coord_curves");
  VertexAttribute first_attr = Device::get_vertex_attribute(program.program, "a_attr_1");
  VertexAttribute second_attr = Device::get_vertex_attribute(program.program, "a_attr_2");
  VertexAttribute third_attr = Device::get_vertex_attribute(program.program, "a_attr_3");

  VertexAttrDescriptor position_desc = {
      VertexAttrClass::Float, VertexAttrType::F32, 2, 36, 0, 0, 0};
  VertexAttrDescriptor color_desc = {VertexAttrClass::Int, VertexAttrType::U8, 4, 36, 8, 0, 0};
  VertexAttrDescriptor tex_coords_desc = {
      VertexAttrClass::Float, VertexAttrType::F16, 2, 36, 12, 0, 0};
  VertexAttrDescriptor tex_coords_curves_desc = {
      VertexAttrClass::Float, VertexAttrType::F32, 2, 36, 16, 0, 0};
  //   VertexAttrClass::Float, VertexAttrType::F16, 2, 36, 20, 0, 0};
  VertexAttrDescriptor first_desc = {VertexAttrClass::Int, VertexAttrType::U32, 1, 36, 24, 0, 0};
  VertexAttrDescriptor second_desc = {VertexAttrClass::Int, VertexAttrType::U32, 1, 36, 28, 0, 0};
  VertexAttrDescriptor third_desc = {VertexAttrClass::Int, VertexAttrType::U32, 1, 36, 32, 0, 0};

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

FillVertexArray::FillVertexArray(const FillProgram& program,
                                 const Buffer& vertex_buffer,
                                 const Buffer& index_buffer)
{
  VertexAttribute position_attr = Device::get_vertex_attribute(program.program, "a_position");
  VertexAttribute color_attr = Device::get_vertex_attribute(program.program, "a_color");
  VertexAttribute tex_coord_attr = Device::get_vertex_attribute(program.program, "a_tex_coord");
  VertexAttribute first_attr = Device::get_vertex_attribute(program.program, "a_attr_1");
  VertexAttribute second_attr = Device::get_vertex_attribute(program.program, "a_attr_2");

  VertexAttrDescriptor position_desc = {
      VertexAttrClass::Float, VertexAttrType::F32, 2, 24, 0, 0, 0};
  VertexAttrDescriptor color_desc = {VertexAttrClass::Int, VertexAttrType::U8, 4, 24, 8, 0, 0};
  VertexAttrDescriptor tex_coords_desc = {
      VertexAttrClass::Float, VertexAttrType::F16, 2, 24, 12, 0, 0};
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

PrimitiveVertexArray::PrimitiveVertexArray(const PrimitiveProgram& program,
                                           const Buffer& instance_buffer,
                                           const Buffer& vertex_buffer)
{
  VertexAttribute position_attr = Device::get_vertex_attribute(program.program, "a_position");
  VertexAttribute instance_attr_1 = Device::get_vertex_attribute(program.program,
                                                                 "a_instance_attr_1");
  VertexAttribute instance_attr_2 = Device::get_vertex_attribute(program.program,
                                                                 "a_instance_attr_2");
  VertexAttribute instance_attr_3 = Device::get_vertex_attribute(program.program,
                                                                 "a_instance_attr_3");
  VertexAttribute instance_color_attr = Device::get_vertex_attribute(program.program,
                                                                     "a_instance_color");

  VertexAttrDescriptor position_desc = {VertexAttrClass::Int, VertexAttrType::U8, 2, 2, 0, 0, 0};
  VertexAttrDescriptor instance_attr_1_desc = {
      VertexAttrClass::Float, VertexAttrType::F32, 2, 24, 0, 1, 1};
  VertexAttrDescriptor instance_attr_2_desc = {
      VertexAttrClass::Float, VertexAttrType::F32, 2, 24, 8, 1, 1};
  VertexAttrDescriptor instance_attr_3_desc = {
      VertexAttrClass::Int, VertexAttrType::U32, 1, 24, 16, 1, 1};
  VertexAttrDescriptor instance_color_desc = {
      VertexAttrClass::Int, VertexAttrType::U8, 4, 24, 20, 1, 1};

  vertex_buffer.bind(vertex_array);
  vertex_array.configure_attribute(position_attr, position_desc);

  instance_buffer.bind(vertex_array);
  vertex_array.configure_attribute(instance_attr_1, instance_attr_1_desc);
  vertex_array.configure_attribute(instance_attr_2, instance_attr_2_desc);
  vertex_array.configure_attribute(instance_attr_3, instance_attr_3_desc);
  vertex_array.configure_attribute(instance_color_attr, instance_color_desc);
}

#ifdef GK_DEBUG

DebugRectVertexArray::DebugRectVertexArray(const DebugRectProgram& program,
                                           const Buffer& vertex_buffer)
{
  VertexAttribute position_attr = Device::get_vertex_attribute(program.program, "a_position");
  VertexAttribute tex_coord_attr = Device::get_vertex_attribute(program.program, "a_tex_coord");
  VertexAttribute color_attr = Device::get_vertex_attribute(program.program, "a_color");
  VertexAttribute primitive_attr = Device::get_vertex_attribute(program.program, "a_primitive");

  VertexAttrDescriptor position_desc = {
      VertexAttrClass::Float, VertexAttrType::F32, 2, 24, 0, 0, 0};
  VertexAttrDescriptor tex_coords_desc = {
      VertexAttrClass::Float, VertexAttrType::F32, 2, 24, 8, 0, 0};
  VertexAttrDescriptor primitive_desc = {
      VertexAttrClass::Int, VertexAttrType::U32, 1, 24, 16, 0, 0};
  VertexAttrDescriptor color_desc = {VertexAttrClass::Int, VertexAttrType::U8, 4, 24, 20, 0, 0};

  vertex_buffer.bind(vertex_array);
  vertex_array.configure_attribute(position_attr, position_desc);
  vertex_array.configure_attribute(tex_coord_attr, tex_coords_desc);
  vertex_array.configure_attribute(primitive_attr, primitive_desc);
  vertex_array.configure_attribute(color_attr, color_desc);
}

#endif

}  // namespace graphick::renderer::GPU
