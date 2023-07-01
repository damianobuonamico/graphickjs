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

  void Renderer::begin_frame(const Editor::Viewport& viewport) {
    OPTICK_EVENT();

    get()->m_viewport = Viewport{
      viewport.size(),
      viewport.dpr(),
      viewport.position(),
      viewport.zoom()
    };

    get()->m_projection = generate_projection_matrix(viewport.size(), viewport.zoom());
    get()->m_translation = mat4{
      1.0f, 0.0f, 0.0f, 0.5f * (-viewport.size().x / viewport.zoom() + 2 * viewport.position().x),
      0.0f, 1.0f, 0.0f, 0.5f * (-viewport.size().y / viewport.zoom() + 2 * viewport.position().y),
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
    };

    vec2 tiles_position = (viewport.position() * viewport.zoom()) % TILE_SIZE - TILE_SIZE;

    get()->m_tiles_projection = generate_projection_matrix(viewport.size(), 1.0f);
    get()->m_tiles_translation = mat4{
      1.0f, 0.0f, 0.0f, 0.5f * (-viewport.size().x + 2 * tiles_position.x),
      0.0f, 1.0f, 0.0f, 0.5f * (-viewport.size().y + 2 * tiles_position.y),
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
    };

    get()->m_tiler.reset(get()->m_viewport);

    GPU::Device::begin_commands();
    GPU::Device::set_viewport(viewport.size(), viewport.dpr());
    GPU::Device::clear({ vec4{ 1.0f, 1.0f, 1.0f, 1.0f }, std::nullopt, std::nullopt });
  }

  void Renderer::end_frame() {
    OPTICK_EVENT();

    get()->draw_opaque_tiles();
    get()->draw_masked_tiles();

    GPU::Memory::Allocator::purge_if_needed();

    GPU::Device::end_commands();
  }

  void Renderer::draw(const Geometry::Path& path) {
    if (path.empty()) return;

    OPTICK_EVENT();

    get()->m_tiler.process_path(path, vec4{ 0.5f, 0.5f, 0.5f, 1.0f });
  }

  void Renderer::draw_outline(const Geometry::Path& path) {
    if (path.empty()) return;

    OPTICK_EVENT();

    std::vector<vec2> vertex_positions;
    std::vector<uint32_t> vertex_indices;

    for (const auto& segment : path.segments()) {
      vertex_positions.push_back(segment.p0());
    }

    vertex_positions.push_back(path.segments().back().p3());

    for (uint32_t i = 0; i < vertex_positions.size() - 1; i++) {
      vertex_indices.insert(vertex_indices.end(), { i, i + 1 });
    }

    uuid vertex_buffer_id = GPU::Memory::Allocator::allocate_general_buffer<vec2>(vertex_positions.size(), "VertexPositions");
    uuid index_buffer_id = GPU::Memory::Allocator::allocate_index_buffer<uint32_t>(vertex_indices.size(), "VertexIndices");

    const GPU::Buffer& vertex_buffer = GPU::Memory::Allocator::get_general_buffer(vertex_buffer_id);
    const GPU::Buffer& index_buffer = GPU::Memory::Allocator::get_index_buffer(index_buffer_id);

    GPU::Device::upload_to_buffer(vertex_buffer, 0, vertex_positions, GPU::BufferTarget::Vertex);
    GPU::Device::upload_to_buffer(index_buffer, 0, vertex_indices, GPU::BufferTarget::Index);

    GPU::LineVertexArray vertex_array(
      get()->m_programs.line_program,
      vertex_buffer,
      index_buffer
    );

    GPU::RenderState state = {
      nullptr,
      get()->m_programs.line_program.program,
      *vertex_array.vertex_array,
      GPU::Primitive::Lines,
      {},
      {},
      {
        {get()->m_programs.line_program.view_projection_uniform, get()->m_projection * get()->m_translation },
        {get()->m_programs.line_program.color_uniform, vec4{0.3f, 0.3f, 0.9f, 1.0f} },
      },
      {
        { 0.0f, 0.0f },
        { (float)get()->m_viewport.size.x * get()->m_viewport.dpr, (float)get()->m_viewport.size.y * get()->m_viewport.dpr }
      },
      {
        std::nullopt,
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

    GPU::Device::draw_elements(vertex_indices.size(), state);

    GPU::Memory::Allocator::free_general_buffer(vertex_buffer_id);
    GPU::Memory::Allocator::free_index_buffer(index_buffer_id);
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

}
