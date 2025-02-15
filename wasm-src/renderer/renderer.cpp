/**
 * @file renderer.cpp
 * @brief The file contains the implementation of the main Graphick renderer.
 *
 * @todo gradients overflow handling
 * @todo path builder clipping rect
 * @todo dynamic number of samples based on dpr and hardware performance
 * @todo fix line width
 */

#include "renderer.h"

#include "../geom/clip.h"
#include "../geom/intersections.h"
#include "../geom/path.h"
#include "../geom/path_builder.h"

#include "../io/resource_manager.h"
#include "../io/text/font.h"
#include "../io/text/unicode.h"

#include "../math/matrix.h"
#include "../math/vector.h"

#include "../utils/assert.h"
#include "../utils/debugger.h"

#include "gpu/device.h"

#include "renderer_cache.h"

#include <unordered_set>

#ifdef EMSCRIPTEN
#  include <emscripten/html5.h>
#endif

namespace graphick::renderer {

/* -- Static Member Initialization -- */

Renderer* Renderer::s_instance = nullptr;

#ifdef GK_DEBUG

#  define __debug_max_rects 2048

inline static GPU::Buffer* __debug_rect_vertex_buffer = nullptr;
inline static GPU::Texture* __debug_font_texture = nullptr;

/**
 * @brief The vertex structure for the debug rect.
 */
struct __debug_rect_vertex {
  vec2 position;       // The position of the vertex.
  vec2 tex_coord;      // The texture coordinate, used for textured quads, circles and lines.
  uint32_t primitive;  // The primitive type (0 = rect, 1 = textured quad, 2 = circle, 3 = line).
  uvec4 color;         // The color of the vertex.
};

#endif

/* -- Renderer -- */

void Renderer::init()
{
#ifdef EMSCRIPTEN
  EmscriptenWebGLContextAttributes attr;
  emscripten_webgl_init_context_attributes(&attr);

  const bool desynchronized = false;

  // Despite
  // <https://developer.mozilla.org/en-US/docs/Web/API/WebGL_API/WebGL_best_practices#avoid_alphafalse_which_can_be_expensive>,
  // when using desynchronized context, it is better to use alpha=false to avoid expensive blending
  // operations if there are elements on top. Emscripten doesn't support desynchronized context
  // yet, so we set it to true after compilation (see script compile.py).

  attr.alpha = !desynchronized;
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

#ifdef GK_DEBUG
  __debug_rect_vertex_buffer = new GPU::Buffer(GPU::BufferTarget::Vertex,
                                               GPU::BufferUploadMode::Stream,
                                               __debug_max_rects * 6 * sizeof(__debug_rect_vertex),
                                               nullptr);
#endif

  s_instance = new Renderer();
}

void Renderer::shutdown()
{
  GK_ASSERT(s_instance != nullptr, "Renderer not initialized, call init() before shutting down!");

#ifdef GK_DEBUG
  delete __debug_rect_vertex_buffer;
#endif

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
  GPU::Device::begin_commands();

  get()->m_viewport = options.viewport;
  get()->m_tiler.setup(options.viewport.zoom);
  get()->m_ui_options = UIOptions(options.viewport.dpr / options.viewport.zoom);
  get()->m_cache = options.cache;

  get()->m_tiles.setup(get()->m_viewport.size,
                       options.viewport.visible(),
                       get()->m_viewport.vp_matrix,
                       get()->m_tiler.LOD(),
                       get()->m_tiler.base_tile_size());

  get()->flush_background_layer();

#ifdef GK_DEBUG
  if (__debug_font_texture == nullptr) {
    const io::text::Font& debug_font = io::ResourceManager::get_font(uuid::null);
    const std::vector<uint8_t> debug_font_atlas = debug_font.__debug_get_atlas(ivec2(128), 11.0f);

    __debug_font_texture = new GPU::Texture(
        GPU::TextureFormat::R8, ivec2(128), 0, debug_font_atlas.data());
  }
#endif
}

void Renderer::end_frame()
{
  get()->flush_scene_layer();

  GPU::Device::default_framebuffer();

  get()->flush_ui_layer();

#ifdef GK_DEBUG
  get()->__debug_flush_layer({true});
  graphick::utils::debugger::render();
#endif

  const size_t time = GPU::Device::end_commands();

  __debug_time_total_record("GPU", time);
}

bool Renderer::draw(const geom::path& path,
                    const mat2x3& transform,
                    const DrawingOptions& options,
                    const uuid id)
{
  if (path.empty() ||
      (options.fill == nullptr && options.stroke == nullptr && options.outline == nullptr))
  {
    return false;
  }

  if (get()->m_cache->has_bounding_rect(id) && get()->m_cache->has_drawable(id)) {
    const drect& bounding_rect = get()->m_cache->get_bounding_rect(id);
    const Drawable& drawable = get()->m_cache->get_drawable(id);

    const double visible_all = geom::rect_rect_intersection_area(bounding_rect,
                                                                 get()->m_viewport.visible());

    if (visible_all <= 0) {
      return false;
    }

    const double visible_clip = geom::rect_rect_intersection_area(drawable.valid_rect,
                                                                  get()->m_viewport.visible());

    const uint8_t LOD = get()->m_tiler.LOD();
    bool is_LOD_valid = false;

    if (LOD == drawable.LOD) {
      is_LOD_valid = true;
    } else if (LOD == drawable.LOD + 1 || drawable.LOD == LOD + 1) {
      // We randomly choose whether to update the LOD or not. To distribute the updates across
      // multiple frames.
      is_LOD_valid = rand() > RAND_MAX / 2;
    }

    if (is_LOD_valid && math::is_almost_equal(visible_all, visible_clip)) {
      get()->m_tiles.push_drawable(&drawable);

      if (options.outline) {
        const geom::dpath transformed_path = path.transformed<double>(transform);
        get()->draw_outline(transformed_path, bounding_rect, *options.outline);
      }

      return true;
    }
  }

  __debug_value_counter("recalculated");

  bool has_transform;

  const geom::dpath transformed_path = path.transformed<double>(transform, &has_transform);
  const math::drect transformed_bounding_rect = transformed_path.bounding_rect();

  std::array<vec2, 4> tex_coords = rect::identity().vertices();

  if (has_transform && options.fill &&
      (options.fill->paint.is_gradient() || options.fill->paint.is_texture()))
  {
    drect raw_bounding_rect = drect(path.bounding_rect());

    const auto& [v0_t, v1_t, v2_t, v3_t] = transformed_bounding_rect.vertices();

    const dmat2x3 inverse_transform = math::inverse(dmat2x3(transform));
    const dvec2 size = raw_bounding_rect.size();

    const vec2 v0 = vec2(inverse_transform * v0_t / size);
    const vec2 v1 = vec2(inverse_transform * v1_t / size);
    const vec2 v2 = vec2(inverse_transform * v2_t / size);
    const vec2 v3 = vec2(inverse_transform * v3_t / size);

    tex_coords = {v0, v1, v2, v3};
  }

  return get()->draw_transformed(
      transformed_path, transformed_bounding_rect, options, tex_coords, id);
}

bool Renderer::draw(const renderer::Text& text,
                    const mat2x3& transform,
                    const DrawingOptions& options,
                    const uuid id)
{
  return false;
}

bool Renderer::draw(const renderer::Image& image,
                    const mat2x3& transform,
                    const DrawingOptions& options,
                    const uuid id)
{
  const bool has_transform = !math::is_identity(transform);

  return false;
}

#ifdef GK_DEBUG

void Renderer::__debug_rect_impl(const rect& rect, const vec4& color)
{
  const uvec4 u_color = uvec4(color * 255.0f);

  std::vector<__debug_rect_vertex> buffer = {
      {rect.min, vec2::zero(), 0, u_color},
      {vec2(rect.max.x, rect.min.y), vec2::zero(), 0, u_color},
      {rect.max, vec2::zero(), 0, u_color},
      {rect.max, vec2::zero(), 0, u_color},
      {vec2(rect.min.x, rect.max.y), vec2::zero(), 0, u_color},
      {rect.min, vec2::zero(), 0, u_color},
  };

  __debug_rect_vertex_buffer->upload(buffer.data(), buffer.size() * sizeof(__debug_rect_vertex));

  GPU::RenderState render_state = GPU::RenderState{
      get()->m_programs.debug_rect_program.program,
      &get()->m_vertex_arrays.debug_rect_vertex_array->vertex_array,
      GPU::Primitive::Triangles,
      irect(ivec2::zero(), get()->m_viewport.size)};

  render_state.default_blend().no_depth().no_stencil();
  render_state.uniforms = {
      {get()->m_programs.debug_rect_program.vp_uniform, get()->m_viewport.screen_vp_matrix}};

  GPU::Device::draw_arrays(6, render_state);
}

void Renderer::__debug_square_impl(const vec2 center, const float radius, const vec4& color)
{
  __debug_rect_impl(rect(center - vec2(radius), center + vec2(radius)), color);
}

void Renderer::__debug_circle_impl(const vec2 center, const float radius, const vec4& color) {}

void Renderer::__debug_line_impl(const vec2 start, const vec2 end, const vec4& color)
{
  const uvec4 u_color = uvec4(color * 255.0f);

  std::vector<__debug_rect_vertex> buffer = {
      {start, vec2::zero(), 3, u_color},
      {end, vec2::zero(), 3, u_color},
  };

  __debug_rect_vertex_buffer->upload(buffer.data(), buffer.size() * sizeof(__debug_rect_vertex));

  GPU::RenderState render_state = GPU::RenderState{
      get()->m_programs.debug_rect_program.program,
      &get()->m_vertex_arrays.debug_rect_vertex_array->vertex_array,
      GPU::Primitive::Lines,
      irect(ivec2::zero(), get()->m_viewport.size)};

  render_state.default_blend().no_depth().no_stencil();
  render_state.uniforms = {
      {get()->m_programs.debug_rect_program.vp_uniform, get()->m_viewport.screen_vp_matrix}};

  GPU::Device::draw_arrays(2, render_state);
}

void Renderer::__debug_lines_impl(const std::vector<vec2>& points, const vec4& color)
{
  const uvec4 u_color = uvec4(color * 255.0f);

  std::vector<__debug_rect_vertex> buffer;

  for (size_t i = 0; i < points.size() - 1; i++) {
    buffer.push_back({points[i], vec2::zero(), 3, u_color});
    buffer.push_back({points[i + 1], vec2::zero(), 3, u_color});
  }

  __debug_rect_vertex_buffer->upload(buffer.data(), buffer.size() * sizeof(__debug_rect_vertex));

  GPU::RenderState render_state = GPU::RenderState{
      get()->m_programs.debug_rect_program.program,
      &get()->m_vertex_arrays.debug_rect_vertex_array->vertex_array,
      GPU::Primitive::Lines,
      irect(ivec2::zero(), get()->m_viewport.size)};

  render_state.default_blend().no_depth().no_stencil();
  render_state.uniforms = {
      {get()->m_programs.debug_rect_program.vp_uniform, get()->m_viewport.screen_vp_matrix}};

  GPU::Device::draw_arrays(buffer.size(), render_state);
}

float Renderer::__debug_text_impl(const std::string& text, const vec2 position, const vec4& color)
{
  std::vector<__debug_rect_vertex> buffer;

  const io::text::Font& font = io::ResourceManager::get_font(uuid::null);
  const uvec4 u_color = uvec4(color * 255.0f);
  const float font_size = 11.0f;

  vec2 cursor = position;

  for (const char c : text) {
    std::pair<rect, rect> quad = font.__debug_get_baked_quad(c, ivec2(128), cursor);

    const auto& [v0, v1, v2, v3] = quad.first.vertices();
    const auto& [t0, t1, t2, t3] = quad.second.vertices();

    if (u_color.a == 0) {
      continue;
    }

    buffer.push_back({math::round(v0), t0, 1, u_color});
    buffer.push_back({math::round(v1), t1, 1, u_color});
    buffer.push_back({math::round(v2), t2, 1, u_color});
    buffer.push_back({math::round(v2), t2, 1, u_color});
    buffer.push_back({math::round(v3), t3, 1, u_color});
    buffer.push_back({math::round(v0), t0, 1, u_color});
  }

  if (u_color.a == 0) {
    return cursor.x - position.x;
  }

  __debug_rect_vertex_buffer->upload(buffer.data(), buffer.size() * sizeof(__debug_rect_vertex));

  GPU::RenderState render_state = GPU::RenderState{
      get()->m_programs.debug_rect_program.program,
      &get()->m_vertex_arrays.debug_rect_vertex_array->vertex_array,
      GPU::Primitive::Triangles,
      irect(ivec2::zero(), get()->m_viewport.size)};

  render_state.default_blend().no_depth().no_stencil();

  render_state.uniforms = {
      {get()->m_programs.debug_rect_program.vp_uniform, get()->m_viewport.screen_vp_matrix}};
  render_state.textures = {{get()->m_programs.debug_rect_program.texture, __debug_font_texture}};

  GPU::Device::draw_arrays(buffer.size(), render_state);

  return cursor.x - position.x;
}

#endif

bool Renderer::draw_transformed(const geom::dpath& path,
                                const drect& bounding_rect,
                                const DrawingOptions& options,
                                const std::array<vec2, 4>& texture_coords,
                                const uuid id)
{
  Drawable drawable;

  drawable.LOD = m_tiler.LOD();
  drawable.appearance = Appearance{BlendingMode::Multiply, 1.0f};

  bool visible = false;

  if (options.fill && options.fill->paint.visible()) {
    geom::dcubic_multipath cubic_path = path.to_cubic_multipath();

    if (!cubic_path.closed()) {
      cubic_path.line_to(cubic_path.front());
    }

    visible |= draw_multipath(cubic_path, bounding_rect, *options.fill, texture_coords, drawable);
  }

  if (options.stroke && options.stroke->paint.visible()) {
    Fill stroke_fill{options.stroke->paint, FillRule::NonZero};

    const geom::StrokingOptions<double> stroking_options{RendererSettings::stroking_tolerance,
                                                         options.stroke->width,
                                                         options.stroke->miter_limit,
                                                         options.stroke->cap,
                                                         options.stroke->join};
    const geom::PathBuilder<double> builder = geom::PathBuilder(path, bounding_rect);
    const geom::StrokeOutline<double> stroke_path = builder.stroke(stroking_options);

    get()->m_cache->set_bounding_rect(id, stroke_path.bounding_rect);

    visible |= draw_multipath(
        stroke_path.path, stroke_path.bounding_rect, stroke_fill, texture_coords, drawable);
  } else {
    get()->m_cache->set_bounding_rect(id, bounding_rect);
  }

  if (!visible) {
    m_cache->set_drawable(id, std::move(drawable));
    return false;
  }

  m_tiles.push_drawable(m_cache->set_drawable(id, std::move(drawable)));

  if (options.outline) {
    draw_outline(path, bounding_rect, *options.outline);
  }

  return true;
}

bool Renderer::draw_multipath(const geom::dcubic_multipath& path,
                              const drect& bounding_rect,
                              const Fill& fill,
                              const std::array<vec2, 4>& texture_coords,
                              Drawable& drawable)
{
  drawable.bounding_rect = bounding_rect;
  drawable.valid_rect = bounding_rect;

  const drect visible = m_viewport.visible();
  const double coverage = geom::rect_rect_intersection_area(bounding_rect, visible) /
                          bounding_rect.area();

  if (coverage <= math::epsilon<double>) {
    drawable.valid_rect = geom::rect_rect_intersection(visible, bounding_rect);
    return false;
  }

  const bool clip = coverage < 0.25;

  if (clip) {
    const double tile_size = m_tiler.tile_size();
    const drect clip_region = {math::floor(visible.min / tile_size - 2) * tile_size,
                               math::ceil(visible.max / tile_size + 2) * tile_size};

    drawable.valid_rect = geom::rect_rect_intersection(clip_region, bounding_rect);

    geom::dcubic_multipath clipped_path = geom::clip(path, clip_region);

    if (clipped_path.empty()) {
      return false;
    }

    const drect clipped_bounding_rect = clipped_path.bounding_rect();
    const std::array<vec2, 4> clipped_tex_coords = reproject_texture_coords(
        bounding_rect, clipped_bounding_rect, texture_coords);

    drawable.bounding_rect = clipped_bounding_rect;

    m_tiler.tile(clipped_path, clipped_bounding_rect, fill, clipped_tex_coords, drawable);
  } else {
    m_tiler.tile(path, bounding_rect, fill, texture_coords, drawable);
  }

  if (fill.paint.is_texture()) {
    request_texture(fill.paint.id());
  }

  return true;
}

void Renderer::draw_outline(const geom::dpath& path,
                            const drect& bounding_rect,
                            const Outline& outline)
{
  const geom::PathBuilder<double> builder = geom::PathBuilder(path, bounding_rect);

  builder.flatten<float>(m_viewport.visible(),
                         RendererSettings::flattening_tolerance / m_viewport.zoom,
                         [&](const vec2 p0, const vec2 p1) {
                           m_instances.push_line(p0, p1, outline.color, m_ui_options.line_width);
                         });

  if (outline.draw_vertices) {
    draw_outline_vertices(path, outline);
  }
}

void Renderer::draw_outline_vertices(const geom::dpath& path, const Outline& outline)
{
  uint32_t i = path.points_count() - 1;
  vec2 last = vec2(path.at(i));

  const std::unordered_set<uint32_t>* selected_vertices =
      static_cast<const std::unordered_set<uint32_t>*>(outline.selected_vertices);

  if (!path.closed()) {
    m_instances.push_rect(last, m_ui_options.vertex_size, outline.color);

    if (selected_vertices && selected_vertices->find(i) == selected_vertices->end()) {
      m_instances.push_rect(last, m_ui_options.vertex_inner_size, vec4::identity());
    }

    const vec2 out_handle = vec2(path.out_handle());

    if (out_handle != last) {
      m_instances.push_circle(out_handle, m_ui_options.handle_radius, outline.color);
      m_instances.push_line(out_handle, last, outline.color, m_ui_options.line_width);
    }
  }

  /* We draw the vertices in reverse order to be coherent with hit testing. */

  path.for_each_reversed(
      [&](const dvec2 p0_raw) {
        const vec2 p0 = vec2(p0_raw);

        m_instances.push_rect(p0, m_ui_options.vertex_size, outline.color);

        if (selected_vertices && selected_vertices->find(i) == selected_vertices->end()) {
          m_instances.push_rect(p0, m_ui_options.vertex_inner_size, vec4::identity());
        }

        if (!path.closed()) {
          vec2 in_handle = vec2(path.in_handle());

          if (in_handle != p0) {
            m_instances.push_circle(in_handle, m_ui_options.handle_radius, outline.color);
            m_instances.push_line(in_handle, p0, outline.color, m_ui_options.line_width);
          }
        }

        last = p0;
        i -= 1;
      },
      [&](const dvec2 p0_raw, const dvec2 p1_raw) {
        const vec2 p0 = vec2(p0_raw);

        m_instances.push_rect(p0, m_ui_options.vertex_size, outline.color);

        if (selected_vertices && selected_vertices->find(i - 1) == selected_vertices->end()) {
          m_instances.push_rect(p0, m_ui_options.vertex_inner_size, vec4::identity());
        }

        last = p0;
        i -= 1;
      },
      [&](const dvec2 p0_raw, const dvec2 p1_raw, const dvec2 p2_raw) {
        const vec2 p0 = vec2(p0_raw);
        const vec2 p1 = vec2(p1_raw);

        m_instances.push_rect(p0, m_ui_options.vertex_size, outline.color);

        if (selected_vertices && selected_vertices->find(i - 2) == selected_vertices->end()) {
          m_instances.push_rect(p0, m_ui_options.vertex_inner_size, vec4::identity());
        }

        if (p1_raw != p0_raw && p2_raw != p0_raw) {
          m_instances.push_circle(p1, m_ui_options.handle_radius, outline.color);
          m_instances.push_line(p1, p0, outline.color, m_ui_options.line_width);
          m_instances.push_line(p1, last, outline.color, m_ui_options.line_width);
        }

        last = p0;
        i -= 2;
      },
      [&](const dvec2 p0_raw, const dvec2 p1_raw, const dvec2 p2_raw, const dvec2 p3_raw) {
        const vec2 p0 = vec2(p0_raw);
        const vec2 p1 = vec2(p1_raw);
        const vec2 p2 = vec2(p2_raw);

        m_instances.push_rect(p0, m_ui_options.vertex_size, outline.color);

        if (selected_vertices && selected_vertices->find(i - 3) == selected_vertices->end()) {
          m_instances.push_rect(p0, m_ui_options.vertex_inner_size, vec4::identity());
        }

        if (p2_raw != p3_raw) {
          m_instances.push_circle(p2, m_ui_options.handle_radius, outline.color);
          m_instances.push_line(p2, last, outline.color, m_ui_options.line_width);
        }

        if (p1_raw != p0_raw) {
          m_instances.push_circle(p1, m_ui_options.handle_radius, outline.color);
          m_instances.push_line(p1, p0, outline.color, m_ui_options.line_width);
        }

        last = p0;
        i -= 3;
      });
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

void Renderer::flush_background_layer()
{
  /* Setup the viewport. */
  GPU::Device::set_viewport(m_viewport.size);
  GPU::Device::clear({m_viewport.background, 1.0f, 0});
}

void Renderer::flush_scene_layer()
{
  m_tiles.flush();
}

void Renderer::flush_ui_layer()
{
  m_instances.flush(m_viewport.size, m_viewport.vp_matrix, m_viewport.zoom);
}

#ifdef GK_DEBUG

void Renderer::__debug_flush_layer(const DebugOptions& options) const
{
  if (options.show_LODs) {
    const double base_tile_size = m_tiler.base_tile_size();
    const drect viewport = m_viewport.visible();

    for (uint8_t LOD = 0; LOD <= m_tiler.LOD(); LOD++) {
      const double tile_size = base_tile_size * std::pow(0.5, LOD);

      const dvec2 min = math::floor(viewport.min / tile_size) * tile_size;
      const dvec2 max = math::ceil(viewport.max / tile_size) * tile_size;

      std::vector<vec2> points;

      bool up = false;

      for (double x = min.x; x < max.x; x += tile_size) {
        if (up) {
          points.push_back(vec2(m_viewport.revert(dvec2(x, max.y))));
          points.push_back(vec2(m_viewport.revert(dvec2(x, min.y))));
        } else {
          points.push_back(vec2(m_viewport.revert(dvec2(x, min.y))));
          points.push_back(vec2(m_viewport.revert(dvec2(x, max.y))));
        }

        up = !up;
      }

      __debug_lines(points, vec4(1.0, 1.0, 1.0, 0.1));

      points.clear();

      for (double y = min.y; y < max.y; y += tile_size) {
        if (up) {
          points.push_back(vec2(m_viewport.revert(dvec2(min.x, y))));
          points.push_back(vec2(m_viewport.revert(dvec2(max.x, y))));
        } else {
          points.push_back(vec2(m_viewport.revert(dvec2(max.x, y))));
          points.push_back(vec2(m_viewport.revert(dvec2(min.x, y))));
        }

        up = !up;
      }

      __debug_lines(points, vec4(1.0, 1.0, 1.0, 0.1));

      points.clear();
    }

    __debug_value("LOD", m_tiler.LOD());
  }
}

#endif

Renderer::Renderer() : m_instances(GK_LARGE_BUFFER_SIZE), m_tiles(GK_LARGE_BUFFER_SIZE)
{
  std::unique_ptr<GPU::PrimitiveVertexArray> primitive_vertex_array =
      std::make_unique<GPU::PrimitiveVertexArray>(m_programs.primitive_program,
                                                  m_instances.instance_buffer(),
                                                  m_instances.vertex_buffer());
  std::unique_ptr<GPU::TileVertexArray> tile_vertex_array = std::make_unique<GPU::TileVertexArray>(
      m_programs.tile_program, m_tiles.tiles_vertex_buffer(), m_tiles.tiles_index_buffer());
  std::unique_ptr<GPU::FillVertexArray> fill_vertex_array = std::make_unique<GPU::FillVertexArray>(
      m_programs.fill_program, m_tiles.fills_vertex_buffer(), m_tiles.fills_index_buffer());

#ifdef GK_DEBUG
  std::unique_ptr<GPU::DebugRectVertexArray> debug_rect_vertex_array =
      std::make_unique<GPU::DebugRectVertexArray>(m_programs.debug_rect_program,
                                                  *__debug_rect_vertex_buffer);
#endif

  m_vertex_arrays = GPU::VertexArrays{
#ifdef GK_DEBUG
      std::move(debug_rect_vertex_array),
#endif
      std::move(primitive_vertex_array),
      std::move(tile_vertex_array),
      std::move(fill_vertex_array)};

  m_instances.update_shader(&m_programs.primitive_program,
                            m_vertex_arrays.primitive_vertex_array.get());
  m_tiles.update_shaders(&m_programs.tile_program,
                         &m_programs.fill_program,
                         m_vertex_arrays.tile_vertex_array.get(),
                         m_vertex_arrays.fill_vertex_array.get(),
                         &m_textures);
}

}  // namespace graphick::renderer
