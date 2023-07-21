#include "renderer.h"

#include "mask.h"
#include "gpu/allocator.h"
#include "geometry/path.h"

#include "../utils/resource_manager.h"
#include "../utils/console.h"

#ifdef EMSCRIPTEN
#include <emscripten/html5.h>
#endif

static const std::vector<float> QUAD_VERTEX_POSITIONS = {
  0.0, 1.0, 0.0, 0.0, 0.0,
  0.0, 0.0,  0.0, 0.0, 1.0,
  1.0, 0.0,  0.0, 1.0, 1.0,
  1.0, 1.0, 0.0, 1.0, 0.0
};
static const std::vector<uint32_t> QUAD_VERTEX_INDICES = { 0, 1, 2, 0, 2, 3 };

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
    if (s_instance != nullptr) {
      console::error("Renderer already initialized, call shutdown() before reinitializing!");
      return;
    }

#ifdef EMSCRIPTEN
    EmscriptenWebGLContextAttributes attr;
    emscripten_webgl_init_context_attributes(&attr);

    // TODO: test with and without alpha performance
    attr.majorVersion = 2;
    attr.alpha = false;
    attr.premultipliedAlpha = true;
    attr.antialias = false;
    attr.depth = false;
    attr.stencil = false;

    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context("#canvas", &attr);
    emscripten_webgl_make_context_current(ctx);
#endif

    GPU::Device::init(GPU::DeviceVersion::GLES3, 0);
    GPU::Memory::Allocator::init();

    s_instance = new Renderer();

#ifdef EMSCRIPTEN
    s_instance->m_ctx = ctx;
#endif

    s_instance->m_quad_vertex_positions_buffer_id = GPU::Memory::Allocator::allocate_general_buffer<float>(QUAD_VERTEX_POSITIONS.size(), "QuadVertexPositions");
    s_instance->m_quad_vertex_indices_buffer_id = GPU::Memory::Allocator::allocate_index_buffer<uint32_t>(QUAD_VERTEX_INDICES.size(), "QuadVertexIndices");
    s_instance->m_frame_texture_id = GPU::Memory::Allocator::allocate_texture({ 1, 1 }, GPU::TextureFormat::RGBA8, "Frame");

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

#ifdef EMSCRIPTEN
    emscripten_webgl_destroy_context(get()->m_ctx);
#endif

    delete s_instance;

    GPU::Memory::Allocator::shutdown();
    GPU::Device::shutdown();
  }

  void Renderer::begin_frame(const Viewport& viewport) {
    OPTICK_EVENT();

    GPU::Device::begin_commands();

    get()->m_tags.clear();
    get()->m_points.clear();
    get()->m_geometries.clear();

    get()->m_viewport = viewport;

    get()->begin_lines_batch();
  }

  void Renderer::end_frame() {
    OPTICK_EVENT();

    get()->update_image_data();

    Blaze::DestinationImage<Blaze::TileDescriptor_16x8>& image = get()->m_image;

    if (get()->m_geometries.size() > 0) {
      const Blaze::ImageData image_data(image.GetImageData(), image.GetImageWidth(), image.GetImageHeight(), image.GetBytesPerRow());

      Blaze::Rasterize<Blaze::TileDescriptor_16x8>(get()->m_geometries.data(), get()->m_geometries.size(), get()->get_matrix(), image.GetThreads(), image_data);

      // Free all the memory allocated by threads.
      image.GetThreads().ResetFrameMemory();

      get()->render_frame_backend();
    }

    get()->flush_lines_batch();

    GPU::Memory::Allocator::purge_if_needed();
    GPU::Device::end_commands();
  }

#define CARDS 1

  void Renderer::draw(const Geometry::Path& path, const Blaze::Matrix& transform, const uint32_t color) {
    if (path.segments().size() < 1) {
      return;
    }

#ifdef CARDS
    Mask mask(path);

    uuid texture_id = GPU::Memory::Allocator::allocate_texture(mask.size(), GPU::TextureFormat::R8, "Frame");

    const rect box = path.bounding_rect();

    std::vector<float> vertices = {
      box.min.x, box.min.y, 0.0f, 0.0f,
      box.max.x, box.min.y, 1.0f, 0.0f,
      box.max.x, box.max.y, 1.0f, 1.0f,
      box.min.x, box.max.y, 0.0f, 1.0f
    };

    const uuid vertices_id = GPU::Memory::Allocator::allocate_general_buffer<float>(vertices.size(), "Vertices");

    const GPU::Texture& texture = GPU::Memory::Allocator::get_texture(texture_id);
    const GPU::Buffer& quad_vertex_positions_buffer = GPU::Memory::Allocator::get_general_buffer(vertices_id);
    const GPU::Buffer& quad_vertex_indices_buffer = GPU::Memory::Allocator::get_index_buffer(get()->m_quad_vertex_indices_buffer_id);

    GPU::Device::upload_to_buffer(quad_vertex_positions_buffer, 0, vertices, GPU::BufferTarget::Vertex);
    GPU::Device::upload_to_texture(texture, { { 0.0f, 0.0f }, { (float)mask.size().x, (float)mask.size().y } }, mask.data());

    GPU::QuadVertexArray vertex_array(
      get()->m_programs.quad_program,
      quad_vertex_positions_buffer,
      quad_vertex_indices_buffer
    );

    mat4 translation = mat4{
      1.0f, 0.0f, 0.0f, 0.5f * (-get()->m_viewport.size.x / (float)get()->m_viewport.zoom + 2 * get()->m_viewport.position.x),
      0.0f, 1.0f, 0.0f, 0.5f * (-get()->m_viewport.size.y / (float)get()->m_viewport.zoom + 2 * get()->m_viewport.position.y),
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
    };

    GPU::RenderState state = {
      nullptr,
      get()->m_programs.quad_program.program,
      *vertex_array.vertex_array,
      GPU::Primitive::Triangles,
      {
        { { get()->m_programs.quad_program.frame_texture_uniform, texture.gl_texture }, texture }
      },
      {},
      {
        { get()->m_programs.quad_program.view_projection_uniform, generate_projection_matrix(get()->m_viewport.size, get()->m_viewport.zoom) * translation }
      },
      {
        { 0.0f, 0.0f },
        { (float)get()->m_viewport.size.x, (float)get()->m_viewport.size.y }
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
          get()->m_viewport.background,
          std::nullopt,
          std::nullopt
        },
        true
      }
    };

    GPU::Device::draw_elements(6, state);
    GPU::Memory::Allocator::free_texture(texture_id);
    GPU::Memory::Allocator::free_general_buffer(vertices_id);

    return;
#endif

    rect bounding_rect = path.bounding_rect();

    std::vector<Blaze::PathTag> tags{};
    std::vector<Blaze::FloatPoint> points{};

    tags.push_back(Blaze::PathTag::Move);
    points.push_back({ path.segments().front().p0().x, path.segments().front().p0().y });

    for (auto& segment : path.segments()) {
      vec2 p3 = segment.p3();

      switch (segment.kind()) {
      case Geometry::Segment::Kind::Linear: {
        tags.push_back(Blaze::PathTag::Line);
        points.push_back({ p3.x, p3.y });
        break;
      }
      case Geometry::Segment::Kind::Quadratic: {
        tags.push_back(Blaze::PathTag::Quadratic);
        points.push_back({ segment.p1().x, segment.p1().y });
        points.push_back({ p3.x, p3.y });
        break;
      }
      case Geometry::Segment::Kind::Cubic: {
        tags.push_back(Blaze::PathTag::Cubic);
        points.push_back({ segment.p1().x, segment.p1().y });
        points.push_back({ segment.p2().x, segment.p2().y });
        points.push_back({ p3.x, p3.y });
        break;
      }
      }
    }

    if (path.closed()) {
      tags.push_back(Blaze::PathTag::Close);
    }

    get()->m_tags.push_back(tags);
    get()->m_points.push_back(points);
    get()->m_geometries.push_back(Blaze::Geometry{
      Blaze::IntRect{
        (int)std::floor(bounding_rect.min.x),
        (int)std::floor(bounding_rect.min.y),
        (int)std::ceil(bounding_rect.size().x),
        (int)std::ceil(bounding_rect.size().y)
      },
      get()->m_tags.back().data(),
      get()->m_points.back().data(),
      transform,
      (int)tags.size(),
      (int)points.size(),
      color,
      Blaze::FillRule::NonZero
      });
  }

  void Renderer::draw_outline(const Geometry::Path& path, const Blaze::Matrix& transform) {
    if (path.empty()) return;

    OPTICK_EVENT();

    get()->add_to_lines_batch(path, transform);
  }

  void Renderer::render_frame(const Viewport& viewport) {
    // OPTICK_EVENT();

    // get()->m_viewport = viewport;

    // get()->update_image_data();

    // get()->m_image.DrawImage(get()->m_vector_image, get()->get_matrix());

    // get()->render_frame_backend();
  }

  void Renderer::update_image_data() {
    m_image.UpdateSize(Blaze::IntSize{ m_viewport.size.x, m_viewport.size.y });
    m_image.ClearImage();
  }

  Blaze::Matrix Renderer::get_matrix() {
    double scale = m_viewport.zoom * m_viewport.dpr;

    Blaze::Matrix m = Blaze::Matrix::CreateScale(scale, scale);
    m.PostTranslate(m_viewport.position.x, m_viewport.position.y);

    return m;
  }

  void Renderer::render_frame_backend() {
    return;
    GPU::Memory::Allocator::free_texture(m_frame_texture_id);

    // TODO: texture resizing
    m_frame_texture_id = GPU::Memory::Allocator::allocate_texture({ m_image.GetImageWidth(), m_image.GetImageHeight() }, GPU::TextureFormat::RGBA8, "Frame");

    const GPU::Texture& frame_texture = GPU::Memory::Allocator::get_texture(m_frame_texture_id);
    const GPU::Buffer& quad_vertex_positions_buffer = GPU::Memory::Allocator::get_general_buffer(m_quad_vertex_positions_buffer_id);
    const GPU::Buffer& quad_vertex_indices_buffer = GPU::Memory::Allocator::get_index_buffer(m_quad_vertex_indices_buffer_id);

    GPU::Device::upload_to_texture(frame_texture, { { 0.0f, 0.0f }, { (float)m_image.GetImageWidth(), (float)m_image.GetImageHeight() } }, m_image.GetImageData());

    GPU::QuadVertexArray vertex_array(
      m_programs.quad_program,
      quad_vertex_positions_buffer,
      quad_vertex_indices_buffer
    );

    GPU::RenderState state = {
      nullptr,
      m_programs.quad_program.program,
      *vertex_array.vertex_array,
      GPU::Primitive::Triangles,
      {
        { { m_programs.quad_program.frame_texture_uniform, frame_texture.gl_texture }, frame_texture }
      },
      {},
      {},
      {
        { 0.0f, 0.0f },
        { (float)m_image.GetImageWidth(), (float)m_image.GetImageHeight() }
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
          m_viewport.background,
          std::nullopt,
          std::nullopt
        },
        true
      }
    };

    GPU::Device::draw_elements(6, state);
  }


  void Renderer::init_batched_lines_renderer() {
    delete[] m_lines_data.instance_buffer;

    m_lines_data.instance_buffer = new vec4[m_lines_data.max_instance_count];
    m_lines_data.instance_buffer_ptr = m_lines_data.instance_buffer;

    if (m_lines_data.instance_buffer_id != 0) {
      GPU::Memory::Allocator::free_general_buffer(m_lines_data.instance_buffer_id);
    }

    m_lines_data.instance_buffer_id = GPU::Memory::Allocator::allocate_general_buffer<vec4>(m_lines_data.max_instance_buffer_size, "Lines");
  }

  void Renderer::begin_lines_batch() {
    m_lines_data.instance_buffer_ptr = m_lines_data.instance_buffer;
  }

  void Renderer::add_to_lines_batch(const Geometry::Path& path, const Blaze::Matrix& transform) {
    for (const auto& segment : path.segments()) {
      auto p1 = transform.Map(segment.p0().x, segment.p0().y);
      auto p3 = transform.Map(segment.p3().x, segment.p3().y);

      *m_lines_data.instance_buffer_ptr = { (float)p1.X, (float)p1.Y, (float)p3.X, (float)p3.Y };
      m_lines_data.instance_buffer_ptr++;
    }

    m_lines_data.instances += (uint32_t)path.segments().size();
  }

  void Renderer::flush_lines_batch() {
    GLsizeiptr instance_buffer_size = (uint8_t*)m_lines_data.instance_buffer_ptr - (uint8_t*)m_lines_data.instance_buffer;
    if (instance_buffer_size == 0 || m_lines_data.instances == 0) return;

    const GPU::Buffer& vertex_buffer = GPU::Memory::Allocator::get_general_buffer(m_quad_vertex_positions_buffer_id);
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
  }

  void Renderer::upload_vector_image(const uint8_t* ptr, const int size) {
    get()->m_vector_image.Parse(ptr, size);
  }

}
