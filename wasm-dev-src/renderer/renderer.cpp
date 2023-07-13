#include "renderer.h"

#include "gpu/allocator.h"
#include "geometry/path.h"

#include "../utils/resource_manager.h"
#include "../utils/console.h"

#ifdef EMSCRIPTEN
#include <emscripten/html5.h>
#endif

static const std::vector<float> QUAD_VERTEX_POSITIONS = {
  // Top left.
  -1.0,  1.0,  0.0,  0.0,  0.0,
  // Bottom left.
  -1.0, -1.0,  0.0,  0.0,  1.0,
  // Bottom right.
  1.0, -1.0,  0.0,  1.0,  1.0,
  // Top right.
  1.0,  1.0,  0.0,  1.0,  0.0
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
  }

  void Renderer::end_frame() {
    OPTICK_EVENT();

    get()->update_image_data();

    Blaze::DestinationImage<Blaze::TileDescriptor_16x8>& image = get()->m_image;

    if (get()->m_geometries.size() < 1) {
      return;
    }

    const Blaze::ImageData image_data(image.GetImageData(), image.GetImageWidth(), image.GetImageHeight(), image.GetBytesPerRow());

    Blaze::Rasterize<Blaze::TileDescriptor_16x8>(get()->m_geometries.data(), get()->m_geometries.size(), get()->get_matrix(), image.GetThreads(), image_data);

    // Free all the memory allocated by threads.
    image.GetThreads().ResetFrameMemory();

    get()->render_frame_backend();

    GPU::Device::end_commands();
  }

  void Renderer::draw(const Geometry::Path& path, const Blaze::Matrix& transform, const uint32_t color) {
    if (path.segments().size() < 1) {
      return;
    }

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

    std::vector<vec2> vertex_positions;
    std::vector<uint32_t> vertex_indices;

    for (const auto& segment : path.segments()) {
      auto p = transform.Map(segment.p0().x, segment.p0().y);
      vertex_positions.push_back({ (float)p.X, (float)p.Y });
    }

    auto p = transform.Map(path.segments().back().p3().x, path.segments().back().p3().y);
    vertex_positions.push_back({ (float)p.X, (float)p.Y });

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

    mat4 translation = mat4{
      1.0f, 0.0f, 0.0f, 0.5f * (-get()->m_viewport.size.x / (float)get()->m_viewport.zoom + 2 * get()->m_viewport.position.x),
      0.0f, 1.0f, 0.0f, 0.5f * (-get()->m_viewport.size.y / (float)get()->m_viewport.zoom + 2 * get()->m_viewport.position.y),
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
    };

    GPU::RenderState state = {
      nullptr,
      get()->m_programs.line_program.program,
      *vertex_array.vertex_array,
      GPU::Primitive::Lines,
      {},
      {},
      {
        {get()->m_programs.line_program.view_projection_uniform, generate_projection_matrix(get()->m_viewport.size, get()->m_viewport.zoom) * translation },
        {get()->m_programs.line_program.color_uniform, vec4{0.3f, 0.3f, 0.9f, 1.0f} },
      },
      {
        { 0.0f, 0.0f },
        { (float)get()->m_viewport.size.x * (float)get()->m_viewport.dpr, (float)get()->m_viewport.size.y * (float)get()->m_viewport.dpr }
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

  void Renderer::render_frame(const Viewport& viewport) {
    OPTICK_EVENT();

    get()->m_viewport = viewport;

    get()->update_image_data();

    // Blaze::DestinationImage<Blaze::TileDescriptor_16x8>& image = get()->m_image;

    // std::vector<Blaze::Geometry> geometries;
    // std::vector<Blaze::FloatPoint> points{
    //   { 0.0f, 0.0f },
    //   { 100.0f, 20.0f },
    //   { 70.0f, 22.0f },
    //   { 70.0f, 50.0f },
    //   { 80.0f, 80.0f },
    //   { 20.0f, 60.0f },
    // };
    // std::vector<Blaze::PathTag> tags{
    //   Blaze::PathTag::Move,
    //   Blaze::PathTag::Line,
    //   Blaze::PathTag::Quadratic,
    //   Blaze::PathTag::Line,
    //   Blaze::PathTag::Line,
    //   Blaze::PathTag::Close
    // };

    // geometries.push_back(Blaze::Geometry{
    //   Blaze::IntRect{ 0, 0, 100, 100 },
    //   tags.data(),
    //   points.data(),
    //   Blaze::Matrix::Identity,
    //   (int)tags.size(),
    //   (int)points.size(),
    //   (uint32_t)0xFF3333CC,
    //   Blaze::FillRule::NonZero
    //   });

    // if (geometries.size() < 1) {
    //   return;
    // }

    // const Blaze::ImageData image_data(image.GetImageData(), image.GetImageWidth(), image.GetImageHeight(), image.GetBytesPerRow());

    // Blaze::Rasterize<Blaze::TileDescriptor_16x8>(geometries.data(), geometries.size(), get()->get_matrix(viewport), image.GetThreads(), image_data);

    // // Free all the memory allocated by threads.
    // image.GetThreads().ResetFrameMemory();

    get()->m_image.DrawImage(get()->m_vector_image, get()->get_matrix());

    get()->render_frame_backend();
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

    GPU::Memory::Allocator::purge_if_needed();
  }

  void Renderer::upload_vector_image(const uint8_t* ptr, const int size) {
    get()->m_vector_image.Parse(ptr, size);
  }

}
