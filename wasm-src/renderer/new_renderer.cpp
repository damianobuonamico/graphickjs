#include "new_renderer.h"

#include "gpu/allocator.h"

#include "../utils/resource_manager.h"
#include "../utils/console.h"

#ifdef EMSCRIPTEN
#include <emscripten/html5.h>
#endif

static const std::vector<uint16_t> QUAD_VERTEX_POSITIONS = { 0, 0, 1, 0, 1, 1, 0, 1 };
static const std::vector<uint32_t> QUAD_VERTEX_INDICES = { 0, 1, 3, 1, 2, 3 };

namespace Graphick::Render {

  Renderer* Renderer::s_instance = nullptr;

  void Renderer::init() {
#ifdef EMSCRIPTEN
    EmscriptenWebGLContextAttributes attr;
    emscripten_webgl_init_context_attributes(&attr);

    // TODO: test with and without alpha performance
    attr.alpha = true;
    attr.premultipliedAlpha = true;
    attr.majorVersion = 2;
    attr.antialias = false;
    attr.stencil = false;
    attr.depth = false;

    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context("#canvas", &attr);
    emscripten_webgl_make_context_current(ctx);
#endif

    ResourceManager::init();
    GPU::Device::init(GPU::DeviceVersion::GLES3, 0);
    GPU::Memory::Allocator::init();

    if (s_instance != nullptr) {
      console::error("Renderer already initialized, call shutdown() before reinitializing!");
      return;
    }

    s_instance = new Renderer();
  }

  void Renderer::shutdown() {
    if (s_instance == nullptr) {
      console::error("Renderer already shutdown, call init() before shutting down!");
      return;
    }

    delete s_instance;

    GPU::Memory::Allocator::shutdown();
    GPU::Device::shutdown();
    ResourceManager::shutdown();
  }

  void Renderer::resize(const ivec2 size, float dpr) {
    GPU::Device::set_viewport(size, dpr);
    get()->m_viewport = { size, dpr };

    int tiles = std::max(size.x, size.y) / TILE_SIZE;

    // GPU::Memory::Allocator::free_framebuffer(get()->m_framebuffer_id);
    // get()->m_framebuffer_id = GPU::Memory::Allocator::allocate_framebuffer(
    //   { tiles * (int)TILE_SIZE, tiles * (int)TILE_SIZE },
    //   GPU::TextureFormat::R32F,
    //   "MaskFramebuffer"
    // );
  }

  void Renderer::begin_frame(const vec2 position, float zoom) {
    OPTICK_EVENT();

    // vec2 pos = { 0.0f, 0.0f };
    vec2 pos = position;
    float tile_size = TILE_SIZE / zoom;
    // vec2 pos = vec2{ std::round(position.x / tile_size), std::round(position.y / tile_size) } *tile_size;
    GPU::Device::begin_commands();
    get()->m_tiler.reset(get()->m_viewport.size, pos, zoom);
    get()->m_lines = Temp::Geo(GL_LINES);
    get()->m_viewport.position = pos;
    get()->m_viewport.zoom = zoom;
  }

  void Renderer::end_frame() {
    OPTICK_EVENT();

    // get()->draw_fills();
    // get()->draw_masks();
    get()->draw_spans();
    get()->draw_tiles();
    // get()->draw_lines();
    GPU::Device::end_commands();
  }

  void Renderer::draw(const Geometry::Path& path, const vec4& color) {
    OPTICK_EVENT();

    get()->m_tiler.process_path(path, color);
  }

  void Renderer::draw(const Temp::Geo& geo) {
    auto& lines = get()->m_lines;

    uint32_t offset = lines.offset();
    lines.push_vertices(geo.vertices());
    lines.reserve_indices(geo.indices().size());

    for (auto index : geo.indices()) {
      lines.push_index(offset + index);
    }
  }

  Renderer::Renderer() {
    // m_framebuffer_id = GPU::Memory::Allocator::allocate_framebuffer(
    //   { 0, 0 },
    //   GPU::TextureFormat::R32F,
    //   "MaskFramebuffer"
    // );
  }

#undef OLD_TILER

  void Renderer::draw_fills() {
    OPTICK_EVENT();

    vec2 position = (m_viewport.position * m_viewport.zoom) % TILE_SIZE - TILE_SIZE;

    float half_width = -m_viewport.size.x * 0.5f;
    float half_height = m_viewport.size.y * 0.5f;

    float right = -half_width;
    float left = half_width;
    float top = -half_height;
    float bottom = half_height;

    mat4 projection = {
      2.0f / (right - left), 0.0f, 0.0f, 0.0f,
      0.0f, 2.0f / (top - bottom), 0.0f, 0.0f,
      0.0f, 0.0f, -1.0f, 0.0f,
      -(right + left) / (right - left), -(top + bottom) / (top - bottom), 0.0f, 1.0f
    };

    mat4 translation = {
      1.0f, 0.0f, 0.0f, 0.5f * (-m_viewport.size.x + 2 * position.x),
      0.0f, 1.0f, 0.0f, 0.5f * (-m_viewport.size.y + 2 * position.y),
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
    };

    mat4 mvp = projection * translation;

#ifdef OLD_TILER

    const std::vector<Fill>& opaque_tiles = m_tiler.opaque_tiles();

    // std::vector<Fill> fills;
    // int tiles = (int)std::ceil((float)m_viewport.size.x / 16) * std::ceil((float)m_viewport.size.y / 16);
    // fills.reserve(tiles);

    // console::log("Tiles", opaque_tiles.size());

    // for (int i = 0; i < tiles; i++) {
    //   fills.push_back({ vec4{ 0.0f, 0.0f, 0.0f, (float)i / tiles }, i });
    // }

    UUID quad_vertex_positions_buffer_id = GPU::Memory::Allocator::allocate_general_buffer<uint16_t>(
      QUAD_VERTEX_POSITIONS.size(),
      "QuadVertexPositions"
    );
    UUID quad_vertex_indices_buffer_id = GPU::Memory::Allocator::allocate_index_buffer<uint32_t>(
      QUAD_VERTEX_INDICES.size(),
      "QuadVertexIndices"
    );
    UUID fill_vertex_buffer_id = GPU::Memory::Allocator::allocate_general_buffer<Fill>(
      opaque_tiles.size(),
      "Fill"
    );

    const GPU::Buffer& quad_vertex_positions_buffer = GPU::Memory::Allocator::get_general_buffer(quad_vertex_positions_buffer_id);
    const GPU::Buffer& quad_vertex_indices_buffer = GPU::Memory::Allocator::get_index_buffer(quad_vertex_indices_buffer_id);
    const GPU::Buffer& fill_vertex_buffer = GPU::Memory::Allocator::get_general_buffer(fill_vertex_buffer_id);

    GPU::Device::upload_to_buffer(quad_vertex_positions_buffer, 0, QUAD_VERTEX_POSITIONS, GPU::BufferTarget::Vertex);
    GPU::Device::upload_to_buffer(quad_vertex_indices_buffer, 0, QUAD_VERTEX_INDICES, GPU::BufferTarget::Index);
    GPU::Device::upload_to_buffer(fill_vertex_buffer, 0, opaque_tiles, GPU::BufferTarget::Vertex);

    GPU::FillVertexArray fill_vertex_array(
      m_programs.fill_program,
      fill_vertex_buffer,
      quad_vertex_positions_buffer,
      quad_vertex_indices_buffer
    );

    GPU::RenderState state = {
      nullptr,
      m_programs.fill_program.program,
      *fill_vertex_array.vertex_array,
      GPU::Primitive::Triangles,
      {},
      {},
      {
        { m_programs.fill_program.view_projection_uniform, mvp },
        { m_programs.fill_program.tile_size_uniform, (int)TILE_SIZE },
        { m_programs.fill_program.framebuffer_size_uniform, m_viewport.size }
      },
      {
        { 0.0f, 0.0f },
        { (float)m_viewport.size.x * m_viewport.dpr, (float)m_viewport.size.y * m_viewport.dpr }
      },
      {
        std::nullopt,
        // GPU::BlendState{
        //   GPU::BlendFactor::SrcAlpha,
        //   GPU::BlendFactor::OneMinusSrcAlpha,
        //   GPU::BlendFactor::SrcAlpha,
        //   GPU::BlendFactor::OneMinusSrcAlpha,
        //   GPU::BlendOp::Add,
        // },
        std::nullopt,
        std::nullopt,
        {
          vec4{ 1.0f, 1.0f, 1.0f, 1.0f},
          std::nullopt,
          std::nullopt
        },
        true
      }
    };

    GPU::Device::draw_elements_instanced(6, (uint32_t)opaque_tiles.size(), state);

    GPU::Memory::Allocator::free_general_buffer(quad_vertex_positions_buffer_id);
    GPU::Memory::Allocator::free_index_buffer(quad_vertex_indices_buffer_id);
    GPU::Memory::Allocator::free_general_buffer(fill_vertex_buffer_id);

#endif
  }

  void Renderer::draw_masks() {
    OPTICK_EVENT();

    float size = (float)std::floor((int)std::max(m_viewport.size.x, m_viewport.size.y) / TILE_SIZE) * TILE_SIZE;

    float half_width = -size * 0.5f;
    float half_height = size * 0.5f;

    float right = -half_width;
    float left = half_width;
    float top = -half_height;
    float bottom = half_height;

    mat4 projection = {
      2.0f / (right - left), 0.0f, 0.0f, 0.0f,
      0.0f, 2.0f / (top - bottom), 0.0f, 0.0f,
      0.0f, 0.0f, -1.0f, 0.0f,
      -(right + left) / (right - left), -(top + bottom) / (top - bottom), 0.0f, 1.0f
    };

    mat4 translation = {
      1.0f, 0.0f, 0.0f, 0.5f * (-size),
      0.0f, 1.0f, 0.0f, 0.5f * (-size),
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
    };

    mat4 mvp = projection * translation;

#ifdef OLD_TILER

    const std::vector<Mask>& masks = m_tiler.masks();
    console::log("masks", masks.size());

    UUID quad_vertex_positions_buffer_id = GPU::Memory::Allocator::allocate_general_buffer<uint16_t>(
      QUAD_VERTEX_POSITIONS.size(),
      "QuadVertexPositions"
    );
    UUID quad_vertex_indices_buffer_id = GPU::Memory::Allocator::allocate_index_buffer<uint32_t>(
      QUAD_VERTEX_INDICES.size(),
      "QuadVertexIndices"
    );
    UUID mask_vertex_buffer_id = GPU::Memory::Allocator::allocate_general_buffer<Mask>(
      masks.size(),
      "Mask"
    );

    const GPU::Buffer& quad_vertex_positions_buffer = GPU::Memory::Allocator::get_general_buffer(quad_vertex_positions_buffer_id);
    const GPU::Buffer& quad_vertex_indices_buffer = GPU::Memory::Allocator::get_index_buffer(quad_vertex_indices_buffer_id);
    const GPU::Buffer& mask_vertex_buffer = GPU::Memory::Allocator::get_general_buffer(mask_vertex_buffer_id);

    GPU::Device::upload_to_buffer(quad_vertex_positions_buffer, 0, QUAD_VERTEX_POSITIONS, GPU::BufferTarget::Vertex);
    GPU::Device::upload_to_buffer(quad_vertex_indices_buffer, 0, QUAD_VERTEX_INDICES, GPU::BufferTarget::Index);
    GPU::Device::upload_to_buffer(mask_vertex_buffer, 0, masks, GPU::BufferTarget::Vertex);

    GPU::MaskVertexArray mask_vertex_array(
      m_programs.mask_program,
      mask_vertex_buffer,
      quad_vertex_positions_buffer,
      quad_vertex_indices_buffer
    );

    const GPU::Framebuffer& framebuffer = GPU::Memory::Allocator::get_framebuffer(m_framebuffer_id);

    GPU::RenderState state = {
      &framebuffer,
      m_programs.mask_program.program,
      *mask_vertex_array.vertex_array,
      GPU::Primitive::Triangles,
      {},
      {},
      {
        { m_programs.mask_program.view_projection_uniform, mvp },
        { m_programs.mask_program.tile_size_uniform, (int)TILE_SIZE },
        { m_programs.mask_program.framebuffer_size_uniform, ivec2{ (int)size, (int)size } }
      },
      {
        { 0.0f, 0.0f },
        { (float)size * m_viewport.dpr, (float)size * m_viewport.dpr }
      },
      {
        // std::nullopt,
        GPU::BlendState{
          GPU::BlendFactor::One,
          GPU::BlendFactor::OneMinusSrcAlpha,
          GPU::BlendFactor::One,
          GPU::BlendFactor::OneMinusSrcAlpha,
          GPU::BlendOp::Add,
        },
        std::nullopt,
        std::nullopt,
        {
          vec4{ 0.0f, 0.0f, 0.0f, 1.0f},
          std::nullopt,
          std::nullopt
        },
        true
      }
    };

    GPU::Device::draw_elements_instanced(6, (uint32_t)masks.size(), state);

    GPU::Memory::Allocator::free_general_buffer(quad_vertex_positions_buffer_id);
    GPU::Memory::Allocator::free_index_buffer(quad_vertex_indices_buffer_id);
    GPU::Memory::Allocator::free_general_buffer(mask_vertex_buffer_id);

#endif
  }

  void Renderer::draw_spans() {
    OPTICK_EVENT();

    vec2 position = (m_viewport.position * m_viewport.zoom) % TILE_SIZE - TILE_SIZE;

    float half_width = -m_viewport.size.x * 0.5f;
    float half_height = m_viewport.size.y * 0.5f;

    float right = -half_width;
    float left = half_width;
    float top = -half_height;
    float bottom = half_height;

    mat4 projection = {
      2.0f / (right - left), 0.0f, 0.0f, 0.0f,
      0.0f, 2.0f / (top - bottom), 0.0f, 0.0f,
      0.0f, 0.0f, -1.0f, 0.0f,
      -(right + left) / (right - left), -(top + bottom) / (top - bottom), 0.0f, 1.0f
    };

    mat4 translation = {
      1.0f, 0.0f, 0.0f, 0.5f * (-m_viewport.size.x + 2 * position.x),
      0.0f, 1.0f, 0.0f, 0.5f * (-m_viewport.size.y + 2 * position.y),
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
    };

    // mat4 mvp = projection * translation;

    const std::vector<Span>& spans = m_tiler.spans();

    // std::vector<Fill> fills;
    // int tiles = (int)std::ceil((float)m_viewport.size.x / 16) * std::ceil((float)m_viewport.size.y / 16);
    // fills.reserve(tiles);

    // console::log("Tiles", spans.size());

    // for (int i = 0; i < tiles; i++) {
    //   fills.push_back({ vec4{ 0.0f, 0.0f, 0.0f, (float)i / tiles }, i });
    // }

    UUID quad_vertex_positions_buffer_id = GPU::Memory::Allocator::allocate_general_buffer<uint16_t>(
      QUAD_VERTEX_POSITIONS.size(),
      "QuadVertexPositions"
    );
    UUID quad_vertex_indices_buffer_id = GPU::Memory::Allocator::allocate_index_buffer<uint32_t>(
      QUAD_VERTEX_INDICES.size(),
      "QuadVertexIndices"
    );
    UUID span_vertex_buffer_id = GPU::Memory::Allocator::allocate_general_buffer<Span>(
      spans.size(),
      "Span"
    );

    const GPU::Buffer& quad_vertex_positions_buffer = GPU::Memory::Allocator::get_general_buffer(quad_vertex_positions_buffer_id);
    const GPU::Buffer& quad_vertex_indices_buffer = GPU::Memory::Allocator::get_index_buffer(quad_vertex_indices_buffer_id);
    const GPU::Buffer& span_vertex_buffer = GPU::Memory::Allocator::get_general_buffer(span_vertex_buffer_id);

    GPU::Device::upload_to_buffer(quad_vertex_positions_buffer, 0, QUAD_VERTEX_POSITIONS, GPU::BufferTarget::Vertex);
    GPU::Device::upload_to_buffer(quad_vertex_indices_buffer, 0, QUAD_VERTEX_INDICES, GPU::BufferTarget::Index);
    GPU::Device::upload_to_buffer(span_vertex_buffer, 0, spans, GPU::BufferTarget::Vertex);

    GPU::SpanVertexArray span_vertex_array(
      m_programs.span_program,
      span_vertex_buffer,
      quad_vertex_positions_buffer,
      quad_vertex_indices_buffer
    );

    GPU::RenderState state = {
      nullptr,
      m_programs.span_program.program,
      *span_vertex_array.vertex_array,
      GPU::Primitive::Triangles,
      {},
      {},
      {
        { m_programs.span_program.view_uniform, translation },
        { m_programs.span_program.projection_uniform, projection },
        { m_programs.span_program.tile_size_uniform, (int)TILE_SIZE },
        { m_programs.span_program.framebuffer_size_uniform, m_viewport.size },
      },
      {
        { 0.0f, 0.0f },
        { (float)m_viewport.size.x * m_viewport.dpr, (float)m_viewport.size.y * m_viewport.dpr }
      },
      {
        // std::nullopt,
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
          vec4{ 1.0f, 1.0f, 1.0f, 1.0f},
          std::nullopt,
          std::nullopt
        },
        true
      }
    };

    GPU::Device::draw_elements_instanced(6, (uint32_t)spans.size(), state);

    GPU::Memory::Allocator::free_general_buffer(quad_vertex_positions_buffer_id);
    GPU::Memory::Allocator::free_index_buffer(quad_vertex_indices_buffer_id);
    GPU::Memory::Allocator::free_general_buffer(span_vertex_buffer_id);
  }

  void Renderer::draw_tiles() {
    OPTICK_EVENT();

    vec2 position = (m_viewport.position * m_viewport.zoom) % TILE_SIZE - TILE_SIZE;

    float half_width = -m_viewport.size.x * 0.5f;
    float half_height = m_viewport.size.y * 0.5f;

    float right = -half_width;
    float left = half_width;
    float top = -half_height;
    float bottom = half_height;

    mat4 projection = {
      2.0f / (right - left), 0.0f, 0.0f, 0.0f,
      0.0f, 2.0f / (top - bottom), 0.0f, 0.0f,
      0.0f, 0.0f, -1.0f, 0.0f,
      -(right + left) / (right - left), -(top + bottom) / (top - bottom), 0.0f, 1.0f
    };

    mat4 translation = {
      1.0f, 0.0f, 0.0f, 0.5f * (-m_viewport.size.x + 2 * position.x),
      0.0f, 1.0f, 0.0f, 0.5f * (-m_viewport.size.y + 2 * position.y),
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
    };

    // mat4 mvp = projection * translation;

    const std::vector<Tile>& opaque_tiles = m_tiler.tiles();

    // std::vector<Fill> fills;
    // int tiles = (int)std::ceil((float)m_viewport.size.x / 16) * std::ceil((float)m_viewport.size.y / 16);
    // fills.reserve(tiles);

    // console::log("Tiles", opaque_tiles.size());

    // for (int i = 0; i < tiles; i++) {
    //   fills.push_back({ vec4{ 0.0f, 0.0f, 0.0f, (float)i / tiles }, i });
    // }

    UUID quad_vertex_positions_buffer_id = GPU::Memory::Allocator::allocate_general_buffer<uint16_t>(
      QUAD_VERTEX_POSITIONS.size(),
      "QuadVertexPositions"
    );
    UUID quad_vertex_indices_buffer_id = GPU::Memory::Allocator::allocate_index_buffer<uint32_t>(
      QUAD_VERTEX_INDICES.size(),
      "QuadVertexIndices"
    );
    UUID tile_vertex_buffer_id = GPU::Memory::Allocator::allocate_general_buffer<Tile>(
      opaque_tiles.size(),
      "Tile"
    );

    const GPU::Buffer& quad_vertex_positions_buffer = GPU::Memory::Allocator::get_general_buffer(quad_vertex_positions_buffer_id);
    const GPU::Buffer& quad_vertex_indices_buffer = GPU::Memory::Allocator::get_index_buffer(quad_vertex_indices_buffer_id);
    const GPU::Buffer& tile_vertex_buffer = GPU::Memory::Allocator::get_general_buffer(tile_vertex_buffer_id);

    GPU::Device::upload_to_buffer(quad_vertex_positions_buffer, 0, QUAD_VERTEX_POSITIONS, GPU::BufferTarget::Vertex);
    GPU::Device::upload_to_buffer(quad_vertex_indices_buffer, 0, QUAD_VERTEX_INDICES, GPU::BufferTarget::Index);
    GPU::Device::upload_to_buffer(tile_vertex_buffer, 0, opaque_tiles, GPU::BufferTarget::Vertex);

    GPU::TileVertexArray tile_vertex_array(
      m_programs.tile_program,
      tile_vertex_buffer,
      quad_vertex_positions_buffer,
      quad_vertex_indices_buffer
    );

    // const GPU::Texture& mask_texture = *GPU::Memory::Allocator::get_framebuffer(m_framebuffer_id).texture;
    UUID segments_texture_id = GPU::Memory::Allocator::allocate_texture(
      { SEGMENTS_TEXTURE_SIZE, SEGMENTS_TEXTURE_SIZE },
      GPU::TextureFormat::R8,
      "Segments"
    );
    const GPU::Texture& segments_texture = GPU::Memory::Allocator::get_texture(segments_texture_id);
    // GPU::Device::upload_to_texture(segments_texture, { { 0.0f, 0.0f }, { (float)SEGMENTS_TEXTURE_SIZE, (float)SEGMENTS_TEXTURE_SIZE } }, m_tiler.segments().data());
    GPU::Device::upload_to_texture(segments_texture, { { 0.0f, 0.0f }, { (float)SEGMENTS_TEXTURE_SIZE, (float)SEGMENTS_TEXTURE_SIZE } }, m_tiler.masks_texture_data());


    GPU::RenderState state = {
      nullptr,
      m_programs.tile_program.program,
      *tile_vertex_array.vertex_array,
      GPU::Primitive::Triangles,
      {
        { { m_programs.tile_program.segments_texture_uniform, segments_texture.gl_texture }, segments_texture }
        // { { m_programs.tile_program.mask_texture_uniform, mask_texture.gl_texture }, mask_texture }
      },
      {},
      {
        // { m_programs.tile_program.view_projection_uniform, mvp },
        { m_programs.tile_program.view_uniform, translation },
        { m_programs.tile_program.projection_uniform, projection },
        { m_programs.tile_program.tile_size_uniform, (int)TILE_SIZE },
        { m_programs.tile_program.framebuffer_size_uniform, m_viewport.size },
      },
      {
        { 0.0f, 0.0f },
        { (float)m_viewport.size.x * m_viewport.dpr, (float)m_viewport.size.y * m_viewport.dpr }
      },
      {
        // std::nullopt,
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
        // vec4{ 1.0f, 1.0f, 1.0f, 1.0f},
        std::nullopt,
        std::nullopt,
        std::nullopt
      },
      true
    }
    };

    GPU::Device::draw_elements_instanced(6, (uint32_t)opaque_tiles.size(), state);

    GPU::Memory::Allocator::free_general_buffer(quad_vertex_positions_buffer_id);
    GPU::Memory::Allocator::free_index_buffer(quad_vertex_indices_buffer_id);
    GPU::Memory::Allocator::free_general_buffer(tile_vertex_buffer_id);
    GPU::Memory::Allocator::free_texture(segments_texture_id);
  }

  void Renderer::draw_lines() {
    OPTICK_EVENT();

    float factor = 0.5f / m_viewport.zoom;

    float half_width = -m_viewport.size.x * factor;
    float half_height = m_viewport.size.y * factor;

    float right = -half_width;
    float left = half_width;
    float top = -half_height;
    float bottom = half_height;

    mat4 projection = {
      2.0f / (right - left), 0.0f, 0.0f, 0.0f,
      0.0f, 2.0f / (top - bottom), 0.0f, 0.0f,
      0.0f, 0.0f, -1.0f, 0.0f,
      -(right + left) / (right - left), -(top + bottom) / (top - bottom), 0.0f, 1.0f
    };

    mat4 translation = {
      1.0f, 0.0f, 0.0f, 0.5f * (-m_viewport.size.x / m_viewport.zoom + 2 * m_viewport.position.x),
      0.0f, 1.0f, 0.0f, 0.5f * (-m_viewport.size.y / m_viewport.zoom + 2 * m_viewport.position.y),
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
    };

    mat4 mvp = projection * translation;

    UUID vertex_positions_buffer_id = GPU::Memory::Allocator::allocate_general_buffer<Vertex>(
      m_lines.vertex_count(),
      "VertexPositions"
    );
    UUID vertex_indices_buffer_id = GPU::Memory::Allocator::allocate_index_buffer<uint32_t>(
      m_lines.index_count(),
      "VertexIndices"
    );

    const GPU::Buffer& vertex_positions_buffer = GPU::Memory::Allocator::get_general_buffer(vertex_positions_buffer_id);
    const GPU::Buffer& vertex_indices_buffer = GPU::Memory::Allocator::get_index_buffer(vertex_indices_buffer_id);

    GPU::Device::upload_to_buffer(vertex_positions_buffer, 0, m_lines.vertices(), GPU::BufferTarget::Vertex);
    GPU::Device::upload_to_buffer(vertex_indices_buffer, 0, m_lines.indices(), GPU::BufferTarget::Index);

    GPU::LineVertexArray vertex_array(
      m_programs.line_program,
      vertex_positions_buffer,
      vertex_indices_buffer
    );

    GPU::RenderState state = {
      nullptr,
      m_programs.line_program.program,
      *vertex_array.vertex_array,
      GPU::Primitive::Lines,
      {},
      {},
      {
        {m_programs.line_program.view_projection_uniform, mvp },
        {m_programs.line_program.color_uniform, vec4{0.5f, 0.5f, 0.9f, 0.3f} },
      },
      {
        { 0.0f, 0.0f },
        { (float)m_viewport.size.x * m_viewport.dpr, (float)m_viewport.size.y * m_viewport.dpr }
      },
      {
        std::nullopt,
        // GPU::BlendState{
        //   GPU::BlendFactor::SrcAlpha,
        //   GPU::BlendFactor::OneMinusSrcAlpha,
        //   GPU::BlendFactor::SrcAlpha,
        //   GPU::BlendFactor::OneMinusSrcAlpha,
        //   GPU::BlendOp::Add,
        // },
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

    GPU::Device::draw_elements(m_lines.index_count(), state);

    GPU::Memory::Allocator::free_general_buffer(vertex_positions_buffer_id);
    GPU::Memory::Allocator::free_index_buffer(vertex_indices_buffer_id);
  }

}
