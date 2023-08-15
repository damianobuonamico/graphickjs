#include "renderer.h"

#include "geometry/path.h"
#include "gpu/allocator.h"

#include "../math/math.h"

#include "../utils/resource_manager.h"
#include "../utils/console.h"

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

    get()->init_instanced_renderers();
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
    GPU::Device::set_viewport(viewport.size, viewport.dpr);
    GPU::Device::clear({ vec4{ 1.0f, 1.0f, 1.0f, 1.0f }, std::nullopt, std::nullopt });
  }

  void Renderer::end_frame() {
    OPTICK_EVENT();

    get()->draw_opaque_tiles();
    get()->draw_masked_tiles();
    get()->flush_gpu_path_instances();
    get()->flush_line_instances();
    get()->flush_square_instances();
    get()->flush_circle_instances();

    GPU::Memory::Allocator::purge_if_needed();
    GPU::Device::end_commands();
  }

  void Renderer::draw(const Geometry::Path& path, const vec4& color) {
    if (path.empty()) return;

    OPTICK_EVENT();

    get()->add_gpu_path_instance(path);
    // get()->m_tiler.process_path(path, color);
  }

  void Renderer::draw_outline(const Geometry::Path& path) {
    if (path.empty()) return;

    get()->add_line_instances(path);
    get()->add_vertex_instances(path);
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
    const std::vector<uint8_t*> reverse_textures = m_tiler.masks_textures_data();

    if (reverse_tiles.empty() || reverse_textures.empty()) return;

    const std::vector<MaskedTile> tiles = std::vector<MaskedTile>(reverse_tiles.rbegin(), reverse_tiles.rend());
    const std::vector<uint8_t*> textures = std::vector<uint8_t*>(reverse_textures.rbegin(), reverse_textures.rend());

    for (size_t i = 0; i < textures.size(); i++) {
      draw_masked_tiles_batch(tiles, i, textures);
    }
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
    if (m_circle_data.instance_buffer_id != 0) {
      GPU::Memory::Allocator::free_general_buffer(m_circle_data.instance_buffer_id);
    }

    if (m_lines_data.vertex_buffer_id != 0) {
      GPU::Memory::Allocator::free_general_buffer(m_lines_data.vertex_buffer_id);
    }
    if (m_square_data.vertex_buffer_id != 0) {
      GPU::Memory::Allocator::free_general_buffer(m_square_data.vertex_buffer_id);
    }
    if (m_circle_data.vertex_buffer_id != 0) {
      GPU::Memory::Allocator::free_general_buffer(m_circle_data.vertex_buffer_id);
    }

    m_lines_data.instance_buffer_id = GPU::Memory::Allocator::allocate_general_buffer<vec4>(m_lines_data.max_instance_buffer_size, "Lines");
    m_square_data.instance_buffer_id = GPU::Memory::Allocator::allocate_general_buffer<vec2>(m_square_data.buffer_size, "Squares");
    m_circle_data.instance_buffer_id = GPU::Memory::Allocator::allocate_general_buffer<vec2>(m_circle_data.buffer_size, "Circles");

    m_lines_data.vertex_buffer_id = GPU::Memory::Allocator::allocate_general_buffer<float>(LINE_VERTEX_POSITIONS.size(), "LineVertices");
    m_square_data.vertex_buffer_id = GPU::Memory::Allocator::allocate_general_buffer<float>(SQUARE_VERTEX_POSITIONS.size(), "SquareVertices");
    m_circle_data.vertex_buffer_id = GPU::Memory::Allocator::allocate_general_buffer<float>(SQUARE_VERTEX_POSITIONS.size(), "CircleVertices");

    const GPU::Buffer& line_vertex_buffer = GPU::Memory::Allocator::get_general_buffer(m_lines_data.vertex_buffer_id);
    const GPU::Buffer& circle_vertex_buffer = GPU::Memory::Allocator::get_general_buffer(m_square_data.vertex_buffer_id);
    const GPU::Buffer& square_vertex_buffer = GPU::Memory::Allocator::get_general_buffer(m_circle_data.vertex_buffer_id);

    GPU::Device::upload_to_buffer(line_vertex_buffer, 0, LINE_VERTEX_POSITIONS, GPU::BufferTarget::Vertex);
    GPU::Device::upload_to_buffer(circle_vertex_buffer, 0, SQUARE_VERTEX_POSITIONS, GPU::BufferTarget::Vertex);
    GPU::Device::upload_to_buffer(square_vertex_buffer, 0, SQUARE_VERTEX_POSITIONS, GPU::BufferTarget::Vertex);

    m_common_data.quad_index_buffer.clear();
    m_common_data.quad_vertex_buffer.clear();

    m_common_data.quad_vertex_buffer.copy(QUAD_VERTEX_POSITIONS);
    m_common_data.quad_index_buffer.copy(QUAD_VERTEX_INDICES);

    m_common_data.quad_vertex_buffer.upload();
    m_common_data.quad_index_buffer.upload();
  }

  struct GPUPathh {
    vec2 path_position;
    vec2 path_size;
    float path_index;
  };

#define PATHS_TEXTURE_SIZE 32

  void Renderer::draw_gpu_path(const Geometry::Path& path, const vec4& color) {
    std::vector<GPUPathh> paths;
    std::vector<float> texture(PATHS_TEXTURE_SIZE * PATHS_TEXTURE_SIZE);

    const auto& segments = path.segments();
    rect rect = path.bounding_rect();

    rect.min *= m_viewport.zoom;
    rect.max *= m_viewport.zoom;

    paths.push_back({
      rect.min,
      rect.size(),
      0.0f
      });

    size_t i = 0;
    texture[i++] = segments.size();

    for (const auto& segment : segments) {
      vec2 p0 = segment.p0() * m_viewport.zoom - rect.min;
      vec2 p3 = segment.p3() * m_viewport.zoom - rect.min;

      texture[i++] = p0.x;
      texture[i++] = p0.y;
      texture[i++] = p3.x;
      texture[i++] = p3.y;
    }

    if (!path.closed()) {
      vec2 p0 = segments.back().p3() * m_viewport.zoom - rect.min;
      vec2 p3 = segments.front().p0() * m_viewport.zoom - rect.min;

      texture[i++] = p0.x;
      texture[i++] = p0.y;
      texture[i++] = p3.x;
      texture[i++] = p3.y;
    }

    // TODO: preallocate and preserve buffers
    uuid paths_buffer_id = GPU::Memory::Allocator::allocate_general_buffer<GPUPathh>(1, "GPUPaths");
    uuid paths_texture_id = GPU::Memory::Allocator::allocate_texture({ PATHS_TEXTURE_SIZE, PATHS_TEXTURE_SIZE }, GPU::TextureFormat::R32F, "PathsTexture");

    const GPU::Buffer& quad_vertex_positions_buffer = GPU::Memory::Allocator::get_general_buffer(m_quad_vertex_positions_buffer_id);
    const GPU::Buffer& quad_vertex_indices_buffer = GPU::Memory::Allocator::get_index_buffer(m_quad_vertex_indices_buffer_id);
    const GPU::Buffer& instance_buffer = GPU::Memory::Allocator::get_general_buffer(paths_buffer_id);
    const GPU::Texture& paths_texture = GPU::Memory::Allocator::get_texture(paths_texture_id);

    GPU::Device::upload_to_buffer(instance_buffer, 0, paths, GPU::BufferTarget::Vertex);
    GPU::Device::upload_to_texture(paths_texture, { { 0.0f, 0.0f }, { (float)PATHS_TEXTURE_SIZE, (float)PATHS_TEXTURE_SIZE } }, texture.data());

    GPU::GPUPathVertexArray gpu_path_vertex_array(
      m_programs.gpu_path_program,
      instance_buffer,
      quad_vertex_positions_buffer,
      quad_vertex_indices_buffer
    );

    mat4 translation = mat4{
      1.0f, 0.0f, 0.0f, 0.5f * (-m_viewport.size.x /*/ (float)m_viewport.zoom*/ + 2.0f * m_viewport.position.x * (float)m_viewport.zoom),
      0.0f, 1.0f, 0.0f, 0.5f * (-m_viewport.size.y /*/ (float)m_viewport.zoom*/ + 2.0f * m_viewport.position.y * (float)m_viewport.zoom),
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
    };

    GPU::RenderState state = {
      nullptr,
      m_programs.gpu_path_program.program,
      *gpu_path_vertex_array.vertex_array,
      GPU::Primitive::Triangles,
      {
        { { m_programs.gpu_path_program.paths_texture_uniform, paths_texture.gl_texture }, paths_texture }
      },
      {},
      {
        { m_programs.gpu_path_program.view_projection_uniform, generate_projection_matrix(m_viewport.size, 1.0f) * translation },
        { m_programs.gpu_path_program.color_uniform, color },
        { m_programs.gpu_path_program.paths_texture_size_uniform, (int)PATHS_TEXTURE_SIZE }
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

    GPU::Device::draw_elements_instanced(6, 1, state);

    GPU::Memory::Allocator::free_general_buffer(paths_buffer_id);
    GPU::Memory::Allocator::free_texture(paths_texture_id);
  }

  void Renderer::begin_instanced_renderers() {
    m_lines_data.instance_buffer_ptr = m_lines_data.instance_buffer;

    m_lines_data.instances = 0;
    m_square_data.instances.clear();
    m_circle_data.instances.clear();
    m_gpu_paths_data.instance_buffer.clear();
    m_gpu_paths_data.segments_texture.clear();
  }

  void Renderer::add_line_instances(const Geometry::Path& path) {
    OPTICK_EVENT();

    for (const auto& segment : path.segments()) {
      vec2 p0 = segment.p0();
      vec2 p3 = segment.p3();
      // auto p0 = transform.Map(segment.p0().x, segment.p0().y);
      // auto p3 = transform.Map(segment.p3().x, segment.p3().y);

      if (segment.is_cubic()) {
        add_cubic_segment_instance(p0, segment.p1(), segment.p2(), p3);
      } else {
        add_linear_segment_instance(p0, p3);
      }
    }
  }

  void Renderer::add_gpu_path_instance(const Geometry::Path& path) {
    // TODO: Check if flush is needed

    rect bounding_rect = path.bounding_rect();
    rect visible = { -m_viewport.position, vec2{ (float)m_viewport.size.x / m_viewport.zoom, (float)m_viewport.size.y / m_viewport.zoom } - m_viewport.position };

    if (!Math::does_rect_intersect_rect(bounding_rect, visible)) return;

    bounding_rect *= m_viewport.zoom;

    m_gpu_paths_data.instance_buffer->position = bounding_rect.min;
    m_gpu_paths_data.instance_buffer->size = bounding_rect.size();
    m_gpu_paths_data.instance_buffer->segments_index = (float)m_gpu_paths_data.segments_texture.count();
    m_gpu_paths_data.instance_buffer->color_index = 0.0f;
    m_gpu_paths_data.instance_buffer++;

    const auto& segments = path.segments();

    m_gpu_paths_data.segments_texture.push({ (float)segments.size(), 0.0f, 0.0f, 0.0f });

    for (const auto& segment : segments) {
      vec2 p0 = segment.p0() * m_viewport.zoom - bounding_rect.min;
      vec2 p3 = segment.p3() * m_viewport.zoom - bounding_rect.min;

      m_gpu_paths_data.segments_texture.push({ p0.x, p0.y, p3.x, p3.y });
    }

    if (!path.closed()) {
      vec2 p0 = segments.back().p3() * m_viewport.zoom - bounding_rect.min;
      vec2 p3 = segments.front().p0() * m_viewport.zoom - bounding_rect.min;

      m_gpu_paths_data.segments_texture.push({ p0.x, p0.y, p3.x, p3.y });
    }
  }

  void Renderer::add_linear_segment_instance(const vec2 p0, const vec2 p3) {
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

  void Renderer::add_vertex_instances(const Geometry::Path& path) {
    for (const auto& segment : path.segments()) {
      vec2 p0 = segment.p0();

      // auto p0 = transform.Map(segment.p0().x, segment.p0().y);

      add_square_instance(p0);

      if (segment.is_cubic()) {
        vec2 p1 = segment.p1();
        vec2 p2 = segment.p2();
        vec2 p3 = segment.p3();

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

    if (path.closed()) {
      add_square_instance(path.segments().back().p3());
    }
  }

  void Renderer::add_square_instance(const vec2 position) {
    m_square_data.instances.push_back(position);
  }

  void Renderer::add_circle_instance(const vec2 position) {
    m_circle_data.instances.push_back(position);
  }

  void Renderer::flush_gpu_path_instances() {
    OPTICK_EVENT();

    {
      OPTICK_EVENT("Upload Instance Buffer");
      m_gpu_paths_data.instance_buffer.upload();
    }

    {
      OPTICK_EVENT("Upload Segments Texture");
      // TODO: try double buffering to avoid stalls
      m_gpu_paths_data.segments_texture.upload();
    }

    GPU::GPUPathVertexArray gpu_paths_vertex_array(
      m_programs.gpu_path_program,
      m_gpu_paths_data.instance_buffer.buffer(),
      m_common_data.quad_vertex_buffer.buffer(),
      m_common_data.quad_index_buffer.buffer()
    );

    const GPU::Texture& segments_texture = m_gpu_paths_data.segments_texture.texture();

    mat4 translation = mat4{
      1.0f, 0.0f, 0.0f, 0.5f * (-m_viewport.size.x /*/ (float)m_viewport.zoom*/ + 2.0f * m_viewport.position.x * (float)m_viewport.zoom),
      0.0f, 1.0f, 0.0f, 0.5f * (-m_viewport.size.y /*/ (float)m_viewport.zoom*/ + 2.0f * m_viewport.position.y * (float)m_viewport.zoom),
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
    };

    GPU::RenderState state = {
      nullptr,
      m_programs.gpu_path_program.program,
      *gpu_paths_vertex_array.vertex_array,
      GPU::Primitive::Triangles,
      {
        { { m_programs.gpu_path_program.paths_texture_uniform, segments_texture.gl_texture }, segments_texture }
      },
      {},
      {
        { m_programs.gpu_path_program.view_projection_uniform, generate_projection_matrix(m_viewport.size, 1.0f) * translation },
        { m_programs.gpu_path_program.paths_texture_size_uniform, (int)SEGMENTS_TEXTURE_SIZE }
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

    GPU::Device::draw_elements_instanced(6, m_gpu_paths_data.instance_buffer.count(), state);

    m_gpu_paths_data.instance_buffer.clear();
    m_gpu_paths_data.segments_texture.clear();
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
        {m_programs.line_program.line_width_uniform, 3.0f / (float)m_viewport.zoom },
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
  }

  void Renderer::flush_square_instances() {
    if (m_square_data.instances.empty()) return;

    ensure_instance_buffer_size(m_square_data);

    const GPU::Buffer& vertex_buffer = GPU::Memory::Allocator::get_general_buffer(m_square_data.vertex_buffer_id);
    const GPU::Buffer& index_buffer = GPU::Memory::Allocator::get_index_buffer(m_quad_vertex_indices_buffer_id);
    const GPU::Buffer& instance_buffer = GPU::Memory::Allocator::get_general_buffer(m_square_data.instance_buffer_id);

    GPU::Device::upload_to_buffer(instance_buffer, 0, m_square_data.instances, GPU::BufferTarget::Vertex);

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
        // TOOD: merge dpr and zoom
        {m_programs.square_program.view_projection_uniform, generate_projection_matrix(m_viewport.size, m_viewport.zoom) * translation },
        {m_programs.square_program.color_uniform, vec4{ 0.22f, 0.76f, 0.95f, 1.0f } },
        {m_programs.square_program.size_uniform, 6.0f / (float)m_viewport.zoom },
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

    GPU::Device::draw_elements_instanced(QUAD_VERTEX_INDICES.size(), m_square_data.instances.size(), state);
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
      1.0f, 0.0f, 0.0f, 0.5f * (-m_viewport.size.x / (float)m_viewport.zoom + 2 * m_viewport.position.x),
      0.0f, 1.0f, 0.0f, 0.5f * (-m_viewport.size.y / (float)m_viewport.zoom + 2 * m_viewport.position.y),
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
        // TOOD: merge dpr and zoom
        { m_programs.circle_program.view_projection_uniform, generate_projection_matrix(m_viewport.size, m_viewport.zoom) * translation },
        { m_programs.circle_program.color_uniform, vec4{ 0.22f, 0.76f, 0.95f, 1.0f } },
        { m_programs.circle_program.radius_uniform, 3.0f / (float)m_viewport.zoom },
        { m_programs.circle_program.zoom_uniform, (float)m_viewport.zoom }
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

    GPU::Device::draw_elements_instanced(QUAD_VERTEX_INDICES.size(), m_circle_data.instances.size(), state);
  }

  void Renderer::ensure_instance_buffer_size(InstancedMeshData& data) {
    if (data.instances.size() <= data.buffer_size) return;

    GPU::Memory::Allocator::free_general_buffer(data.instance_buffer_id);

    data.buffer_size = (uint32_t)data.instances.size() * 2;
    data.instance_buffer_id = GPU::Memory::Allocator::allocate_general_buffer<vec2>(data.buffer_size, data.name);
  }

}
