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

#include "../path/path.h"

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

    data.instance_buffer_id = GPU::Memory::Allocator::allocate_general_buffer<T>(data.max_instances, "instanced_data");

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
    if (data.instances.empty()) {
      return;
    }

    const GPU::Buffer& instance_buffer = GPU::Memory::Allocator::get_general_buffer(data.instance_buffer_id);
    const GPU::Buffer& vertex_buffer = GPU::Memory::Allocator::get_general_buffer(data.vertex_buffer_id);

    GPU::Device::upload_to_buffer(instance_buffer, 0, data.instances, GPU::BufferTarget::Vertex);

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

    GPU::Device::draw_arrays_instanced(data.vertices.size(), data.instances.size(), state);

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
      static_cast<float>(viewport.dpr / viewport.zoom * ui_line_width<double>),
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

  void Renderer::draw(const path::QuadraticPath& path, const Stroke& stroke, const Fill& fill, const mat2x3& transform, const rect* bounding_rect) {
    const rect bounds = bounding_rect ? *bounding_rect : path.approx_bounding_rect();

    draw(path, fill, transform, &bounds);
    draw(path, stroke, transform, &bounds);
  }

  void Renderer::draw(const path::QuadraticPath& path, const Stroke& stroke, const mat2x3& transform, const rect* bounding_rect) {
    if (path.empty()) {
      return;
    }

    const float radius_safe = 0.5f * stroke.width * (stroke.join == LineJoin::Miter ? stroke.miter_limit : 1.0f);
    rect bounds = bounding_rect ? *bounding_rect : path.approx_bounding_rect();

    bounds.min -= radius_safe;
    bounds.max += radius_safe;

    // TODO: iterate quadraticpaths returned
    const geometry::PathBuilder::StrokeOutline stroked_path = geometry::PathBuilder(path, transform, bounding_rect).stroke(stroke, 0.5f);
    const Fill fill = {
      stroke.color,
      FillRule::NonZero,
      stroke.z_index
    };

    // TODO: actually render both inner and outer

    draw(stroked_path.outer, fill, transform, &bounds);
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
      get()->m_circle_instances.instances.emplace_back(transform * p1, get()->m_ui_options.handle_radius, vec4(0.2f, 0.8f, 0.2f, 1.0f));
      // get()->m_vertex_instances.instances.push_back(transform * p2);
    }
  }

  void Renderer::draw(const path::QuadraticPath& path, const Fill& fill, const mat2x3& transform, const rect* bounding_rect) {
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

    data.instances.emplace_back(
      transform, bounds.min, bounds_size, fill.color,
      curves_start_index, bands_start_index,
      horizontal_bands, vertical_bands
    );

    for (size_t i = 0; i < path.size(); i++) {
      const vec2 p0 = path[i * 2];
      const vec2 p1 = path[i * 2 + 1];
      const vec2 p2 = path[i * 2 + 2];

      // get()->m_vertex_instances.instances.push_back(transform * p0);
      // get()->m_handle_instances.instances.push_back(transform * p1);
      // get()->m_vertex_instances.instances.push_back(transform * p2);
      get()->m_circle_instances.instances.emplace_back(transform * p2, get()->m_ui_options.handle_radius, vec4(0.2f, 0.8f, 0.2f, 1.0f));
      // get()->m_handle_instances.instances.push_back(transform * p2);
    }
  }

  void Renderer::draw_outline(const path::QuadraticPath& path, const mat2x3& transform, const float tolerance, const Stroke* stroke, const rect* bounding_rect) {
    // TODO: fix
    // geometry::PathBuilder(path, transform, bounding_rect).flatten(get()->m_viewport.visible(), tolerance, get()->m_line_instances.instances);
  }

  void Renderer::draw_outline(const path::Path& path, const mat2x3& transform, const float tolerance, const Stroke* stroke, const rect* bounding_rect) {
    // TODO: fix

    // if (path.size() == 1) {
    //   const vec2 p0 = transform * path.point_at(0);
    //   const vec2 p1 = transform * path.point_at(1);
    //   const vec2 p2 = transform * path.point_at(2);
    //   const vec2 p3 = transform * path.point_at(3);

    //   vec2 a = -p0 + 3.0 * p1 - 3.0 * p2 + p3;
    //   vec2 b = 3.0 * p0 - 6.0 * p1 + 3.0 * p2;
    //   vec2 c = -3.0 * p0 + 3.0 * p1;
    //   vec2 p;

    //   float conc = std::max(std::hypot(b.x, b.y), std::hypot(a.x + b.x, a.y + b.y));
    //   float dt = std::sqrtf((std::sqrt(8.0) * tolerance) / conc);
    //   float t = dt;

    //   while (t < 1.0f) {
    //     float t_sq = t * t;

    //     p = a * t_sq * t + b * t_sq + c * t + p0;

    //     const float last_x = get()->m_line_instances.instances.size() ? get()->m_line_instances.instances.back().b : p0.x;
    //     const float last_y = get()->m_line_instances.instances.size() ? get()->m_line_instances.instances.back().a : p0.y;

    //     get()->m_line_instances.instances.emplace_back(last_x, last_y, p.x, p.y);

    //     t += dt;
    //   }

    //   get()->m_line_instances.instances.emplace_back(get()->m_line_instances.instances.back().b, get()->m_line_instances.instances.back().a, p3.x, p3.y);

    //   return;
    // }


    // geometry::PathBuilder(path.to_quadratics(tolerance), transform, bounding_rect).flatten(get()->m_viewport.visible(), tolerance, get()->m_line_instances.instances);
  }

  void Renderer::draw_outline_vertices(const path::Path& path, const mat2x3& transform, const std::unordered_set<size_t>* selected_vertices, const Stroke* stroke, const rect* bounding_rect) {
    if (path.vacant()) {
      return;
    }

    const UIOptions& ui_options = get()->m_ui_options;

    std::vector<LineInstance>& lines = get()->m_line_instances.instances;
    std::vector<RectInstance>& rects = get()->m_rect_instances.instances;
    // std::vector<vec2>& filled = get()->m_vertex_instances.instances;
    // std::vector<vec2>& white = get()->m_white_vertex_instances.instances;
    std::vector<CircleInstance>& circles = get()->m_circle_instances.instances;

    size_t i = path.points_size() - 1;
    vec2 last_raw = path.point_at(i);
    vec2 last = transform * last_raw;

    if (!path.closed()) {
      rects.emplace_back(last, ui_options.vertex_size, ui_options.primary_color);
      // filled.push_back(last);

      if (selected_vertices && selected_vertices->find(i) == selected_vertices->end()) {
        rects.emplace_back(last, ui_options.vertex_inner_size, vec4::identity());
        // white.push_back(last);
      }

      const vec2 out_handle = path.point_at(path::Path::out_handle_index);

      if (out_handle != last_raw) {
        const vec2 h = transform * out_handle;

        circles.emplace_back(h, ui_options.handle_radius, ui_options.primary_color);
        lines.emplace_back(h, last, ui_options.line_width, ui_options.primary_color_05);
      }
    }

    path.for_each_reversed(
      [&](const vec2 p0) {
        const vec2 p = transform * p0;

        rects.emplace_back(p, ui_options.vertex_size, ui_options.primary_color);
        // filled.push_back(p);

        if (selected_vertices && selected_vertices->find(i) == selected_vertices->end()) {
          rects.emplace_back(p, ui_options.vertex_inner_size, vec4::identity());
          // white.push_back(p);
        }

        if (!path.closed()) {
          vec2 in_handle = path.point_at(path::Path::in_handle_index);

          if (in_handle != p0) {
            const vec2 h = transform * in_handle;

            circles.emplace_back(h, ui_options.handle_radius, ui_options.primary_color);
            lines.emplace_back(h, p, ui_options.line_width, ui_options.primary_color_05);
          }
        }

        last = p;
        i -= 1;
      },
      [&](const vec2 p0, const vec2 p1) {
        const vec2 p = transform * p0;

        rects.emplace_back(p, ui_options.vertex_size, ui_options.primary_color);
        // filled.push_back(p);

        if (selected_vertices && selected_vertices->find(i - 1) == selected_vertices->end()) {
          rects.emplace_back(p, ui_options.vertex_inner_size, vec4::identity());
          // white.push_back(p);
        }

        last = p;
        i -= 1;
      },
      [&](const vec2 p0, const vec2 p1, const vec2 p2) {
        const vec2 p = transform * p0;

        rects.emplace_back(p, ui_options.vertex_size, ui_options.primary_color);
        // filled.push_back(p);

        if (selected_vertices && selected_vertices->find(i - 2) == selected_vertices->end()) {
          rects.emplace_back(p, ui_options.vertex_inner_size, vec4::identity());
          // white.push_back(p);
        }

        if (p1 != p0 && p2 != p0) {
          const vec2 h = transform * p1;

          circles.emplace_back(h, ui_options.handle_radius, ui_options.primary_color);
          lines.emplace_back(h, p, ui_options.line_width, ui_options.primary_color_05);
          lines.emplace_back(h, last, ui_options.line_width, ui_options.primary_color_05);
        }

        last = p;
        i -= 2;
      },
      [&](const vec2 p0, const vec2 p1, const vec2 p2, const vec2 p3) {
        const vec2 p = transform * p0;

        rects.emplace_back(p, ui_options.vertex_size, ui_options.primary_color);
        // filled.push_back(p);

        if (selected_vertices && selected_vertices->find(i - 3) == selected_vertices->end()) {
          rects.emplace_back(p, ui_options.vertex_inner_size, vec4::identity());
          // white.push_back(p);
        }

        if (p2 != p3) {
          const vec2 h = transform * p2;

          circles.emplace_back(h, ui_options.handle_radius, ui_options.primary_color);
          lines.emplace_back(h, last, ui_options.line_width, ui_options.primary_color_05);
        }

        if (p1 != p0) {
          const vec2 h = transform * p1;

          circles.emplace_back(h, ui_options.handle_radius, ui_options.primary_color);
          lines.emplace_back(h, p, ui_options.line_width, ui_options.primary_color_05);
        }

        last = p;
        i -= 3;
      }
    );
  }

#ifdef GK_DEBUG
  void Renderer::draw_debug_overlays(const geom::cubic_bezier& cubic) {
    vec2 cursor = vec2::zero();

    const float padding = 10.0f;
    const float resolution = 100;

    const vec2 xy_graph_size = vec2(200.0f, -200.0f);
    const vec2 graph_safe_size = xy_graph_size + 2.0f * vec2(-padding, padding);

    for (int j = 0; j < 2; j++) {
      const vec2 graph_position = vec2(get()->m_viewport.size.x - xy_graph_size.x, -xy_graph_size.y) + cursor;

      get()->m_rect_instances.instances.emplace_back(
        get()->m_viewport.project(graph_position + xy_graph_size / 2.0f),
        xy_graph_size / get()->m_viewport.zoom,
        vec4{ 0.0f, 0.0f, 0.0f, 0.5f }
      );

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

        get()->m_line_instances.instances.emplace_back(p0, p1, get()->m_ui_options.line_width, vec4(1.0f, 1.0f, 1.0f, 1.0f));
      }

      cursor += vec2(0.0f, -xy_graph_size.y + padding / 4.0f);
    }

    cursor = vec2::zero();

    const std::vector<geom::quadratic_bezier> quads = geom::cubic_to_quadratics(cubic);

    for (int j = 0; j < 2; j++) {
      const vec2 graph_position = vec2(get()->m_viewport.size.x - xy_graph_size.x, -xy_graph_size.y) + cursor;
      const vec2 graph_safe_position = graph_position + vec2(padding, -padding);

      const rect curve_bounds = cubic.bounding_rect();
      const vec2 bounds_size = curve_bounds.size();

      for (const geom::quadratic_bezier& quad : quads) {
        for (int i = 0; i < static_cast<int>(resolution); i++) {
          const float t0 = static_cast<float>(i) / resolution;
          const float t1 = static_cast<float>(i + 1) / resolution;

          const vec2 p0_norm = (quad.sample(t0) - curve_bounds.min) / bounds_size;
          const vec2 p1_norm = (quad.sample(t1) - curve_bounds.min) / bounds_size;

          const vec2 p0 = get()->m_viewport.project(vec2(t0, p0_norm[j]) * graph_safe_size + graph_safe_position);
          const vec2 p1 = get()->m_viewport.project(vec2(t1, p1_norm[j]) * graph_safe_size + graph_safe_position);

          get()->m_line_instances.instances.emplace_back(p0, p1, get()->m_ui_options.line_width * 2.0f, vec4(0.8f, 0.2f, 0.2f, 1.0f));
        }
      }

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
