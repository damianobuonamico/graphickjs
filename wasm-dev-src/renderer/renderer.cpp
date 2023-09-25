#include "renderer.h"

#include "geometry/path.h"
#include "geometry/internal.h"
#include "gpu/allocator.h"

#include "../math/math.h"

#include "../utils/resource_manager.h"
#include "../utils/console.h"

#include "../editor/editor.h"

#ifdef EMSCRIPTEN
#include <emscripten/html5.h>
#endif

static const std::vector<uint16_t> QUAD_VERTEX_POSITIONS = { 0, 0, 1, 0, 1, 1, 0, 1 };
static const std::vector<uint32_t> QUAD_VERTEX_INDICES = { 0, 1, 3, 1, 2, 3 };
static const std::vector<float> SQUARE_VERTEX_POSITIONS = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f };
static const std::vector<float> LINE_VERTEX_POSITIONS = {
  0.0f, 1.0f, 0.0f, 0.0f,
  0.0f, 0.0f, 0.0f, 1.0f,
  1.0f, 0.0f, 1.0f, 1.0f,
  1.0f, 1.0f, 1.0f, 0.0f
};

namespace Graphick::Renderer {

  Renderer* Renderer::s_instance = nullptr;

  static mat4 generate_projection_matrix(const vec2 size, float zoom) {
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
    attr.depth = true;

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
    s_instance->m_masks_texture_id = GPU::Memory::Allocator::allocate_texture({ SEGMENTS_TEXTURE_SIZE, SEGMENTS_TEXTURE_SIZE }, GPU::TextureFormat::RGBA8, "Masks");

    const GPU::Buffer& quad_vertex_positions_buffer = GPU::Memory::Allocator::get_general_buffer(s_instance->m_quad_vertex_positions_buffer_id);
    const GPU::Buffer& quad_vertex_indices_buffer = GPU::Memory::Allocator::get_index_buffer(s_instance->m_quad_vertex_indices_buffer_id);

    GPU::Device::upload_to_buffer(quad_vertex_positions_buffer, 0, QUAD_VERTEX_POSITIONS, GPU::BufferTarget::Vertex);
    GPU::Device::upload_to_buffer(quad_vertex_indices_buffer, 0, QUAD_VERTEX_INDICES, GPU::BufferTarget::Index);

    get()->init_instanced_renderers();
#if GK_USE_DEBUGGER
    get()->init_text_renderer();
#endif
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

    tiles_position = { 0.0f, 0.0f };

    get()->m_tiles_projection = generate_projection_matrix(viewport.size, 1.0f);
    get()->m_tiles_translation = mat4{
      1.0f, 0.0f, 0.0f, 0.5f * (-viewport.size.x + 2 * tiles_position.x),
      0.0f, 1.0f, 0.0f, 0.5f * (-viewport.size.y + 2 * tiles_position.y),
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
    };

    get()->m_tiler.reset(get()->m_viewport);
    get()->begin_instanced_renderers();

    GPU::Device::begin_commands();
    GPU::Device::set_viewport(viewport.size);
    GPU::Device::clear({ vec4{ 1.0f, 1.0f, 1.0f, 1.0f }, 1.0f, std::nullopt });
  }

  void Renderer::end_frame() {
    OPTICK_EVENT();

    get()->draw_opaque_tiles();
    get()->draw_masked_tiles();
    get()->flush_line_instances();
    get()->flush_square_instances();
    get()->flush_white_square_instances();
    get()->flush_circle_instances();

    GPU::Memory::Allocator::purge_if_needed();
    GPU::Device::end_commands();
  }

  void Renderer::draw(const Geometry::Path& path, const float z_index, const vec2 translation, const vec4& color) {
    if (path.empty()) return;

    OPTICK_EVENT();

    get()->m_tiler.process_path(path, translation, color, z_index);
  }

  void Renderer::draw_outline(const uuid id, const Geometry::Path& path, const vec2 translation) {
    if (path.vacant()) return;

    get()->add_line_instances(path, translation);
    get()->add_vertex_instances(id, path, translation);
  }

  void Renderer::draw_outline(const Geometry::Internal::PathInternal& path, const vec2 translation) {
    if (path.empty()) return;

    get()->add_line_instances(path, translation);
  }

  void Renderer::debug_rect(const Math::rect rect, const vec4& color) {
    const GPU::Buffer& vertex_buffer = GPU::Memory::Allocator::get_general_buffer(get()->m_debug_vertex_buffer_id);

    std::vector<float> vertices = {
      rect.min.x, rect.min.y, -1.0f, -1.0f,
      rect.max.x, rect.min.y, -1.0f, -1.0f,
      rect.max.x, rect.max.y, -1.0f, -1.0f,
      rect.max.x, rect.max.y, -1.0f, -1.0f,
      rect.min.x, rect.max.y, -1.0f, -1.0f,
      rect.min.x, rect.min.y, -1.0f, -1.0f
    };

    GPU::Device::upload_to_buffer(vertex_buffer, 0, vertices, GPU::BufferTarget::Vertex);

    GPU::DefaultVertexArray vertex_array(
      get()->m_programs.default_program,
      vertex_buffer
    );

    mat4 translation = mat4{
      1.0f, 0.0f, 0.0f, -0.5f * get()->m_viewport.size.x,
      0.0f, 1.0f, 0.0f, -0.5f * get()->m_viewport.size.y,
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
    };

    GPU::RenderState state = {
      nullptr,
      get()->m_programs.default_program.program,
      *vertex_array.vertex_array,
      GPU::Primitive::Triangles,
      {},
      {},
      {
        {get()->m_programs.default_program.view_projection_uniform, generate_projection_matrix(get()->m_viewport.size, 1.0f) * translation },
        {get()->m_programs.default_program.color_uniform, color },
      },
      {
        { 0.0f, 0.0f },
        get()->m_viewport.size
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

    GPU::Device::draw_arrays(6, state);
  }

  void Renderer::debug_text(const std::string& text, const vec2 position, const vec4& color) {
    size_t byte_size = text.size() * 6 * 4;

    const GPU::Buffer& vertex_buffer = GPU::Memory::Allocator::get_general_buffer(get()->m_debug_vertex_buffer_id);

    float x = position.x;
    float y = position.y;
    stbtt_aligned_quad q;

    std::vector<float> vertices;
    vertices.reserve(byte_size);

    for (char c : text) {
      stbtt_GetBakedQuad(get()->m_cdata, 128, 128, c - 32, &x, &y, &q, 1);

      vertices.insert(vertices.end(), {
        q.x0, q.y0, q.s0, q.t0,
        q.x1, q.y0, q.s1, q.t0,
        q.x1, q.y1, q.s1, q.t1,
        q.x1, q.y1, q.s1, q.t1,
        q.x0, q.y1, q.s0, q.t1,
        q.x0, q.y0, q.s0, q.t0
        });
    }

    GPU::Device::upload_to_buffer(vertex_buffer, 0, vertices, GPU::BufferTarget::Vertex);

    GPU::DefaultVertexArray vertex_array(
      get()->m_programs.default_program,
      vertex_buffer
    );

    mat4 translation = mat4{
      1.0f, 0.0f, 0.0f, -0.5f * get()->m_viewport.size.x,
      0.0f, 1.0f, 0.0f, -0.5f * get()->m_viewport.size.y,
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
    };

    const GPU::Texture& font_atlas = GPU::Memory::Allocator::get_texture(get()->m_debug_font_atlas_id);

    GPU::Device::upload_to_texture(font_atlas, { { 0.0f, 0.0f }, { (float)128, (float)128 } }, get()->m_bitmap);

    GPU::RenderState state = {
      nullptr,
      get()->m_programs.default_program.program,
      *vertex_array.vertex_array,
      GPU::Primitive::Triangles,
      {
        { { get()->m_programs.default_program.texture_uniform, font_atlas.gl_texture }, font_atlas }
      },
      {},
      {
        {get()->m_programs.default_program.view_projection_uniform, generate_projection_matrix(get()->m_viewport.size, 1.0f) * translation },
        {get()->m_programs.default_program.color_uniform, color },
      },
      {
        { 0.0f, 0.0f },
        get()->m_viewport.size
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

    GPU::Device::draw_arrays(byte_size, state);
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
        { m_programs.opaque_tile_program.offset_uniform, (m_viewport.position * m_viewport.zoom) % TILE_SIZE - TILE_SIZE },
        { m_programs.opaque_tile_program.tile_size_uniform, (int)TILE_SIZE },
        { m_programs.opaque_tile_program.framebuffer_size_uniform, m_viewport.size },
      },
      {
        { 0.0f, 0.0f },
        m_viewport.size
      },
      {
        GPU::BlendState{
          GPU::BlendFactor::SrcAlpha,
          GPU::BlendFactor::OneMinusSrcAlpha,
          GPU::BlendFactor::SrcAlpha,
          GPU::BlendFactor::OneMinusSrcAlpha,
          GPU::BlendOp::Add,
        },
        GPU::DepthState{
          GPU::DepthFunc::Lequal,
          true
        },
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
    const uint8_t* segments = m_tiler.segments();

    const std::vector<MaskedTile> tiles = std::vector<MaskedTile>(reverse_tiles.rbegin(), reverse_tiles.rend());

    uuid tiles_buffer_id = GPU::Memory::Allocator::allocate_general_buffer<MaskedTile>(tiles.size(), "MaskedTiles");

    const GPU::Buffer& quad_vertex_positions_buffer = GPU::Memory::Allocator::get_general_buffer(m_quad_vertex_positions_buffer_id);
    const GPU::Buffer& quad_vertex_indices_buffer = GPU::Memory::Allocator::get_index_buffer(m_quad_vertex_indices_buffer_id);
    const GPU::Buffer& tiles_buffer = GPU::Memory::Allocator::get_general_buffer(tiles_buffer_id);
    const GPU::Texture& segments_texture = GPU::Memory::Allocator::get_texture(m_masks_texture_id);

    GPU::Device::upload_to_buffer(tiles_buffer, 0, tiles, GPU::BufferTarget::Vertex);
    GPU::Device::upload_to_texture(segments_texture, { { 0.0f, 0.0f }, { (float)SEGMENTS_TEXTURE_SIZE, (float)SEGMENTS_TEXTURE_SIZE } }, segments);

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
        { { m_programs.masked_tile_program.masks_texture_uniform, segments_texture.gl_texture }, segments_texture }
      },
      {},
      {
        { m_programs.masked_tile_program.offset_uniform, (m_viewport.position * m_viewport.zoom) % TILE_SIZE - TILE_SIZE },
        { m_programs.masked_tile_program.tile_size_uniform, (int)TILE_SIZE },
        { m_programs.masked_tile_program.framebuffer_size_uniform, m_viewport.size },
        { m_programs.masked_tile_program.masks_texture_size_uniform, (int)SEGMENTS_TEXTURE_SIZE }
      },
      {
        { 0.0f, 0.0f },
        m_viewport.size,
      },
      {
        GPU::BlendState{
          GPU::BlendFactor::SrcAlpha,
          GPU::BlendFactor::OneMinusSrcAlpha,
          GPU::BlendFactor::SrcAlpha,
          GPU::BlendFactor::OneMinusSrcAlpha,
          GPU::BlendOp::Add,
        },
        GPU::DepthState{
          GPU::DepthFunc::Lequal,
          false
        },
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

    // const std::vector<MaskedTile>& reverse_tiles = m_tiler.masked_tiles();
    // const std::vector<uint8_t*> reverse_textures = m_tiler.masks_textures_data();

    // if (reverse_tiles.empty() || reverse_textures.empty()) return;

    // const std::vector<MaskedTile> tiles = std::vector<MaskedTile>(reverse_tiles.rbegin(), reverse_tiles.rend());
    // const std::vector<uint8_t*> textures = std::vector<uint8_t*>(reverse_textures.rbegin(), reverse_textures.rend());

    // for (size_t i = 0; i < textures.size(); i++) {
    //   draw_masked_tiles_batch(tiles, i, textures);
    // }
  }

  void Renderer::draw_masked_tiles_batch(const std::vector<MaskedTile> tiles, const size_t i, const std::vector<uint8_t*> textures) {
    size_t index = 0;
    size_t count = std::min((size_t)MASKS_PER_BATCH, tiles.size() - (textures.size() - i - 1) * MASKS_PER_BATCH);

    for (int j = 0; j < i; j++) {
      index += std::min((size_t)MASKS_PER_BATCH, tiles.size() - (textures.size() - j - 1) * MASKS_PER_BATCH);
    }

    // TODO: preallocate and preserve buffers
    uuid tiles_buffer_id = GPU::Memory::Allocator::allocate_general_buffer<MaskedTile>(count, "MaskedTiles");

    const GPU::Buffer& quad_vertex_positions_buffer = GPU::Memory::Allocator::get_general_buffer(m_quad_vertex_positions_buffer_id);
    const GPU::Buffer& quad_vertex_indices_buffer = GPU::Memory::Allocator::get_index_buffer(m_quad_vertex_indices_buffer_id);
    const GPU::Buffer& tiles_buffer = GPU::Memory::Allocator::get_general_buffer(tiles_buffer_id);
    const GPU::Texture& masks_texture = GPU::Memory::Allocator::get_texture(m_masks_texture_id);

    GPU::Device::upload_to_buffer(tiles_buffer, 0, tiles.data() + index, count * sizeof(MaskedTile), GPU::BufferTarget::Vertex);
    GPU::Device::upload_to_texture(masks_texture, { { 0.0f, 0.0f }, { (float)MASKS_TEXTURE_SIZE, (float)MASKS_TEXTURE_SIZE } }, textures[i]);

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
        { m_programs.masked_tile_program.offset_uniform, (m_viewport.position * m_viewport.zoom) % TILE_SIZE - TILE_SIZE },
        { m_programs.masked_tile_program.tile_size_uniform, (int)TILE_SIZE },
        { m_programs.masked_tile_program.framebuffer_size_uniform, m_viewport.size },
        { m_programs.masked_tile_program.masks_texture_size_uniform, (int)MASKS_TEXTURE_SIZE }
      },
      {
        { 0.0f, 0.0f },
        m_viewport.size
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

    GPU::Device::draw_elements_instanced(6, count, state);

    GPU::Memory::Allocator::free_general_buffer(tiles_buffer_id);
  }

  void Renderer::init_instanced_renderers() {
    delete[] m_lines_data.instance_buffer;

    m_lines_data.instance_buffer = new vec4[m_lines_data.max_instance_count];
    m_lines_data.instance_buffer_ptr = m_lines_data.instance_buffer;

    if (m_lines_data.instance_buffer_id != 0) {
      GPU::Memory::Allocator::free_general_buffer(m_lines_data.instance_buffer_id);
    }
    if (m_square_data.instance_buffer_id != 0) {
      GPU::Memory::Allocator::free_general_buffer(m_square_data.instance_buffer_id);
    }
    if (m_white_square_data.instance_buffer_id != 0) {
      GPU::Memory::Allocator::free_general_buffer(m_white_square_data.instance_buffer_id);
    }
    if (m_circle_data.instance_buffer_id != 0) {
      GPU::Memory::Allocator::free_general_buffer(m_circle_data.instance_buffer_id);
    }

    if (m_lines_data.vertex_buffer_id != 0) {
      GPU::Memory::Allocator::free_general_buffer(m_lines_data.vertex_buffer_id);
    }
    if (m_square_data.vertex_buffer_id != 0) {
      GPU::Memory::Allocator::free_general_buffer(m_square_data.vertex_buffer_id);
    }
    if (m_white_square_data.vertex_buffer_id != 0) {
      GPU::Memory::Allocator::free_general_buffer(m_white_square_data.vertex_buffer_id);
    }
    if (m_circle_data.vertex_buffer_id != 0) {
      GPU::Memory::Allocator::free_general_buffer(m_circle_data.vertex_buffer_id);
    }

    m_lines_data.instance_buffer_id = GPU::Memory::Allocator::allocate_general_buffer<vec4>(m_lines_data.max_instance_buffer_size, "Lines");
    m_square_data.instance_buffer_id = GPU::Memory::Allocator::allocate_general_buffer<vec2>(m_square_data.buffer_size, "Squares");
    m_white_square_data.instance_buffer_id = GPU::Memory::Allocator::allocate_general_buffer<vec2>(m_white_square_data.buffer_size, "WhiteSquares");
    m_circle_data.instance_buffer_id = GPU::Memory::Allocator::allocate_general_buffer<vec2>(m_circle_data.buffer_size, "Circles");

    m_lines_data.vertex_buffer_id = GPU::Memory::Allocator::allocate_general_buffer<float>(LINE_VERTEX_POSITIONS.size(), "LineVertices");
    m_square_data.vertex_buffer_id = GPU::Memory::Allocator::allocate_general_buffer<float>(SQUARE_VERTEX_POSITIONS.size(), "SquareVertices");
    m_white_square_data.vertex_buffer_id = GPU::Memory::Allocator::allocate_general_buffer<float>(SQUARE_VERTEX_POSITIONS.size(), "WhiteSquareVertices");
    m_circle_data.vertex_buffer_id = GPU::Memory::Allocator::allocate_general_buffer<float>(SQUARE_VERTEX_POSITIONS.size(), "CircleVertices");

    const GPU::Buffer& line_vertex_buffer = GPU::Memory::Allocator::get_general_buffer(m_lines_data.vertex_buffer_id);
    const GPU::Buffer& square_vertex_buffer = GPU::Memory::Allocator::get_general_buffer(m_square_data.vertex_buffer_id);
    const GPU::Buffer& white_square_vertex_buffer = GPU::Memory::Allocator::get_general_buffer(m_white_square_data.vertex_buffer_id);
    const GPU::Buffer& circle_vertex_buffer = GPU::Memory::Allocator::get_general_buffer(m_circle_data.vertex_buffer_id);

    GPU::Device::upload_to_buffer(line_vertex_buffer, 0, LINE_VERTEX_POSITIONS, GPU::BufferTarget::Vertex);
    GPU::Device::upload_to_buffer(square_vertex_buffer, 0, SQUARE_VERTEX_POSITIONS, GPU::BufferTarget::Vertex);
    GPU::Device::upload_to_buffer(white_square_vertex_buffer, 0, SQUARE_VERTEX_POSITIONS, GPU::BufferTarget::Vertex);
    GPU::Device::upload_to_buffer(circle_vertex_buffer, 0, SQUARE_VERTEX_POSITIONS, GPU::BufferTarget::Vertex);

    m_common_data.quad_index_buffer.clear();
    m_common_data.quad_vertex_buffer.clear();

    m_common_data.quad_vertex_buffer.copy(QUAD_VERTEX_POSITIONS);
    m_common_data.quad_index_buffer.copy(QUAD_VERTEX_INDICES);

    m_common_data.quad_vertex_buffer.upload();
    m_common_data.quad_index_buffer.upload();
  }

  void Renderer::begin_instanced_renderers() {
    m_lines_data.instance_buffer_ptr = m_lines_data.instance_buffer;

    m_lines_data.instances = 0;
    m_square_data.instances.clear();
    m_white_square_data.instances.clear();
    m_circle_data.instances.clear();
  }

  void Renderer::add_line_instances(const Geometry::Path& path, const vec2 translation) {
    if (path.empty()) return;

    OPTICK_EVENT();

    for (const auto& segment : path.segments()) {
      vec2 p0 = segment->p0() + translation;
      vec2 p3 = segment->p3() + translation;
      // auto p0 = transform.Map(segment.p0().x, segment.p0().y);
      // auto p3 = transform.Map(segment.p3().x, segment.p3().y);

      if (segment->is_cubic()) {
        add_cubic_segment_instance(p0, segment->p1() + translation, segment->p2() + translation, p3);
      } else {
        add_linear_segment_instance(p0, p3);
      }
    }
  }

  void Renderer::add_line_instances(const Geometry::Internal::PathInternal& path, const vec2 translation) {
    OPTICK_EVENT();

    for (const auto& segment : path.segments()) {
      vec2 p0 = segment.p0() + translation;
      vec2 p3 = segment.p3() + translation;

      if (segment.is_cubic()) {
        add_cubic_segment_instance(p0, segment.p1() + translation, segment.p2() + translation, p3);
      } else {
        add_linear_segment_instance(p0, p3);
      }
    }
  }

  void Renderer::add_linear_segment_instance(const vec2 p0, const vec2 p3) {
    // TODO: bounds checking and batching
    *m_lines_data.instance_buffer_ptr = { p0.x, p0.y, p3.x, p3.y };
    m_lines_data.instance_buffer_ptr++;
    m_lines_data.instances++;
  }

  static float tolerance = 0.25f;

  void Renderer::add_cubic_segment_instance(const vec2 p0, const vec2 p1, const vec2 p2, const vec2 p3) {
    vec2 prev = p0;

    vec2 a = -1.0f * p0 + 3.0f * p1 - 3.0f * p2 + p3;
    vec2 b = 3.0f * (p0 - 2.0f * p1 + p2);

    float conc = std::max(Math::length(b), Math::length(a + b));
    float dt = std::sqrtf((std::sqrtf(8.0f) * (tolerance / m_viewport.zoom)) / conc);
    float t = 0.0f;

    while (t < 1.0f) {
      t = std::min(t + dt, 1.0f);

      vec2 p01 = Math::lerp(p0, p1, t);
      vec2 p12 = Math::lerp(p1, p2, t);
      vec2 p23 = Math::lerp(p2, p3, t);
      vec2 p012 = Math::lerp(p01, p12, t);
      vec2 p123 = Math::lerp(p12, p23, t);

      vec2 p = Math::lerp(p012, p123, t);

      add_linear_segment_instance(prev, p);

      prev = p;
    }
  }

  void Renderer::add_vertex_instances(const uuid id, const Geometry::Path& path, const vec2 translation) {
    Editor::Scene& scene = Editor::Editor::scene();

    vec2 first_pos, last_pos;

    auto in_handle_ptr = path.in_handle_ptr();
    auto out_handle_ptr = path.out_handle_ptr();

    if (path.empty()) {
      auto p = path.last().lock();
      vec2 p_pos = p->get() + translation;

      add_square_instance(p_pos);
      if (!scene.selection.has_vertex(p->id, id, true)) {
        add_white_square_instance(p_pos);
      }

      first_pos = p_pos;
      last_pos = p_pos;
    } else {
      first_pos = path.segments().front()->p0() + translation;
      last_pos = path.segments().back()->p3() + translation;
    }

    if (!path.closed()) {
      if (in_handle_ptr.has_value()) {
        vec2 p = in_handle_ptr.value()->get() + translation;

        add_circle_instance(p);
        add_linear_segment_instance(first_pos, p);
      }

      if (out_handle_ptr.has_value()) {
        vec2 p = out_handle_ptr.value()->get() + translation;

        add_circle_instance(p);
        add_linear_segment_instance(last_pos, p);
      }
    }

    if (path.empty()) return;

    for (const auto& segment : path.segments()) {
      vec2 p0 = segment->p0() + translation;
      uuid p0_id = segment->p0_id();
      // auto p0 = transform.Map(segment.p0().x, segment.p0().y);

      add_square_instance(p0);
      if (!scene.selection.has_vertex(p0_id, id, true)) {
        add_white_square_instance(p0);
      }

      if (segment->is_cubic()) {
        vec2 p1 = segment->p1() + translation;
        vec2 p2 = segment->p2() + translation;
        vec2 p3 = segment->p3() + translation;

        if (p1 != p0) {
          add_circle_instance(p1);
          add_linear_segment_instance(p0, p1);
        }
        if (p2 != p3) {
          add_circle_instance(p2);
          add_linear_segment_instance(p2, p3);
        }
      }
    }

    if (!path.closed()) {
      uuid p3_id = path.segments().back()->p3_id();

      add_square_instance(last_pos);
      if (!scene.selection.has_vertex(p3_id, id, true)) {
        add_white_square_instance(last_pos);
      }
    }
  }

  void Renderer::add_square_instance(const vec2 position) {
    m_square_data.instances.push_back(position);
  }

  void Renderer::add_white_square_instance(const vec2 position) {
    m_white_square_data.instances.push_back(position);
  }

  void Renderer::add_circle_instance(const vec2 position) {
    m_circle_data.instances.push_back(position);
  }

  void Renderer::flush_line_instances() {
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
      1.0f, 0.0f, 0.0f, 0.5f * (-m_viewport.size.x / m_viewport.zoom + 2 * m_viewport.position.x),
      0.0f, 1.0f, 0.0f, 0.5f * (-m_viewport.size.y / m_viewport.zoom + 2 * m_viewport.position.y),
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
        {m_programs.line_program.view_projection_uniform, generate_projection_matrix(m_viewport.size, m_viewport.zoom) * translation },
        {m_programs.line_program.color_uniform, vec4{ 0.22f, 0.76f, 0.95f, 1.0f } - vec4{ 0.05f, 0.05f, 0.05f, 0.0f } },
        {m_programs.line_program.line_width_uniform, 3.0f / (float)m_viewport.zoom },
        {m_programs.line_program.zoom_uniform, (float)m_viewport.zoom },
      },
      {
        { 0.0f, 0.0f },
        m_viewport.size
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
  }

  void Renderer::flush_generic_square_instances(InstancedMeshData& data, const vec4& color, const float size) {
    if (data.instances.empty()) return;

    ensure_instance_buffer_size(data);

    const GPU::Buffer& vertex_buffer = GPU::Memory::Allocator::get_general_buffer(data.vertex_buffer_id);
    const GPU::Buffer& index_buffer = GPU::Memory::Allocator::get_index_buffer(m_quad_vertex_indices_buffer_id);
    const GPU::Buffer& instance_buffer = GPU::Memory::Allocator::get_general_buffer(data.instance_buffer_id);

    GPU::Device::upload_to_buffer(instance_buffer, 0, data.instances, GPU::BufferTarget::Vertex);

    GPU::SquareVertexArray vertex_array(
      m_programs.square_program,
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
      m_programs.square_program.program,
      *vertex_array.vertex_array,
      GPU::Primitive::Triangles,
      {},
      {},
      {
        {m_programs.square_program.view_projection_uniform, generate_projection_matrix(m_viewport.size, m_viewport.zoom) * translation },
        {m_programs.square_program.color_uniform, color },
        {m_programs.square_program.size_uniform, size },
      },
      {
        { 0.0f, 0.0f },
        m_viewport.size
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

    GPU::Device::draw_elements_instanced(QUAD_VERTEX_INDICES.size(), data.instances.size(), state);
  }

  void Renderer::flush_square_instances() {
    flush_generic_square_instances(m_square_data, vec4{ 0.22f, 0.76f, 0.95f, 1.0f }, 5.0f / (float)m_viewport.zoom);
  }

  void Renderer::flush_white_square_instances() {
    flush_generic_square_instances(m_white_square_data, vec4{ 1.0f, 1.0f, 1.0f, 1.0f }, 3.0f / (float)m_viewport.zoom);
  }

  void Renderer::flush_circle_instances() {
    if (m_circle_data.instances.empty()) return;

    ensure_instance_buffer_size(m_circle_data);

    const GPU::Buffer& vertex_buffer = GPU::Memory::Allocator::get_general_buffer(m_circle_data.vertex_buffer_id);
    const GPU::Buffer& index_buffer = GPU::Memory::Allocator::get_index_buffer(m_quad_vertex_indices_buffer_id);
    const GPU::Buffer& instance_buffer = GPU::Memory::Allocator::get_general_buffer(m_circle_data.instance_buffer_id);

    GPU::Device::upload_to_buffer(instance_buffer, 0, m_circle_data.instances, GPU::BufferTarget::Vertex);

    GPU::CircleVertexArray vertex_array(
      m_programs.circle_program,
      instance_buffer,
      vertex_buffer,
      index_buffer
    );

    mat4 translation = mat4{
      1.0f, 0.0f, 0.0f, 0.5f * (-m_viewport.size.x / m_viewport.zoom + 2 * m_viewport.position.x),
      0.0f, 1.0f, 0.0f, 0.5f * (-m_viewport.size.y / m_viewport.zoom + 2 * m_viewport.position.y),
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
    };

    GPU::RenderState state = {
      nullptr,
      m_programs.circle_program.program,
      *vertex_array.vertex_array,
      GPU::Primitive::Triangles,
      {},
      {},
      {
        { m_programs.circle_program.view_projection_uniform, generate_projection_matrix(m_viewport.size, m_viewport.zoom) * translation },
        { m_programs.circle_program.color_uniform, vec4{ 0.22f, 0.76f, 0.95f, 1.0f } },
        { m_programs.circle_program.radius_uniform, 2.5f / (float)m_viewport.zoom },
        { m_programs.circle_program.zoom_uniform, (float)m_viewport.zoom }
      },
      {
        { 0.0f, 0.0f },
        m_viewport.size
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

    GPU::Device::draw_elements_instanced(QUAD_VERTEX_INDICES.size(), m_circle_data.instances.size(), state);
  }

  void Renderer::ensure_instance_buffer_size(InstancedMeshData& data) {
    if (data.instances.size() <= data.buffer_size) return;

    GPU::Memory::Allocator::free_general_buffer(data.instance_buffer_id);

    data.buffer_size = (uint32_t)data.instances.size() * 2;
    data.instance_buffer_id = GPU::Memory::Allocator::allocate_general_buffer<vec2>(data.buffer_size, data.name);
  }

  void Renderer::init_text_renderer() {
    /* load font file */
    long size;
    unsigned char* font_buffer;

    FILE* font_file = fopen("res\\fonts\\consolas.ttf", "rb");
    fseek(font_file, 0, SEEK_END);
    size = ftell(font_file); /* how long is the file ? */
    fseek(font_file, 0, SEEK_SET); /* reset */

    font_buffer = new unsigned char[size];

    fread(font_buffer, size, 1, font_file);
    fclose(font_file);

    int baked = stbtt_BakeFontBitmap(font_buffer, 0, 12.0f, m_bitmap, 128, 128, 32, 96, m_cdata); /* no guarantee this fits! */

    delete[] font_buffer;

    m_debug_font_atlas_id = GPU::Memory::Allocator::allocate_texture({ 128, 128 }, GPU::TextureFormat::R8, "DebugFontAtlas");
    // TODO: Proper batching, now limited to 200 letters
    m_debug_vertex_buffer_id = GPU::Memory::Allocator::allocate_general_buffer<float>(6 * 4 * 200, "DebugVertices");
  }

}
