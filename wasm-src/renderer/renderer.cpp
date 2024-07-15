/**
 * @file renderer.cpp
 * @brief The file contains the implementation of the main Graphick renderer.
 *
 * @todo batches of instanced data overflow handling
 * @todo path builder clipping rect
 * @todo dynamic number of samples based on dpr and hardware performance
 */

#include "renderer.h"

#include "gpu/allocator.h"
#include "gpu/device.h"

#include "geometry/path_builder.h"

#include "../math/vector.h"
#include "../math/matrix.h"

#include "../geom/curve_ops.h"

#include "../geom/path_builder.h"
#include "../geom/path.h"

#include "../utils/defines.h"
#include "../utils/assert.h"

#include <algorithm>

#ifdef EMSCRIPTEN
#include <emscripten/html5.h>
#endif

namespace graphick::renderer {

  /* -- Static -- */

  /**
   * @brief The flush data structure.
   *
   * This structure is used to pass data to the flush function.
   *
   * @struct FlushData
   */
  struct FlushData {
    std::vector<GPU::TextureBinding<GPU::TextureParameter, const GPU::Texture&>> textures;    /* The textures to bind. */
    std::vector<GPU::UniformBinding<GPU::Uniform>> uniforms;                        /* The uniforms to bind. */

    vec2 viewport_size;                                                                                                     /* The size of the viewport. */
  };

  /**
   * @brief Generates an orthographic projection matrix.
   *
   * @param size The size of the viewport.
   * @param zoom The zoom level.
   * @return The orthographic projection matrix p.
   */
  static dmat4 orthographic_projection(const vec2 size, double zoom) {
    const dvec2 dsize = dvec2(size);

    const double factor = 0.5f / zoom;
    const double half_width = -dsize.x * factor;
    const double half_height = dsize.y * factor;

    const double right = -half_width;
    const double left = half_width;
    const double top = -half_height;
    const double bottom = half_height;

    return dmat4{
      2.0 / (right - left), 0.0, 0.0, 0.0,
      0.0, 2.0 / (top - bottom), 0.0, 0.0,
      0.0, 0.0, -1.0, 0.0,
      -(right + left) / (right - left), -(top + bottom) / (top - bottom), 0.0, 1.0
    };
  }

  /**
   * @brief Generates an orthographic translation matrix.
   *
   * @param size The size of the viewport.
   * @param position The position of the camera.
   * @param zoom The zoom level.
   * @return The orthographic translation matrix v.
   */
  static dmat4 orthographic_translation(const vec2 size, const vec2 position, const double zoom) {
    const dvec2 dsize = dvec2(size);
    const dvec2 dposition = dvec2(position);

    return dmat4{
      1.0f, 0.0f, 0.0f, 0.5f * (-dsize.x / zoom + 2 * dposition.x),
      0.0f, 1.0f, 0.0f, 0.5f * (-dsize.y / zoom + 2 * dposition.y),
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
    };
  }

  /**
   * @brief Generates a model matrix from a 2x3 transformation matrix.
   *
   * @param transform The 2x3 transformation matrix.
   * @return The model matrix.
   */
  static dmat4 model_matrix(const mat2x3& transform) {
    return dmat4{
      static_cast<double>(transform[0][0]), static_cast<double>(transform[0][1]), static_cast<double>(transform[0][2]), 0.0,
      static_cast<double>(transform[1][0]), static_cast<double>(transform[1][1]), static_cast<double>(transform[1][2]), 0.0,
      0.0, 0.0, 1.0, 0.0,
      0.0, 0.0, 0.0, 1.0
    };
  }

  /**
   * @brief Generates the vertices of a quad centered at a given position.
   *
   * @param position The position of the quad.
   * @param size The size of the quad.
   * @return The vertices of the quad.
   */
  static std::vector<vec2> centered_quad_vertices(const vec2 position, const vec2 size) {
    const vec2 half_size = size * 0.5f;

    return {
      position + vec2{ -half_size.x, -half_size.y },
      position + vec2{ half_size.x, -half_size.y },
      position + vec2{ half_size.x, half_size.y },
      position + vec2{ half_size.x, half_size.y },
      position + vec2{ -half_size.x, half_size.y },
      position + vec2{ -half_size.x, -half_size.y }
    };
  }

  /**
   * @brief Generates the vertices of a quad.
   *
   * @param min The minimum point of the quad.
   * @param max The maximum point of the quad.
   * @return The vertices of the quad.
   */
  static std::vector<vec2> quad_vertices(const vec2 min, const vec2 max) {
    return {
      min,
      vec2{ max.x, min.y },
      max,
      max,
      vec2{ min.x, max.y },
      min
    };
  }

  /**
   * @brief Prepares the renderer for instanced rendering.
   *
   * @param data The instanced data to initialize.
   * @param vertex_buffer_id The ID of the vertex buffer to use if shared, default is uuid::null.
   */
  template <typename T>
  static void init_instanced(InstancedData<T>& data, const uuid vertex_buffer_id = uuid::null) {
    if (data.instance_buffer_id != uuid::null) {
      GPU::Memory::Allocator::free_general_buffer(data.instance_buffer_id);
    }

    data.instance_buffer_id = GPU::Memory::Allocator::allocate_general_buffer<T>(data.max_instances(), "instanced_data");

    if (vertex_buffer_id == uuid::null) {
      if (data.vertex_buffer_id != uuid::null) {
        GPU::Memory::Allocator::free_general_buffer(data.vertex_buffer_id);
      }

      data.vertex_buffer_id = GPU::Memory::Allocator::allocate_general_buffer<vec2>(data.vertices.size(), "instance_vertices");

      const GPU::Buffer& vertex_buffer = GPU::Memory::Allocator::get_general_buffer(data.vertex_buffer_id);

      GPU::Device::upload_to_buffer(vertex_buffer, 0, data.vertices, GPU::BufferTarget::Vertex);
    } else {
      data.vertex_buffer_id = vertex_buffer_id;
    }
  }

  /**
   * @brief Flushes the instanced data to the GPU.
   *
   * Here the GPU draw calls are actually issued.
   *
   * @param data The instanced data to flush.
   * @param program The shader program to use.
   * @param flush_data The flush data to use.
   */
  template <typename T, typename S, typename V>
  static void flush(InstancedData<T>& data, const S& program, const FlushData flush_data) {
    if (data.instances.batches[0].empty()) {
      return;
    }

    const GPU::Buffer& instance_buffer = GPU::Memory::Allocator::get_general_buffer(data.instance_buffer_id);
    const GPU::Buffer& vertex_buffer = GPU::Memory::Allocator::get_general_buffer(data.vertex_buffer_id);

    for (const std::vector<T>& batch : data.instances.batches) {
      GPU::Device::upload_to_buffer(instance_buffer, 0, batch, GPU::BufferTarget::Vertex);

      V vertex_array(program, instance_buffer, vertex_buffer);

      GPU::RenderState state = {
        nullptr,
        program.program,
        *vertex_array.vertex_array,
        data.primitive,
        flush_data.textures,
        {},
        flush_data.uniforms,
        {
          { 0.0f, 0.0f },
          flush_data.viewport_size
        },
        {
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
            std::nullopt,
            std::nullopt,
            std::nullopt
          },
          true
        }
      };

      GPU::Device::draw_arrays_instanced(data.vertices.size(), batch.size(), state);
    }

    data.instances.clear();
  }

  /* -- Static member initialization -- */

  Renderer* Renderer::s_instance = nullptr;

  /* -- Renderer -- */

  void Renderer::init() {
    GK_ASSERT(s_instance == nullptr, "Renderer already initialized, call shutdown() before reinitializing!");

#ifdef EMSCRIPTEN
    EmscriptenWebGLContextAttributes attr;
    emscripten_webgl_init_context_attributes(&attr);

    /* https://developer.mozilla.org/en-US/docs/Web/API/WebGL_API/WebGL_best_practices#avoid_alphafalse_which_can_be_expensive */
    attr.alpha = true;
    attr.premultipliedAlpha = false;
    attr.majorVersion = 2;
    attr.antialias = false;
    attr.stencil = false;
    attr.depth = true;

    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context("#canvas", &attr);
    emscripten_webgl_make_context_current(ctx);

    GPU::Device::init(GPU::DeviceVersion::GLES3, 0);
#else
    GPU::Device::init(GPU::DeviceVersion::GL3, 0);
#endif
    GPU::Memory::Allocator::init();

    s_instance = new Renderer();

    get()->m_path_instances.vertices = quad_vertices({ 0.0f, 0.0f }, { 1.0f, 1.0f });
    get()->m_line_instances.vertices = quad_vertices({ 0.0f, 0.0f }, { 1.0f, 1.0f });
    get()->m_circle_instances.vertices = quad_vertices({ 0.0f, 0.0f }, { 1.0f, 1.0f });
    get()->m_rect_instances.vertices = quad_vertices({ 0.0f, 0.0f }, { 1.0f, 1.0f });
    // get()->m_handle_instances.vertices = quad_vertices({ 0.0f, 0.0f }, { 1.0f, 1.0f });
    // get()->m_vertex_instances.vertices = quad_vertices({ 0.0f, 0.0f }, { 1.0f, 1.0f });
    // get()->m_white_vertex_instances.vertices = quad_vertices({ 0.0f, 0.0f }, { 1.0f, 1.0f });

    init_instanced(get()->m_path_instances);
    init_instanced(get()->m_line_instances, get()->m_path_instances.vertex_buffer_id);
    init_instanced(get()->m_circle_instances, get()->m_path_instances.vertex_buffer_id);
    init_instanced(get()->m_rect_instances, get()->m_path_instances.vertex_buffer_id);
    // init_instanced(get()->m_handle_instances, get()->m_path_instances.vertex_buffer_id);
    // init_instanced(get()->m_vertex_instances, get()->m_path_instances.vertex_buffer_id);
    // init_instanced(get()->m_white_vertex_instances, get()->m_path_instances.vertex_buffer_id);

    if (get()->m_path_instances.curves_texture_id != uuid::null) {
      GPU::Memory::Allocator::free_texture(get()->m_path_instances.curves_texture_id);
    }
    if (get()->m_path_instances.bands_texture_id != uuid::null) {
      GPU::Memory::Allocator::free_texture(get()->m_path_instances.bands_texture_id);
    }

    get()->m_path_instances.curves_texture_id = GPU::Memory::Allocator::allocate_texture({ 512, 512 }, GPU::TextureFormat::RGBA32F, "Curves");
    get()->m_path_instances.bands_texture_id = GPU::Memory::Allocator::allocate_texture({ 512, 512 }, GPU::TextureFormat::R16UI, "Bands");
  }

  void Renderer::shutdown() {
    GK_ASSERT(s_instance != nullptr, "Renderer not initialized, call init() before shutting down!");

    delete s_instance;
    s_instance = nullptr;

    GPU::Memory::Allocator::shutdown();
    GPU::Device::shutdown();

#ifdef EMSCRIPTEN
    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_get_current_context();
    emscripten_webgl_destroy_context(ctx);
#endif
  }

  void Renderer::begin_frame(const Viewport& viewport) {
    const dmat4 view_matrix = orthographic_translation(viewport.size, viewport.position, viewport.zoom);
    const dmat4 projection_matrix = orthographic_projection(viewport.size, viewport.zoom);

    get()->m_viewport = viewport;
    get()->m_vp_matrix = projection_matrix * view_matrix;

    get()->m_transforms.clear();

    // TODO: fix line width
    get()->m_ui_options = UIOptions{
      vec2(viewport.dpr / viewport.zoom * ui_handle_size<double>),
      vec2(viewport.dpr / viewport.zoom * (ui_handle_size<double> -2.0)),
      static_cast<float>(viewport.dpr / viewport.zoom / 2.0 * ui_handle_size<double>),
      static_cast<float>(viewport.dpr * ui_line_width<double>),
      vec4(0.22f, 0.76f, 0.95f, 1.0f),
      vec4(0.22f, 0.76f, 0.95f, 1.0f) * vec4(0.95f, 0.95f, 0.95f, 1.0f),
    };

    GPU::Device::begin_commands();
    GPU::Device::set_viewport(viewport.size);
    GPU::Device::clear({ viewport.background, 1.0f, std::nullopt });
  }

  void Renderer::end_frame() {
    get()->flush_meshes();

    GPU::Memory::Allocator::purge_if_needed();
    size_t time = GPU::Device::end_commands();

    console::log("GPU", static_cast<double>(time) / 1000000.0);
  }

  void Renderer::draw(const geom::quadratic_path& path, const Stroke& stroke, const Fill& fill, const mat2x3& transform, const rect* bounding_rect) {
    const rect bounds = bounding_rect ? *bounding_rect : path.approx_bounding_rect();

    draw(path, fill, transform, &bounds);
    draw(path, stroke, transform, &bounds);
  }

  void Renderer::draw(const geom::quadratic_path& path, const Stroke& stroke, const mat2x3& transform, const rect* bounding_rect) {
    return;

    if (path.empty()) {
      return;
    }

    const float radius_safe = 0.5f * stroke.width * (stroke.join == LineJoin::Miter ? stroke.miter_limit : 1.0f);
    rect bounds = bounding_rect ? *bounding_rect : path.approx_bounding_rect();

    bounds.min -= radius_safe;
    bounds.max += radius_safe;

    auto transformation = math::decompose(transform);
    float scale = std::max(transformation.scale.x, transformation.scale.y);

    // TODO: iterate quadraticpaths returned
    const geom::StrokeOutline stroked_path = geom::path_builder(path, transform, &bounds).stroke(
      geom::StrokingOptions<float>{ stroke.width, stroke.miter_limit, geom::LineCap::Round, stroke.join },
      0.25f / scale
    );

      // const geometry::PathBuilder::StrokeOutline stroked_path = geometry::PathBuilder(path, transform, bounding_rect).stroke(stroke, 0.5f);
    const Fill fill = {
      stroke.color,
      FillRule::NonZero,
      stroke.z_index
    };

    // TODO: actually render both inner and outer

    // TODO: check if is translation only, in that case transform on the GPU
    draw(stroked_path.outer, fill, mat2x3::identity(), &bounds);
    draw(stroked_path.inner, fill, mat2x3::identity(), &bounds);

    // draw_outline(stroked_path.outer, transform, 0.25f, nullptr, nullptr);

    // for (size_t i = 0; i < stroked_path.outer.size(); i++) {
    //   const vec2 p0 = stroked_path.outer[i * 2];
    //   const vec2 p1 = stroked_path.outer[i * 2 + 1];
    //   const vec2 p2 = stroked_path.outer[i * 2 + 2];

    //   get()->m_vertex_instances.instances.push_back(transform * p0);
    //   get()->m_handle_instances.instances.push_back(transform * p1);
    //   get()->m_vertex_instances.instances.push_back(transform * p2);
    // }

    for (size_t i = 0; i < path.size(); i++) {
      const vec2 p0 = path[i * 2];
      const vec2 p1 = path[i * 2 + 1];
      const vec2 p2 = path[i * 2 + 2];

      // get()->m_vertex_instances.instances.push_back(transform * p0);
      // get()->m_circle_instances.instances.push_back({ transform * p1, get()->m_ui_options.handle_radius / 2.0f, vec4(0.2f, 0.8f, 0.2f, 1.0f) });
      // get()->m_vertex_instances.instances.push_back(transform * p2);
    }
  }

  void Renderer::draw(const geom::quadratic_path& path, const Fill& fill, const mat2x3& transform, const rect* bounding_rect) {
    GK_TOTAL("Renderer::draw(fill)");

    if (path.empty()) {
      return;
    }

    // TODO: test different layouts
    // pos: 8bytes, color: 4bytes, params: 1byte = 13bytes * 4 = 52bytes per instance (square)
    // pos: 8bytes, tex: 8bytes, color: 4bytes, params: 1byte = 21bytes * 4 = 84bytes per instance (any size)
    // transform: 24bytes, color: 4bytes, params: 1byte = 29bytes per instance (path)

    // TODO: exact bounding box can perform better
    const rect bounds = bounding_rect ? *bounding_rect : path.approx_bounding_rect();
    const vec2 bounds_size = bounds.size();

    PathInstancedData& data = get()->m_path_instances;

    /* Starting indices for this path. */

    const size_t curves_start_index = data.curves.size() / 2;
    const size_t bands_start_index = data.bands.size();

    /* Copy the curves, close the path and add padding, so that each path starts at xy and not zw. */

    const bool closed = path.closed();
    const size_t len = closed ? path.size() : (path.size() + 1);

    data.curves.insert(data.curves.end(), path.points.begin(), path.points.end());

    if (!closed) {
      data.curves.insert(data.curves.end(), { path[0], path[0] });
    }

    if (path.points.size() % 2 != 0) {
      data.curves.emplace_back(0.0f, 0.0f);
    }

    /* Bands count is always between 1 and 16, based on the number of segments. */

    const float max_size = std::max(bounds_size.x, bounds_size.y);

    const uint8_t horizontal_bands = static_cast<uint8_t>(std::clamp(len * bounds_size.y / max_size / 2.0f, 1.0f, 16.0f));
    const uint8_t vertical_bands = static_cast<uint8_t>(std::clamp(len * bounds_size.x / max_size / 2.0f, 1.0f, 16.0f));
    // const uint8_t horizontal_bands = 1;
    // const uint8_t vertical_bands = 1;

    /* Cache min and max x and y for each curve. */

    std::vector<vec2> min_x_y(len);
    std::vector<vec2> max_x_y(len);

    for (size_t i = 0; i < path.size(); i++) {
      const vec2 p0 = path[i * 2];
      const vec2 p1 = path[i * 2 + 1];
      const vec2 p2 = path[i * 2 + 2];

      min_x_y[i] = vec2{
        std::min({ p0.x, p1.x, p2.x }),
        std::min({ p0.y, p1.y, p2.y }),
      };

      max_x_y[i] = vec2{
        std::max({ p0.x, p1.x, p2.x }),
        std::max({ p0.y, p1.y, p2.y }),
      };
    }

    if (!closed) {
      const vec2 p0 = path.points.back();
      const vec2 p1 = path[0];

      min_x_y.back() = vec2{
        std::min(p0.x, p1.x),
        std::min(p0.y, p1.y)
      };

      max_x_y.back() = vec2{
        std::max(p0.x, p1.x),
        std::max(p0.y, p1.y)
      };
    }

    /* Sort curves by descending max x and y. */

    std::vector<uint16_t> h_indices(len);
    std::vector<uint16_t> v_indices(len);

    std::iota(h_indices.begin(), h_indices.end(), 0);
    std::iota(v_indices.begin(), v_indices.end(), 0);

    std::sort(h_indices.begin(), h_indices.end(), [&](const uint16_t a, const uint16_t b) {
      return max_x_y[a].x > max_x_y[b].x;
    });

    std::sort(v_indices.begin(), v_indices.end(), [&](const uint16_t a, const uint16_t b) {
      return max_x_y[a].y > max_x_y[b].y;
    });

    /* Calculate band metrics. */

    const vec2 band_delta = bounds_size / vec2(uvec2(vertical_bands, horizontal_bands));

    vec2 band_min = bounds.min;
    vec2 band_max = bounds.min + band_delta;

    /* Preallocate horizontal and vertical bands header. */

    data.bands.resize(data.bands.size() + horizontal_bands * 2 + vertical_bands * 2, 0);

    /* Determine which curves are in each horizontal band. */

    for (int i = 0; i < horizontal_bands; i++) {
      const size_t band_start = data.bands.size();

      for (uint16_t j = 0; j < h_indices.size(); j++) {
        const float min_y = min_x_y[h_indices[j]].y;
        const float max_y = max_x_y[h_indices[j]].y;

        if (min_y == max_y || min_y > band_max.y + math::geometric_epsilon<float> || max_y < band_min.y - math::geometric_epsilon<float>) {
          continue;
        }

        data.bands.push_back(h_indices[j]);
      }

      const size_t band_end = data.bands.size();

      /* Each band header is an offset from this path's bands data start and the number of curves in the band. */

      data.bands[bands_start_index + i * 2] = static_cast<uint16_t>(band_start - bands_start_index);
      data.bands[bands_start_index + i * 2 + 1] = static_cast<uint16_t>(band_end - band_start);

      band_min.y += band_delta.y;
      band_max.y += band_delta.y;
    }

    /* Determine which curves are in each vertical band. */

    for (int i = 0; i < vertical_bands; i++) {
      const size_t band_start = data.bands.size();

      for (uint16_t j = 0; j < v_indices.size(); j++) {
        float min_x = min_x_y[v_indices[j]].x;
        float max_x = max_x_y[v_indices[j]].x;

        if (min_x == max_x || min_x > band_max.x || max_x < band_min.x) {
          continue;
        }

        data.bands.push_back(v_indices[j]);
      }

      const size_t band_end = data.bands.size();

      /* Each band header is an offset from this path's bands data start and the number of curves in the band. */

      data.bands[bands_start_index + (horizontal_bands + i) * 2] = static_cast<uint16_t>(band_start - bands_start_index);
      data.bands[bands_start_index + (horizontal_bands + i) * 2 + 1] = static_cast<uint16_t>(band_end - band_start);

      band_min.x += band_delta.x;
      band_max.x += band_delta.x;
    }

    /* Push instance. */

    data.instances.push_back({
      transform, bounds.min, bounds_size, fill.color,
      curves_start_index, bands_start_index,
      horizontal_bands, vertical_bands,
      true, fill.rule == FillRule::EvenOdd
    });

    // for (size_t i = 0; i < path.size(); i++) {
    //   const vec2 p0 = path[i * 2];
    //   const vec2 p1 = path[i * 2 + 1];
    //   const vec2 p2 = path[i * 2 + 2];

    //   // get()->m_vertex_instances.instances.push_back(transform * p0);
    //   // get()->m_handle_instances.instances.push_back(transform * p1);
    //   // get()->m_vertex_instances.instances.push_back(transform * p2);
    //   get()->m_circle_instances.instances.push_back({ transform * p2, get()->m_ui_options.handle_radius / 1.5f, vec4(0.2f, 0.8f, 0.2f, 1.0f) });
    //   // get()->m_handle_instances.instances.push_back(transform * p2);
    // }
  }

  void Renderer::draw(const geom::cubic_path& path, const Fill& fill, const mat2x3& transform, const rect* bounding_rect) {
    GK_TOTAL("Renderer::draw(fill, cubic)");

    if (path.empty()) {
      return;
    }

    const rect bounds = bounding_rect ? *bounding_rect : path.approx_bounding_rect();
    const vec2 bounds_size = bounds.size();

    PathInstancedData& data = get()->m_path_instances;

    /* Starting indices for this path. */

    const size_t curves_start_index = data.curves.size() / 2;
    const size_t bands_start_index = data.bands.size();

    /* Copy the curves, close the path and add padding, so that each curve starts at xy and not zw. */

    const bool closed = path.closed();
    const size_t len = closed ? path.size() : (path.size() + 1);

    // data.curves.insert(data.curves.end(), path.points.begin(), path.points.end());

    data.curves.reserve(path.size() * 4);

    for (size_t i = 0; i < path.size(); i++) {
      const vec2 p0 = path[i * 3];
      const vec2 p1 = path[i * 3 + 1];
      const vec2 p2 = path[i * 3 + 2];
      const vec2 p3 = path[i * 3 + 3];

      data.curves.insert(data.curves.end(), { p0, p1, p2, p3 });
    }

    if (!closed) {
      const vec2 p0 = path.points.back();

      data.curves.insert(data.curves.end(), { p0, p0, path[0], path[0] });
    }

    /* Bands count is always between 1 and 16, based on the number of segments. */

    const float max_size = std::max(bounds_size.x, bounds_size.y);

    const uint8_t horizontal_bands = static_cast<uint8_t>(std::clamp(len * bounds_size.y / max_size / 2.0f, 1.0f, 16.0f));
    const uint8_t vertical_bands = static_cast<uint8_t>(std::clamp(len * bounds_size.x / max_size / 2.0f, 1.0f, 16.0f));

    /* Cache min and max x and y for each curve. */

    std::vector<vec2> min_x_y(len);
    std::vector<vec2> max_x_y(len);

    for (size_t i = 0; i < path.size(); i++) {
      const vec2 p0 = path[i * 3];
      const vec2 p1 = path[i * 3 + 1];
      const vec2 p2 = path[i * 3 + 2];
      const vec2 p3 = path[i * 3 + 3];

      min_x_y[i] = vec2{
        std::min({ p0.x, p1.x, p2.x, p3.x }),
        std::min({ p0.y, p1.y, p2.y, p3.y }),
      };

      max_x_y[i] = vec2{
        std::max({ p0.x, p1.x, p2.x, p3.x }),
        std::max({ p0.y, p1.y, p2.y, p3.y }),
      };
    }

    if (!closed) {
      const vec2 p0 = path.points.back();
      const vec2 p1 = path[0];

      min_x_y.back() = vec2{
        std::min(p0.x, p1.x),
        std::min(p0.y, p1.y)
      };

      max_x_y.back() = vec2{
        std::max(p0.x, p1.x),
        std::max(p0.y, p1.y)
      };
    }

    /* Sort curves by descending max x and y. */

    std::vector<uint16_t> h_indices(len);
    std::vector<uint16_t> v_indices(len);

    std::iota(h_indices.begin(), h_indices.end(), 0);
    std::iota(v_indices.begin(), v_indices.end(), 0);

    std::sort(h_indices.begin(), h_indices.end(), [&](const uint16_t a, const uint16_t b) {
      return max_x_y[a].x > max_x_y[b].x;
    });

    std::sort(v_indices.begin(), v_indices.end(), [&](const uint16_t a, const uint16_t b) {
      return max_x_y[a].y > max_x_y[b].y;
    });

    /* Calculate band metrics. */

    const vec2 band_delta = bounds_size / vec2(uvec2(vertical_bands, horizontal_bands));

    vec2 band_min = bounds.min;
    vec2 band_max = bounds.min + band_delta;

    /* Preallocate horizontal and vertical bands header. */

    data.bands.resize(data.bands.size() + horizontal_bands * 2 + vertical_bands * 2, 0);

    /* Determine which curves are in each horizontal band. */

    for (int i = 0; i < horizontal_bands; i++) {
      const size_t band_start = data.bands.size();

      for (uint16_t j = 0; j < h_indices.size(); j++) {
        const float min_y = min_x_y[h_indices[j]].y;
        const float max_y = max_x_y[h_indices[j]].y;

        if (min_y == max_y || min_y > band_max.y || max_y < band_min.y) {
          continue;
        }

        data.bands.push_back(h_indices[j]);
      }

      const size_t band_end = data.bands.size();

      /* Each band header is an offset from this path's bands data start and the number of curves in the band. */

      data.bands[bands_start_index + i * 2] = static_cast<uint16_t>(band_start - bands_start_index);
      data.bands[bands_start_index + i * 2 + 1] = static_cast<uint16_t>(band_end - band_start);

      band_min.y += band_delta.y;
      band_max.y += band_delta.y;
    }

    /* Determine which curves are in each vertical band. */

    for (int i = 0; i < vertical_bands; i++) {
      const size_t band_start = data.bands.size();

      for (uint16_t j = 0; j < v_indices.size(); j++) {
        float min_x = min_x_y[v_indices[j]].x;
        float max_x = max_x_y[v_indices[j]].x;

        if (min_x == max_x || min_x > band_max.x || max_x < band_min.x) {
          continue;
        }

        data.bands.push_back(v_indices[j]);
      }

      const size_t band_end = data.bands.size();

      /* Each band header is an offset from this path's bands data start and the number of curves in the band. */

      data.bands[bands_start_index + (horizontal_bands + i) * 2] = static_cast<uint16_t>(band_start - bands_start_index);
      data.bands[bands_start_index + (horizontal_bands + i) * 2 + 1] = static_cast<uint16_t>(band_end - band_start);

      band_min.x += band_delta.x;
      band_max.x += band_delta.x;
    }

    /* Push instance. */

    data.instances.push_back({
      transform, bounds.min, bounds_size, fill.color,
      curves_start_index, bands_start_index,
      horizontal_bands, vertical_bands,
      false, fill.rule == FillRule::EvenOdd
    });

    /* Culling attempt. */


    band_min = bounds.min;
    band_max = bounds.min + band_delta;

    // get()->m_line_instances.instances.push_back({ bounds.min, bounds.min + vec2(bounds.width(), 0.0f), 1.0f, vec4(1.0f, 1.0f, 1.0f, 1.0f) });
    // get()->m_line_instances.instances.push_back({ bounds.min + vec2(0.0f, band_delta.y), bounds.min + vec2(bounds.width(), band_delta.y), 1.0f, vec4(1.0f, 1.0f, 1.0f, 1.0f) });

    // std::vector<float> top_intersections;
    // std::vector<float> bottom_intersections;

    struct LineIntersections {
      float line_y;
      std::vector<float> intersections;
    };

    std::vector<LineIntersections> lines;

    for (int8_t l = 0; l < horizontal_bands; l++) {
      const float y = bounds.min.y + l * band_delta.y;

      LineIntersections& line = lines.emplace_back(LineIntersections{ y, {} });

      for (size_t i = 0; i < path.size(); i++) {
        const vec2 p0 = path[i * 3];
        const vec2 p1 = path[i * 3 + 1];
        const vec2 p2 = path[i * 3 + 2];
        const vec2 p3 = path[i * 3 + 3];

        bool is_downwards = p0.y > y || p3.y < y;

        if (
          (is_downwards && ((p0.y < y && p3.y <= y) || (p0.y > y && p3.y >= y))) ||
          (!is_downwards && ((p0.y <= y && p3.y < y) || (p0.y >= y && p3.y > y)))
        ) {
          continue;
        }

        const geom::cubic_bezier curve = geom::cubic_bezier(p0, p1, p2, p3);
        const auto& [a, b, c, d] = curve.coefficients();

        float t = -(p0.y - y) / (p3.y - p0.y);

        for (int i = 0; i < 3; i++) {
          float t_sq = t * t;
          float f = a.y * t_sq * t + b.y * t_sq + c.y * t + d.y - y;
          float f_prime = 3.0f * a.y * t_sq + 2.0f * b.y * t + c.y;
          float f_second = 6.0f * a.y * t + 2.0f * b.y;
          float f_third = 6.0f * a.y;

          t = t - 3.0f * f * (3.0f * f_prime * f_prime - f * f_second) /
            (9.0f * f_prime * f_prime * f_prime - 9.0f * f * f_prime * f_second + f * f * f_third);
        }

        if (t >= 0.0f && t <= 1.0f) {
          line.intersections.push_back(curve.sample(t).x);
        }
      }
    }

    struct Range {
      float min;
      float max;
    };

    std::vector<std::vector<Range>> ranges;

    for (auto& line : lines) {
      std::sort(line.intersections.begin(), line.intersections.end());
      std::vector<Range>& line_ranges = ranges.emplace_back();

      int winding = 0;

      for (int j = 0; j < line.intersections.size(); j++) {
        if (winding % 2 == 0) {
          line_ranges.emplace_back(Range{ line.intersections[j], line.intersections[j] });
        } else if (!line_ranges.empty()) {
          line_ranges.back().max = line.intersections[j];
        }

        winding++;
      }

      get()->m_line_instances.instances.push_back({ vec2(bounds.min.x, line.line_y), vec2(bounds.max.x, line.line_y), 1.0f, vec4(1.0f, 1.0f, 1.0f, 1.0f) });

      for (float intersection : line.intersections) {
        get()->m_circle_instances.instances.push_back({ vec2(intersection, line.line_y), get()->m_ui_options.handle_radius, vec4(1.0f, 1.0f, 1.0f, 1.0f) });
      }

      for (const Range& range : line_ranges) {
        get()->m_line_instances.instances.push_back({ vec2(range.min, line.line_y), vec2(range.max, line.line_y), 1.0f, vec4(0.9f, 0.1f, 0.1f, 1.0f) });
      }
    }

    // std::sort(lines[0].intersections.begin(), lines[0].intersections.end());

#if 0
    enum class PivotType {
      Top,
      Middle,
      Bottom
    };

    struct Pivot {
      float x;
      PivotType type;
    };

    struct ActiveRect {
      float in_x;
      float out_x;
      float active = false;
    };
#endif

    {
      GK_TOTAL("Calculating Occlusion");

      struct ActivePoint {
        float x;
        int index;
      };

      for (int i = 0; i < lines.size() - 1; i++) {
        const std::vector<Range> top_ranges = ranges[i];
        const std::vector<Range> bottom_ranges = ranges[i + 1];

        // std::vector<float> points;

        // for (uint16_t j = data.bands[bands_start_index + i * 2]; j < data.bands[bands_start_index + i * 2] + data.bands[bands_start_index + i * 2 + 1]; j++) {
        //   const uint16_t curve_index = data.bands[bands_start_index + j];

        //   const vec2 p0 = data.curves[curve_index * 4];
        //   const vec2 p3 = data.curves[curve_index * 4 + 3];

        //   if (p0.y > lines[i].line_y + math::geometric_epsilon<float> && p0.y < lines[i + 1].line_y - math::geometric_epsilon<float>) {
        //     points.push_back(p0.x);
        //   }

        //   if (p3.y > lines[i].line_y + math::geometric_epsilon<float> && p3.y < lines[i + 1].line_y - math::geometric_epsilon<float>) {
        //     points.push_back(p3.x);
        //   }
        // }

        for (const Range& top_range : top_ranges) {
          for (const Range& bottom_range : bottom_ranges) {
            Range intersection = { std::max(top_range.min, bottom_range.min), std::min(top_range.max, bottom_range.max) };

#if 0
            // If the ranges intersect, there could be a filled rect.
            if (intersection.min < intersection.max) {
              // To be valid, a range shouldn't have any points inside.

              ActivePoint active_left = { intersection.max, -1 };
              ActivePoint active_right = { intersection.min, -1 };

              points.insert(points.end(), { intersection.min, intersection.max });

              std::sort(points.begin(), points.end());

              for (int h = 0; h < points.size(); h++) {
                if (points[h] > intersection.min) break;

                active_left = { points[h], h };
              }

              for (int h = points.size() - 1; h >= 0; h--) {
                if (points[h] < intersection.max) break;

                active_right = { points[h], h };
              }

              bool valid = std::abs(active_left.index - active_right.index) == 1;

              // if (std::abs(active_left.index - active_right.index) == 1) {

              // }

              vec4 color = valid ? vec4(0.1f, 0.9f, 0.1f, 1.0f) : vec4(0.9f, 0.1f, 0.1f, 1.0f);

              get()->m_line_instances.instances.push_back({ vec2(intersection.min, lines[i].line_y), vec2(intersection.max, lines[i].line_y), 2.0f, color });
              get()->m_line_instances.instances.push_back({ vec2(intersection.min, lines[i + 1].line_y), vec2(intersection.max, lines[i + 1].line_y), 2.0f, color });
              get()->m_line_instances.instances.push_back({ vec2(intersection.min, lines[i].line_y), vec2(intersection.min, lines[i + 1].line_y), 2.0f, color });
              get()->m_line_instances.instances.push_back({ vec2(intersection.max, lines[i].line_y), vec2(intersection.max, lines[i + 1].line_y), 2.0f, color });
              get()->m_line_instances.instances.push_back({ vec2(intersection.min, lines[i].line_y), vec2(intersection.max, lines[i + 1].line_y), 2.0f, color });
              get()->m_line_instances.instances.push_back({ vec2(intersection.max, lines[i].line_y), vec2(intersection.min, lines[i + 1].line_y), 2.0f, color });
            }
#endif
#if 1
            // If the ranges intersect, there could be a filled rect.
            if (intersection.min < intersection.max) {
              // To check if the intersection is a valid rect, we need to check if there aren't any intersections between the vertical sides and the curves within the band.
              math::Vec2<bool> valid = { true, true };

              for (uint16_t j = data.bands[bands_start_index + i * 2]; j < data.bands[bands_start_index + i * 2] + data.bands[bands_start_index + i * 2 + 1]; j++) {
                const uint16_t curve_index = data.bands[bands_start_index + j];

                const vec2 p0 = data.curves[curve_index * 4];
                const vec2 p1 = data.curves[curve_index * 4 + 1];
                const vec2 p2 = data.curves[curve_index * 4 + 2];
                const vec2 p3 = data.curves[curve_index * 4 + 3];

                for (float x : std::array<float, 2>{ intersection.min, intersection.max }) {
                  if ((p0.x <= x && p3.x <= x) || (p0.x >= x && p3.x >= x)) {
                    continue;
                  }

                  geom::cubic_bezier curve = geom::cubic_bezier(p0, p1, p2, p3);
                  const auto& [a, b, c, d] = curve.coefficients();

                  float t = -(p0.x - x) / (p3.x - p0.x);

                  for (int i = 0; i < 3; i++) {
                    float t_sq = t * t;
                    float f = a.x * t_sq * t + b.x * t_sq + c.x * t + d.x - x;
                    float f_prime = 3.0f * a.x * t_sq + 2.0f * b.x * t + c.x;
                    float f_second = 6.0f * a.x * t + 2.0f * b.x;
                    float f_third = 6.0f * a.x;

                    t = t - 3.0f * f * (3.0f * f_prime * f_prime - f * f_second) /
                      (9.0f * f_prime * f_prime * f_prime - 9.0f * f * f_prime * f_second + f * f * f_third);
                  }

                  if (t > 0.0f && t < 1.0f) {
                    vec2 p = curve.sample(t);
                    get()->m_circle_instances.instances.push_back({ p, get()->m_ui_options.handle_radius, vec4(0.1f, 0.9f, 0.1f, 1.0f) });

                    if (p.y > lines[i].line_y + math::geometric_epsilon<float> && p.y < lines[i + 1].line_y - math::geometric_epsilon<float>) {
                      // TODO: this check is awful
                      valid[x == intersection.min ? 0 : 1] = false;
                    }
                  }
                }
              }

#if 1
              if (valid.x != valid.y) {
                // One of the sides is invalid, so we can't draw a rect yet.
                std::vector<float> points;

                for (uint16_t j = data.bands[bands_start_index + i * 2]; j < data.bands[bands_start_index + i * 2] + data.bands[bands_start_index + i * 2 + 1]; j++) {
                  const uint16_t curve_index = data.bands[bands_start_index + j];

                  const vec2 p0 = data.curves[curve_index * 4];
                  const vec2 p1 = data.curves[curve_index * 4 + 1];
                  const vec2 p2 = data.curves[curve_index * 4 + 2];
                  const vec2 p3 = data.curves[curve_index * 4 + 3];

                  if (p0.y > lines[i].line_y && p0.y < lines[i + 1].line_y) {
                    points.push_back(p0.x);
                  }

                  if (p3.y > lines[i].line_y && p3.y < lines[i + 1].line_y) {
                    points.push_back(p3.x);
                  }
                }

                std::sort(points.begin(), points.end());

                for (float point : points) {
                  get()->m_circle_instances.instances.push_back({ vec2(point, lines[i].line_y), get()->m_ui_options.handle_radius, vec4(0.9f, 0.1f, 0.1f, 1.0f) });
                }

                // Starting from the other side, we need to find the first point.
                if (valid.x) {
                  for (auto& it = points.begin(); it != points.end(); it++) {
                    if (*it > intersection.min) {
                      if (*it < intersection.max) {
                        intersection.max = *it;
                        valid.y = true;
                      }

                      break;
                    }
                  }
                } else {
                  for (auto& it = points.rbegin(); it != points.rend(); it++) {
                    if (*it < intersection.min) {
                      if (*it > intersection.max) {
                        intersection.min = *it;
                        valid.x = true;
                      }
                    }
                  }
                }
              }
#endif

              vec4 color = (valid.x && valid.y) ? vec4(0.1f, 0.9f, 0.1f, 1.0f) : vec4(0.9f, 0.1f, 0.1f, 1.0f);

              get()->m_line_instances.instances.push_back({ vec2(intersection.min, lines[i].line_y), vec2(intersection.max, lines[i].line_y), 2.0f, color });
              get()->m_line_instances.instances.push_back({ vec2(intersection.min, lines[i + 1].line_y), vec2(intersection.max, lines[i + 1].line_y), 2.0f, color });
              get()->m_line_instances.instances.push_back({ vec2(intersection.min, lines[i].line_y), vec2(intersection.min, lines[i + 1].line_y), 2.0f, color });
              get()->m_line_instances.instances.push_back({ vec2(intersection.max, lines[i].line_y), vec2(intersection.max, lines[i + 1].line_y), 2.0f, color });
              get()->m_line_instances.instances.push_back({ vec2(intersection.min, lines[i].line_y), vec2(intersection.max, lines[i + 1].line_y), 2.0f, color });
              get()->m_line_instances.instances.push_back({ vec2(intersection.max, lines[i].line_y), vec2(intersection.min, lines[i + 1].line_y), 2.0f, color });
            }
#endif

          }
        }
            // get()->m_line_instances.instances.push_back({ vec2(top_range.min, lines[i].line_y), vec2(bottom_range.min, lines[i + 1].line_y), 1.0f, vec4(0.9f, 0.1f, 0.1f, 1.0f) });
            // get()->m_line_instances.instances.push_back({ vec2(top_range.max, lines[i].line_y), vec2(bottom_range.max, lines[i + 1].line_y), 1.0f, vec4(0.9f, 0.1f, 0.1f, 1.0f) });

#if 0
      // std::sort(lines[i + 1].intersections.begin(), lines[i + 1].intersections.end());

        const auto& top = lines[i].intersections;
        const auto& bottom = lines[i + 1].intersections;

        if (top.empty() || bottom.empty()) {
          continue;
        }

        std::vector<Pivot> pivots;

        for (float x : top) {
          pivots.push_back({ x, PivotType::Top });
        }

        for (float x : bottom) {
          pivots.push_back({ x, PivotType::Bottom });
        }

        for (int j = 0; j < h_indices.size(); j++) {
          const vec2 p0 = path[h_indices[j] * 3];
          const vec2 p3 = path[h_indices[j] * 3 + 3];

          if (p0.y > lines[i].line_y && p0.y < lines[i + 1].line_y) {
            pivots.push_back({ p0.x, PivotType::Middle });
          }
        }

        std::sort(pivots.begin(), pivots.end(), [](const Pivot& a, const Pivot& b) {
          return a.x < b.x;
          });

        for (const Pivot& pivot : pivots) {
          get()->m_line_instances.instances.push_back({ vec2(pivot.x, lines[i].line_y), vec2(pivot.x, lines[i + 1].line_y), 1.0f, vec4(1.0f, 1.0f, 1.0f, 1.0f) });
        }
#endif
      // ActiveRect active_rect{};
      // bool last_top = pivots.front().top;
      // bool in = false;

      // for (const Pivot& pivot : pivots) {
      //   if (active_rect.active) {
      //     get()->m_line_instances.instances.push_back({ vec2(pivot.x, lines[i].line_y), vec2(pivot.x, lines[i].line_y), 1.0f, vec4(1.0f, 1.0f, 1.0f, 1.0f) });

      //     active_rect.active = false;
      //   }

      //   if (last_top != pivot.top) {
      //     get()->m_line_instances.instances.push_back({ vec2(pivot.x, lines[i].line_y), vec2(pivot.x, lines[i + 1].line_y), 1.0f, vec4(1.0f, 1.0f, 1.0f, 1.0f) });
      //     active_rect.active = true;
      //     active_rect.in_x = pivot.x;
      //   } else {

      //   }

      //   last_top = pivot.top;
      //   active_rect.in_x = pivot.x;
      // }

      // for (float intersection : line.intersections) {
      //   for (float next_intersection : next_line.intersections) {
      //     get()->m_line_instances.instances.push_back({ vec2(intersection, line.line_y), vec2(next_intersection, next_line.line_y), 1.0f, vec4(0.9f, 0.1f, 0.1f, 1.0f) });
      //   }
      // }
      }

      // for (size_t i = 0; i < path.size(); i++) {
      //   get()->m_circle_instances.instances.push_back({ transform * path[i * 3 + 3], get()->m_ui_options.handle_radius / 1.5f, vec4(0.2f, 0.8f, 0.2f, 1.0f) });
      // }
    }
  }

  void Renderer::draw_outline(const geom::quadratic_path& path, const mat2x3& transform, const float tolerance, const Stroke* stroke, const rect* bounding_rect) {
    const float line_width = get()->m_ui_options.line_width;
    const vec4 color = get()->m_ui_options.primary_color_05;

    int count = 0;

    const auto sink_callback = [&](const vec2 p0, const vec2 p1) {
      get()->m_line_instances.instances.push_back({ p0, p1, line_width, color });
      count++;
      };

    geom::path_builder(path, transform, bounding_rect).flatten(get()->m_viewport.visible(), tolerance, sink_callback);
  }

  void Renderer::draw_outline(const geom::Path<float, std::enable_if<true>>& path, const mat2x3& transform, const float tolerance, const Stroke* stroke, const rect* bounding_rect) {
    // TODO: fix

    if (path.size() == 1) {
      const vec2 p0 = transform * path.at(0);
      const vec2 p1 = transform * path.at(1);
      const vec2 p2 = transform * path.at(2);
      const vec2 p3 = transform * path.at(3);

      vec2 a = -p0 + 3.0 * p1 - 3.0 * p2 + p3;
      vec2 b = 3.0 * p0 - 6.0 * p1 + 3.0 * p2;
      vec2 c = -3.0 * p0 + 3.0 * p1;
      vec2 p;

      float conc = std::max(std::hypot(b.x, b.y), std::hypot(a.x + b.x, a.y + b.y));
      float dt = std::sqrtf((std::sqrt(8.0) * tolerance) / conc);
      float t = dt;

      vec2 last = p0;

      while (t < 1.0f) {
        float t_sq = t * t;

        p = a * t_sq * t + b * t_sq + c * t + p0;

        get()->m_line_instances.instances.push_back({ last, p, get()->m_ui_options.line_width, vec4(1.0f, 1.0f, 1.0f, 1.0f) });

        last = p;
        t += dt;
      }

      get()->m_line_instances.instances.push_back({ last, p3, get()->m_ui_options.line_width, vec4(1.0f, 1.0f, 1.0f, 1.0f) });

      return;
    }


    // geometry::PathBuilder(path.to_quadratics(tolerance), transform, bounding_rect).flatten(get()->m_viewport.visible(), tolerance, get()->m_line_instances.instances);
  }

  void Renderer::draw_outline_vertices(const geom::Path<float, std::enable_if<true>>& path, const mat2x3& transform, const std::unordered_set<uint32_t>* selected_vertices, const Stroke* stroke, const rect* bounding_rect) {
    // {
    //   if (path.empty()) {
    //     return;
    //   }

    //   const float radius_safe = 0.5f * 20.0f * (false ? 10.0f : 1.0f);
    //   rect bounds = bounding_rect ? *bounding_rect : path.approx_bounding_rect();

    //   bounds.min -= radius_safe;
    //   bounds.max += radius_safe;

    //   auto transformation = math::decompose(transform);
    //   float scale = std::max(transformation.scale.x, transformation.scale.y);

    //   // TODO: iterate quadraticpaths returned
    //   const geom::StrokeOutline stroked_path = geom::path_builder(path.to_quadratic_path(), transform, nullptr).stroke(
    //     path,
    //     geom::StrokingOptions<float>{ 20.0f, 10.0f, geom::LineCap::Round, geom::LineJoin::Round },
    //     1e-4f / scale
    //   );

    //     // const geometry::PathBuilder::StrokeOutline stroked_path = geometry::PathBuilder(path, transform, bounding_rect).stroke(stroke, 0.5f);
    //   const Fill fill = {
    //     vec4(0.6f, 0.3f, 0.3f, 1.0f),
    //     FillRule::NonZero,
    //     1
    //   };

    //   // TODO: actually render both inner and outer

    //   // TODO: check if is translation only, in that case transform on the GPU
    //   draw(stroked_path.outer, fill, mat2x3::identity(), nullptr);
    //   draw(stroked_path.inner, fill, mat2x3::identity(), nullptr);
    // }

    if (path.vacant()) {
      return;
    }

    const UIOptions& ui_options = get()->m_ui_options;

    InstanceBuffer<LineInstance>& lines = get()->m_line_instances.instances;
    InstanceBuffer<RectInstance>& rects = get()->m_rect_instances.instances;
    // std::vector<vec2>& filled = get()->m_vertex_instances.instances;
    // std::vector<vec2>& white = get()->m_white_vertex_instances.instances;
    InstanceBuffer<CircleInstance>& circles = get()->m_circle_instances.instances;

    uint32_t i = path.points_count() - 1;
    vec2 last_raw = path.at(i);
    vec2 last = transform * last_raw;

    if (!path.closed()) {
      rects.push_back({ last, ui_options.vertex_size, ui_options.primary_color });
      // filled.push_back(last);

      if (selected_vertices && selected_vertices->find(i) == selected_vertices->end()) {
        rects.push_back({ last, ui_options.vertex_inner_size, vec4::identity() });
        // white.push_back(last);
      }

      const vec2 out_handle = path.out_handle();

      if (out_handle != last_raw) {
        const vec2 h = transform * out_handle;

        circles.push_back({ h, ui_options.handle_radius, ui_options.primary_color });
        lines.push_back({ h, last, ui_options.line_width, ui_options.primary_color_05 });
      }
    }

    path.for_each_reversed(
      [&](const vec2 p0) {
        const vec2 p = transform * p0;

        rects.push_back({ p, ui_options.vertex_size, ui_options.primary_color });
        // filled.push_back(p);

        if (selected_vertices && selected_vertices->find(i) == selected_vertices->end()) {
          rects.push_back({ p, ui_options.vertex_inner_size, vec4::identity() });
          // white.push_back(p);
        }

        if (!path.closed()) {
          vec2 in_handle = path.in_handle();

          if (in_handle != p0) {
            const vec2 h = transform * in_handle;

            circles.push_back({ h, ui_options.handle_radius, ui_options.primary_color });
            lines.push_back({ h, p, ui_options.line_width, ui_options.primary_color_05 });
          }
        }

        last = p;
        i -= 1;
      },
      [&](const vec2 p0, const vec2 p1) {
        const vec2 p = transform * p0;

        rects.push_back({ p, ui_options.vertex_size, ui_options.primary_color });
        // filled.push_back(p);

        if (selected_vertices && selected_vertices->find(i - 1) == selected_vertices->end()) {
          rects.push_back({ p, ui_options.vertex_inner_size, vec4::identity() });
          // white.push_back(p);
        }

        last = p;
        i -= 1;
      },
      [&](const vec2 p0, const vec2 p1, const vec2 p2) {
        const vec2 p = transform * p0;

        rects.push_back({ p, ui_options.vertex_size, ui_options.primary_color });
        // filled.push_back(p);

        if (selected_vertices && selected_vertices->find(i - 2) == selected_vertices->end()) {
          rects.push_back({ p, ui_options.vertex_inner_size, vec4::identity() });
          // white.push_back(p);
        }

        if (p1 != p0 && p2 != p0) {
          const vec2 h = transform * p1;

          circles.push_back({ h, ui_options.handle_radius, ui_options.primary_color });
          lines.push_back({ h, p, ui_options.line_width, ui_options.primary_color_05 });
          lines.push_back({ h, last, ui_options.line_width, ui_options.primary_color_05 });
        }

        last = p;
        i -= 2;
      },
      [&](const vec2 p0, const vec2 p1, const vec2 p2, const vec2 p3) {
        const vec2 p = transform * p0;

        rects.push_back({ p, ui_options.vertex_size, ui_options.primary_color });
        // filled.push_back(p);

        if (selected_vertices && selected_vertices->find(i - 3) == selected_vertices->end()) {
          rects.push_back({ p, ui_options.vertex_inner_size, vec4::identity() });
          // white.push_back(p);
        }

        if (p2 != p3) {
          const vec2 h = transform * p2;

          circles.push_back({ h, ui_options.handle_radius, ui_options.primary_color });
          lines.push_back({ h, last, ui_options.line_width, ui_options.primary_color_05 });
        }

        if (p1 != p0) {
          const vec2 h = transform * p1;

          circles.push_back({ h, ui_options.handle_radius, ui_options.primary_color });
          lines.push_back({ h, p, ui_options.line_width, ui_options.primary_color_05 });
        }

        last = p;
        i -= 3;
      }
    );
  }

  void Renderer::draw_rect(const rect& rect, const std::optional<vec4> color) {
    get()->m_rect_instances.instances.push_back({ rect.center(), rect.size(), color.value_or(get()->m_ui_options.primary_color) });
  }

  void Renderer::draw_rect(const vec2 center, const vec2 size, const std::optional<vec4> color) {
    get()->m_rect_instances.instances.push_back({ center, size, color.value_or(get()->m_ui_options.primary_color) });
  }

#ifdef GK_DEBUG
  void Renderer::draw_debug_overlays(const geom::cubic_bezier& cubic) {
    vec2 cursor = vec2::zero();

    const float padding = 10.0f;
    const float resolution = 100;

    const vec2 xy_graph_size = vec2(250.0f, -250.0f);
    const vec2 graph_safe_size = xy_graph_size + 2.0f * vec2(-padding, padding);

    for (int j = 0; j < 2; j++) {
      const vec2 graph_position = vec2(get()->m_viewport.size.x - xy_graph_size.x, -xy_graph_size.y) + cursor;

      get()->m_rect_instances.instances.push_back({
        get()->m_viewport.project(graph_position + xy_graph_size / 2.0f),
        xy_graph_size / get()->m_viewport.zoom,
        vec4{ 0.0f, 0.0f, 0.0f, 0.5f }
      });

      const vec2 graph_safe_position = graph_position + vec2(padding, -padding);

      const rect curve_bounds = cubic.bounding_rect();
      const vec2 bounds_size = curve_bounds.size();

      for (int i = 0; i < static_cast<int>(resolution); i++) {
        const float t0 = static_cast<float>(i) / resolution;
        const float t1 = static_cast<float>(i + 1) / resolution;

        const vec2 p0_norm = (cubic.sample(t0) - curve_bounds.min) / bounds_size;
        const vec2 p1_norm = (cubic.sample(t1) - curve_bounds.min) / bounds_size;

        const vec2 p0 = get()->m_viewport.project(vec2(t0, p0_norm[j]) * graph_safe_size + graph_safe_position);
        const vec2 p1 = get()->m_viewport.project(vec2(t1, p1_norm[j]) * graph_safe_size + graph_safe_position);

        get()->m_line_instances.instances.push_back({ p0, p1, get()->m_ui_options.line_width, vec4(1.0f, 1.0f, 1.0f, 1.0f) });
      }

      cursor += vec2(0.0f, -xy_graph_size.y + padding / 4.0f);
    }

    cursor = vec2::zero();

    // const std::vector<geom::quadratic_bezier> quads_d = geom::cubic_to_quadratics(cubic);
    // const std::vector<std::pair<geom::quadratic_bezier, vec2>> quads_d = geom::cubic_to_quadratics_with_intervals(cubic);

    // for (const auto& quad : quads_d) {
    //   for (int i = 0; i < static_cast<int>(resolution); i++) {
    //     const float t0 = static_cast<float>(i) / resolution;
    //     const float t1 = static_cast<float>(i + 1) / resolution;

    //     const vec2 p0 = quad.sample(t0);
    //     const vec2 p1 = quad.sample(t1);

    //     get()->m_line_instances.instances.push_back(p0, p1, get()->m_ui_options.line_width * 2.0f, get()->m_ui_options.primary_color_05);
    //   }
    // }

    const std::vector<std::pair<geom::quadratic_bezier, vec2>> quads = geom::cubic_to_quadratics_with_intervals(cubic);

    for (int j = 0; j < 2; j++) {
      const vec2 graph_position = vec2(get()->m_viewport.size.x - xy_graph_size.x, -xy_graph_size.y) + cursor;
      const vec2 graph_safe_position = graph_position + vec2(padding, -padding);

      const rect curve_bounds = cubic.bounding_rect();
      const vec2 bounds_size = curve_bounds.size();

      for (const auto& [quad, interval] : quads) {
        for (int i = static_cast<int>(resolution * interval[0]); i < static_cast<int>(resolution * interval[1]); i++) {
          const float t0 = static_cast<float>(i) / resolution;
          const float t1 = static_cast<float>(i + 1) / resolution;

          const vec2 p0_norm = (quad.sample(t0) - curve_bounds.min) / bounds_size;
          const vec2 p1_norm = (quad.sample(t1) - curve_bounds.min) / bounds_size;

          const vec2 p0 = get()->m_viewport.project(vec2(t0, p0_norm[j]) * graph_safe_size + graph_safe_position);
          const vec2 p1 = get()->m_viewport.project(vec2(t1, p1_norm[j]) * graph_safe_size + graph_safe_position);

          get()->m_line_instances.instances.push_back({ p0, p1, get()->m_ui_options.line_width * 2.0f, vec4(0.8f, 0.2f, 0.2f, 1.0f) });
        }

        const vec2 p0_norm = (quad.sample(interval[0]) - curve_bounds.min) / bounds_size;
        const vec2 p0 = get()->m_viewport.project(vec2(interval[0], p0_norm[j]) * graph_safe_size + graph_safe_position);

        get()->m_circle_instances.instances.push_back({ p0, get()->m_ui_options.handle_radius, vec4(1.0f, 1.0f, 1.0f, 1.0f) });
      }

      const vec2 p1_norm = (quads.back().first.sample(1.0f) - curve_bounds.min) / bounds_size;
      const vec2 p1 = get()->m_viewport.project(vec2(1.0f, p1_norm[j]) * graph_safe_size + graph_safe_position);

      get()->m_circle_instances.instances.push_back({ p1, get()->m_ui_options.handle_radius, vec4(1.0f, 1.0f, 1.0f, 1.0f) });

      cursor += vec2(0.0f, -xy_graph_size.y + padding / 4.0f);
    }
  }
#endif

  Renderer::Renderer() :
    m_path_instances(GK_LARGE_BUFFER_SIZE),
    m_line_instances(GK_LARGE_BUFFER_SIZE),
    m_circle_instances(GK_BUFFER_SIZE),
    m_rect_instances(GK_BUFFER_SIZE) {}
    // m_handle_instances(GK_BUFFER_SIZE),
    // m_vertex_instances(GK_BUFFER_SIZE),
    // m_white_vertex_instances(GK_BUFFER_SIZE) {}

  void Renderer::flush_meshes() {
    const GPU::Texture& curves_texture = GPU::Memory::Allocator::get_texture(m_path_instances.curves_texture_id);
    const GPU::Texture& bands_texture = GPU::Memory::Allocator::get_texture(m_path_instances.bands_texture_id);

    // TODO: should preallocate the texture
    if (m_path_instances.curves.size() < 512 * 512 * 2) {
      m_path_instances.curves.resize(512 * 512 * 2);
    }

    std::vector<uint16_t> bands;

    bands.insert(bands.end(), m_path_instances.bands.begin(), m_path_instances.bands.end());

    // size_t bands_data_start = bands.size();

    // bands.insert(bands.end(), m_path_instances.bands_data.begin(), m_path_instances.bands_data.end());

    if (bands.size() < 512 * 512) {
      bands.resize(512 * 512);
    }

    GPU::Device::upload_to_texture(
      curves_texture,
      {
        { 0.0f, 0.0f },
        { 512.0f, 512.0f },
        // { static_cast<float>((m_path_instances.curves.size() / 2) % 512), static_cast<float>((m_path_instances.curves.size() / 2) / 512) }
      },
      m_path_instances.curves.data()
      );

    GPU::Device::upload_to_texture(
      bands_texture,
      {
        { 0.0f, 0.0f },
        { 512.0f, 512.0f },
      },
      bands.data()
      );

    flush<PathInstance, GPU::PathProgram, GPU::PathVertexArray>(
      m_path_instances,
      m_programs.path_program,
      {
        {
          { m_programs.path_program.curves_texture, curves_texture },
          { m_programs.path_program.bands_texture, bands_texture }
        },
        {
          { m_programs.path_program.vp_uniform, m_vp_matrix },
          { m_programs.path_program.viewport_size_uniform, m_viewport.size },
          { m_programs.path_program.min_samples_uniform, 4 },
          { m_programs.path_program.max_samples_uniform, 16 }
        },
        m_viewport.size
      }
    );

    m_path_instances.clear();

    flush<LineInstance, GPU::LineProgram, GPU::LineVertexArray>(
      m_line_instances,
      m_programs.line_program,
      {
        {},
        {
          { m_programs.line_program.vp_uniform, m_vp_matrix },
          // { m_programs.line_program.line_width_uniform, static_cast<float>((2.0 * m_viewport.dpr) / m_viewport.zoom) },
          { m_programs.line_program.zoom_uniform, static_cast<float>(m_viewport.zoom) }
        },
        m_viewport.size
      }
    );

    flush<RectInstance, GPU::RectProgram, GPU::RectVertexArray>(
      m_rect_instances,
      m_programs.rect_program,
      {
        {},
        {
          { m_programs.rect_program.vp_uniform, m_vp_matrix }
          // { m_programs.rect_program.color_uniform, vec4{ 0.22f, 0.76f, 0.95f, 1.0f } },
          // { m_programs.rect_program.size_uniform, static_cast<float>(std::round(5.0 * m_viewport.dpr) / m_viewport.zoom) }
        },
        m_viewport.size
      }
    );

    // flush<vec2, GPU::RectProgram, GPU::RectVertexArray>(
    //   m_white_vertex_instances,
    //   m_programs.rect_program,
    //   {
    //     {},
    //     {
    //       { m_programs.rect_program.vp_uniform, m_vp_matrix }
    //       // { m_programs.rect_program.color_uniform, vec4{ 1.0f, 1.0f, 1.0f, 1.0f } },
    //       // { m_programs.rect_program.size_uniform, static_cast<float>(std::round(3.0 * m_viewport.dpr) / m_viewport.zoom) }
    //     },
    //     m_viewport.size
    //   }
    // );

    flush<CircleInstance, GPU::CircleProgram, GPU::CircleVertexArray>(
      m_circle_instances,
      m_programs.circle_program,
      {
        {},
        {
          { m_programs.circle_program.vp_uniform, m_vp_matrix },
          { m_programs.circle_program.zoom_uniform, static_cast<float>(m_viewport.zoom) }
        },
        m_viewport.size
      }
    );
  }

}
