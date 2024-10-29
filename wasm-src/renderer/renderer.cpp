/**
 * @file renderer.cpp
 * @brief The file contains the implementation of the main Graphick renderer.
 *
 * @todo gradients overflow handling
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
#  include <emscripten/html5.h>
#endif

namespace graphick::renderer {

/* -- Static Methods -- */

/**
 * @brief Generates an orthographic projection matrix from a viewport.
 *
 * @param viewport The viewport to generate the projection matrix for.
 * @return The orthographic projection matrix p.
 */
static dmat4 orthographic_projection(const Viewport& viewport)
{
  const dvec2 dsize = dvec2(viewport.size);

  const double factor = 0.5 / viewport.zoom;
  const double half_width = -dsize.x * factor;
  const double half_height = dsize.y * factor;

  return dmat4{{-1.0 / half_width, 0.0, 0.0, 0.0},
               {0.0, -1.0 / half_height, 0.0, 0.0},
               {0.0, 0.0, -1.0, 0.0},
               {0.0, 0.0, 0.0, 1.0}};
}

/**
 * @brief Generates an orthographic translation matrix from a viewport.
 *
 * @param viewport The viewport to generate the translation matrix for.
 * @return The orthographic translation matrix v.
 */
static dmat4 orthographic_translation(const Viewport& viewport)
{
  const dvec2 dsize = dvec2(viewport.size);

  return dmat4{{1.0, 0.0, 0.0, 0.5 * (-dsize.x / viewport.zoom + 2.0 * viewport.position.x)},
               {0.0, 1.0, 0.0, 0.5 * (-dsize.y / viewport.zoom + 2.0 * viewport.position.y)},
               {0.0, 0.0, 1.0, 0.0},
               {0.0, 0.0, 0.0, 1.0}};
}

#define RENDER_STATE(name) \
  GPU::RenderState \
  { \
    m_programs.name##_program.program, &m_vertex_arrays.name##_vertex_array->vertex_array, \
        m_##name##_instances.primitive, irect(ivec2::zero(), m_viewport.size) \
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
template<typename T>
static void flush(InstancedData<T>& data, const GPU::RenderState render_state)
{
  if (data.instances.batches[0].empty()) {
    return;
  }

  for (const std::vector<T>& batch : data.instances.batches) {
    data.instance_buffer.upload(batch.data(), batch.size() * sizeof(T));

    GPU::Device::draw_arrays_instanced(
        data.vertex_buffer.size / data.vertex_size, batch.size(), render_state);
  }

  data.instances.clear();
}

/* -- Static Member Initialization -- */

Renderer* Renderer::s_instance = nullptr;

/* -- Renderer -- */

void Renderer::init()
{
  GK_ASSERT(s_instance == nullptr,
            "Renderer already initialized, call shutdown() before reinitializing!");

#ifdef EMSCRIPTEN
  EmscriptenWebGLContextAttributes attr;
  emscripten_webgl_init_context_attributes(&attr);

  /* Despite
   * <https://developer.mozilla.org/en-US/docs/Web/API/WebGL_API/WebGL_best_practices#avoid_alphafalse_which_can_be_expensive>,
   * when using desynchronized context (essential for responsive input), is better to use
   * alpha=false to avoid expensive blending operations if there are elements on top.
   */
  attr.alpha = false;
  attr.desynchronized = true;
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

void Renderer::shutdown()
{
  GK_ASSERT(s_instance != nullptr, "Renderer not initialized, call init() before shutting down!");

  delete s_instance;
  s_instance = nullptr;

  GPU::Device::shutdown();

#ifdef EMSCRIPTEN
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_get_current_context();
  emscripten_webgl_destroy_context(ctx);
#endif
}

void Renderer::begin_frame(const RenderOptions& options)
{
  const dmat4 view_matrix = orthographic_translation(options.viewport);
  const dmat4 projection_matrix = orthographic_projection(options.viewport);

  get()->m_last_viewport = get()->m_viewport;
  get()->m_viewport = options.viewport;
  get()->m_vp_matrix = projection_matrix * view_matrix;
  get()->m_cache = options.cache;
  get()->m_cached_rendering = false;  // get()->m_cached_rendering = !options.ignore_cache;
  get()->m_ui_options = UIOptions(options.viewport.dpr / options.viewport.zoom);

  GPU::Device::begin_commands();

  get()->flush_cache();
}

void Renderer::end_frame()
{
  get()->flush_meshes();
  get()->flush_overlay_meshes();

  size_t time = GPU::Device::end_commands();

  GK_TOTAL_RECORD("GPU", time);
}

void Renderer::draw(const geom::Path<float, std::enable_if<true>>& path,
                    const mat2x3& transform,
                    const Fill* fill,
                    const Stroke* stroke,
                    const rect* bounding_rect,
                    const bool pretransformed_rect)
{
  if (path.empty() || (fill == nullptr && stroke == nullptr)) {
    return;
  }

  // TODO: when not provided, calculate bounding rect from path, stroke and possibly effects. Also
  // transform * bounds is not the best approximation but should be good enough if most of the
  // paths are transformed in scene.render()
  const rect bounds = bounding_rect ? *bounding_rect : path.approx_bounding_rect();
  const rect transformed_bounds = pretransformed_rect ? bounds : transform * bounds;

  // TODO: clip if necessary

  if (math::is_identity(transform)) {
    get()->draw_transformed(path, transformed_bounds, fill, stroke);
  } else {
    get()->draw_transformed(path.transformed(transform), transformed_bounds, fill, stroke);
  }
}

void Renderer::draw_outline(const geom::Path<float, std::enable_if<true>>& path,
                            const mat2x3& transform,
                            const float tolerance,
                            const bool draw_vertices,
                            const std::unordered_set<uint32_t>* selected_vertices,
                            const Stroke* stroke,
                            const rect* bounding_rect,
                            const bool pretransformed_rect)
{
  if (path.empty()) {
    return;
  }

  geom::PathBuilder(path, transform, bounding_rect, pretransformed_rect)
      .flatten(get()->m_viewport.visible(), tolerance, [&](const vec2 p0, const vec2 p1) {
        get()->m_line_instances.instances.push_back(
            {p0, p1, get()->m_ui_options.line_width, get()->m_ui_options.primary_color_05});
      });

  if (draw_vertices) {
    get()->draw_outline_vertices(path, transform, tolerance, selected_vertices, stroke);
  }
}

void Renderer::draw_rect(const rect& rect, const std::optional<vec4> color)
{
  get()->m_rect_instances.instances.push_back(
      {rect.center(), rect.size(), color.value_or(get()->m_ui_options.primary_color)});
}

void Renderer::draw_rect(const vec2 center, const vec2 size, const std::optional<vec4> color)
{
  get()->m_rect_instances.instances.push_back(
      {center, size, color.value_or(get()->m_ui_options.primary_color)});
}

void Renderer::draw_line(const vec2 start, const vec2 end, const std::optional<vec4> color)
{
  get()->m_line_instances.instances.push_back(
      {start,
       end,
       get()->m_ui_options.line_width,
       color.value_or(get()->m_ui_options.primary_color_05)});
}

Renderer::Renderer()
    : m_batch(GK_LARGE_BUFFER_SIZE),
      m_line_instances(GK_LARGE_BUFFER_SIZE, quad_vertices(uvec2::zero(), uvec2::identity())),
      m_circle_instances(GK_BUFFER_SIZE, quad_vertices(uvec2::zero(), uvec2::identity())),
      m_rect_instances(GK_BUFFER_SIZE, quad_vertices(uvec2::zero(), uvec2::identity())),
      m_image_instances(GK_SMALL_BUFFER_SIZE, quad_vertices(uvec2::zero(), uvec2::identity())),
      m_framebuffer(ivec2::identity(), true)
{
  std::unique_ptr<GPU::TileVertexArray> tile_vertex_array = std::make_unique<GPU::TileVertexArray>(
      m_programs.tile_program, m_batch.tiles.vertex_buffer, m_batch.tiles.index_buffer);

  std::unique_ptr<GPU::FillVertexArray> fill_vertex_array = std::make_unique<GPU::FillVertexArray>(
      m_programs.fill_program, m_batch.fills.vertex_buffer, m_batch.fills.index_buffer);

  std::unique_ptr<GPU::LineVertexArray> line_vertex_array = std::make_unique<GPU::LineVertexArray>(
      m_programs.line_program, m_line_instances.instance_buffer, m_line_instances.vertex_buffer);

  std::unique_ptr<GPU::RectVertexArray> rect_vertex_array = std::make_unique<GPU::RectVertexArray>(
      m_programs.rect_program, m_rect_instances.instance_buffer, m_rect_instances.vertex_buffer);

  std::unique_ptr<GPU::CircleVertexArray> circle_vertex_array =
      std::make_unique<GPU::CircleVertexArray>(m_programs.circle_program,
                                               m_circle_instances.instance_buffer,
                                               m_circle_instances.vertex_buffer);

  std::unique_ptr<GPU::ImageVertexArray> image_vertex_array =
      std::make_unique<GPU::ImageVertexArray>(m_programs.image_program,
                                              m_image_instances.instance_buffer,
                                              m_image_instances.vertex_buffer);

  m_vertex_arrays = GPU::VertexArrays{std::move(tile_vertex_array),
                                      std::move(fill_vertex_array),
                                      std::move(line_vertex_array),
                                      std::move(rect_vertex_array),
                                      std::move(circle_vertex_array),
                                      std::move(image_vertex_array)};
}

void Renderer::draw_transformed(const geom::Path<float, std::enable_if<true>>& path,
                                const rect& bounding_rect,
                                const Fill* fill,
                                const Stroke* stroke)
{
  const bool culling = bounding_rect.area() * m_viewport.zoom * m_viewport.zoom > 18.0f * 18.0f;

  if (fill) {
    geom::cubic_path cubic_path = path.to_cubic_path();

    if (!cubic_path.closed()) {
      cubic_path.line_to(cubic_path.front());
    }

    draw_cubic_path(cubic_path, bounding_rect, *fill, culling);
  }

  if (stroke) {
    Fill stroke_fill{stroke->color, FillRule::NonZero, stroke->z_index};
    geom::StrokingOptions<float> stroking_options{
        stroke->width, stroke->miter_limit, stroke->cap, stroke->join};

    // TODO: remove identity transform from path builder
    geom::StrokeOutline<float> stroke_path = geom::PathBuilder(
                                                 path, mat2x3::identity(), &bounding_rect, true)
                                                 .stroke(stroking_options, 0.0001f);

    if (stroke_path.inner.empty()) {
      draw_cubic_path(stroke_path.outer, stroke_path.bounding_rect, stroke_fill, culling);
    } else {
      draw_cubic_paths({&stroke_path.outer, &stroke_path.inner},
                       stroke_path.bounding_rect,
                       stroke_fill,
                       culling);
    }
  }
}

bool Renderer::draw_cubic_path_impl(PathDrawable& drawable)
{
  TileBatchData& tiles = m_batch.tiles;

  /* Bands count is always between 1 and 16, based on the viewport coverage. */

  // TODO: this number should be different from the culling bands
  drawable.horizontal_bands = std::clamp(
      static_cast<int>(drawable.bounds_size.y * m_viewport.zoom / GK_VIEWPORT_BANDS_HEIGHT),
      1,
      64);
  drawable.band_delta = drawable.bounds_size.y / drawable.horizontal_bands;

  /* Sort curves by descending max x. */

  std::vector<uint16_t> h_indices(drawable.length);

  std::iota(h_indices.begin(), h_indices.end(), 0);
  std::sort(h_indices.begin(), h_indices.end(), [&](const uint16_t a, const uint16_t b) {
    return drawable.max[a].x > drawable.max[b].x;
  });

  /* Calculate band metrics. */

  float band_min = drawable.bounding_rect.min.y;
  float band_max = drawable.bounding_rect.min.y + drawable.band_delta;

  /* Preallocate horizontal bands header. */

  tiles.bands_ptr += drawable.horizontal_bands * 2;

  /* Determine which curves are in each horizontal band. */

  for (uint8_t i = 0; i < drawable.horizontal_bands; i++) {
    const size_t band_start = tiles.bands_ptr - tiles.bands;

    for (uint16_t j = 0; j < h_indices.size(); j++) {
      const float min_y = drawable.min[h_indices[j]].y;
      const float max_y = drawable.max[h_indices[j]].y;

      if (min_y == max_y || min_y > band_max || max_y < band_min) {
        continue;
      }

      *(tiles.bands_ptr++) = h_indices[j];
    }

    /* Each band header is an offset from this path's bands data start and the number of curves in
     * the band. */

    const size_t band_end = tiles.bands_ptr - tiles.bands;

    tiles.bands[drawable.bands_start_index + i * 2] = static_cast<uint16_t>(
        band_start - drawable.bands_start_index);
    tiles.bands[drawable.bands_start_index + i * 2 + 1] = static_cast<uint16_t>(band_end -
                                                                                band_start);

    band_min += drawable.band_delta;
    band_max += drawable.band_delta;
  }

  // TODO: check if direct rendering is an option
  // TODO: atlasing (when rendering text merge characters together)
  // TODO: adjust instance buffer sizes and flush when necessary
  // TODO: masking (decide whether to use CPU or GPU)

  /* Push whole instance if culling is disabled. */

  if (!drawable.culling) {
    const uvec4 color = uvec4(drawable.fill.color * 255.0f);
    const uint32_t attr_1 = TileVertex::create_attr_1(0, 0, drawable.curves_start_index);
    const uint32_t attr_2 = TileVertex::create_attr_2(
        drawable.fill.z_index, false, drawable.fill.rule == FillRule::EvenOdd, 0);
    const uint32_t attr_3 = TileVertex::create_attr_3(drawable.horizontal_bands,
                                                      drawable.bands_start_index);

    (*tiles.vertices_ptr++) = TileVertex{
        {drawable.bounding_rect.min.x, drawable.bounding_rect.min.y},
        color,
        {0.0f, 0.0f},
        {0.0f, 0.0f},
        attr_1,
        attr_2,
        attr_3};
    (*tiles.vertices_ptr++) = TileVertex{
        {drawable.bounding_rect.max.x, drawable.bounding_rect.min.y},
        color,
        {1.0f, 0.0f},
        {1.0f, 0.0f},
        attr_1,
        attr_2,
        attr_3};
    (*tiles.vertices_ptr++) = TileVertex{
        {drawable.bounding_rect.max.x, drawable.bounding_rect.max.y},
        color,
        {1.0f, 1.0f},
        {1.0f, 1.0f},
        attr_1,
        attr_2,
        attr_3};
    (*tiles.vertices_ptr++) = TileVertex{
        {drawable.bounding_rect.min.x, drawable.bounding_rect.max.y},
        color,
        {0.0f, 1.0f},
        {0.0f, 1.0f},
        attr_1,
        attr_2,
        attr_3};

    return true;
  }

  return false;
}

void Renderer::draw_cubic_path_cull(PathDrawable& drawable,
                                    PathCullingData& data,
                                    const geom::CubicPath<float, std::enable_if<true>>& path)
{
  for (uint16_t i = 0; i < path.size(); i++) {
    /* Being monotonic, it is straightforward to determine which bands the curve intersects. */

    const float start_band_factor = (drawable.min[data.accumulator + i].y -
                                     drawable.bounding_rect.min.y) /
                                    drawable.band_delta;
    const float end_band_factor = (drawable.max[data.accumulator + i].y -
                                   drawable.bounding_rect.min.y) /
                                  drawable.band_delta;
    const int start_band = std::clamp(
        static_cast<int>(start_band_factor), 0, drawable.horizontal_bands - 1);
    const int end_band = std::clamp(
        static_cast<int>(end_band_factor), 0, drawable.horizontal_bands - 1);

    if (start_band >= end_band) {
      /* Curve is within one band. */
      data.bands[start_band].push_curve(drawable.min[data.accumulator + i].x,
                                        drawable.max[data.accumulator + i].x);
      continue;
    }

    /* Calculate intersections with band boundaries. */

    const vec2 p0 = path[i * 3];
    const vec2 p1 = path[i * 3 + 1];
    const vec2 p2 = path[i * 3 + 2];
    const vec2 p3 = path[i * 3 + 3];

    const auto& [a, b, c, d] = geom::cubic_coefficients(p0, p1, p2, p3);

    const bool b01 = std::abs(p1.x - p0.x) + std::abs(p1.y - p0.y) <
                     math::geometric_epsilon<float>;
    const bool b12 = std::abs(p2.x - p1.x) + std::abs(p2.y - p1.y) <
                     math::geometric_epsilon<float>;
    const bool b23 = std::abs(p3.x - p2.x) + std::abs(p3.y - p2.y) <
                     math::geometric_epsilon<float>;

    const bool linear = (b01 && (b23 || b12)) || (b23 && b12);

    std::optional<float> last_intersection = std::nullopt;

    for (int j = start_band; j <= end_band; j++) {
      const float band_top = drawable.bounding_rect.min.y + j * drawable.band_delta -
                             math::geometric_epsilon<float>;
      const float band_bottom = drawable.bounding_rect.min.y + (j + 1) * drawable.band_delta +
                                math::geometric_epsilon<float>;

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
      const float y = drawable.bounding_rect.min.y + (j + 1) * drawable.band_delta;
      const bool is_downwards = p0.y > y || p3.y < y;

      if ((is_downwards && ((p0.y < y && p3.y <= y) || (p0.y > y && p3.y >= y))) ||
          (!is_downwards && ((p0.y <= y && p3.y < y) || (p0.y >= y && p3.y > y))))
      {
        /* Curve does not intersect the band. */
        data.bands[j].push_curve(clipped_min, clipped_max);
        continue;
      }

      const float t0 = (y - p0.y) / (p3.y - p0.y);

      if (linear) {
        if (t0 >= -math::geometric_epsilon<float> && t0 <= 1.0f + math::geometric_epsilon<float>) {
          const float x = p0.x + t0 * (p3.x - p0.x);

          last_intersection = x;

          clipped_min = std::min(clipped_min, x);
          clipped_max = std::max(clipped_max, x);

          data.bottom_intersections[j].push_back({x, is_downwards});
        }
      } else {
        const float t = geom::cubic_line_intersect_approx(a.y, b.y, c.y, d.y, y, t0);

        if (t >= -math::geometric_epsilon<float> && t <= 1.0f + math::geometric_epsilon<float>) {
          const float t_sq = t * t;
          const float x = a.x * t_sq * t + b.x * t_sq + c.x * t + d.x;

          last_intersection = x;

          clipped_min = std::min(clipped_min, x);
          clipped_max = std::max(clipped_max, x);

          data.bottom_intersections[j].push_back({x, is_downwards});
        }
      }

      data.bands[j].push_curve(clipped_min, clipped_max);
    }
  }

  data.accumulator += path.size();
}

void Renderer::draw_cubic_path_cull_commit(PathDrawable& drawable, PathCullingData& data)
{
  TileBatchData& tiles = m_batch.tiles;
  FillBatchData& fills = m_batch.fills;

  const uvec4 color = uvec4(drawable.fill.color * 255.0f);
  const uint32_t attr_1 = TileVertex::create_attr_1(0, 0, drawable.curves_start_index);
  const uint32_t attr_2 = TileVertex::create_attr_2(
      drawable.fill.z_index, false, drawable.fill.rule == FillRule::EvenOdd, 0);
  const uint32_t attr_3 = TileVertex::create_attr_3(drawable.horizontal_bands,
                                                    drawable.bands_start_index);

  /* The last band cannot have filled spans. */

  data.bottom_intersections.back().clear();

  /* Calculate filled spans. */

  for (int i = 0; i < data.bands.size(); i++) {
    const Band& band = data.bands[i];
    const float top_y = drawable.bounding_rect.min.y + i * drawable.band_delta;
    const float bottom_y = drawable.bounding_rect.min.y + (i + 1) * drawable.band_delta;

    std::sort(data.bottom_intersections[i].begin(),
              data.bottom_intersections[i].end(),
              [&](const Intersection& a, const Intersection& b) { return a.x < b.x; });

    int winding = 0;
    int winding_k = 0;

    if (!band.disabled_spans.empty()) {
      const Span& span = band.disabled_spans.front();

      const vec2 span_min = {span.min, top_y};
      const vec2 span_max = {span.max, bottom_y};
      const vec2 tex_coord_min = (span_min - drawable.bounding_rect.min) / drawable.bounds_size;
      const vec2 tex_coord_max = (span_max - drawable.bounding_rect.min) / drawable.bounds_size;

      (*tiles.vertices_ptr++) = TileVertex{{span_min.x, span_min.y},
                                           color,
                                           {0.0f, 0.0f},
                                           {tex_coord_min.x, tex_coord_min.y},
                                           attr_1,
                                           attr_2,
                                           attr_3};
      (*tiles.vertices_ptr++) = TileVertex{{span_max.x, span_min.y},
                                           color,
                                           {0.0f, 0.0f},
                                           {tex_coord_max.x, tex_coord_min.y},
                                           attr_1,
                                           attr_2,
                                           attr_3};
      (*tiles.vertices_ptr++) = TileVertex{{span_max.x, span_max.y},
                                           color,
                                           {0.0f, 0.0f},
                                           {tex_coord_max.x, tex_coord_max.y},
                                           attr_1,
                                           attr_2,
                                           attr_3};
      (*tiles.vertices_ptr++) = TileVertex{{span_min.x, span_max.y},
                                           color,
                                           {0.0f, 0.0f},
                                           {tex_coord_min.x, tex_coord_max.y},
                                           attr_1,
                                           attr_2,
                                           attr_3};
    }

    for (int j = 0; j < static_cast<int>(band.disabled_spans.size()) - 1; j++) {
      const Span& span1 = band.disabled_spans[j];
      const Span& span2 = band.disabled_spans[j + 1];

      const vec2 span_min = {span2.min, top_y};
      const vec2 span_max = {span2.max, bottom_y};
      const vec2 tex_coord_min = (span_min - drawable.bounding_rect.min) / drawable.bounds_size;
      const vec2 tex_coord_max = (span_max - drawable.bounding_rect.min) / drawable.bounds_size;

      (*tiles.vertices_ptr++) = TileVertex{{span_min.x, span_min.y},
                                           color,
                                           {0.0f, 0.0f},
                                           {tex_coord_min.x, tex_coord_min.y},
                                           attr_1,
                                           attr_2,
                                           attr_3};
      (*tiles.vertices_ptr++) = TileVertex{{span_max.x, span_min.y},
                                           color,
                                           {0.0f, 0.0f},
                                           {tex_coord_max.x, tex_coord_min.y},
                                           attr_1,
                                           attr_2,
                                           attr_3};
      (*tiles.vertices_ptr++) = TileVertex{{span_max.x, span_max.y},
                                           color,
                                           {0.0f, 0.0f},
                                           {tex_coord_max.x, tex_coord_max.y},
                                           attr_1,
                                           attr_2,
                                           attr_3};
      (*tiles.vertices_ptr++) = TileVertex{{span_min.x, span_max.y},
                                           color,
                                           {0.0f, 0.0f},
                                           {tex_coord_min.x, tex_coord_max.y},
                                           attr_1,
                                           attr_2,
                                           attr_3};

      for (; winding_k < data.bottom_intersections[i].size(); winding_k++) {
        if (data.bottom_intersections[i][winding_k].x > (span1.max + span2.min) * 0.5f) {
          break;
        }

        winding -= int(data.bottom_intersections[i][winding_k].downwards) * 2 - 1;
      }

      if (drawable.fill.rule == FillRule::NonZero ? (winding != 0) : (winding % 2 != 0)) {
        (*fills.vertices_ptr++) = FillVertex{
            {span1.max, top_y}, color, {0.0f, 0.0f}, attr_1, attr_2};
        (*fills.vertices_ptr++) = FillVertex{
            {span2.min, top_y}, color, {0.0f, 0.0f}, attr_1, attr_2};
        (*fills.vertices_ptr++) = FillVertex{
            {span2.min, bottom_y}, color, {0.0f, 0.0f}, attr_1, attr_2};
        (*fills.vertices_ptr++) = FillVertex{
            {span1.max, bottom_y}, color, {0.0f, 0.0f}, attr_1, attr_2};
      }
    }
  }
}

void Renderer::draw_cubic_path(const geom::CubicPath<float, std::enable_if<true>>& path,
                               const rect& bounding_rect,
                               const Fill& fill,
                               const bool culling)
{
  TileBatchData& tiles = m_batch.tiles;

  /* Starting indices for this path. */

  const size_t length = path.size();

  // TODO: true overflow handling when caching is implemented (so that we know beforehand if we
  // need to flush, its easier)
  if (!tiles.can_handle_curves(length) || !tiles.can_handle_quads(length * 10)) {
    flush_meshes();
  }

  PathDrawable drawable{bounding_rect,
                        bounding_rect.size(),
                        fill,
                        tiles.curves_count(),
                        tiles.bands_count(),
                        length,
                        culling,
                        std::vector<vec2>(length),
                        std::vector<vec2>(length),
                        0,
                        0.0f};

  /* Copy the curves, and cache min_max values. */

  for (uint16_t i = 0; i < length; i++) {
    const vec2 p0 = path[i * 3];
    const vec2 p1 = path[i * 3 + 1];
    const vec2 p2 = path[i * 3 + 2];
    const vec2 p3 = path[i * 3 + 3];

    *(tiles.curves_ptr++) = (p0 - bounding_rect.min) / drawable.bounds_size;
    *(tiles.curves_ptr++) = (p1 - bounding_rect.min) / drawable.bounds_size;
    *(tiles.curves_ptr++) = (p2 - bounding_rect.min) / drawable.bounds_size;
    *(tiles.curves_ptr++) = (p3 - bounding_rect.min) / drawable.bounds_size;

    drawable.min[i] = math::min(p0, p3);
    drawable.max[i] = math::max(p0, p3);
  }

  /* Draw the path if not culled. */

  if (draw_cubic_path_impl(drawable)) {
    return;
  }

  /* Calculate culling data. */

  PathCullingData culling_data(drawable.horizontal_bands);

  draw_cubic_path_cull(drawable, culling_data, path);
  draw_cubic_path_cull_commit(drawable, culling_data);
}

void Renderer::draw_cubic_paths(
    const std::vector<const geom::CubicPath<float, std::enable_if<true>>*>& paths,
    const rect& bounding_rect,
    const Fill& fill,
    const bool culling)
{
  TileBatchData& tiles = m_batch.tiles;

  /* Starting indices for this path. */

  const size_t length = std::accumulate(
      paths.begin(), paths.end(), 0, [](const size_t acc, const auto* p) {
        return acc + p->size();
      });

  // TODO: true overflow handling when caching is implemented (so that we know beforehand if we
  // need to flush, its easier)
  if (!tiles.can_handle_curves(length) || !tiles.can_handle_quads(length * 10)) {
    flush_meshes();
  }

  PathDrawable drawable{bounding_rect,
                        bounding_rect.size(),
                        fill,
                        tiles.curves_count(),
                        tiles.bands_count(),
                        length,
                        culling,
                        std::vector<vec2>(length),
                        std::vector<vec2>(length),
                        0,
                        0.0f};

  /* Copy the curves, and cache min_max values. */

  size_t accumulator = 0;

  for (const auto* path : paths) {
    for (uint16_t i = 0; i < path->size(); i++) {
      const vec2 p0 = path->at(i * 3);
      const vec2 p1 = path->at(i * 3 + 1);
      const vec2 p2 = path->at(i * 3 + 2);
      const vec2 p3 = path->at(i * 3 + 3);

      *(tiles.curves_ptr++) = (p0 - bounding_rect.min) / drawable.bounds_size;
      *(tiles.curves_ptr++) = (p1 - bounding_rect.min) / drawable.bounds_size;
      *(tiles.curves_ptr++) = (p2 - bounding_rect.min) / drawable.bounds_size;
      *(tiles.curves_ptr++) = (p3 - bounding_rect.min) / drawable.bounds_size;

      drawable.min[accumulator + i] = math::min(p0, p3);
      drawable.max[accumulator + i] = math::max(p0, p3);
    }

    accumulator += path->size();
  }

  /* Draw the path if not culled. */

  if (draw_cubic_path_impl(drawable)) {
    return;
  }

  /* Calculate culling data. */

  PathCullingData culling_data(drawable.horizontal_bands);

  for (const auto* path : paths) {
    draw_cubic_path_cull(drawable, culling_data, *path);
  }

  draw_cubic_path_cull_commit(drawable, culling_data);
}

void Renderer::draw_outline_vertices(const geom::Path<float, std::enable_if<true>>& path,
                                     const mat2x3& transform,
                                     const float tolerance,
                                     const std::unordered_set<uint32_t>* selected_vertices,
                                     const Stroke* stroke)
{
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
      });
}

void Renderer::flush_cache()
{
  /* Bind the default framebuffer. */
  get()->m_framebuffer.unbind();

  /* Setup the viewport. */
  GPU::Device::set_viewport(m_viewport.size);
  GPU::Device::clear({m_viewport.background, 1.0f, 0});

  if (!m_cached_rendering)
    return;

  const rect cached = get()->m_last_viewport.visible();
  const rect visible = get()->m_viewport.visible();

  m_safe_clip_rect = cached;

  /* Enlarge the clipping region to the avoid elements at the edges being rerendered. */
  if (cached.min.y <= visible.min.y)
    m_safe_clip_rect.min.y = std::numeric_limits<float>::lowest();
  if (cached.min.x <= visible.min.x)
    m_safe_clip_rect.min.x = std::numeric_limits<float>::lowest();
  if (cached.max.y >= visible.max.y)
    m_safe_clip_rect.max.y = std::numeric_limits<float>::max();
  if (cached.max.x >= visible.max.x)
    m_safe_clip_rect.max.x = std::numeric_limits<float>::max();

  if (m_framebuffer.complete && geom::does_rect_intersect_rect(cached, visible)) {
    GPU::RenderState render_state;

    m_image_instances.instances.push_back({cached.center(), cached.size()});

    render_state = RENDER_STATE(image).no_blend().no_depth().add_stencil();
    render_state.textures = std::vector<GPU::TextureBinding>{
        {m_programs.image_program.image_texture, &m_framebuffer.texture}};
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

  if (m_framebuffer.size() != m_viewport.size) {
    m_framebuffer = GPU::Framebuffer(m_viewport.size, true);
  }

  if (m_framebuffer.complete) {
    GPU::Device::blit_framebuffer(m_framebuffer,
                                  irect{ivec2::zero(), m_framebuffer.texture.size},
                                  irect{ivec2::zero(), m_viewport.size},
                                  true);
  }
}

void Renderer::flush_meshes()
{
  TileBatchData& tiles = m_batch.tiles;
  FillBatchData& fills = m_batch.fills;
  BatchData& data = m_batch.data;

  GPU::RenderState state;

  // Render the fills.

  // TODO: maybe reverse the fills
  fills.vertex_buffer.upload(fills.vertices, fills.vertices_count() * sizeof(FillVertex));

  state = GPU::RenderState().no_blend().default_depth().no_stencil();

  state.program = m_programs.fill_program.program;
  state.vertex_array = &m_vertex_arrays.fill_vertex_array->vertex_array;
  state.primitive = fills.primitive;
  state.viewport = irect{ivec2::zero(), m_viewport.size};

  state.uniforms = {{m_programs.fill_program.vp_uniform, m_vp_matrix}};
  state.texture_arrays = std::vector<GPU::TextureArrayBinding>{
      {m_programs.tile_program.textures_uniform, {&data.gradients_texture}}};

  GPU::Device::draw_elements(fills.indices_count(), state);

  // Render the tiles.

  tiles.vertex_buffer.upload(tiles.vertices, tiles.vertices_count() * sizeof(TileVertex));

  // TODO: upload only the portion of the buffer that is in use
  tiles.curves_texture.upload(tiles.curves, tiles.max_curves * sizeof(vec2));
  tiles.bands_texture.upload(tiles.bands, tiles.max_bands * sizeof(uint16_t));

  state = GPU::RenderState().default_blend().no_depth_write().no_stencil();

  state.program = m_programs.tile_program.program;
  state.vertex_array = &m_vertex_arrays.tile_vertex_array->vertex_array;
  state.primitive = tiles.primitive;
  state.viewport = irect{ivec2::zero(), m_viewport.size};

  state.uniforms = {{m_programs.tile_program.vp_uniform, m_vp_matrix},
                    {m_programs.tile_program.samples_uniform, 3}};
  state.textures = std::vector<GPU::TextureBinding>{
      {m_programs.tile_program.bands_texture_uniform, &tiles.bands_texture},
      {m_programs.tile_program.curves_texture_uniform, &tiles.curves_texture}};
  state.texture_arrays = std::vector<GPU::TextureArrayBinding>{
      {m_programs.tile_program.textures_uniform, {&data.gradients_texture}}};

  GPU::Device::draw_elements(tiles.indices_count(), state);

  m_batch.clear();
}

void Renderer::flush_overlay_meshes()
{
  GPU::RenderState render_state;

  render_state = RENDER_STATE(line).default_blend().no_depth().no_stencil();
  render_state.uniforms = {
      {m_programs.line_program.vp_uniform, m_vp_matrix},
      {m_programs.line_program.zoom_uniform, static_cast<float>(m_viewport.zoom)}};

  flush(m_line_instances, render_state);

  render_state = RENDER_STATE(rect).default_blend().no_depth().no_stencil();
  render_state.uniforms = {{m_programs.rect_program.vp_uniform, m_vp_matrix}};

  flush(m_rect_instances, render_state);

  render_state = RENDER_STATE(circle).default_blend().no_depth().no_stencil();
  render_state.uniforms = {
      {m_programs.circle_program.vp_uniform, m_vp_matrix},
      {m_programs.circle_program.zoom_uniform, static_cast<float>(m_viewport.zoom)}};

  flush(m_circle_instances, render_state);

  render_state = RENDER_STATE(image).default_blend().no_depth().no_stencil();
  render_state.uniforms = {{m_programs.image_program.vp_uniform, m_vp_matrix}};

  flush(m_image_instances, render_state);
}

}  // namespace graphick::renderer
