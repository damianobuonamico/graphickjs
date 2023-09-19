#include "shaders.h"

namespace Graphick::Renderer::GPU {

  DefaultProgram::DefaultProgram() :
    program(Device::create_program("default")),
    view_projection_uniform(Device::get_uniform(program, "uViewProjection").value()),
    color_uniform(Device::get_uniform(program, "uColor").value()),
    texture_uniform(Device::get_uniform(program, "uTexture").value()) {}

  OpaqueTileProgram::OpaqueTileProgram() :
    program(Device::create_program("opaque_tile")),
    offset_uniform(Device::get_uniform(program, "uOffset").value()),
    framebuffer_size_uniform(Device::get_uniform(program, "uFramebufferSize").value()),
    tile_size_uniform(Device::get_uniform(program, "uTileSize").value()) {}

  MaskedTileProgram::MaskedTileProgram() :
    program(Device::create_program("masked_tile")),
    offset_uniform(Device::get_uniform(program, "uOffset").value()),
    framebuffer_size_uniform(Device::get_uniform(program, "uFramebufferSize").value()),
    tile_size_uniform(Device::get_uniform(program, "uTileSize").value()),
    masks_texture_uniform(Device::get_uniform(program, "uMasksTexture").value()),
    masks_texture_size_uniform(Device::get_uniform(program, "uMasksTextureSize").value()) {}

  LineProgram::LineProgram() :
    program(Device::create_program("line")),
    view_projection_uniform(Device::get_uniform(program, "uViewProjection").value()),
    color_uniform(Device::get_uniform(program, "uColor").value()),
    line_width_uniform(Device::get_uniform(program, "uLineWidth").value()),
    zoom_uniform(Device::get_uniform(program, "uZoom").value()) {}

  SquareProgram::SquareProgram() :
    program(Device::create_program("square")),
    view_projection_uniform(Device::get_uniform(program, "uViewProjection").value()),
    color_uniform(Device::get_uniform(program, "uColor").value()),
    size_uniform(Device::get_uniform(program, "uSize").value()) {}

  CircleProgram::CircleProgram() :
    program(Device::create_program("circle")),
    view_projection_uniform(Device::get_uniform(program, "uViewProjection").value()),
    color_uniform(Device::get_uniform(program, "uColor").value()),
    radius_uniform(Device::get_uniform(program, "uRadius").value()),
    zoom_uniform(Device::get_uniform(program, "uZoom").value()) {}

  GPUPathProgram::GPUPathProgram() :
    program(Device::create_program("gpu_path")),
    view_projection_uniform(Device::get_uniform(program, "uViewProjection").value()),
    paths_texture_uniform(Device::get_uniform(program, "uPathsTexture").value()),
    paths_texture_size_uniform(Device::get_uniform(program, "uPathsTextureSize").value()) {}

  Programs::Programs() :
    opaque_tile_program(),
    masked_tile_program(),
    line_program(),
    square_program(),
    circle_program(),
    gpu_path_program() {}

  DefaultVertexArray::DefaultVertexArray(
    const DefaultProgram& default_program,
    const Buffer& vertex_buffer
  )
    : vertex_array(Device::create_vertex_array())
  {
    VertexAttr position_attr = Device::get_vertex_attr(default_program.program, "aPosition").value();
    VertexAttr tex_coord_attr = Device::get_vertex_attr(default_program.program, "aTexCoord").value();

    VertexAttrDescriptor position_desc = {
     2,
     VertexAttrClass::Float,
     VertexAttrType::F32,
     16,
     0,
     0,
     0
    };

    VertexAttrDescriptor tex_coord_desc = {
     2,
     VertexAttrClass::Float,
     VertexAttrType::F32,
     16,
     8,
     0,
     0
    };

    Device::bind_buffer(*vertex_array, vertex_buffer, BufferTarget::Vertex);
    Device::configure_vertex_attr(*vertex_array, position_attr, position_desc);
    Device::configure_vertex_attr(*vertex_array, tex_coord_attr, tex_coord_desc);
  }

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
    VertexAttr z_index_attr = Device::get_vertex_attr(opaque_tile_program.program, "aZIndex").value();

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

    VertexAttrDescriptor z_index_desc = {
      1,
      VertexAttrClass::Float,
      VertexAttrType::F32,
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
    Device::configure_vertex_attr(*vertex_array, z_index_attr, z_index_desc);

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
    VertexAttr mask_index_attr = Device::get_vertex_attr(masked_tile_program.program, "aSegmentsCoords").value();
    VertexAttr z_index_attr = Device::get_vertex_attr(masked_tile_program.program, "aZIndex").value();

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
      32,
      0,
      1,
      1
    };

    VertexAttrDescriptor index_desc = {
      1,
      VertexAttrClass::Int,
      VertexAttrType::I32,
      32,
      16,
      1,
      1
    };

    VertexAttrDescriptor mask_index_desc = {
      2,
      VertexAttrClass::Int,
      VertexAttrType::I32,
      32,
      20,
      1,
      1
    };

    VertexAttrDescriptor z_index_desc = {
      1,
      VertexAttrClass::Float,
      VertexAttrType::F32,
      32,
      28,
      1,
      1
    };

    Device::bind_buffer(*vertex_array, quad_vertex_positions_buffer, BufferTarget::Vertex);
    Device::configure_vertex_attr(*vertex_array, position_attr, position_desc);

    Device::bind_buffer(*vertex_array, vertex_buffer, BufferTarget::Vertex);
    Device::configure_vertex_attr(*vertex_array, color_attr, color_desc);
    Device::configure_vertex_attr(*vertex_array, index_attr, index_desc);
    Device::configure_vertex_attr(*vertex_array, mask_index_attr, mask_index_desc);
    Device::configure_vertex_attr(*vertex_array, z_index_attr, z_index_desc);

    Device::bind_buffer(*vertex_array, quad_vertex_indices_buffer, BufferTarget::Index);
  }

  LineVertexArray::LineVertexArray(
    const LineProgram& line_program,
    const Buffer& instance_buffer,
    const Buffer& vertex_positions_buffer,
    const Buffer& vertex_indices_buffer
  )
    : vertex_array(Device::create_vertex_array())
  {
    VertexAttr position_attr = Device::get_vertex_attr(line_program.program, "aPosition").value();
    VertexAttr tex_coord_attr = Device::get_vertex_attr(line_program.program, "aTexCoord").value();
    VertexAttr instance_from_attr = Device::get_vertex_attr(line_program.program, "aInstanceFrom").value();
    VertexAttr instance_to_attr = Device::get_vertex_attr(line_program.program, "aInstanceTo").value();

    VertexAttrDescriptor position_desc = {
      2,
      VertexAttrClass::Float,
      VertexAttrType::F32,
      16,
      0,
      0,
      0
    };

    VertexAttrDescriptor tex_coord_desc = {
      2,
      VertexAttrClass::Float,
      VertexAttrType::F32,
      16,
      8,
      0,
      0
    };

    VertexAttrDescriptor instance_from_desc = {
      2,
      VertexAttrClass::Float,
      VertexAttrType::F32,
      16,
      0,
      1,
      1
    };

    VertexAttrDescriptor instance_to_desc = {
      2,
      VertexAttrClass::Float,
      VertexAttrType::F32,
      16,
      8,
      1,
      1
    };

    Device::bind_buffer(*vertex_array, vertex_positions_buffer, BufferTarget::Vertex);
    Device::configure_vertex_attr(*vertex_array, position_attr, position_desc);
    Device::configure_vertex_attr(*vertex_array, tex_coord_attr, tex_coord_desc);

    Device::bind_buffer(*vertex_array, instance_buffer, BufferTarget::Vertex);
    Device::configure_vertex_attr(*vertex_array, instance_from_attr, instance_from_desc);
    Device::configure_vertex_attr(*vertex_array, instance_to_attr, instance_to_desc);

    Device::bind_buffer(*vertex_array, vertex_indices_buffer, BufferTarget::Index);
  }

  SquareVertexArray::SquareVertexArray(
    const SquareProgram& square_program,
    const Buffer& instance_buffer,
    const Buffer& vertex_positions_buffer,
    const Buffer& vertex_indices_buffer
  )
    : vertex_array(Device::create_vertex_array())
  {
    VertexAttr position_attr = Device::get_vertex_attr(square_program.program, "aPosition").value();
    VertexAttr instance_position_attr = Device::get_vertex_attr(square_program.program, "aInstancePosition").value();

    VertexAttrDescriptor position_desc = {
      2,
      VertexAttrClass::Float,
      VertexAttrType::F32,
      8,
      0,
      0,
      0
    };

    VertexAttrDescriptor instance_position_desc = {
      2,
      VertexAttrClass::Float,
      VertexAttrType::F32,
      8,
      0,
      1,
      1
    };

    Device::bind_buffer(*vertex_array, vertex_positions_buffer, BufferTarget::Vertex);
    Device::configure_vertex_attr(*vertex_array, position_attr, position_desc);

    Device::bind_buffer(*vertex_array, instance_buffer, BufferTarget::Vertex);
    Device::configure_vertex_attr(*vertex_array, instance_position_attr, instance_position_desc);

    Device::bind_buffer(*vertex_array, vertex_indices_buffer, BufferTarget::Index);
  }

  CircleVertexArray::CircleVertexArray(
    const CircleProgram& circle_program,
    const Buffer& instance_buffer,
    const Buffer& vertex_positions_buffer,
    const Buffer& vertex_indices_buffer
  )
    : vertex_array(Device::create_vertex_array())
  {
    VertexAttr position_attr = Device::get_vertex_attr(circle_program.program, "aPosition").value();
    VertexAttr instance_position_attr = Device::get_vertex_attr(circle_program.program, "aInstancePosition").value();

    VertexAttrDescriptor position_desc = {
      2,
      VertexAttrClass::Float,
      VertexAttrType::F32,
      8,
      0,
      0,
      0
    };

    VertexAttrDescriptor instance_position_desc = {
      2,
      VertexAttrClass::Float,
      VertexAttrType::F32,
      8,
      0,
      1,
      1
    };

    Device::bind_buffer(*vertex_array, vertex_positions_buffer, BufferTarget::Vertex);
    Device::configure_vertex_attr(*vertex_array, position_attr, position_desc);

    Device::bind_buffer(*vertex_array, instance_buffer, BufferTarget::Vertex);
    Device::configure_vertex_attr(*vertex_array, instance_position_attr, instance_position_desc);

    Device::bind_buffer(*vertex_array, vertex_indices_buffer, BufferTarget::Index);
  }

  GPUPathVertexArray::GPUPathVertexArray(
    const GPUPathProgram& gpu_path_program,
    const Buffer& instance_buffer,
    const Buffer& quad_vertex_positions_buffer,
    const Buffer& quad_vertex_indices_buffer
  )
    : vertex_array(Device::create_vertex_array())
  {
    VertexAttr position_attr = Device::get_vertex_attr(gpu_path_program.program, "aPosition").value();
    VertexAttr path_position_attr = Device::get_vertex_attr(gpu_path_program.program, "aPathPosition").value();
    VertexAttr path_size_attr = Device::get_vertex_attr(gpu_path_program.program, "aPathSize").value();
    VertexAttr segments_index_attr = Device::get_vertex_attr(gpu_path_program.program, "aSegmentsIndex").value();
    VertexAttr color_index_attr = Device::get_vertex_attr(gpu_path_program.program, "aColorIndex").value();

    VertexAttrDescriptor position_desc = {
      2,
      VertexAttrClass::Int,
      VertexAttrType::U16,
      4,
      0,
      0,
      0
    };

    VertexAttrDescriptor path_position_desc = {
      2,
      VertexAttrClass::Float,
      VertexAttrType::F32,
      24,
      0,
      1,
      1
    };

    VertexAttrDescriptor path_size_desc = {
      2,
      VertexAttrClass::Float,
      VertexAttrType::F32,
      24,
      8,
      1,
      1
    };

    VertexAttrDescriptor segments_index_desc = {
      1,
      VertexAttrClass::Float,
      VertexAttrType::F32,
      24,
      16,
      1,
      1
    };

    VertexAttrDescriptor color_index_desc = {
      1,
      VertexAttrClass::Float,
      VertexAttrType::F32,
      24,
      20,
      1,
      1
    };

    Device::bind_buffer(*vertex_array, quad_vertex_positions_buffer, BufferTarget::Vertex);
    Device::configure_vertex_attr(*vertex_array, position_attr, position_desc);

    Device::bind_buffer(*vertex_array, instance_buffer, BufferTarget::Vertex);
    Device::configure_vertex_attr(*vertex_array, path_position_attr, path_position_desc);
    Device::configure_vertex_attr(*vertex_array, path_size_attr, path_size_desc);
    Device::configure_vertex_attr(*vertex_array, segments_index_attr, segments_index_desc);
    Device::configure_vertex_attr(*vertex_array, color_index_attr, color_index_desc);

    Device::bind_buffer(*vertex_array, quad_vertex_indices_buffer, BufferTarget::Index);
  }

}
