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

// TODO: this is temp, the renderer should not know about the editor
#include "../editor/scene/cache.h"
#include "../editor/scene/components/text.h"

#include "../geom/intersections.h"
#include "../geom/path.h"
#include "../geom/path_builder.h"

#include "../io/resource_manager.h"
#include "../io/text/font.h"
#include "../io/text/unicode.h"

#include "../math/matrix.h"
#include "../math/vector.h"

#include "../utils/assert.h"
#include "../utils/console.h"
#include "../utils/defines.h"

#include <algorithm>

#ifdef EMSCRIPTEN
#  include <emscripten/html5.h>
#endif

namespace graphick::renderer {

void Renderer::init() {}

void Renderer::shutdown() {}

void Renderer::begin_frame(const RenderOptions& options) {}

void Renderer::end_frame() {}

bool Renderer::draw(const geom::path& path,
                    const mat2x3& transform,
                    const DrawingOptions& options,
                    const uuid)
{
  return false;
}

bool Renderer::draw(const renderer::Text& text,
                    const mat2x3& transform,
                    const DrawingOptions& options,
                    const uuid)
{
  return false;
}

bool Renderer::draw(const renderer::Image& image,
                    const mat2x3& transform,
                    const DrawingOptions& options,
                    const uuid)
{
  return false;
}

void Renderer::ui_rect(const rect& rect, const vec4& color) {}

void Renderer::ui_square(const vec2 center, const float radius, const vec4& color) {}

void Renderer::ui_circle(const vec2 center, const float radius, const vec4& color) {}

#ifdef GK_DEBUG

void Renderer::__debug_rect_impl(const rect& rect, const vec4& color) {}

void Renderer::__debug_square_impl(const vec2 center, const float radius, const vec4& color) {}

void Renderer::__debug_circle_impl(const vec2 center, const float radius, const vec4& color) {}

void Renderer::__debug_line_impl(const vec2 start, const vec2 end, const vec4& color) {}

void Renderer::__debug_text_impl(const std::string& text, const vec2 position, const vec4& color)
{
}

#endif

Renderer::Renderer() {}

}  // namespace graphick::renderer

#if 0

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

#  define RENDER_STATE(name) \
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

#  ifdef EMSCRIPTEN
  EmscriptenWebGLContextAttributes attr;
  emscripten_webgl_init_context_attributes(&attr);

  /* Despite
   * <https://developer.mozilla.org/en-US/docs/Web/API/WebGL_API/WebGL_best_practices#avoid_alphafalse_which_can_be_expensive>,
   * when using desynchronized context (essential for responsive input), is better to use
   * alpha=false to avoid expensive blending operations if there are elements on top.
   * Emscripten doesn't support desynchronized context yet, so we set it to true after compilation
   * (see script compile.py).
   */
  attr.alpha = false;
  attr.premultipliedAlpha = false;
  attr.majorVersion = 2;
  attr.antialias = false;
  attr.stencil = true;
  attr.depth = true;

  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context("#canvas", &attr);
  emscripten_webgl_make_context_current(ctx);

  GPU::Device::init(GPU::DeviceVersion::GLES3);
#  else
  GPU::Device::init(GPU::DeviceVersion::GL3);
#  endif

  s_instance = new Renderer();
}

void Renderer::shutdown()
{
  GK_ASSERT(s_instance != nullptr, "Renderer not initialized, call init() before shutting down!");

  delete s_instance;
  s_instance = nullptr;

  GPU::Device::shutdown();

#  ifdef EMSCRIPTEN
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_get_current_context();
  emscripten_webgl_destroy_context(ctx);
#  endif
}

void Renderer::begin_frame(const RenderOptions& options)
{
  const dmat4 view_matrix = orthographic_translation(options.viewport);
  const dmat4 projection_matrix = orthographic_projection(options.viewport);

  get()->m_last_viewport = get()->m_viewport;
  get()->m_viewport = options.viewport;
  get()->m_vp_matrix = projection_matrix * view_matrix;
  get()->m_cache = &options.cache->renderer_cache;
  get()->m_cached_rendering = false;  // get()->m_cached_rendering = !options.ignore_cache;
  get()->m_ui_options = UIOptions(options.viewport.dpr / options.viewport.zoom);
  get()->m_z_index = 0;

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

bool Renderer::draw(const geom::path& path, const mat2x3& transform, const DrawingOptions& options)
{
  if (path.empty() ||
      (options.fill == nullptr && options.stroke == nullptr && options.outline == nullptr))
  {
    return false;
  }

  const bool has_transform = !math::is_identity(transform);

  drect bounding_rect{};

  // TODO: extract into functions (there is duplicated code in the draw(..., id) method)
  if (has_transform && options.fill &&
      (options.fill->paint.is_gradient() || options.fill->paint.is_texture()))
  {
    geom::dpath transformed_path = path.transformed<double>(transform, bounding_rect);

    rect raw_bounding_rect = path.bounding_rect();

    const dmat2x3 inverse_transform = math::inverse(dmat2x3(transform));

    const auto& [v0_t, v1_t, v2_t, v3_t] = bounding_rect.vertices();

    const dvec2 v0 = inverse_transform * v0_t;
    const dvec2 v1 = inverse_transform * v1_t;
    const dvec2 v2 = inverse_transform * v2_t;
    const dvec2 v3 = inverse_transform * v3_t;

    const vec2 size = raw_bounding_rect.size();

    const vec2 tex0 = vec2(v0) / size;
    const vec2 tex1 = vec2(v1) / size;
    const vec2 tex2 = vec2(v2) / size;
    const vec2 tex3 = vec2(v3) / size;

    return get()->draw_transformed(
        transformed_path, bounding_rect, options, {tex0, tex1, tex2, tex3});
  }

  if (!has_transform) {
    geom::dpath dpath = geom::dpath(path);
    bounding_rect = dpath.bounding_rect();
    dpath.bounding_rect();

    return get()->draw_transformed(
        dpath,
        bounding_rect,
        options,
        {vec2::zero(), vec2(1.0, 0.0), vec2::identity(), vec2(0.0, 1.0)});
  } else {
    geom::dpath dpath = path.transformed<double>(transform, bounding_rect);

    return get()->draw_transformed(
        dpath,
        bounding_rect,
        options,
        {vec2::zero(), vec2(1.0, 0.0), vec2::identity(), vec2(0.0, 1.0)});
  }
}

// TODO: if stroking is the bottleneck when manipulating a path, consider approximate stroking
// during manipulation
// TODO: if manipulation is only translation, consider translating the cached
// drawable
bool Renderer::draw(const geom::path& path,
                    const mat2x3& transform,
                    const DrawingOptions& options,
                    const uuid id)
{
  GK_TOTAL("-----------------------DRAW-----------------------");

  if (path.empty() ||
      (options.fill == nullptr && options.stroke == nullptr && options.outline == nullptr))
  {
    return false;
  }

  const bool has_transform = !math::is_identity(transform);
  const bool outline_only = options.fill == nullptr && options.stroke == nullptr &&
                            options.outline;
  const bool cached = !outline_only && get()->m_cache->has_bounding_rect(id) &&
                      get()->m_cache->has_drawable(id);

  if (cached) {
    const drect& bounding_rect = get()->m_cache->get_bounding_rect(id);

    if (!geom::does_rect_intersect_rect(bounding_rect, get()->m_viewport.visible())) {
      return false;
    }

    get()->draw(get()->m_cache->get_drawable(id));

    if (options.outline) {
      const DrawingOptions outline_options{nullptr, nullptr, options.outline};

      geom::dpath transformed_path = has_transform ? path.transformed<double>(transform) :
                                                     geom::dpath(path);

      get()->draw_transformed(transformed_path,
                              bounding_rect,
                              outline_options,
                              {vec2::zero(), vec2(1.0, 0.0), vec2::identity(), vec2(0.0, 1.0)});
    }

    return true;
  }

  geom::dpath transformed_path;
  drect bounding_rect;

  if (has_transform) {
    transformed_path = path.transformed<double>(transform, bounding_rect);
  } else {
    transformed_path = geom::dpath(path);
    bounding_rect = transformed_path.bounding_rect();
  }

  const double radius = options.stroke ? (0.5 * options.stroke->width *
                                          (options.stroke->join == LineJoin::Miter ?
                                               std::max(1.0, options.stroke->miter_limit) :
                                               1.0)) :
                                         0.0;

  if (!geom::does_rect_intersect_rect(drect::expand(bounding_rect, radius),
                                      get()->m_viewport.visible()))
  {
    return false;
  }

  // TODO: extract into functions (there is duplicated code in the draw(..., id) method)
  if (has_transform && options.fill &&
      (options.fill->paint.is_gradient() || options.fill->paint.is_texture()))
  {
    rect raw_bounding_rect = path.bounding_rect();

    const dmat2x3 inverse_transform = math::inverse(dmat2x3(transform));

    const auto& [v0_t, v1_t, v2_t, v3_t] = bounding_rect.vertices();

    const dvec2 v0 = inverse_transform * v0_t;
    const dvec2 v1 = inverse_transform * v1_t;
    const dvec2 v2 = inverse_transform * v2_t;
    const dvec2 v3 = inverse_transform * v3_t;

    const vec2 size = raw_bounding_rect.size();

    const vec2 tex0 = vec2(v0) / size;
    const vec2 tex1 = vec2(v1) / size;
    const vec2 tex2 = vec2(v2) / size;
    const vec2 tex3 = vec2(v3) / size;

    return get()->draw_transformed(
        transformed_path, bounding_rect, options, {tex0, tex1, tex2, tex3}, id);
  }

  return get()->draw_transformed(transformed_path,
                                 bounding_rect,
                                 options,
                                 {vec2::zero(), vec2(1.0, 0.0), vec2::identity(), vec2(0.0, 1.0)},
                                 id);
}

bool Renderer::draw(const renderer::Text& text,
                    const mat2x3& transform,
                    const DrawingOptions& options,
                    const uuid id)
{
  const io::text::Font& font = io::ResourceManager::get_font(text.font_id);
  const float font_size = 11.0f;

  Drawable drawable;

  float cursor = 0.0f;
  int prev_index = 0;

  const std::vector<int> codepoints = io::text::utf8_decode(text.text);

  for (const int codepoint : codepoints) {
    // TODO: handle line breaks, tabs, etc.
    const io::text::Glyph& glyph = font.get_glyph(codepoint);
    const float kerning = font.get_kerning(prev_index, glyph.index);

    cursor += kerning * font_size;

    if (glyph.path.empty()) {
      cursor += glyph.advance * font_size;
      drawable.bounding_rect.max.x += glyph.advance * font_size;
      prev_index = glyph.index;

      continue;
    }

    const size_t num = glyph.path.size();
    const size_t bands_offset = drawable.bands.size();
    const size_t curves_offset = drawable.curves.size() / 2;

    /* Reserve space and setup drawable. */

    drect glyph_bounds = drect(glyph.bounding_rect) * font_size + dvec2(cursor, 0.0);

    drawable.curves.reserve(drawable.curves.size() + num * 4);

    draw_rect(vec2(cursor, 0.0f), vec2(0.3f));

    /* Copy the curves, and cache min_max values. */

    std::vector<vec2> min(num);
    std::vector<vec2> max(num);

    size_t accumulated_size = 0;

    for (int k = 0; k < glyph.path.starts.size(); k++) {
      const size_t start_i = glyph.path.starts[k];
      const size_t end_i = k + 1 < glyph.path.starts.size() ? glyph.path.starts[k + 1] - 2 :
                                                              glyph.path.points.size() - 2;

      // TODO: Horizontal segments should not be added
      // TODO: already added glyphs should not be added again, offsetts should be cached
      for (size_t i = start_i; i < end_i; i += 2) {
        const vec2 p0 = glyph.path[i];
        const vec2 p1 = glyph.path[i + 1];
        const vec2 p2 = glyph.path[i + 2];

        drawable.push_curve(p0, p1, p2);

        min[(i - k) / 2] = math::min(p0, p2);
        max[(i - k) / 2] = math::max(p0, p2);
      }

      accumulated_size += end_i - start_i;
    }

    int horizontal_bands = 1;
    float band_delta = 1.0f / horizontal_bands;

    /* Sort curves by descending max x. */

    std::vector<uint16_t> h_indices(num);

    std::iota(h_indices.begin(), h_indices.end(), 0);
    std::sort(h_indices.begin(), h_indices.end(), [&](const uint16_t a, const uint16_t b) {
      return max[a].x > max[b].x;
    });

    /* Calculate band metrics. */

    float band_min = 0.0f;
    float band_max = band_delta;

    /* Preallocate horizontal bands header. */

    const size_t header_index = drawable.bands.size();
    drawable.bands.resize(drawable.bands.size() + horizontal_bands * 2);

    /* Determine which curves are in each horizontal band. */

    for (uint8_t i = 0; i < horizontal_bands; i++) {
      const size_t band_start = drawable.bands.size();

      for (uint16_t j = 0; j < h_indices.size(); j++) {
        const float min_y = min[h_indices[j]].y;
        const float max_y = max[h_indices[j]].y;

        if (min_y == max_y || min_y > band_max || max_y < band_min) {
          continue;
        }

        drawable.push_band(h_indices[j]);
      }

      /* Each band header is an offset from this path's bands data start and the number of curves
       * in the band. */

      const size_t band_end = drawable.bands.size();

      drawable.bands[header_index + i * 2] = static_cast<uint16_t>(band_start - bands_offset);
      drawable.bands[header_index + i * 2 + 1] = static_cast<uint16_t>(band_end - band_start);

      band_min += band_delta;
      band_max += band_delta;
    }

    /* Setup attributes. */

    if (options.fill) {
      const uvec4 col = uvec4(options.fill->paint.color() * 255.0f);
      const uint32_t attr_1 = TileVertex::create_attr_1(0, 0, curves_offset);
      const uint32_t attr_2 = TileVertex::create_attr_2(0, true, true, 0);
      const uint32_t attr_3 = TileVertex::create_attr_3(horizontal_bands, bands_offset);

      drawable.push_tile(glyph_bounds,
                         col,
                         {vec2::zero(), vec2(1.0f, 0.0f), vec2::identity(), vec2(0.0f, 1.0f)},
                         attr_1,
                         attr_2,
                         attr_3);
    }

    drawable.bounding_rect = drect::from_rects(drawable.bounding_rect, glyph_bounds);

    cursor += glyph.advance * font_size;
    prev_index = glyph.index;
  }

  drawable.paints.push_back(
      {drawable.tiles.size(), drawable.fills.size(), Paint::Type::ColorPaint, uuid::null});

  get()->draw(drawable);

  return true;
}

bool Renderer::draw_image(const uuid image_id, const mat2x3& transform)
{
  const io::Image& image = io::ResourceManager::get_image(image_id);

  geom::path path;
  Fill fill = {Paint(image_id, Paint::Type::TexturePaint), FillRule::NonZero};

  path.rect(vec2::zero(), vec2(image.size));

  return get()->draw(path, transform, {&fill, nullptr, nullptr});
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

bool Renderer::draw_transformed(const geom::dpath& path,
                                const drect& bounding_rect,
                                const DrawingOptions& options,
                                const std::array<vec2, 4>& fill_texture_coords,
                                const uuid id)
{
  // TODO: clip if necessary
  // if (!geom::does_rect_intersect_rect(entity_rect, visible_rect))
  //   continue;

  const bool culling = bounding_rect.area() * m_viewport.zoom * m_viewport.zoom >
                       RendererSettings::culling_threshold * RendererSettings::culling_threshold;

  Drawable drawable;

  // TODO: check if intersects visible area
  if (options.fill && options.fill->paint.visible()) {
    geom::dcubic_path cubic_path = path.to_cubic_path();

    if (!cubic_path.closed()) {
      cubic_path.line_to(cubic_path.front());
    }

    draw_cubic_path(
        cubic_path, bounding_rect, *options.fill, culling, fill_texture_coords, drawable);
  }

  // TODO: check if intersects visible area
  if (options.stroke && options.stroke->paint.visible()) {
    Fill stroke_fill{options.stroke->paint, FillRule::NonZero};

    const geom::StrokingOptions<double> stroking_options{RendererSettings::stroking_tolerance,
                                                         options.stroke->width,
                                                         options.stroke->miter_limit,
                                                         options.stroke->cap,
                                                         options.stroke->join};
    const geom::PathBuilder<double> builder = geom::PathBuilder(path, bounding_rect);
    const geom::StrokeOutline<double> stroke_path = builder.stroke(stroking_options);

    if (id != uuid::null) {
      m_cache->set_bounding_rect(id, stroke_path.bounding_rect);
    }

    // TODO: stroke texture coordinates
    if (stroke_path.inner.empty()) {
      draw_cubic_path(stroke_path.outer,
                      stroke_path.bounding_rect,
                      stroke_fill,
                      culling,
                      fill_texture_coords,
                      drawable);
    } else {
      draw_cubic_paths({&stroke_path.outer, &stroke_path.inner},
                       stroke_path.bounding_rect,
                       stroke_fill,
                       culling,
                       fill_texture_coords,
                       drawable);
    }
  } else if (id != uuid::null) {
    m_cache->set_bounding_rect(id, bounding_rect);
  }

  if (drawable.tiles.size() || drawable.fills.size()) {
    draw(drawable);

    if (id != uuid::null) {
      m_cache->set_drawable(id, std::move(drawable));
    }
  }

  if (options.outline) {
    const geom::PathBuilder<double> builder = geom::PathBuilder(path, bounding_rect);

    builder.flatten<float>(
        get()->m_viewport.visible(),
        RendererSettings::flattening_tolerance / m_viewport.zoom,
        [&](const vec2 p0, const vec2 p1) {
          get()->m_line_instances.instances.push_back(
              {p0, p1, get()->m_ui_options.line_width, get()->m_ui_options.primary_color_05});
        });

    if (options.outline->draw_vertices) {
      get()->draw_outline_vertices(path, *options.outline);
    }
  }

  return true;
}

bool Renderer::draw_cubic_path_impl(PathData& data)
{
  /* Bands count is always between 1 and 16, based on the viewport coverage. */

  // TODO: this number should be different from the culling bands
  data.horizontal_bands = std::clamp(
      static_cast<int>(data.bounds_size.y * m_viewport.zoom / GK_VIEWPORT_BANDS_HEIGHT), 1, 64);
  data.band_delta = data.bounds_size.y / data.horizontal_bands;

  /* Sort curves by descending max x. */

  std::vector<uint16_t> h_indices(data.num);

  std::iota(h_indices.begin(), h_indices.end(), 0);
  std::sort(h_indices.begin(), h_indices.end(), [&](const uint16_t a, const uint16_t b) {
    return data.max[a].x > data.max[b].x;
  });

  /* Calculate band metrics. */

  float band_min = data.bounding_rect.min.y;
  float band_max = data.bounding_rect.min.y + data.band_delta;

  /* Preallocate horizontal bands header. */

  const size_t header_index = data.drawable.bands.size();
  data.drawable.bands.resize(data.drawable.bands.size() + data.horizontal_bands * 2);

  /* Determine which curves are in each horizontal band. */

  for (uint8_t i = 0; i < data.horizontal_bands; i++) {
    const size_t band_start = data.drawable.bands.size();

    for (uint16_t j = 0; j < h_indices.size(); j++) {
      const float min_y = data.min[h_indices[j]].y;
      const float max_y = data.max[h_indices[j]].y;

      if (min_y == max_y || min_y > band_max || max_y < band_min) {
        continue;
      }

      data.drawable.push_band(h_indices[j]);
    }

    /* Each band header is an offset from this path's bands data start and the number of curves in
     * the band. */

    const size_t band_end = data.drawable.bands.size();

    data.drawable.bands[header_index + i * 2] = static_cast<uint16_t>(band_start -
                                                                      data.bands_offset);
    data.drawable.bands[header_index + i * 2 + 1] = static_cast<uint16_t>(band_end - band_start);

    band_min += data.band_delta;
    band_max += data.band_delta;
  }

  // TODO: check if direct rendering is an option
  // TODO: atlasing (when rendering text, merge characters together)
  // TODO: adjust instance buffer sizes and flush when necessary
  // TODO: masking (decide whether to use CPU or GPU)

  /* Push whole instance if culling is disabled. */

  if (!data.culling) {
    const uvec4 color = data.fill.paint.is_color() ? uvec4(data.fill.paint.color() * 255.0f) :
                                                     uvec4(255, 255, 255, 255);
    const uint32_t attr_1 = TileVertex::create_attr_1(
        0, data.fill.paint.type(), data.curves_offset);
    const uint32_t attr_2 = TileVertex::create_attr_2(
        0, false, data.fill.rule == FillRule::EvenOdd, 0);
    const uint32_t attr_3 = TileVertex::create_attr_3(data.horizontal_bands, data.bands_offset);

    data.drawable.push_tile(
        data.bounding_rect, color, data.texture_coords, attr_1, attr_2, attr_3);

    if (data.fill.paint.is_texture()) {
      const uuid texture_id = data.fill.paint.id();

      request_texture(texture_id);
    }

    data.drawable.paints.push_back({data.drawable.tiles.size(),
                                    data.drawable.fills.size(),
                                    data.fill.paint.type(),
                                    data.fill.paint.id()});

    return true;
  }

  return false;
}

void Renderer::draw_cubic_path_cull(const geom::dcubic_path& path,
                                    PathData& data,
                                    PathCullingData& culling_data)
{
  for (uint16_t i = 0; i < path.size(); i++) {
    /* Being monotonic, it is straightforward to determine which bands the curve intersects. */

    const double start_band_factor = (data.min[culling_data.accumulator + i].y -
                                      data.bounding_rect.min.y) /
                                     data.band_delta;
    const double end_band_factor = (data.max[culling_data.accumulator + i].y -
                                    data.bounding_rect.min.y) /
                                   data.band_delta;
    const int start_band = std::clamp(
        static_cast<int>(start_band_factor), 0, data.horizontal_bands - 1);
    const int end_band = std::clamp(
        static_cast<int>(end_band_factor), 0, data.horizontal_bands - 1);

    if (start_band >= end_band) {
      /* Curve is within one band. */
      culling_data.bands[start_band].push_curve(data.min[culling_data.accumulator + i].x,
                                                data.max[culling_data.accumulator + i].x);
      continue;
    }

    /* Calculate intersections with band boundaries. */

    const dvec2 p0 = path[i * 3];
    const dvec2 p1 = path[i * 3 + 1];
    const dvec2 p2 = path[i * 3 + 2];
    const dvec2 p3 = path[i * 3 + 3];

    const auto& [a, b, c, d] = geom::cubic_coefficients(p0, p1, p2, p3);

    const bool b01 = std::abs(p1.x - p0.x) + std::abs(p1.y - p0.y) <
                     math::geometric_epsilon<double>;
    const bool b12 = std::abs(p2.x - p1.x) + std::abs(p2.y - p1.y) <
                     math::geometric_epsilon<double>;
    const bool b23 = std::abs(p3.x - p2.x) + std::abs(p3.y - p2.y) <
                     math::geometric_epsilon<double>;

    const bool linear = (b01 && (b23 || b12)) || (b23 && b12);

    std::optional<double> last_intersection = std::nullopt;

    for (int j = start_band; j <= end_band; j++) {
      const double band_top = data.bounding_rect.min.y + j * data.band_delta -
                              math::geometric_epsilon<double>;
      const double band_bottom = data.bounding_rect.min.y + (j + 1) * data.band_delta +
                                 math::geometric_epsilon<double>;

      double clipped_min = std::numeric_limits<double>::infinity();
      double clipped_max = -std::numeric_limits<double>::infinity();

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
      const double y = data.bounding_rect.min.y + (j + 1) * data.band_delta;
      const bool is_downwards = p0.y > y || p3.y < y;

      if ((is_downwards && ((p0.y < y && p3.y <= y) || (p0.y > y && p3.y >= y))) ||
          (!is_downwards && ((p0.y <= y && p3.y < y) || (p0.y >= y && p3.y > y))))
      {
        /* Curve does not intersect the band. */
        culling_data.bands[j].push_curve(clipped_min, clipped_max);
        continue;
      }

      const double t0 = (y - p0.y) / (p3.y - p0.y);

      if (linear) {
        if (t0 >= -math::geometric_epsilon<double> && t0 <= 1.0 + math::geometric_epsilon<double>)
        {
          const double x = p0.x + t0 * (p3.x - p0.x);

          last_intersection = x;

          clipped_min = std::min(clipped_min, x);
          clipped_max = std::max(clipped_max, x);

          culling_data.bottom_intersections[j].push_back({x, is_downwards});
        }
      } else {
        const double t = geom::cubic_line_intersect_approx(a.y, b.y, c.y, d.y, y, t0);

        if (t >= -math::geometric_epsilon<double> && t <= 1.0 + math::geometric_epsilon<double>) {
          const double t_sq = t * t;
          const double x = a.x * t_sq * t + b.x * t_sq + c.x * t + d.x;

          last_intersection = x;

          clipped_min = std::min(clipped_min, x);
          clipped_max = std::max(clipped_max, x);

          culling_data.bottom_intersections[j].push_back({x, is_downwards});
        }
      }

      culling_data.bands[j].push_curve(clipped_min, clipped_max);
    }
  }

  culling_data.accumulator += path.size();
}

void Renderer::draw_cubic_path_cull_commit(PathData& data, PathCullingData& culling_data)
{
  const uvec4 color = data.fill.paint.is_color() ? uvec4(data.fill.paint.color() * 255.0f) :
                                                   uvec4(255, 255, 255, 255);
  const uint32_t attr_1 = TileVertex::create_attr_1(0, data.fill.paint.type(), data.curves_offset);
  const uint32_t attr_2 = TileVertex::create_attr_2(
      0, false, data.fill.rule == FillRule::EvenOdd, 0);
  const uint32_t attr_3 = TileVertex::create_attr_3(data.horizontal_bands, data.bands_offset);

  /* The last band cannot have filled spans. */

  culling_data.bottom_intersections.back().clear();

  /* Calculate filled spans. */

  for (int i = 0; i < culling_data.bands.size(); i++) {
    const Band& band = culling_data.bands[i];
    const float top_y = data.bounding_rect.min.y + i * data.band_delta;
    const float bottom_y = data.bounding_rect.min.y + (i + 1) * data.band_delta;

    std::sort(culling_data.bottom_intersections[i].begin(),
              culling_data.bottom_intersections[i].end(),
              [&](const Intersection& a, const Intersection& b) { return a.x < b.x; });

    int winding = 0;
    int winding_k = 0;

    if (!band.disabled_spans.empty()) {
      const Span& span = band.disabled_spans.front();

      const vec2 span_min = {static_cast<float>(span.min), top_y};
      const vec2 span_max = {static_cast<float>(span.max), bottom_y};

      const vec2 tex_coord_curves_min = vec2((dvec2(span_min) - data.bounding_rect.min) /
                                             data.bounds_size);
      const vec2 tex_coord_curves_max = vec2((dvec2(span_max) - data.bounding_rect.min) /
                                             data.bounds_size);

      const vec2 interp_03_min_y = math::lerp(
          data.texture_coords[0], data.texture_coords[3], tex_coord_curves_min.y);
      const vec2 interp_12_min_y = math::lerp(
          data.texture_coords[1], data.texture_coords[2], tex_coord_curves_min.y);
      const vec2 interp_03_max_y = math::lerp(
          data.texture_coords[0], data.texture_coords[3], tex_coord_curves_max.y);
      const vec2 interp_12_max_y = math::lerp(
          data.texture_coords[1], data.texture_coords[2], tex_coord_curves_max.y);

      const std::array<vec2, 4> tex_coords = {
          math::lerp(interp_03_min_y, interp_12_min_y, tex_coord_curves_min.x),
          math::lerp(interp_03_min_y, interp_12_min_y, tex_coord_curves_max.x),
          math::lerp(interp_03_max_y, interp_12_max_y, tex_coord_curves_max.x),
          math::lerp(interp_03_max_y, interp_12_max_y, tex_coord_curves_min.x)};

      data.drawable.push_tile(span_min,
                              span_max,
                              color,
                              tex_coord_curves_min,
                              tex_coord_curves_max,
                              tex_coords,
                              attr_1,
                              attr_2,
                              attr_3);
    }

    for (int j = 0; j < static_cast<int>(band.disabled_spans.size()) - 1; j++) {
      const Span& span1 = band.disabled_spans[j];
      const Span& span2 = band.disabled_spans[j + 1];

      const vec2 span_min = vec2(span2.min, top_y);
      const vec2 span_max = vec2(span2.max, bottom_y);

      const vec2 tex_coord_curves_min = vec2((dvec2(span_min) - data.bounding_rect.min) /
                                             data.bounds_size);
      const vec2 tex_coord_curves_max = vec2((dvec2(span_max) - data.bounding_rect.min) /
                                             data.bounds_size);

      const vec2 interp_03_min_y = math::lerp(
          data.texture_coords[0], data.texture_coords[3], tex_coord_curves_min.y);
      const vec2 interp_12_min_y = math::lerp(
          data.texture_coords[1], data.texture_coords[2], tex_coord_curves_min.y);
      const vec2 interp_03_max_y = math::lerp(
          data.texture_coords[0], data.texture_coords[3], tex_coord_curves_max.y);
      const vec2 interp_12_max_y = math::lerp(
          data.texture_coords[1], data.texture_coords[2], tex_coord_curves_max.y);

      const std::array<vec2, 4> tex_coords = {
          math::lerp(interp_03_min_y, interp_12_min_y, tex_coord_curves_min.x),
          math::lerp(interp_03_min_y, interp_12_min_y, tex_coord_curves_max.x),
          math::lerp(interp_03_max_y, interp_12_max_y, tex_coord_curves_max.x),
          math::lerp(interp_03_max_y, interp_12_max_y, tex_coord_curves_min.x)};

      data.drawable.push_tile(span_min,
                              span_max,
                              color,
                              tex_coord_curves_min,
                              tex_coord_curves_max,
                              tex_coords,
                              attr_1,
                              attr_2,
                              attr_3);

      for (; winding_k < culling_data.bottom_intersections[i].size(); winding_k++) {
        if (culling_data.bottom_intersections[i][winding_k].x > (span1.max + span2.min) * 0.5f) {
          break;
        }

        winding -= int(culling_data.bottom_intersections[i][winding_k].downwards) * 2 - 1;
      }

      if (data.fill.rule == FillRule::NonZero ? (winding != 0) : (winding % 2 != 0)) {
        const vec2 fill_min = vec2(span1.max, top_y);
        const vec2 fill_max = vec2(span2.min, bottom_y);

        const vec2 tex_coord_curves_min = vec2((dvec2(fill_min) - data.bounding_rect.min) /
                                               data.bounds_size);
        const vec2 tex_coord_curves_max = vec2((dvec2(fill_max) - data.bounding_rect.min) /
                                               data.bounds_size);

        const vec2 interp_03_min_y = math::lerp(
            data.texture_coords[0], data.texture_coords[3], tex_coord_curves_min.y);
        const vec2 interp_12_min_y = math::lerp(
            data.texture_coords[1], data.texture_coords[2], tex_coord_curves_min.y);
        const vec2 interp_03_max_y = math::lerp(
            data.texture_coords[0], data.texture_coords[3], tex_coord_curves_max.y);
        const vec2 interp_12_max_y = math::lerp(
            data.texture_coords[1], data.texture_coords[2], tex_coord_curves_max.y);

        const std::array<vec2, 4> tex_coords = {
            math::lerp(interp_03_min_y, interp_12_min_y, tex_coord_curves_min.x),
            math::lerp(interp_03_min_y, interp_12_min_y, tex_coord_curves_max.x),
            math::lerp(interp_03_max_y, interp_12_max_y, tex_coord_curves_max.x),
            math::lerp(interp_03_max_y, interp_12_max_y, tex_coord_curves_min.x)};

        data.drawable.push_fill(fill_min, fill_max, color, tex_coords, attr_1, attr_2);
      }
    }
  }

  if (data.fill.paint.is_texture()) {
    const uuid texture_id = data.fill.paint.id();

    request_texture(texture_id);
  }

  data.drawable.paints.push_back({data.drawable.tiles.size(),
                                  data.drawable.fills.size(),
                                  data.fill.paint.type(),
                                  data.fill.paint.id()});
}

void Renderer::draw_cubic_path(const geom::dcubic_path& path,
                               const drect& bounding_rect,
                               const Fill& fill,
                               const bool culling,
                               const std::array<vec2, 4>& texture_coords,
                               Drawable& drawable)
{
  /* Starting indices for this path. */

  const size_t num = path.size();

  PathData data{drawable,
                texture_coords,
                bounding_rect,
                bounding_rect.size(),
                fill,
                num,
                drawable.curves.size() / 2,
                drawable.bands.size(),
                culling,
                std::vector<dvec2>(num),
                std::vector<dvec2>(num),
                0.0,
                0};

  /* Reserve space and setup drawable. */

  data.drawable.bounding_rect = drect::from_rects(drawable.bounding_rect, bounding_rect);
  data.drawable.curves.reserve(drawable.curves.size() + num * 4);

  /* Copy the curves, and cache min_max values. */

  for (uint32_t i = 0; i < num; i++) {
    const dvec2 p0 = path[i * 3];
    const dvec2 p1 = path[i * 3 + 1];
    const dvec2 p2 = path[i * 3 + 2];
    const dvec2 p3 = path[i * 3 + 3];

    data.drawable.push_curve(vec2((p0 - bounding_rect.min) / data.bounds_size),
                             vec2((p1 - bounding_rect.min) / data.bounds_size),
                             vec2((p2 - bounding_rect.min) / data.bounds_size),
                             vec2((p3 - bounding_rect.min) / data.bounds_size));

    data.min[i] = math::min(p0, p3);
    data.max[i] = math::max(p0, p3);
  }

  /* Draw the path if not culled. */

  if (draw_cubic_path_impl(data)) {
    return;
  }

  /* Calculate culling data. */

  PathCullingData culling_data(data.horizontal_bands);

  draw_cubic_path_cull(path, data, culling_data);
  draw_cubic_path_cull_commit(data, culling_data);
}

void Renderer::draw_cubic_paths(const std::vector<const geom::dcubic_path*>& paths,
                                const drect& bounding_rect,
                                const Fill& fill,
                                const bool culling,
                                const std::array<vec2, 4>& texture_coords,
                                Drawable& drawable)
{
  /* Starting indices for this path. */

  const size_t num = std::accumulate(
      paths.begin(), paths.end(), 0, [](const size_t acc, const auto* p) {
        return acc + p->size();
      });

  PathData data{drawable,
                texture_coords,
                bounding_rect,
                bounding_rect.size(),
                fill,
                num,
                drawable.curves.size() / 2,
                drawable.bands.size(),
                culling,
                std::vector<dvec2>(num),
                std::vector<dvec2>(num),
                0.0,
                0};

  /* Reserve space and setup drawable. */

  data.drawable.bounding_rect = drect::from_rects(drawable.bounding_rect, bounding_rect);
  data.drawable.curves.reserve(drawable.curves.size() + num * 4);

  /* Copy the curves, and cache min_max values. */

  size_t accumulator = 0;

  for (const auto* path : paths) {
    for (uint32_t i = 0; i < path->size(); i++) {
      const dvec2 p0 = path->at(i * 3);
      const dvec2 p1 = path->at(i * 3 + 1);
      const dvec2 p2 = path->at(i * 3 + 2);
      const dvec2 p3 = path->at(i * 3 + 3);

      data.drawable.push_curve(vec2((p0 - bounding_rect.min) / data.bounds_size),
                               vec2((p1 - bounding_rect.min) / data.bounds_size),
                               vec2((p2 - bounding_rect.min) / data.bounds_size),
                               vec2((p3 - bounding_rect.min) / data.bounds_size));

      data.min[accumulator + i] = math::min(p0, p3);
      data.max[accumulator + i] = math::max(p0, p3);
    }

    accumulator += path->size();
  }

  /* Draw the path if not culled. */

  if (draw_cubic_path_impl(data)) {
    return;
  }

  /* Calculate culling data. */

  PathCullingData culling_data(data.horizontal_bands);

  for (const auto* path : paths) {
    draw_cubic_path_cull(*path, data, culling_data);
  }

  draw_cubic_path_cull_commit(data, culling_data);
}

void Renderer::draw_outline_vertices(const geom::dpath& path, const Outline& outline)
{
  InstanceBuffer<LineInstance>& lines = m_line_instances.instances;
  InstanceBuffer<RectInstance>& rects = m_rect_instances.instances;
  InstanceBuffer<CircleInstance>& circles = get()->m_circle_instances.instances;

  uint32_t i = path.points_count() - 1;
  vec2 last = vec2(path.at(i));

  if (!path.closed()) {
    rects.push_back({last, m_ui_options.vertex_size, m_ui_options.primary_color});

    if (outline.selected_vertices &&
        outline.selected_vertices->find(i) == outline.selected_vertices->end())
    {
      rects.push_back({last, m_ui_options.vertex_inner_size, vec4::identity()});
    }

    const vec2 out_handle = vec2(path.out_handle());

    if (out_handle != last) {
      circles.push_back({out_handle, m_ui_options.handle_radius, m_ui_options.primary_color});
      lines.push_back({out_handle, last, m_ui_options.line_width, m_ui_options.primary_color_05});
    }
  }

  /* We draw the vertices in reverse order to be coherent with hit testing. */

  path.for_each_reversed(
      [&](const dvec2 p0_raw) {
        const vec2 p0 = vec2(p0_raw);

        rects.push_back({p0, m_ui_options.vertex_size, m_ui_options.primary_color});

        if (outline.selected_vertices &&
            outline.selected_vertices->find(i) == outline.selected_vertices->end())
        {
          rects.push_back({p0, m_ui_options.vertex_inner_size, vec4::identity()});
        }

        if (!path.closed()) {
          vec2 in_handle = vec2(path.in_handle());

          if (in_handle != p0) {
            circles.push_back({in_handle, m_ui_options.handle_radius, m_ui_options.primary_color});
            lines.push_back(
                {in_handle, p0, m_ui_options.line_width, m_ui_options.primary_color_05});
          }
        }

        last = p0;
        i -= 1;
      },
      [&](const dvec2 p0_raw, const dvec2 p1_raw) {
        const vec2 p0 = vec2(p0_raw);

        rects.push_back({p0, m_ui_options.vertex_size, m_ui_options.primary_color});

        if (outline.selected_vertices &&
            outline.selected_vertices->find(i - 1) == outline.selected_vertices->end())
        {
          rects.push_back({p0, m_ui_options.vertex_inner_size, vec4::identity()});
        }

        last = p0;
        i -= 1;
      },
      [&](const dvec2 p0_raw, const dvec2 p1_raw, const dvec2 p2_raw) {
        const vec2 p0 = vec2(p0_raw);
        const vec2 p1 = vec2(p1_raw);

        rects.push_back({p0, m_ui_options.vertex_size, m_ui_options.primary_color});

        if (outline.selected_vertices &&
            outline.selected_vertices->find(i - 2) == outline.selected_vertices->end())
        {
          rects.push_back({p0, m_ui_options.vertex_inner_size, vec4::identity()});
        }

        if (p1_raw != p0_raw && p2_raw != p0_raw) {
          const vec2 h = vec2(p1_raw);

          circles.push_back({p1, m_ui_options.handle_radius, m_ui_options.primary_color});
          lines.push_back({p1, p0, m_ui_options.line_width, m_ui_options.primary_color_05});
          lines.push_back({p1, last, m_ui_options.line_width, m_ui_options.primary_color_05});
        }

        last = p0;
        i -= 2;
      },
      [&](const dvec2 p0_raw, const dvec2 p1_raw, const dvec2 p2_raw, const dvec2 p3_raw) {
        const vec2 p0 = vec2(p0_raw);
        const vec2 p1 = vec2(p1_raw);
        const vec2 p2 = vec2(p2_raw);

        rects.push_back({p0, m_ui_options.vertex_size, m_ui_options.primary_color});

        if (outline.selected_vertices &&
            outline.selected_vertices->find(i - 3) == outline.selected_vertices->end())
        {
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

void Renderer::draw(const Drawable& drawable)
{
  // TODO: check for gradients, textures, etc.
  if (!m_batch.fills.can_handle_quads(drawable.fills.size() / 4) ||
      !m_batch.tiles.can_handle_quads(drawable.tiles.size() / 4) ||
      !m_batch.tiles.can_handle_curves(drawable.curves.size() / 4) ||
      !m_batch.tiles.can_handle_bands(drawable.bands.size()))
  {
    flush_meshes();
  }

  uint32_t texture_index = 0;
  // TODO: should be non_color_paint
  bool has_texture_paint = false;

  for (const DrawablePaintBinding& binding : drawable.paints) {
    if (binding.paint_type != Paint::Type::TexturePaint) {
      continue;
    }

    has_texture_paint = true;

    const auto it = m_textures.find(binding.paint_id);
    const auto binded_it = m_binded_textures.find(binding.paint_id);

    if (it == m_textures.end()) {
      continue;
    }

    if (binded_it != m_binded_textures.end()) {
      texture_index = binded_it->second;
    } else {
      /* We add 1 to the index to avoid binding the gradients texture. */
      m_binded_textures.insert(std::make_pair(binding.paint_id, m_binded_textures.size() + 1));
    }
  }

  if (!has_texture_paint && drawable.paints.size() == 1) {
    m_batch.fills.upload(drawable, m_z_index);
    m_batch.tiles.upload(drawable, m_z_index);
  } else {
    m_batch.fills.upload(drawable, m_z_index, m_binded_textures);
    m_batch.tiles.upload(drawable, m_z_index, m_binded_textures);
  }

  m_z_index += drawable.paints.size();
}

// TODO: error handling
void Renderer::request_texture(const uuid texture_id)
{
  if (m_textures.find(texture_id) != m_textures.end()) {
    return;
  }

  io::Image image_texture = io::ResourceManager::get_image(texture_id);

  GPU::TextureFormat format;

  switch (image_texture.channels) {
    case 1:
      format = GPU::TextureFormat::R8;
      break;
    case 3:
      format = GPU::TextureFormat::RGB8;
      break;
    case 4:
    default:
      format = GPU::TextureFormat::RGBA8;
      break;
  }

  GPU::Texture texture = GPU::Texture(format,
                                      image_texture.size,
                                      GPU::TextureSamplingFlagRepeatU |
                                          GPU::TextureSamplingFlagRepeatV |
                                          GPU::TextureSamplingFlagMipmapMin,
                                      image_texture.data,
                                      true);

  m_textures.insert(std::make_pair(texture_id, std::move(texture)));
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

    // m_invalid_rects = m_cache->get_invalid_rects();

    // for (const auto& r : m_invalid_rects) {
    //   draw_rect(r, m_viewport.background);
    // }

    // render_state = RENDER_STATE(rect).no_blend().no_depth().subtract_stencil();
    // render_state.uniforms = {{m_programs.rect_program.vp_uniform, m_vp_matrix}};

    // /* Draw the background and stencil in the invalid regions. */
    // flush(m_rect_instances, render_state);

    // m_cached_rendering = true;
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

  // TODO: binded textures should be an array, not a map, like this texture may be swapped
  for (const auto& [texture_id, _] : m_binded_textures) {
    const auto it = m_textures.find(texture_id);

    if (it != m_textures.end()) {
      state.texture_arrays[0].second.push_back(&it->second);
    } else {
      // TODO: default texture
    }
  }

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

  // TODO: binded textures should be an array, not a map
  for (const auto& [texture_id, _] : m_binded_textures) {
    const auto it = m_textures.find(texture_id);

    if (it != m_textures.end()) {
      state.texture_arrays[0].second.push_back(&it->second);
    } else {
      // TODO: default texture
    }
  }

  GPU::Device::draw_elements(tiles.indices_count(), state);

  m_batch.clear();
  m_binded_textures.clear();
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

#endif
