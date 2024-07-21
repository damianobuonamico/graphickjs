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

#include "../geom/intersections.h"
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
  static std::vector<uvec2> quad_vertices(const uvec2 min, const uvec2 max) {
    return {
      min,
      uvec2{ max.x, min.y },
      max,
      max,
      uvec2{ min.x, max.y },
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
  static void flush(InstancedData<T>& data, const S& program, const FlushData flush_data, const std::optional<GPU::DepthState> depth_state = std::nullopt) {
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
          depth_state,
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

    get()->m_path_instances.vertices = quad_vertices({ 0, 0 }, { 1, 1 });
    get()->m_boundary_spans.vertices = quad_vertices({ 0, 0 }, { 1, 1 });
    get()->m_filled_spans.vertices = quad_vertices({ 0, 0 }, { 1, 1 });
    get()->m_line_instances.vertices = quad_vertices({ 0, 0 }, { 1, 1 });
    get()->m_circle_instances.vertices = quad_vertices({ 0, 0 }, { 1, 1 });
    get()->m_rect_instances.vertices = quad_vertices({ 0, 0 }, { 1, 1 });

    init_instanced(get()->m_path_instances);
    init_instanced(get()->m_boundary_spans, get()->m_path_instances.vertex_buffer_id);
    init_instanced(get()->m_filled_spans, get()->m_path_instances.vertex_buffer_id);
    init_instanced(get()->m_line_instances, get()->m_path_instances.vertex_buffer_id);
    init_instanced(get()->m_circle_instances, get()->m_path_instances.vertex_buffer_id);
    init_instanced(get()->m_rect_instances, get()->m_path_instances.vertex_buffer_id);

    if (get()->m_path_instances.curves_texture_id != uuid::null) {
      GPU::Memory::Allocator::free_texture(get()->m_path_instances.curves_texture_id);
    }
    if (get()->m_path_instances.bands_texture_id != uuid::null) {
      GPU::Memory::Allocator::free_texture(get()->m_path_instances.bands_texture_id);
    }

    if (get()->m_boundary_spans.curves_texture_id != uuid::null) {
      GPU::Memory::Allocator::free_texture(get()->m_boundary_spans.curves_texture_id);
    }

    get()->m_path_instances.curves_texture_id = GPU::Memory::Allocator::allocate_texture({ 512, 512 }, GPU::TextureFormat::RGBA32F, "PathCurves");
    get()->m_path_instances.bands_texture_id = GPU::Memory::Allocator::allocate_texture({ 512, 512 }, GPU::TextureFormat::R16UI, "PathBands");
    get()->m_boundary_spans.curves_texture_id = GPU::Memory::Allocator::allocate_texture({ 512, 512 }, GPU::TextureFormat::RGBA32F, "Curves");
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

    get()->m_transform_vectors.clear();
    get()->m_transform_vectors.reserve(get()->m_max_transform_vectors);
    get()->m_transform_vectors.push_back(vec4(1.0f, 0.0f, 0.0f, 0.0f));
    get()->m_transform_vectors.push_back(vec4(0.0f, 1.0f, 0.0f, 0.0f));

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

    std::vector<vec2> min(len);
    std::vector<vec2> max(len);

    for (size_t i = 0; i < path.size(); i++) {
      const vec2 p0 = path[i * 2];
      const vec2 p1 = path[i * 2 + 1];
      const vec2 p2 = path[i * 2 + 2];

      min[i] = vec2{
        std::min({ p0.x, p1.x, p2.x }),
        std::min({ p0.y, p1.y, p2.y }),
      };

      max[i] = vec2{
        std::max({ p0.x, p1.x, p2.x }),
        std::max({ p0.y, p1.y, p2.y }),
      };
    }

    if (!closed) {
      const vec2 p0 = path.points.back();
      const vec2 p1 = path[0];

      min.back() = vec2{
        std::min(p0.x, p1.x),
        std::min(p0.y, p1.y)
      };

      max.back() = vec2{
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
      return max[a].x > max[b].x;
    });

    std::sort(v_indices.begin(), v_indices.end(), [&](const uint16_t a, const uint16_t b) {
      return max[a].y > max[b].y;
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
        const float min_y = min[h_indices[j]].y;
        const float max_y = max[h_indices[j]].y;

        if (min_y == max_y || min_y > band_max.y + math::geometric_epsilon<float> || max_y < band_min.y - math::geometric_epsilon<float>) {
          continue;
        }

        data.bands.push_back(h_indices[j]);
      }

      const size_t band_end = data.bands.size();

      /* Each band header is an offset from this path's bands data start and the number of curves in the band. */

      data.bands[bands_start_index + i * 2] = static_cast<uint16_t>(band_start - bands_start_index);
      data.bands[bands_start_index + i * 2 + 1] = static_cast<uint16_t>(band_end - band_start);

      band_min += band_delta.y;
      band_max += band_delta.y;
    }

    /* Determine which curves are in each vertical band. */

    for (int i = 0; i < vertical_bands; i++) {
      const size_t band_start = data.bands.size();

      for (uint16_t j = 0; j < v_indices.size(); j++) {
        float min_x = min[v_indices[j]].x;
        float max_x = max[v_indices[j]].x;

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

  void Renderer::draw(geom::cubic_path&& path, const Fill& fill, const mat2x3& transform, const rect* bounding_rect) {
    GK_TOTAL("Renderer::draw(fill, cubic)");

    if (path.empty()) {
      return;
    }

    const rect bounds = bounding_rect ? *bounding_rect : path.approx_bounding_rect();
    const vec2 bounds_size = bounds.size();

#if 0
    PathInstancedData& data = get()->m_path_instances;

    /* Starting indices for this path. */

    const size_t curves_start_index = data.curves.size() / 2;
    const size_t bands_start_index = data.bands.size();

    /* Copy the curves, close the path and cache min_max values. */

    const bool closed = path.closed();
    const size_t len = closed ? path.size() : (path.size() + 1);

    std::vector<vec2> min(len);
    std::vector<vec2> max(len);

    data.curves.reserve(path.size() * 4);

    for (size_t i = 0; i < path.size(); i++) {
      const vec2 p0 = path[i * 3];
      const vec2 p1 = path[i * 3 + 1];
      const vec2 p2 = path[i * 3 + 2];
      const vec2 p3 = path[i * 3 + 3];

      data.curves.insert(data.curves.end(), { p0, p1, p2, p3 });

      min[i] = math::min(p0, p3);
      max[i] = math::max(p0, p3);
    }

    if (!closed) {
      const vec2 p0 = path.points.back();
      const vec2 p3 = path[0];

      data.curves.insert(data.curves.end(), { p0, p0, p3, p3 });

      min.back() = math::min(p0, p3);
      max.back() = math::max(p0, p3);
    }

    /* Bands count is always between 1 and 16, based on the number of segments. */

    const float max_size = std::max(bounds_size.x, bounds_size.y);

    const uint8_t horizontal_bands = static_cast<uint8_t>(std::clamp(len * bounds_size.y / max_size / 2.0f, 3.0f, 16.0f));

    /* Sort curves by descending max x. */

    std::vector<uint16_t> h_indices(len);

    std::iota(h_indices.begin(), h_indices.end(), 0);
    std::sort(h_indices.begin(), h_indices.end(), [&](const uint16_t a, const uint16_t b) {
      return max[a].x > max[b].x;
    });

    /* Calculate band metrics. */

    const float band_delta = bounds_size.y / horizontal_bands;

    float band_min = bounds.min.y;
    float band_max = bounds.min.y + band_delta;

    /* Preallocate horizontal and vertical bands header. */

    data.bands.resize(data.bands.size() + horizontal_bands * 2, 0);

    /* Determine which curves are in each horizontal band. */

    for (uint8_t i = 0; i < horizontal_bands; i++) {
      const size_t band_start = data.bands.size();

      for (uint16_t j = 0; j < h_indices.size(); j++) {
        const float min_y = min[h_indices[j]].y;
        const float max_y = max[h_indices[j]].y;

        if (min_y == max_y || min_y > band_max || max_y < band_min) {
          continue;
        }

        data.bands.push_back(h_indices[j]);

        const vec2 p0 = data.curves[curves_start_index + h_indices[j] * 3];
        const vec2 p1 = data.curves[curves_start_index + h_indices[j] * 3 + 1];
        const vec2 p2 = data.curves[curves_start_index + h_indices[j] * 3 + 2];
        const vec2 p3 = data.curves[curves_start_index + h_indices[j] * 3 + 3];

        const float y = band_min;
        const bool is_downwards = p0.y > y || p3.y < y;

        if (
         (is_downwards && ((p0.y < y && p3.y <= y) || (p0.y > y && p3.y >= y))) ||
         (!is_downwards && ((p0.y <= y && p3.y < y) || (p0.y >= y && p3.y > y)))
        ) {
          continue;
        }
      }

      /* Each band header is an offset from this path's bands data start and the number of curves in the band. */

      const size_t band_end = data.bands.size();

      data.bands[bands_start_index + i * 2] = static_cast<uint16_t>(band_start - bands_start_index);
      data.bands[bands_start_index + i * 2 + 1] = static_cast<uint16_t>(band_end - band_start);

      band_min += band_delta;
      band_max += band_delta;
    }

    /* Push instance. */

    data.instances.push_back({
      transform, bounds.min, bounds_size, fill.color,
      curves_start_index, bands_start_index,
      horizontal_bands, 0,
      false, fill.rule == FillRule::EvenOdd
    });
#endif

    /* Culling attempt. */

    // for (int i = 1; i < horizontal_bands; i++) {
    //   const float y = bounds.min.y + i * band_delta;

    //   get()->m_line_instances.instances.push_back({ transform * vec2(bounds.min.x, y), transform * vec2(bounds.max.x, y), 1.0f, vec4(1.0f, 1.0f, 1.0f, 1.0f) });
    // }

    if (!path.closed()) {
      path.line_to(path.points.front());
    }

    struct Intersection {
      float x;
      bool downwards;
    };

    struct BoundarySpan {
      float min;
      float max;

      int winding = 0;

      std::unordered_set<uint16_t> indices;

      BoundarySpan(float min, float max, std::unordered_set<uint16_t>&& indices) : min(min), max(max), indices(std::move(indices)) {}
    };

    struct FilledSpan {
      float min;
      float max;
    };

    struct Band {
      float top_y;
      float bottom_y;

      std::vector<BoundarySpan> boundary_spans;
      std::vector<FilledSpan> filled_spans;

      void push_curve(float min, float max, const uint16_t index) {
        if (boundary_spans.empty()) {
          boundary_spans.emplace_back(BoundarySpan{ min, max, { index } });
          return;
        }

        int unioned = 0;
        int potential_index = boundary_spans.size();

        for (int i = 0; i < boundary_spans.size(); i++) {
          if (min >= boundary_spans[i].min && max <= boundary_spans[i].max) {
            boundary_spans[i].indices.insert(index);
            return;
          }

          vec2 intersection = { std::max(min, boundary_spans[i].min), std::min(max, boundary_spans[i].max) };

          // TODO: fix span joining based on current transform * zoom

          /* If there is an intersection, we can perform an union. */
          if (intersection.x <= intersection.y + 0.1f) {
            boundary_spans[i].indices.insert(index);
            boundary_spans[i].min = std::min(boundary_spans[i].min, min);
            boundary_spans[i].max = std::max(boundary_spans[i].max, max);

            unioned++;
          } else if (min < boundary_spans[i].min) {
            potential_index = std::min(potential_index, i);
          }
        }

        if (unioned == 0) {
          boundary_spans.insert(boundary_spans.begin() + potential_index, BoundarySpan{ min, max, { index } });
          return;
        }

        for (BoundarySpan& span1 : boundary_spans) {
          if (span1.indices.empty()) continue;

          for (BoundarySpan& span2 : boundary_spans) {
            if (&span1 == &span2 || span2.indices.empty()) {
              continue;
            }

            vec2 intersection = { std::max(span1.min, span2.min), std::min(span1.max, span2.max) };

            /* If there is an intersection, we can perform an union. */
            if (intersection.x <= intersection.y) {
              span1.min = std::min(span1.min, span2.min);
              span1.max = std::max(span1.max, span2.max);

              span1.indices.insert(span2.indices.begin(), span2.indices.end());

              span2.indices.clear();
            }
          }
        }

        boundary_spans.erase(std::remove_if(
          boundary_spans.begin(), boundary_spans.end(),
          [](const BoundarySpan& span) {
            return span.indices.empty();
          }
        ), boundary_spans.end());
      }
    };

    uint32_t transform_index = 0;

    if (math::not_identity(transform)) {
      // TODO: check if there is space in the transform vector
      transform_index = static_cast<uint32_t>(get()->m_transform_vectors.size() / 2);

      get()->m_transform_vectors.push_back(vec4(transform[0][0], transform[0][1], transform[0][2], 0.0f));
      get()->m_transform_vectors.push_back(vec4(transform[1][0], transform[1][1], transform[1][2], 0.0f));
    }

    BoundarySpanInstancedData& data = get()->m_boundary_spans;

    const size_t curves_start_index = data.curves.size() / 2;

    const float max_size = std::max(bounds_size.x, bounds_size.y);
    const uint8_t horizontal_bands = static_cast<uint8_t>(std::clamp(path.size() * bounds_size.y / max_size / 2.0f, 3.0f, 16.0f));

    const float band_delta = bounds_size.y / horizontal_bands;

    std::vector<Band> bands(horizontal_bands);
    std::vector<std::vector<Intersection>> band_bottom_intersections(horizontal_bands);

    for (int i = 0; i < bands.size(); i++) {
      bands[i].top_y = bounds.min.y + i * band_delta;
      bands[i].bottom_y = bounds.min.y + (i + 1) * band_delta;
    }

    for (uint16_t i = 0; i < path.size(); i++) {
      const vec2 p0 = path[i * 3];
      const vec2 p1 = path[i * 3 + 1];
      const vec2 p2 = path[i * 3 + 2];
      const vec2 p3 = path[i * 3 + 3];

      const vec2 min = math::min(p0, p3);
      const vec2 max = math::max(p0, p3);

      /* Being monotonic, it is straightforward to determine which bands the curve intersects. */
      const float start_band_factor = (min.y - bounds.min.y) / band_delta;
      const float end_band_factor = (max.y - bounds.min.y) / band_delta;
      const int start_band = std::clamp(static_cast<int>(start_band_factor), 0, horizontal_bands - 1);
      const int end_band = std::clamp(static_cast<int>(end_band_factor), 0, horizontal_bands - 1);

      // TODO: fix banding boundaries based on current transform * zoom

      /* To avoid rendering issues near endpoints, we also add segments that lie just outside the bands. */
      if (start_band > 0 && std::abs(start_band_factor - start_band) < 0.1f) {
        /* We need to determine which endpoint is closer to the boundary. */
        const vec2 p = p0.y < p3.y ? p0 : p3;

        bands[start_band - 1].push_curve(p.x - math::geometric_epsilon<float>, p.x + math::geometric_epsilon<float>, i);
      }

      if (end_band < horizontal_bands - 1 && std::abs(end_band_factor - 1.0f - end_band) < 0.1f) {
        /* We need to determine which endpoint is closer to the boundary. */
        const vec2 p = p0.y > p3.y ? p0 : p3;

        bands[end_band + 1].push_curve(p.x - math::geometric_epsilon<float>, p.x + math::geometric_epsilon<float>, i);
      }

      if (start_band >= end_band) {
        /* Curve is within one band. */
        bands[start_band].push_curve(min.x, max.x, i);
        continue;
      }

      const auto& [a, b, c, d] = geom::cubic_coefficients(p0, p1, p2, p3);

      const bool b01 = std::abs(p1.x - p0.x) + std::abs(p1.y - p0.y) < math::geometric_epsilon<float>;
      const bool b12 = std::abs(p2.x - p1.x) + std::abs(p2.y - p1.y) < math::geometric_epsilon<float>;
      const bool b23 = std::abs(p3.x - p2.x) + std::abs(p3.y - p2.y) < math::geometric_epsilon<float>;

      const bool linear = (b01 && (b23 || b12)) || (b23 && b12);

      std::optional<float> last_intersection = std::nullopt;

      for (int j = start_band; j <= end_band; j++) {
        const float band_top = bands[j].top_y - math::geometric_epsilon<float>;
        const float band_bottom = bands[j].bottom_y + math::geometric_epsilon<float>;

        float clipped_min = std::numeric_limits<float>::infinity();
        float clipped_max = -std::numeric_limits<float>::infinity();

        /* One of the endpoints could be within the band. */
        if (p0.y >= band_top && p0.y <= band_bottom) {
          clipped_min = p0.x;
          clipped_max = p0.x;
        }

        /* An else if here could cause problems when an endpoint lies on a boundary. */
        if (p3.y >= band_top && p3.y <= band_bottom) {
          clipped_min = p3.x;
          clipped_max = p3.x;
        }

        /* Intersections with the top boundary are cached. */
        if (last_intersection.has_value()) {
          clipped_min = std::min(clipped_min, last_intersection.value());
          clipped_max = std::max(clipped_max, last_intersection.value());
        }

        /* We need to check intersections with the bottom boundary of the band. */
        const float y = bands[j].bottom_y;
        const bool is_downwards = p0.y > y || p3.y < y;

        if (
          (is_downwards && ((p0.y < y && p3.y <= y) || (p0.y > y && p3.y >= y))) ||
          (!is_downwards && ((p0.y <= y && p3.y < y) || (p0.y >= y && p3.y > y)))
        ) {
          bands[j].push_curve(clipped_min, clipped_max, i);
          continue;
        }

        const float t0 = (y - p0.y) / (p3.y - p0.y);

        if (linear) {
          if (t0 >= -math::geometric_epsilon<float> && t0 <= 1.0f + math::geometric_epsilon<float>) {
            const float x = p0.x + t0 * (p3.x - p0.x);

            last_intersection = x;

            clipped_min = std::min(clipped_min, x);
            clipped_max = std::max(clipped_max, x);

            band_bottom_intersections[j].push_back({ x, is_downwards });
          }
        } else {
          const float t = geom::cubic_line_intersect_approx(a.y, b.y, c.y, d.y, y, t0);

          if (t >= -math::geometric_epsilon<float> && t <= 1.0f + math::geometric_epsilon<float>) {
            const float t_sq = t * t;
            const float x = a.x * t_sq * t + b.x * t_sq + c.x * t + d.x;

            last_intersection = x;

            clipped_min = std::min(clipped_min, x);
            clipped_max = std::max(clipped_max, x);

            band_bottom_intersections[j].push_back({ x, is_downwards });
          }

        }

        bands[j].push_curve(clipped_min, clipped_max, i);
      }
    }

    /* The last band cannot have filled spans. */
    band_bottom_intersections.back().clear();

    for (int i = 0; i < bands.size(); i++) {
      // for (const Intersection& inter : band_bottom_intersections[i]) {
      //   get()->m_circle_instances.instances.push_back({ transform * vec2(inter.x, bands[i].bottom_y), get()->m_ui_options.handle_radius, vec4(1.0f, 1.0f, 1.0f, 1.0f) });
      // }

      Band& band = bands[i];

      std::sort(band_bottom_intersections[i].begin(), band_bottom_intersections[i].end(), [&](const Intersection& a, const Intersection& b) {
        return a.x < b.x;
      });

      int winding = 0;
      int winding_k = 0;

      for (int j = 0; j < static_cast<int>(band.boundary_spans.size()) - 1; j++) {
        BoundarySpan& span1 = band.boundary_spans[j];
        BoundarySpan& span2 = band.boundary_spans[j + 1];

        for (winding_k; winding_k < band_bottom_intersections[i].size(); winding_k++) {
          if (band_bottom_intersections[i][winding_k].x > (span1.max + span2.min) * 0.5f) {
            break;
          }

          winding -= int(band_bottom_intersections[i][winding_k].downwards) * 2 - 1;
        }

        span1.winding = winding;

        if (fill.rule == FillRule::NonZero ? (winding != 0) : (winding % 2 != 0)) {
          band.filled_spans.push_back({ span1.max, span2.min });
        }
      }

      for (const BoundarySpan& boundary_span : band.boundary_spans) {
        const size_t curves_start_index = data.curves.size() / 2;

        data.curves.reserve(boundary_span.indices.size() * 4);

        for (const uint16_t index : boundary_span.indices) {
          const vec2 p0 = path[index * 3];
          const vec2 p1 = path[index * 3 + 1];
          const vec2 p2 = path[index * 3 + 2];
          const vec2 p3 = path[index * 3 + 3];

          data.curves.insert(data.curves.end(), { p0, p1, p2, p3 });
        }

        get()->m_boundary_spans.instances.push_back({
          vec2(boundary_span.min, band.top_y),
          vec2(boundary_span.max - boundary_span.min, band_delta),
          fill.color,
          // boundary_span.winding == 0 ? vec4(0.9f, 0.1f, 0.1f, 1.0f) : vec4(0.1f, 0.9f, 0.1f, 1.0f),
          static_cast<int16_t>(boundary_span.winding),
          curves_start_index,
          static_cast<uint16_t>(boundary_span.indices.size()),
          false,
          fill.rule == FillRule::EvenOdd,
          fill.z_index,
          transform_index
        });

        // get()->m_line_instances.instances.push_back({ transform * vec2(boundary_span.min, band.top_y), transform * vec2(boundary_span.max, band.top_y), 2.0f, vec4{ boundary_span.winding < 0 ? 0.9f : 0.1f, 0.1f, 0.1f, 1.0f } });
        // get()->m_line_instances.instances.push_back({ transform * vec2(boundary_span.min, band.bottom_y), transform * vec2(boundary_span.max, band.bottom_y), 2.0f, vec4{ boundary_span.winding < 0 ? 0.9f : 0.1f, 0.1f, 0.1f, 1.0f } });
        // get()->m_line_instances.instances.push_back({ transform * vec2(boundary_span.min, band.top_y), transform * vec2(boundary_span.min, band.bottom_y), 2.0f, vec4{ boundary_span.winding < 0 ? 0.9f : 0.1f, 0.1f, 0.1f, 1.0f } });
        // get()->m_line_instances.instances.push_back({ transform * vec2(boundary_span.max, band.top_y), transform * vec2(boundary_span.max, band.bottom_y), 2.0f, vec4{ boundary_span.winding < 0 ? 0.9f : 0.1f, 0.1f, 0.1f, 1.0f } });
        // get()->m_line_instances.instances.push_back({ transform * vec2(boundary_span.min, band.top_y), transform * vec2(boundary_span.max, band.bottom_y), 2.0f, vec4{ boundary_span.winding < 0 ? 0.9f : 0.1f, 0.1f, 0.1f, 1.0f } });
        // get()->m_line_instances.instances.push_back({ transform * vec2(boundary_span.max, band.top_y), transform * vec2(boundary_span.min, band.bottom_y), 2.0f, vec4{ boundary_span.winding < 0 ? 0.9f : 0.1f, 0.1f, 0.1f, 1.0f } });

      }

      for (const FilledSpan& filled_span : band.filled_spans) {
        get()->m_filled_spans.instances.push_back({
          vec2(filled_span.min, band.top_y),
          vec2(filled_span.max - filled_span.min, band_delta),
          fill.color,
          fill.z_index,
          transform_index
        });
        // get()->m_line_instances.instances.push_back({ transform * vec2(filled_span.min, band.top_y), transform * vec2(filled_span.max, band.top_y), 2.0f, vec4{ 0.1f, 0.9f, 0.1f, 1.0f } });
        // get()->m_line_instances.instances.push_back({ transform * vec2(filled_span.min, band.bottom_y), transform * vec2(filled_span.max, band.bottom_y), 2.0f, vec4{ 0.1f, 0.9f, 0.1f, 1.0f } });
        // get()->m_line_instances.instances.push_back({ transform * vec2(filled_span.min, band.top_y), transform * vec2(filled_span.min, band.bottom_y), 2.0f, vec4{ 0.1f, 0.9f, 0.1f, 1.0f } });
        // get()->m_line_instances.instances.push_back({ transform * vec2(filled_span.max, band.top_y), transform * vec2(filled_span.max, band.bottom_y), 2.0f, vec4{ 0.1f, 0.9f, 0.1f, 1.0f } });
        // get()->m_line_instances.instances.push_back({ transform * vec2(filled_span.min, band.top_y), transform * vec2(filled_span.max, band.bottom_y), 2.0f, vec4{ 0.1f, 0.9f, 0.1f, 1.0f } });
        // get()->m_line_instances.instances.push_back({ transform * vec2(filled_span.max, band.top_y), transform * vec2(filled_span.min, band.bottom_y), 2.0f, vec4{ 0.1f, 0.9f, 0.1f, 1.0f } });
      }
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
    m_boundary_spans(GK_LARGE_BUFFER_SIZE),
    m_line_instances(GK_LARGE_BUFFER_SIZE),
    m_circle_instances(GK_BUFFER_SIZE),
    m_rect_instances(GK_BUFFER_SIZE),
    m_filled_spans(GK_BUFFER_SIZE),
    m_transform_vectors(GPU::Device::max_vertex_uniform_vectors() - 6),
    m_max_transform_vectors(GPU::Device::max_vertex_uniform_vectors() - 6) {}

  void Renderer::flush_meshes() {
    const GPU::Texture& boundary_curves_texture = GPU::Memory::Allocator::get_texture(m_boundary_spans.curves_texture_id);
    const GPU::Texture& curves_texture = GPU::Memory::Allocator::get_texture(m_path_instances.curves_texture_id);
    const GPU::Texture& bands_texture = GPU::Memory::Allocator::get_texture(m_path_instances.bands_texture_id);

    // TODO: should preallocate the texture
    if (m_path_instances.curves.size() < 512 * 512 * 2) {
      m_path_instances.curves.resize(512 * 512 * 2);
    }

    if (m_boundary_spans.curves.size() < 512 * 512 * 2) {
      m_boundary_spans.curves.resize(512 * 512 * 2);
    }

    std::vector<uint16_t> bands;

    bands.insert(bands.end(), m_path_instances.bands.begin(), m_path_instances.bands.end());

    // size_t bands_data_start = bands.size();

    // bands.insert(bands.end(), m_path_instances.bands_data.begin(), m_path_instances.bands_data.end());

    if (bands.size() < 512 * 512) {
      bands.resize(512 * 512);
    }

    GPU::Device::upload_to_texture(
      boundary_curves_texture,
      {
        { 0.0f, 0.0f },
        { 512.0f, 512.0f }
      },
      m_boundary_spans.curves.data()
    );

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

    std::reverse(m_filled_spans.instances.batches.begin(), m_filled_spans.instances.batches.end());

    for (auto& batch : m_filled_spans.instances.batches) {
      std::reverse(batch.begin(), batch.end());
    }

    flush<FilledSpanInstance, GPU::FilledSpanProgram, GPU::FilledSpanVertexArray>(
      m_filled_spans,
      m_programs.filled_span_program,
      {
        {},
        {
          { m_programs.filled_span_program.vp_uniform, m_vp_matrix },
          { m_programs.filled_span_program.models_uniform, m_transform_vectors }
        },
        m_viewport.size
      },
      GPU::DepthState{
        GPU::DepthFunc::Less,
        true
      }
    );

    flush<BoundarySpanInstance, GPU::BoundarySpanProgram, GPU::BoundarySpanVertexArray>(
      m_boundary_spans,
      m_programs.boundary_span_program,
      {
        {
          { m_programs.boundary_span_program.curves_texture, boundary_curves_texture },
        },
        {
          { m_programs.boundary_span_program.vp_uniform, m_vp_matrix },
          { m_programs.boundary_span_program.viewport_size_uniform, m_viewport.size },
          { m_programs.boundary_span_program.max_samples_uniform, 3 },
          { m_programs.boundary_span_program.models_uniform, m_transform_vectors }
        },
        m_viewport.size
      },
      GPU::DepthState{
        GPU::DepthFunc::Less,
        false
      }
    );

    m_boundary_spans.clear();
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
