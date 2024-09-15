/**
 * @file renderer.cpp
 * @brief The file contains the implementation of the main Graphick renderer.
 *
 * @todo batches of instanced data overflow handling
 * @todo path builder clipping rect
 * @todo dynamic number of samples based on dpr and hardware performance
 * @todo use exact bounding box and cache it
 * @todo fix line width
 */

#include "renderer.h"

#include "gpu/device.h"

#include "../math/matrix.h"
#include "../math/vector.h"

#include "../geom/intersections.h"
#include "../geom/path.h"
#include "../geom/path_builder.h"

#include "../editor/scene/cache.h"

#include "../utils/assert.h"
#include "../utils/console.h"
#include "../utils/defines.h"

#include <algorithm>

#ifdef EMSCRIPTEN
#include <emscripten/html5.h>
#endif

namespace graphick::renderer {

/* -- Static Methods -- */

/**
 * @brief Generates an orthographic projection matrix.
 *
 * @param size The size of the viewport.
 * @param zoom The zoom level.
 * @return The orthographic projection matrix p.
 */
static dmat4 orthographic_projection(const ivec2 size, double zoom) {
  const dvec2 dsize = dvec2(size);

  const double factor = 0.5f / zoom;
  const double half_width = -dsize.x * factor;
  const double half_height = dsize.y * factor;

  const double right = -half_width;
  const double left = half_width;
  const double top = -half_height;
  const double bottom = half_height;

  return dmat4{
    2.0 / (right - left),
    0.0,
    0.0,
    0.0,
    0.0,
    2.0 / (top - bottom),
    0.0,
    0.0,
    0.0,
    0.0,
    -1.0,
    0.0,
    -(right + left) / (right - left),
    -(top + bottom) / (top - bottom),
    0.0,
    1.0
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
static dmat4 orthographic_translation(const ivec2 size, const vec2 position, const double zoom) {
  const dvec2 dsize = dvec2(size);
  const dvec2 dposition = dvec2(position);

  return dmat4{
    1.0f,
    0.0f,
    0.0f,
    0.5f * (-dsize.x / zoom + 2 * dposition.x),
    0.0f,
    1.0f,
    0.0f,
    0.5f * (-dsize.y / zoom + 2 * dposition.y),
    0.0f,
    0.0f,
    1.0f,
    0.0f,
    0.0f,
    0.0f,
    0.0f,
    1.0f
  };
}

#define RENDER_STATE(name)                                                                                                       \
  GPU::RenderState {                                                                                                             \
    m_programs.name##_program.program, &m_vertex_arrays.name##_vertex_array->vertex_array, m_##name##_instances.primitive,       \
      irect(ivec2::zero(), m_viewport.size)                                                                                      \
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
template <typename T>
static void flush(InstancedData<T>& data, const GPU::RenderState render_state) {
  if (data.instances.batches[0].empty()) {
    return;
  }

  for (const std::vector<T>& batch : data.instances.batches) {
    data.instance_buffer.upload(batch.data(), batch.size() * sizeof(T));

    GPU::Device::draw_arrays_instanced(data.vertex_buffer.size / data.vertex_size, batch.size(), render_state);
  }

  data.instances.clear();
}

/* -- Static Member Initialization -- */

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
  attr.stencil = true;
  attr.depth = true;

  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context("#canvas", &attr);
  emscripten_webgl_make_context_current(ctx);

  GPU::Device::init(GPU::DeviceVersion::GLES3);
#else
  GPU::Device::init(GPU::DeviceVersion::GL3);
#endif

  s_instance = new Renderer();
}

void Renderer::shutdown() {
  GK_ASSERT(s_instance != nullptr, "Renderer not initialized, call init() before shutting down!");

  delete s_instance;
  s_instance = nullptr;

  GPU::Device::shutdown();

#ifdef EMSCRIPTEN
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_get_current_context();
  emscripten_webgl_destroy_context(ctx);
#endif
}

void Renderer::begin_frame(const RenderOptions& options) {
  const dmat4 view_matrix = orthographic_translation(options.viewport.size, options.viewport.position, options.viewport.zoom);
  const dmat4 projection_matrix = orthographic_projection(options.viewport.size, options.viewport.zoom);

  get()->m_last_viewport = get()->m_viewport;
  get()->m_viewport = options.viewport;
  get()->m_vp_matrix = projection_matrix * view_matrix;
  get()->m_cache = options.cache;
  // get()->m_cached_rendering = !options.ignore_cache;
  get()->m_cached_rendering = false;

  get()->m_transform_vectors.clear();
  get()->m_transform_vectors.reserve(get()->m_max_transform_vectors);
  get()->m_transform_vectors.push_back(vec4(1.0f, 0.0f, 0.0f, 0.0f));
  get()->m_transform_vectors.push_back(vec4(0.0f, 1.0f, 0.0f, 0.0f));

  get()->m_ui_options = UIOptions{
    vec2(options.viewport.dpr / options.viewport.zoom * ui_handle_size<double>),
    vec2(options.viewport.dpr / options.viewport.zoom * (ui_handle_size<double> - 2.0)),
    static_cast<float>(options.viewport.dpr / options.viewport.zoom / 2.0 * ui_handle_size<double>),
    static_cast<float>(options.viewport.dpr * ui_line_width<double>),
    vec4(0.22f, 0.76f, 0.95f, 1.0f),
    vec4(0.22f, 0.76f, 0.95f, 1.0f) * vec4(0.95f, 0.95f, 0.95f, 1.0f),
  };

  GPU::Device::begin_commands();

  get()->flush_cache();
}

void Renderer::end_frame() {
  get()->flush_meshes();
  get()->flush_overlay_meshes();

  size_t time = GPU::Device::end_commands();

  GK_TOTAL_RECORD("GPU", time);
}

void Renderer::draw(
  const geom::Path<float, std::enable_if<true>>& path,
  const mat2x3& transform,
  const Fill* fill,
  const Stroke* stroke,
  const rect* bounding_rect,
  const bool pretransformed_rect
) {
  GK_TOTAL("Renderer::draw(fill, cubic)");

  if (path.empty() || (fill == nullptr && stroke == nullptr)) {
    return;
  }

  const rect bounds = bounding_rect ? *bounding_rect : path.approx_bounding_rect();
  const rect transformed_bounds = pretransformed_rect ? bounds : transform * bounds;

  if (get()->m_cached_rendering && geom::is_rect_in_rect(transformed_bounds, get()->m_safe_clip_rect)) {
    if (get()->m_invalid_rects.empty()) {
      return;
    }

    bool intersects = false;

    for (const auto& r : get()->m_invalid_rects) {
      if (geom::does_rect_intersect_rect(transformed_bounds, r)) {
        intersects = true;
        break;
      }
    }

    if (!intersects) {
      return;
    }
  }

  const float coverage =
    geom::rect_rect_intersection_area(transformed_bounds, get()->m_viewport.visible()) / transformed_bounds.area();
  const float dimension = transformed_bounds.area() / get()->m_viewport.visible().area();

  // if (dimension < 0.000001f * get()->m_viewport.size.x * get()->m_viewport.size.y) {
  //   return;
  // }

  /* If element is too small, it is not worth creating culling data and bounding polygons. */

  const bool culling = transformed_bounds.area() * get()->m_viewport.zoom * get()->m_viewport.zoom > 18.0f * 18.0f;

  if (fill) {
    geom::cubic_path cubic_path = path.to_cubic_path();

    if (!cubic_path.closed()) {
      cubic_path.line_to(cubic_path.front());
    }

    // TODO: when all transforms will be on the CPU and cached we can use the transformed bounds
    get()->draw_no_clipping(
      std::move(cubic_path),
      *fill,
      transform,
      pretransformed_rect ? path.approx_bounding_rect() : bounds,
      transformed_bounds,
      culling
    );
  }

  if (stroke) {
    // TODO: should calculate stroke before fill, beacuse drawing takes ownership of the path
    Fill stroke_fill{stroke->color, FillRule::NonZero, stroke->z_index};

    geom::StrokingOptions<float> stroking_options{
      stroke->width,
      stroke->miter_limit,
      stroke->cap,
      stroke->join,
    };

    geom::cubic_path stroke_path =
      geom::PathBuilder(path, transform, bounding_rect, pretransformed_rect).stroke(stroking_options, 0.0001f);

    // geom::path stroke_pppath;
    // stroke_pppath.move_to(stroke_path[0]);

    // for (size_t i = 0; i < stroke_path.size() * 3; i += 3) {
    //   stroke_pppath.cubic_to(stroke_path[i + 1], stroke_path[i + 2], stroke_path[i + 3]);
    // }

    // geom::PathBuilder(stroke_pppath, mat2x3::identity(), bounding_rect, true)
    //   .flatten(get()->m_viewport.visible(), 0.25f, [&](const vec2 p0, const vec2 p1) {
    //     get()->m_line_instances.instances.push_back({p0, p1, get()->m_ui_options.line_width,
    //     get()->m_ui_options.primary_color_05}
    //     );
    //   });

    const rect stroke_rect = stroke_path.approx_bounding_rect();

    // TODO: should use the stroke bounding_rect...
    get()->draw_no_clipping(std::move(stroke_path), stroke_fill, mat2x3::identity(), stroke_rect, stroke_rect, culling);
  }
}

void Renderer::draw_outline(
  const geom::Path<float, std::enable_if<true>>& path,
  const mat2x3& transform,
  const float tolerance,
  const bool draw_vertices,
  const std::unordered_set<uint32_t>* selected_vertices,
  const Stroke* stroke,
  const rect* bounding_rect,
  const bool pretransformed_rect
) {
  if (path.empty()) {
    return;
  }

  geom::PathBuilder(path, transform, bounding_rect, pretransformed_rect)
    .flatten(get()->m_viewport.visible(), tolerance, [&](const vec2 p0, const vec2 p1) {
      get()->m_line_instances.instances.push_back({p0, p1, get()->m_ui_options.line_width, get()->m_ui_options.primary_color_05});
    });

  if (draw_vertices) {
    get()->draw_outline_vertices(path, transform, tolerance, selected_vertices, stroke);
  }
}

void Renderer::draw_rect(const rect& rect, const std::optional<vec4> color) {
  get()->m_rect_instances.instances.push_back({rect.center(), rect.size(), color.value_or(get()->m_ui_options.primary_color)});
}

void Renderer::draw_rect(const vec2 center, const vec2 size, const std::optional<vec4> color) {
  get()->m_rect_instances.instances.push_back({center, size, color.value_or(get()->m_ui_options.primary_color)});
}

Renderer::Renderer() :
  m_path_instances(GK_LARGE_BUFFER_SIZE),
  m_boundary_span_instances(GK_LARGE_BUFFER_SIZE),
  m_line_instances(GK_LARGE_BUFFER_SIZE, quad_vertices({0, 0}, {1, 1})),
  m_circle_instances(GK_BUFFER_SIZE, quad_vertices({0, 0}, {1, 1})),
  m_rect_instances(GK_BUFFER_SIZE, quad_vertices({0, 0}, {1, 1})),
  m_image_instances(GK_BUFFER_SIZE, quad_vertices({0, 0}, {1, 1})),
  m_filled_span_instances(GK_BUFFER_SIZE, quad_vertices({0, 0}, {1, 1})),
  m_transform_vectors(GPU::Device::max_vertex_uniform_vectors() - 6),
  m_max_transform_vectors(GPU::Device::max_vertex_uniform_vectors() - 6) {
  std::unique_ptr<GPU::PathVertexArray> path_vertex_array = std::make_unique<GPU::PathVertexArray>(
    m_programs.path_program,
    m_path_instances.instance_buffer,
    m_path_instances.vertex_buffer
  );

  std::unique_ptr<GPU::BoundarySpanVertexArray> boundary_span_vertex_array = std::make_unique<GPU::BoundarySpanVertexArray>(
    m_programs.boundary_span_program,
    m_boundary_span_instances.instance_buffer,
    m_boundary_span_instances.vertex_buffer
  );

  std::unique_ptr<GPU::FilledSpanVertexArray> filled_span_vertex_array = std::make_unique<GPU::FilledSpanVertexArray>(
    m_programs.filled_span_program,
    m_filled_span_instances.instance_buffer,
    m_filled_span_instances.vertex_buffer
  );

  std::unique_ptr<GPU::LineVertexArray> line_vertex_array = std::make_unique<GPU::LineVertexArray>(
    m_programs.line_program,
    m_line_instances.instance_buffer,
    m_line_instances.vertex_buffer
  );

  std::unique_ptr<GPU::RectVertexArray> rect_vertex_array = std::make_unique<GPU::RectVertexArray>(
    m_programs.rect_program,
    m_rect_instances.instance_buffer,
    m_rect_instances.vertex_buffer
  );

  std::unique_ptr<GPU::CircleVertexArray> circle_vertex_array = std::make_unique<GPU::CircleVertexArray>(
    m_programs.circle_program,
    m_circle_instances.instance_buffer,
    m_circle_instances.vertex_buffer
  );

  std::unique_ptr<GPU::ImageVertexArray> image_vertex_array = std::make_unique<GPU::ImageVertexArray>(
    m_programs.image_program,
    m_image_instances.instance_buffer,
    m_image_instances.vertex_buffer
  );

  m_vertex_arrays = GPU::VertexArrays{
    std::move(path_vertex_array),
    std::move(boundary_span_vertex_array),
    std::move(filled_span_vertex_array),
    std::move(line_vertex_array),
    std::move(rect_vertex_array),
    std::move(circle_vertex_array),
    std::move(image_vertex_array)
  };

  m_framebuffer = std::make_unique<GPU::Framebuffer>(ivec2::identity(), true);
}

uint32_t Renderer::push_transform(const mat2x3& transform) {
  if (math::is_identity(transform)) return 0;

  uint32_t transform_index = static_cast<uint32_t>(get()->m_transform_vectors.size() / 2);

  const vec4 row0 = vec4(transform[0][0], transform[0][1], transform[0][2], 0.0f);
  const vec4 row1 = vec4(transform[1][0], transform[1][1], transform[1][2], 0.0f);

  for (uint32_t i = 0; i < m_transform_vectors.size(); i += 2) {
    if (m_transform_vectors[i] == row0 && m_transform_vectors[i + 1] == row1) {
      return i / 2;
    }
  }

  m_transform_vectors.insert(m_transform_vectors.end(), {row0, row1});

  return transform_index;
}

void Renderer::draw_no_clipping(
  geom::cubic_path&& path,
  const Fill& fill,
  const mat2x3& transform,
  const rect& bounding_rect,
  const rect& transformed_bounding_rect,
  const bool culling
) {
  PathInstancedData& data = m_path_instances;

  /* Starting indices for this path. */

  const size_t curves_start_index = data.curves.size() / 2;
  const size_t bands_start_index = data.bands.size();

  /* Copy the curves, close the path and cache min_max values. */

  const bool closed = path.closed();
  const size_t len = closed ? path.size() : (path.size() + 1);
  const vec2 bounds_size = bounding_rect.size();

  std::vector<vec2> min(len);
  std::vector<vec2> max(len);

  for (size_t i = 0; i < path.size(); i++) {
    const vec2 p0 = path[i * 3];
    const vec2 p1 = path[i * 3 + 1];
    const vec2 p2 = path[i * 3 + 2];
    const vec2 p3 = path[i * 3 + 3];

    data.curves.insert(
      data.curves.end(),
      {p0 - bounding_rect.min, p1 - bounding_rect.min, p2 - bounding_rect.min, p3 - bounding_rect.min}
    );

    min[i] = math::min(p0, p3);
    max[i] = math::max(p0, p3);
  }

  /* Bands count is always between 1 and 16, based on the number of segments. */

  const float max_size = std::max(bounds_size.x, bounds_size.y);

  const uint8_t horizontal_bands =
    std::clamp(static_cast<int>(transformed_bounding_rect.size().y * m_viewport.zoom / 15.0f), 3, 32);

  /* Sort curves by descending max x. */

  std::vector<uint16_t> h_indices(len);

  std::iota(h_indices.begin(), h_indices.end(), 0);
  std::sort(h_indices.begin(), h_indices.end(), [&](const uint16_t a, const uint16_t b) { return max[a].x > max[b].x; });

  /* Calculate band metrics. */

  const float band_delta = bounds_size.y / horizontal_bands;

  float band_min = bounding_rect.min.y;
  float band_max = bounding_rect.min.y + band_delta;

  /* Preallocate horizontal and vertical bands header. */

  data.bands.resize(data.bands.size() + horizontal_bands * 4, 0);

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
    }

    /* Each band header is an offset from this path's bands data start and the number of curves in the band. */

    const size_t band_end = data.bands.size();

    data.bands[bands_start_index + i * 4] = static_cast<uint16_t>(band_start - bands_start_index);
    data.bands[bands_start_index + i * 4 + 1] = static_cast<uint16_t>(band_end - band_start);

    band_min += band_delta;
    band_max += band_delta;
  }

  /* Push transform. */

  const uint32_t transform_index = push_transform(transform);

  /* Push instance. */

  data.instances.push_back(
    {bounding_rect.min,
     bounds_size,
     fill.color,
     curves_start_index,
     bands_start_index,
     horizontal_bands,
     false,
     fill.rule == FillRule::EvenOdd,
     culling,
     fill.z_index,
     transform_index}
  );

  if (!culling) {
    return;
  }

  /* Calculate culling data. */

  std::vector<Band> bands(horizontal_bands);
  std::vector<std::vector<Intersection>> band_bottom_intersections(horizontal_bands);

  for (int i = 0; i < bands.size(); i++) {
    bands[i].top_y = bounding_rect.min.y + i * band_delta;
    bands[i].bottom_y = bounding_rect.min.y + (i + 1) * band_delta;
  }

  // TODO: can optimize, we already know which curves are in each band
  for (uint16_t i = 0; i < path.size(); i++) {
    const vec2 p0 = path[i * 3];
    const vec2 p1 = path[i * 3 + 1];
    const vec2 p2 = path[i * 3 + 2];
    const vec2 p3 = path[i * 3 + 3];

    /* Being monotonic, it is straightforward to determine which bands the curve intersects. */

    const float start_band_factor = (min[i].y - bounding_rect.min.y) / band_delta;
    const float end_band_factor = (max[i].y - bounding_rect.min.y) / band_delta;
    const int start_band = std::clamp(static_cast<int>(start_band_factor), 0, horizontal_bands - 1);
    const int end_band = std::clamp(static_cast<int>(end_band_factor), 0, horizontal_bands - 1);

    if (start_band >= end_band) {
      /* Curve is within one band. */
      bands[start_band].push_curve(min[i].x, max[i].x);
      continue;
    }

    /* Calculate intersections with band boundaries. */

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
        clipped_min = std::min(clipped_min, p3.x);
        clipped_max = std::max(clipped_max, p3.x);
      }

      /* Intersections with the top boundary are cached. */
      if (last_intersection.has_value()) {
        clipped_min = std::min(clipped_min, last_intersection.value());
        clipped_max = std::max(clipped_max, last_intersection.value());
      }

      /* We need to check intersections with the bottom boundary of the band. */
      const float y = bands[j].bottom_y;
      const bool is_downwards = p0.y > y || p3.y < y;

      if ((is_downwards && ((p0.y < y && p3.y <= y) || (p0.y > y && p3.y >= y))) ||
          (!is_downwards && ((p0.y <= y && p3.y < y) || (p0.y >= y && p3.y > y)))) {
        /* Curve does not intersect the band. */
        bands[j].push_curve(clipped_min, clipped_max);
        continue;
      }

      const float t0 = (y - p0.y) / (p3.y - p0.y);

      if (linear) {
        if (t0 >= -math::geometric_epsilon<float> && t0 <= 1.0f + math::geometric_epsilon<float>) {
          const float x = p0.x + t0 * (p3.x - p0.x);

          last_intersection = x;

          clipped_min = std::min(clipped_min, x);
          clipped_max = std::max(clipped_max, x);

          band_bottom_intersections[j].push_back({x, is_downwards});
        }
      } else {
        const float t = geom::cubic_line_intersect_approx(a.y, b.y, c.y, d.y, y, t0);

        if (t >= -math::geometric_epsilon<float> && t <= 1.0f + math::geometric_epsilon<float>) {
          const float t_sq = t * t;
          const float x = a.x * t_sq * t + b.x * t_sq + c.x * t + d.x;

          last_intersection = x;

          clipped_min = std::min(clipped_min, x);
          clipped_max = std::max(clipped_max, x);

          band_bottom_intersections[j].push_back({x, is_downwards});
        }
      }

      bands[j].push_curve(clipped_min, clipped_max);
    }
  }

  /* The last band cannot have filled spans. */

  band_bottom_intersections.back().clear();

  /* Calculate filled spans. */

  for (int i = 0; i < bands.size(); i++) {
    Band& band = bands[i];

    std::sort(
      band_bottom_intersections[i].begin(),
      band_bottom_intersections[i].end(),
      [&](const Intersection& a, const Intersection& b) { return a.x < b.x; }
    );

    int winding = 0;
    int winding_k = 0;

    for (int j = 0; j < static_cast<int>(band.disabled_spans.size()) - 1; j++) {
      Span& span1 = band.disabled_spans[j];
      Span& span2 = band.disabled_spans[j + 1];

      for (; winding_k < band_bottom_intersections[i].size(); winding_k++) {
        if (band_bottom_intersections[i][winding_k].x > (span1.max + span2.min) * 0.5f) {
          break;
        }

        winding -= int(band_bottom_intersections[i][winding_k].downwards) * 2 - 1;
      }

      if (fill.rule == FillRule::NonZero ? (winding != 0) : (winding % 2 != 0)) {
        m_filled_span_instances.instances.push_back(
          {vec2(span1.max, band.top_y), vec2(span2.min - span1.max, band_delta), fill.color, fill.z_index, transform_index}
        );
      }
    }

    if (band.disabled_spans.empty()) {
      data.bands[bands_start_index + i * 4 + 2] = 0;
      data.bands[bands_start_index + i * 4 + 3] = std::numeric_limits<uint16_t>::max();
    } else {
      data.bands[bands_start_index + i * 4 + 2] =
        static_cast<uint16_t>(std::max(std::floor(band.disabled_spans.front().min - bounding_rect.min.x), 0.0f));
      data.bands[bands_start_index + i * 4 + 3] =
        static_cast<uint16_t>(std::max(std::ceil(band.disabled_spans.back().max - bounding_rect.min.x), 0.0f));
    }
  }
}

void Renderer::draw_with_clipping(geom::cubic_path&& path, const Fill& fill, const mat2x3& transform, const rect& bounding_rect) {

}

void Renderer::draw_outline_vertices(
  const geom::Path<float, std::enable_if<true>>& path,
  const mat2x3& transform,
  const float tolerance,
  const std::unordered_set<uint32_t>* selected_vertices,
  const Stroke* stroke
) {
  InstanceBuffer<LineInstance>& lines = m_line_instances.instances;
  InstanceBuffer<RectInstance>& rects = m_rect_instances.instances;
  InstanceBuffer<CircleInstance>& circles = get()->m_circle_instances.instances;

  uint32_t i = path.points_count() - 1;
  vec2 last_raw = path.at(i);
  vec2 last = transform * last_raw;

  if (!path.closed()) {
    rects.push_back({last, m_ui_options.vertex_size, m_ui_options.primary_color});

    if (selected_vertices && selected_vertices->find(i) == selected_vertices->end()) {
      rects.push_back({last, m_ui_options.vertex_inner_size, vec4::identity()});
    }

    const vec2 out_handle = path.out_handle();

    if (out_handle != last_raw) {
      const vec2 h = transform * out_handle;

      circles.push_back({h, m_ui_options.handle_radius, m_ui_options.primary_color});
      lines.push_back({h, last, m_ui_options.line_width, m_ui_options.primary_color_05});
    }
  }

  /* We draw the vertices in reverse order to be coherent with hit testing. */

  path.for_each_reversed(
    [&](const vec2 p0_raw) {
      const vec2 p0 = transform * p0_raw;

      rects.push_back({p0, m_ui_options.vertex_size, m_ui_options.primary_color});

      if (selected_vertices && selected_vertices->find(i) == selected_vertices->end()) {
        rects.push_back({p0, m_ui_options.vertex_inner_size, vec4::identity()});
      }

      if (!path.closed()) {
        vec2 in_handle = path.in_handle();

        if (in_handle != p0_raw) {
          const vec2 h = transform * in_handle;

          circles.push_back({h, m_ui_options.handle_radius, m_ui_options.primary_color});
          lines.push_back({h, p0, m_ui_options.line_width, m_ui_options.primary_color_05});
        }
      }

      last = p0;
      i -= 1;
    },
    [&](const vec2 p0_raw, const vec2 p1_raw) {
      const vec2 p0 = transform * p0_raw;

      rects.push_back({p0, m_ui_options.vertex_size, m_ui_options.primary_color});

      if (selected_vertices && selected_vertices->find(i - 1) == selected_vertices->end()) {
        rects.push_back({p0, m_ui_options.vertex_inner_size, vec4::identity()});
      }

      last = p0;
      i -= 1;
    },
    [&](const vec2 p0_raw, const vec2 p1_raw, const vec2 p2_raw) {
      const vec2 p0 = transform * p0_raw;
      const vec2 p1 = transform * p1_raw;

      rects.push_back({p0, m_ui_options.vertex_size, m_ui_options.primary_color});

      if (selected_vertices && selected_vertices->find(i - 2) == selected_vertices->end()) {
        rects.push_back({p0, m_ui_options.vertex_inner_size, vec4::identity()});
      }

      if (p1_raw != p0_raw && p2_raw != p0_raw) {
        const vec2 h = transform * p1_raw;

        circles.push_back({p1, m_ui_options.handle_radius, m_ui_options.primary_color});
        lines.push_back({p1, p0, m_ui_options.line_width, m_ui_options.primary_color_05});
        lines.push_back({p1, last, m_ui_options.line_width, m_ui_options.primary_color_05});
      }

      last = p0;
      i -= 2;
    },
    [&](const vec2 p0_raw, const vec2 p1_raw, const vec2 p2_raw, const vec2 p3_raw) {
      const vec2 p0 = transform * p0_raw;
      const vec2 p1 = transform * p1_raw;
      const vec2 p2 = transform * p2_raw;

      rects.push_back({p0, m_ui_options.vertex_size, m_ui_options.primary_color});

      if (selected_vertices && selected_vertices->find(i - 3) == selected_vertices->end()) {
        rects.push_back({p0, m_ui_options.vertex_inner_size, vec4::identity()});
      }

      if (p2_raw != p3_raw) {
        circles.push_back({p2, m_ui_options.handle_radius, m_ui_options.primary_color});
        lines.push_back({p2, last, m_ui_options.line_width, m_ui_options.primary_color_05});
      }

      if (p1_raw != p0_raw) {
        circles.push_back({p1, m_ui_options.handle_radius, m_ui_options.primary_color});
        lines.push_back({p1, p0, m_ui_options.line_width, m_ui_options.primary_color_05});
      }

      last = p0;
      i -= 3;
    }
  );
}

void Renderer::flush_cache() {
  /* Bind the default framebuffer. */
  get()->m_framebuffer->unbind();

  /* Setup the viewport. */
  GPU::Device::set_viewport(m_viewport.size);
  GPU::Device::clear({m_viewport.background, 1.0f, 0});

  if (!m_cached_rendering) return;

  const rect cached = get()->m_last_viewport.visible();
  const rect visible = get()->m_viewport.visible();

  m_safe_clip_rect = cached;

  /* Enlarge the clipping region to the avoid elements at the edges being rerendered. */
  if (cached.min.y <= visible.min.y) m_safe_clip_rect.min.y = std::numeric_limits<float>::lowest();
  if (cached.min.x <= visible.min.x) m_safe_clip_rect.min.x = std::numeric_limits<float>::lowest();
  if (cached.max.y >= visible.max.y) m_safe_clip_rect.max.y = std::numeric_limits<float>::max();
  if (cached.max.x >= visible.max.x) m_safe_clip_rect.max.x = std::numeric_limits<float>::max();

  if (m_framebuffer->complete && geom::does_rect_intersect_rect(cached, visible)) {
    GPU::RenderState render_state;

    m_image_instances.instances.push_back({cached.center(), cached.size()});

    render_state = RENDER_STATE(image).no_blend().no_depth().add_stencil();
    render_state.textures = std::vector<GPU::TextureBinding>{{m_programs.image_program.image_texture, m_framebuffer->texture}};
    render_state.uniforms = {{m_programs.image_program.vp_uniform, m_vp_matrix}};

    /* Draw the cached image and stencil out its bounding box. */
    flush(m_image_instances, render_state);

    m_invalid_rects = m_cache->get_invalid_rects();

    for (const auto& r : m_invalid_rects) {
      draw_rect(r, m_viewport.background);
    }

    render_state = RENDER_STATE(rect).no_blend().no_depth().subtract_stencil();
    render_state.uniforms = {{m_programs.rect_program.vp_uniform, m_vp_matrix}};

    /* Draw the background and stencil in the invalid regions. */
    flush(m_rect_instances, render_state);

    m_cached_rendering = true;
  } else {
    m_invalid_rects.clear();
    m_cached_rendering = false;
  }
}

void Renderer::flush_meshes() {
  GPU::RenderState render_state;

  if (!m_filled_span_instances.instances.batches.empty() & !m_filled_span_instances.instances.batches.front().empty()) {
    std::reverse(m_filled_span_instances.instances.batches.begin(), m_filled_span_instances.instances.batches.end());

    for (auto& batch : m_filled_span_instances.instances.batches) {
      std::reverse(batch.begin(), batch.end());
    }

    render_state = RENDER_STATE(filled_span).no_blend().default_depth().keep_stencil();
    render_state.uniforms = {
      {m_programs.filled_span_program.vp_uniform, m_vp_matrix},
      {m_programs.filled_span_program.models_uniform, m_transform_vectors}
    };

    flush(m_filled_span_instances, render_state);
  }

  if (!m_path_instances.instances.batches.empty() & !m_path_instances.instances.batches.front().empty()) {
    m_path_instances.curves_texture.upload(m_path_instances.curves.data(), m_path_instances.curves.size() * sizeof(vec2));
    m_path_instances.bands_texture.upload(m_path_instances.bands.data(), m_path_instances.bands.size() * sizeof(uint16_t));

    render_state = RENDER_STATE(path).default_blend().default_depth().keep_stencil();
    render_state.textures = std::vector<GPU::TextureBinding>{
      {m_programs.path_program.bands_texture, m_path_instances.bands_texture},
      {m_programs.path_program.curves_texture, m_path_instances.curves_texture}
    };
    render_state.uniforms = {
      {m_programs.path_program.vp_uniform, m_vp_matrix},
      {m_programs.path_program.viewport_size_uniform, vec2(m_viewport.size)},
      {m_programs.path_program.samples_uniform, 3},
      {m_programs.path_program.models_uniform, m_transform_vectors}
    };

    flush(m_path_instances, render_state);
  }

  m_path_instances.clear();

  if (get()->m_framebuffer->size() != ivec2(get()->m_viewport.size)) {
    get()->m_framebuffer.reset();
    get()->m_framebuffer = std::make_unique<GPU::Framebuffer>(ivec2(get()->m_viewport.size), true);
  }

  if (get()->m_framebuffer->complete) {
    GPU::Device::blit_framebuffer(
      *get()->m_framebuffer,
      irect{ivec2::zero(), get()->m_framebuffer->texture.size},
      irect{ivec2::zero(), get()->m_viewport.size},
      true
    );
  }
}

void Renderer::flush_overlay_meshes() {
  GPU::RenderState render_state;

  render_state = RENDER_STATE(line).default_blend().no_depth().no_stencil();
  render_state.uniforms = {
    {m_programs.line_program.vp_uniform, m_vp_matrix},
    {m_programs.line_program.zoom_uniform, static_cast<float>(m_viewport.zoom)}
  };

  flush(m_line_instances, render_state);

  render_state = RENDER_STATE(rect).default_blend().no_depth().no_stencil();
  render_state.uniforms = {{m_programs.rect_program.vp_uniform, m_vp_matrix}};

  flush(m_rect_instances, render_state);

  render_state = RENDER_STATE(circle).default_blend().no_depth().no_stencil();
  render_state.uniforms = {
    {m_programs.circle_program.vp_uniform, m_vp_matrix},
    {m_programs.circle_program.zoom_uniform, static_cast<float>(m_viewport.zoom)}
  };

  flush(m_circle_instances, render_state);
}
}
