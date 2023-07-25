#include "renderer.h"

#include "geometry/path.h"
#include "gpu/allocator.h"

#include "../utils/resource_manager.h"
#include "../utils/console.h"

#ifdef EMSCRIPTEN
#include <emscripten/html5.h>
#endif

static const std::vector<uint16_t> QUAD_VERTEX_POSITIONS = { 0, 0, 1, 0, 1, 1, 0, 1 };
static const std::vector<uint32_t> QUAD_VERTEX_INDICES = { 0, 1, 3, 1, 2, 3 };
static const std::vector<float> LINE_VERTEX_POSITIONS = {
  0.0, 1.0, 0.0, 0.0,
  0.0, 0.0, 0.0, 1.0,
  1.0, 0.0, 1.0, 1.0,
  1.0, 1.0, 1.0, 0.0
};

//0, 1, 2, 0, 2, 3

namespace Graphick::Renderer {

  Renderer* Renderer::s_instance = nullptr;

  static mat4 generate_projection_matrix(const ivec2 size, float zoom) {
    float factor = 0.5f / zoom;

    float half_width = -size.x * factor;
    float half_height = size.y * factor;

    float right = -half_width;
    float left = half_width;
    float top = -half_height;
    float bottom = half_height;

    return mat4{
      2.0f / (right - left), 0.0f, 0.0f, 0.0f,
      0.0f, 2.0f / (top - bottom), 0.0f, 0.0f,
      0.0f, 0.0f, -1.0f, 0.0f,
      -(right + left) / (right - left), -(top + bottom) / (top - bottom), 0.0f, 1.0f
    };
  }

  void Renderer::init() {
#ifdef EMSCRIPTEN
    EmscriptenWebGLContextAttributes attr;
    emscripten_webgl_init_context_attributes(&attr);

    // TODO: test with and without alpha performance
    attr.alpha = false;
    attr.premultipliedAlpha = false;
    attr.majorVersion = 2;
    attr.antialias = false;
    attr.stencil = false;
    attr.depth = false;

    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context("#canvas", &attr);
    emscripten_webgl_make_context_current(ctx);
#endif

    if (s_instance != nullptr) {
      console::error("Renderer already initialized, call shutdown() before reinitializing!");
      return;
    }

    GPU::Device::init(GPU::DeviceVersion::GLES3, 0);
    GPU::Memory::Allocator::init();

    s_instance = new Renderer();

    s_instance->m_quad_vertex_positions_buffer_id = GPU::Memory::Allocator::allocate_general_buffer<uint16_t>(QUAD_VERTEX_POSITIONS.size(), "QuadVertexPositions");
    s_instance->m_quad_vertex_indices_buffer_id = GPU::Memory::Allocator::allocate_index_buffer<uint32_t>(QUAD_VERTEX_INDICES.size(), "QuadVertexIndices");
    s_instance->m_masks_texture_id = GPU::Memory::Allocator::allocate_texture({ MASKS_TEXTURE_SIZE, MASKS_TEXTURE_SIZE }, GPU::TextureFormat::R8, "Masks");

    const GPU::Buffer& quad_vertex_positions_buffer = GPU::Memory::Allocator::get_general_buffer(s_instance->m_quad_vertex_positions_buffer_id);
    const GPU::Buffer& quad_vertex_indices_buffer = GPU::Memory::Allocator::get_index_buffer(s_instance->m_quad_vertex_indices_buffer_id);

    GPU::Device::upload_to_buffer(quad_vertex_positions_buffer, 0, QUAD_VERTEX_POSITIONS, GPU::BufferTarget::Vertex);
    GPU::Device::upload_to_buffer(quad_vertex_indices_buffer, 0, QUAD_VERTEX_INDICES, GPU::BufferTarget::Index);

    get()->init_batched_lines_renderer();
  }

  void Renderer::shutdown() {
    if (s_instance == nullptr) {
      console::error("Renderer already shutdown, call init() before shutting down!");
      return;
    }

    delete s_instance;

    GPU::Memory::Allocator::shutdown();
    GPU::Device::shutdown();
  }

  void Renderer::begin_frame(const Viewport& viewport) {
    OPTICK_EVENT();

    get()->m_viewport = viewport;
    get()->m_projection = generate_projection_matrix(viewport.size, viewport.zoom);
    get()->m_translation = mat4{
      1.0f, 0.0f, 0.0f, 0.5f * (-viewport.size.x / viewport.zoom + 2 * viewport.position.x),
      0.0f, 1.0f, 0.0f, 0.5f * (-viewport.size.y / viewport.zoom + 2 * viewport.position.y),
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
    };

    vec2 tiles_position = (viewport.position * viewport.zoom) % TILE_SIZE - TILE_SIZE;

    get()->m_tiles_projection = generate_projection_matrix(viewport.size, 1.0f);
    get()->m_tiles_translation = mat4{
      1.0f, 0.0f, 0.0f, 0.5f * (-viewport.size.x + 2 * tiles_position.x),
      0.0f, 1.0f, 0.0f, 0.5f * (-viewport.size.y + 2 * tiles_position.y),
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
    };

    get()->m_tiler.reset(get()->m_viewport);
    get()->begin_lines_batch();

    GPU::Device::begin_commands();
    GPU::Device::set_viewport(viewport.size, viewport.dpr);
    GPU::Device::clear({ vec4{ 1.0f, 1.0f, 1.0f, 1.0f }, std::nullopt, std::nullopt });
  }

  void Renderer::end_frame() {
    OPTICK_EVENT();

    get()->draw_opaque_tiles();
    get()->draw_masked_tiles();
    get()->flush_lines_batch();

    GPU::Memory::Allocator::purge_if_needed();
    GPU::Device::end_commands();
  }

  void Renderer::draw(const Geometry::Path& path, const vec4& color) {
    if (path.empty()) return;

    OPTICK_EVENT();

    get()->m_tiler.process_path(path, color);
  }

  void Renderer::draw_outline(const Geometry::Path& path) {
    if (path.empty()) return;

    get()->add_to_lines_batch(path);
  }

  void Renderer::draw_opaque_tiles() {
    const std::vector<OpaqueTile>& tiles = m_tiler.opaque_tiles();
    if (tiles.empty()) return;

    OPTICK_EVENT();

    uuid tiles_buffer_id = GPU::Memory::Allocator::allocate_general_buffer<OpaqueTile>(tiles.size(), "OpaqueTiles");

    const GPU::Buffer& quad_vertex_positions_buffer = GPU::Memory::Allocator::get_general_buffer(m_quad_vertex_positions_buffer_id);
    const GPU::Buffer& quad_vertex_indices_buffer = GPU::Memory::Allocator::get_index_buffer(m_quad_vertex_indices_buffer_id);
    const GPU::Buffer& tiles_buffer = GPU::Memory::Allocator::get_general_buffer(tiles_buffer_id);

    GPU::Device::upload_to_buffer(tiles_buffer, 0, tiles, GPU::BufferTarget::Vertex);

    GPU::OpaqueTileVertexArray vertex_array(
      m_programs.opaque_tile_program,
      tiles_buffer,
      quad_vertex_positions_buffer,
      quad_vertex_indices_buffer
    );

    GPU::RenderState state = {
      nullptr,
      m_programs.opaque_tile_program.program,
      *vertex_array.vertex_array,
      GPU::Primitive::Triangles,
      {},
      {},
      {
        { m_programs.opaque_tile_program.view_uniform, m_tiles_translation },
        { m_programs.opaque_tile_program.projection_uniform, m_tiles_projection },
        { m_programs.opaque_tile_program.tile_size_uniform, (int)TILE_SIZE },
        { m_programs.opaque_tile_program.framebuffer_size_uniform, m_viewport.size },
      },
      {
        { 0.0f, 0.0f },
        { (float)m_viewport.size.x * m_viewport.dpr, (float)m_viewport.size.y * m_viewport.dpr }
      },
      {
        GPU::BlendState{
          GPU::BlendFactor::SrcAlpha,
          GPU::BlendFactor::OneMinusSrcAlpha,
          GPU::BlendFactor::SrcAlpha,
          GPU::BlendFactor::OneMinusSrcAlpha,
          GPU::BlendOp::Add,
        },
        std::nullopt,
        std::nullopt,
        {
          std::nullopt,
          std::nullopt,
          std::nullopt
        },
        true
      }
    };

    GPU::Device::draw_elements_instanced(6, tiles.size(), state);

    GPU::Memory::Allocator::free_general_buffer(tiles_buffer_id);
  }

  void Renderer::draw_masked_tiles() {
    const std::vector<MaskedTile>& reverse_tiles = m_tiler.masked_tiles();
    if (reverse_tiles.empty()) return;

    const std::vector<MaskedTile> tiles = std::vector<MaskedTile>(reverse_tiles.rbegin(), reverse_tiles.rend());

    uuid tiles_buffer_id = GPU::Memory::Allocator::allocate_general_buffer<MaskedTile>(tiles.size(), "MaskedTiles");

    const GPU::Buffer& quad_vertex_positions_buffer = GPU::Memory::Allocator::get_general_buffer(m_quad_vertex_positions_buffer_id);
    const GPU::Buffer& quad_vertex_indices_buffer = GPU::Memory::Allocator::get_index_buffer(m_quad_vertex_indices_buffer_id);
    const GPU::Buffer& tiles_buffer = GPU::Memory::Allocator::get_general_buffer(tiles_buffer_id);
    const GPU::Texture& masks_texture = GPU::Memory::Allocator::get_texture(m_masks_texture_id);

    GPU::Device::upload_to_buffer(tiles_buffer, 0, tiles, GPU::BufferTarget::Vertex);
    GPU::Device::upload_to_texture(masks_texture, { { 0.0f, 0.0f }, { (float)MASKS_TEXTURE_SIZE, (float)MASKS_TEXTURE_SIZE } }, m_tiler.masks_texture_data());

    GPU::MaskedTileVertexArray tile_vertex_array(
      m_programs.masked_tile_program,
      tiles_buffer,
      quad_vertex_positions_buffer,
      quad_vertex_indices_buffer
    );

    GPU::RenderState state = {
      nullptr,
      m_programs.masked_tile_program.program,
      *tile_vertex_array.vertex_array,
      GPU::Primitive::Triangles,
      {
        { { m_programs.masked_tile_program.masks_texture_uniform, masks_texture.gl_texture }, masks_texture }
      },
      {},
      {
        { m_programs.masked_tile_program.view_uniform, m_tiles_translation },
        { m_programs.masked_tile_program.projection_uniform, m_tiles_projection },
        { m_programs.masked_tile_program.tile_size_uniform, (int)TILE_SIZE },
        { m_programs.masked_tile_program.framebuffer_size_uniform, m_viewport.size },
      },
      {
        { 0.0f, 0.0f },
        { (float)m_viewport.size.x * m_viewport.dpr, (float)m_viewport.size.y * m_viewport.dpr }
      },
      {
        GPU::BlendState{
          GPU::BlendFactor::SrcAlpha,
          GPU::BlendFactor::OneMinusSrcAlpha,
          GPU::BlendFactor::SrcAlpha,
          GPU::BlendFactor::OneMinusSrcAlpha,
          GPU::BlendOp::Add,
        },
        std::nullopt,
        std::nullopt,
        {
        std::nullopt,
        std::nullopt,
        std::nullopt
      },
      true
    }
    };

    GPU::Device::draw_elements_instanced(6, tiles.size(), state);

    GPU::Memory::Allocator::free_general_buffer(tiles_buffer_id);
  }

  void Renderer::init_batched_lines_renderer() {
    delete[] m_lines_data.instance_buffer;

    m_lines_data.instance_buffer = new vec4[m_lines_data.max_instance_count];
    m_lines_data.instance_buffer_ptr = m_lines_data.instance_buffer;

    if (m_lines_data.instance_buffer_id != 0) {
      GPU::Memory::Allocator::free_general_buffer(m_lines_data.instance_buffer_id);
    }
    if (m_lines_data.vertex_buffer_id != 0) {
      GPU::Memory::Allocator::free_general_buffer(m_lines_data.vertex_buffer_id);
    }

    m_lines_data.instance_buffer_id = GPU::Memory::Allocator::allocate_general_buffer<vec4>(m_lines_data.max_instance_buffer_size, "Lines");
    m_lines_data.vertex_buffer_id = GPU::Memory::Allocator::allocate_general_buffer<float>(LINE_VERTEX_POSITIONS.size(), "LinesVertices");

    const GPU::Buffer& vertex_buffer = GPU::Memory::Allocator::get_general_buffer(m_lines_data.vertex_buffer_id);

    GPU::Device::upload_to_buffer(vertex_buffer, 0, LINE_VERTEX_POSITIONS, GPU::BufferTarget::Vertex);
  }

  void Renderer::begin_lines_batch() {
    m_lines_data.instance_buffer_ptr = m_lines_data.instance_buffer;
    m_lines_data.instances = 0;
  }

  void Renderer::add_to_lines_batch(const Geometry::Path& path) {
    for (const auto& segment : path.segments()) {
      vec2 p0 = segment.p0();
      vec2 p3 = segment.p3();
      // auto p0 = transform.Map(segment.p0().x, segment.p0().y);
      // auto p3 = transform.Map(segment.p3().x, segment.p3().y);

      *m_lines_data.instance_buffer_ptr = { (float)p0.x, (float)p0.y, (float)p3.x, (float)p3.y };
      m_lines_data.instance_buffer_ptr++;
    }

    m_lines_data.instances += (uint32_t)path.segments().size();
  }

  void Renderer::flush_lines_batch() {
    GLsizeiptr instance_buffer_size = (uint8_t*)m_lines_data.instance_buffer_ptr - (uint8_t*)m_lines_data.instance_buffer;
    if (instance_buffer_size == 0 || m_lines_data.instances == 0) return;

    const GPU::Buffer& vertex_buffer = GPU::Memory::Allocator::get_general_buffer(m_lines_data.vertex_buffer_id);
    const GPU::Buffer& index_buffer = GPU::Memory::Allocator::get_index_buffer(m_quad_vertex_indices_buffer_id);
    const GPU::Buffer& instance_buffer = GPU::Memory::Allocator::get_general_buffer(m_lines_data.instance_buffer_id);

    GPU::Device::upload_to_buffer(instance_buffer, 0, m_lines_data.instance_buffer, instance_buffer_size, GPU::BufferTarget::Vertex);

    GPU::LineVertexArray vertex_array(
      m_programs.line_program,
      instance_buffer,
      vertex_buffer,
      index_buffer
    );

    mat4 translation = mat4{
      1.0f, 0.0f, 0.0f, 0.5f * (-m_viewport.size.x / (float)m_viewport.zoom + 2 * m_viewport.position.x),
      0.0f, 1.0f, 0.0f, 0.5f * (-m_viewport.size.y / (float)m_viewport.zoom + 2 * m_viewport.position.y),
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
    };

    GPU::RenderState state = {
      nullptr,
      m_programs.line_program.program,
      *vertex_array.vertex_array,
      GPU::Primitive::Triangles,
      {},
      {},
      {
        // TOOD: merge dpr and zoom
        {m_programs.line_program.view_projection_uniform, generate_projection_matrix(m_viewport.size, m_viewport.zoom) * translation },
        {m_programs.line_program.color_uniform, vec4{ 0.22f, 0.76f, 0.95f, 1.0f } },
        {m_programs.line_program.line_width_uniform, 2.0f / (float)m_viewport.zoom },
        {m_programs.line_program.zoom_uniform, (float)m_viewport.zoom },
      },
      {
        { 0.0f, 0.0f },
        { (float)m_viewport.size.x * (float)m_viewport.dpr, (float)m_viewport.size.y * (float)m_viewport.dpr }
      },
      {
        GPU::BlendState{
          GPU::BlendFactor::SrcAlpha,
          GPU::BlendFactor::OneMinusSrcAlpha,
          GPU::BlendFactor::SrcAlpha,
          GPU::BlendFactor::OneMinusSrcAlpha,
          GPU::BlendOp::Add,
        },
        std::nullopt,
        std::nullopt,
        {
          std::nullopt,
          std::nullopt,
          std::nullopt
        },
        true
      }
    };

    GPU::Device::draw_elements_instanced(QUAD_VERTEX_INDICES.size(), m_lines_data.instances, state);

    console::log("instances", m_lines_data.instances);
  }

}
