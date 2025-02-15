/**
 * @file renderer/tiles.cpp
 * @brief The file contains the implementation of the Tiler class.
 */

#include "tiles.h"

#include "../geom/intersections.h"

#include "../math/math.h"

#include "gpu/device.h"

// TEMP
#include "../utils/console.h"
#include "../utils/debugger.h"
#include "renderer.h"

#include <algorithm>

namespace graphick::renderer {

std::array<vec2, 4> reproject_texture_coords(const drect bounding_rect,
                                             const drect clipped_rect,
                                             const std::array<vec2, 4>& texture_coords,
                                             vec2& r_tex_coord_curves_min,
                                             vec2& r_tex_coord_curves_max)
{
  const dvec2 bounds_size = bounding_rect.size();

  r_tex_coord_curves_min = vec2((clipped_rect.min - bounding_rect.min) / bounds_size);
  r_tex_coord_curves_max = vec2((clipped_rect.max - bounding_rect.min) / bounds_size);

  const vec2 interp_03_min_y = math::lerp(
      texture_coords[0], texture_coords[3], r_tex_coord_curves_min.y);
  const vec2 interp_12_min_y = math::lerp(
      texture_coords[1], texture_coords[2], r_tex_coord_curves_min.y);
  const vec2 interp_03_max_y = math::lerp(
      texture_coords[0], texture_coords[3], r_tex_coord_curves_max.y);
  const vec2 interp_12_max_y = math::lerp(
      texture_coords[1], texture_coords[2], r_tex_coord_curves_max.y);

  const std::array<vec2, 4> clipped_tex_coords = {
      math::lerp(interp_03_min_y, interp_12_min_y, r_tex_coord_curves_min.x),
      math::lerp(interp_03_min_y, interp_12_min_y, r_tex_coord_curves_max.x),
      math::lerp(interp_03_max_y, interp_12_max_y, r_tex_coord_curves_max.x),
      math::lerp(interp_03_max_y, interp_12_max_y, r_tex_coord_curves_min.x)};

  return clipped_tex_coords;
}

std::array<vec2, 4> reproject_texture_coords(const drect bounding_rect,
                                             const drect clipped_rect,
                                             const std::array<vec2, 4>& texture_coords)
{
  vec2 tex_coord_curves_min;
  vec2 tex_coord_curves_max;

  return reproject_texture_coords(
      bounding_rect, clipped_rect, texture_coords, tex_coord_curves_min, tex_coord_curves_max);
}

Tiler::Tiler() : m_zoom(1.0), m_base_cell_size(512.0), m_LOD(0) {}

void Tiler::setup(const double zoom)
{
  const double raw_log = -std::log2(13.0 / (m_base_cell_size * zoom));

  m_zoom = zoom;
  m_LOD = static_cast<uint8_t>(math::clamp(std::round(raw_log), 0.0, 24.0));

  // m_LOD = std::min(m_LOD, uint8_t(2));

  m_cell_size = m_base_cell_size * std::pow(0.5, m_LOD);
}

void Tiler::tile(const geom::dcubic_multipath& path,
                 const drect& bounding_rect,
                 const Fill& fill,
                 const std::array<vec2, 4>& texture_coords,
                 Drawable& drawable)
{
  /* A segment is considered in a cell if within the tolerance of the cell boundary. */

  const double tolerance = 2.0 / m_zoom;

  const uvec4 color = fill.paint.is_color() ?
                          uvec4(fill.paint.color() * drawable.appearance.opacity * 255.0f) :
                          uvec4(vec4(1.0, 1.0, 1.0, drawable.appearance.opacity) * 255.0f);

  const uint32_t attr_1 = TileVertex::create_attr_1(0, fill.paint.type(), drawable.curves.size());
  const uint32_t attr_2 = TileVertex::create_attr_2(0, false, fill.rule == FillRule::EvenOdd, 0);

  const dvec2 bounds_size = bounding_rect.size();

  const bool create_fills = color.a == 255 && drawable.appearance.blending == BlendingMode::Normal;

  /* Setting up the workspace, a 1 cell padding in all directions is applied. */

  const ivec2 path_start_cell = ivec2(math::floor(bounding_rect.min / m_cell_size)) - 1;
  const ivec2 path_end_cell = ivec2(math::ceil(bounding_rect.max / m_cell_size)) - 1;
  const ivec2 path_cell_count = path_end_cell - path_start_cell + 2;

  m_cells.clear_and_resize(path_cell_count.x, path_cell_count.y);
  m_curves_map.clear();

#if SORT_CURVES
  m_curves_max.clear();
  m_curves_max.resize(path.size());
#endif

  size_t start = path.starts.size() > 1 ? path.starts[1] : std::numeric_limits<size_t>::max();
  uint16_t offset = 0;

  for (uint16_t i = 0; i < path.size(); i++) {
    if (size_t(i) * 3 + 1 == start) {
      offset += 1;
      start = path.starts.size() > offset + 1 ? path.starts[offset + 1] :
                                                std::numeric_limits<size_t>::max();
    }

    /* Being monotonic, it is straightforward to determine which cells the curve intersects. */

    const dvec2 p0 = path[i * 3 + offset];
    const dvec2 p1 = path[i * 3 + offset + 1];
    const dvec2 p2 = path[i * 3 + offset + 2];
    const dvec2 p3 = path[i * 3 + offset + 3];

#if SORT_CURVES
    m_curves_max[i] = std::max(p0.x, p3.x);
#endif

    if (math::squared_distance(p0, p3) < math::geometric_epsilon<double>
        //  || math::is_almost_equal(p0.y, p3.y, math::geometric_epsilon<double>)
    )
    {
      continue;
    }

    const bool right = p3.x >= p0.x;
    const bool up = p3.y <= p0.y;

    const double x_tol = tolerance * (right ? 1.0 : -1.0);
    const double y_tol = tolerance * (up ? -1.0 : 1.0);

    const dvec2 tol = dvec2(x_tol, y_tol);

    const int y_inc = up ? 1 : -1;

    const ivec2 start_cell = ivec2(math::floor((p0 - tol) / m_cell_size)) - path_start_cell;
    const ivec2 end_cell = ivec2(math::floor((p3 + tol) / m_cell_size)) - path_start_cell;

    if (start_cell == end_cell) {
      /* Curve is within one cell. */
      m_cells.insert(start_cell.x, start_cell.y, i);
      continue;
    }

    /* Calculate intersections with cell boundaries. */

    const bool b01 = std::abs(p1.x - p0.x) + std::abs(p1.y - p0.y) <
                     math::geometric_epsilon<double>;
    const bool b12 = std::abs(p2.x - p1.x) + std::abs(p2.y - p1.y) <
                     math::geometric_epsilon<double>;
    const bool b23 = std::abs(p3.x - p2.x) + std::abs(p3.y - p2.y) <
                     math::geometric_epsilon<double>;

    const bool linear = (b01 && (b23 || b12)) || (b23 && b12);

    int x = start_cell.x;
    int y = start_cell.y;

    if (linear && math::is_almost_equal(p0.y, p3.y, math::geometric_epsilon<double>)) {
      for (; y_inc * y >= y_inc * end_cell.y; y -= y_inc) {
        for (int xc = std::min(x, end_cell.x); xc <= std::max(x, end_cell.x); xc++) {
          m_cells.insert(xc, y, i);
        }
      }

      continue;
    }

    m_cells.insert(x, y, i);

    if (linear) {
      for (; y_inc * y > y_inc * end_cell.y; y -= y_inc) {
        const double y0 = (y + path_start_cell.y - (y_inc - 1) / 2) * m_cell_size;
        const double t0 = (y0 - p0.y) / (p3.y - p0.y);
        const double clamped_t0 = math::clamp(t0, 0.0, 1.0);

        const double x0 = p0.x + clamped_t0 * (p3.x - p0.x);

        const int x_cell = int(std::floor(x0 / m_cell_size)) - path_start_cell.x;
        const int x_cell_tol = int(std::floor((x0 + x_tol) / m_cell_size)) - path_start_cell.x;
        const int x_cell_alt_tol = int(std::floor((x0 - x_tol) / m_cell_size)) - path_start_cell.x;

        for (int xc = std::min(x, x_cell_tol); xc <= std::max(x, x_cell_tol); xc++) {
          m_cells.insert(xc, y, i);
        }

        if (x_cell_alt_tol != x_cell) {
          m_cells.insert(x_cell_alt_tol, y, i);
        }

        if ((up && t0 >= -math::epsilon<double> && t0 < 1.0 - math::epsilon<double>) ||
            (!up && t0 > math::epsilon<double> && t0 <= 1.0 + math::epsilon<double>))
        {
          m_cells.intersection(y - (y_inc - 1) / 2, {x0, up ? int8_t(1) : int8_t(-1)});
        }

        x = x_cell;
      }
    } else {
      const auto& [a, b, c, d] = geom::cubic_coefficients(p0, p1, p2, p3);

      for (; y_inc * y > y_inc * end_cell.y; y -= y_inc) {
        const double y0 = (y + path_start_cell.y - (y_inc - 1) / 2) * m_cell_size;
        const double t0 = (y0 - p0.y) / (p3.y - p0.y);
        const double clamped_t0 = math::clamp(t0, 0.0, 1.0);

        const double t = math::is_almost_zero_or_one(clamped_t0) ?
                             clamped_t0 :
                             geom::cubic_line_intersect_approx(a.y, b.y, c.y, d.y, y0, t0);
        const double t_sq = t * t;
        const double x0 = math::clamp(
            a.x * t_sq * t + b.x * t_sq + c.x * t + d.x, bounding_rect.min.x, bounding_rect.max.x);

        const int x_cell = int(std::floor(x0 / m_cell_size)) - path_start_cell.x;
        const int x_cell_tol = int(std::floor((x0 + x_tol) / m_cell_size)) - path_start_cell.x;
        const int x_cell_alt_tol = int(std::floor((x0 - x_tol) / m_cell_size)) - path_start_cell.x;

        for (int xc = std::min(x, x_cell_tol); xc <= std::max(x, x_cell_tol); xc++) {
          m_cells.insert(xc, y, i);
        }

        if (x_cell_alt_tol != x_cell) {
          m_cells.insert(x_cell_alt_tol, y, i);
        }

        if ((up && t0 >= -math::epsilon<double> && t0 < 1.0 - math::epsilon<double>) ||
            (!up && t0 > math::epsilon<double> && t0 <= 1.0 + math::epsilon<double>))
        {
          m_cells.intersection(y - (y_inc - 1) / 2, {x0, up ? int8_t(1) : int8_t(-1)});
        }

        x = x_cell;
      }
    }

    if (x != end_cell.x) {
      for (int xc = std::min(x, end_cell.x); xc <= std::max(x, end_cell.x); xc++) {
        m_cells.insert(xc, end_cell.y, i);
      }
    }

    m_cells.insert(end_cell.x, end_cell.y, i);
  }

  /* Create tiles and fills. */

  for (int y = 0; y < path_cell_count.y; y++) {
    int intersection_index = 0;
    int winding = 0;

    int fill_start = -1;
    int tile_start = -1;
    int tile_start_winding = 0;

    std::sort(m_cells.intersections(y).begin(),
              m_cells.intersections(y).end(),
              [](const Intersection a, const Intersection b) { return a.x > b.x; });

    uint32_t row_curves_offset = drawable.curves.size() / 2;
    uint16_t row_curves_count = m_cells[y].size();

    std::vector<uint16_t> curves;

    curves.assign(m_cells[y].begin(), m_cells[y].end());
    std::sort(curves.begin(), curves.end());

    // std::sort(curves.begin(), curves.end(), [&](const uint16_t a, const uint16_t b) {
    //   return m_curves_max[a] > m_curves_max[b];
    // });

    const int hash = math::hash(curves, 0, curves.size());
    auto it = m_curves_map.find(hash);

    if (it == m_curves_map.end()) {
      m_curves_map[hash] = row_curves_offset;

      size_t start = path.starts.size() > 1 ? path.starts[1] : std::numeric_limits<size_t>::max();
      uint16_t offset = 0;

      for (const uint16_t i : curves) {
        if (size_t(i) * 3 + 1 >= start) {
          offset += 1;
          start = path.starts.size() > offset + 1 ? path.starts[offset + 1] :
                                                    std::numeric_limits<size_t>::max();
        }

        const vec2 p0 = vec2((path[i * 3 + offset] - bounding_rect.min) / bounds_size);
        const vec2 p1 = vec2((path[i * 3 + offset + 1] - bounding_rect.min) / bounds_size);
        const vec2 p2 = vec2((path[i * 3 + offset + 2] - bounding_rect.min) / bounds_size);
        const vec2 p3 = vec2((path[i * 3 + offset + 3] - bounding_rect.min) / bounds_size);

        drawable.curves.insert(drawable.curves.end(), {p0, p1, p2, p3});
      }
    } else {
      row_curves_offset = it->second;
    }

    for (int x = path_cell_count.x - 1; x >= 0; x--) {
      if (!m_cells.is_tile(x, y)) {
        if (tile_start > -1) {
          const dvec2 cell_min = dvec2(path_start_cell + ivec2(x + 1, y)) * m_cell_size;
          const dvec2 cell_max = cell_min + dvec2(tile_start - x, 1) * m_cell_size;

          vec2 tex_coord_curves_min;
          vec2 tex_coord_curves_max;

          const std::array<vec2, 4> transformed_tex_coords = reproject_texture_coords(
              bounding_rect,
              drect(cell_min, cell_max),
              texture_coords,
              tex_coord_curves_min,
              tex_coord_curves_max);

          const uint32_t attr_1 = TileVertex::create_attr_1(
              0, fill.paint.type(), row_curves_offset);
          const uint32_t attr_3 = TileVertex::create_attr_3(tile_start_winding, row_curves_count);

          drawable.push_tile(vec2(cell_min),
                             vec2(cell_max),
                             tex_coord_curves_min,
                             tex_coord_curves_max,
                             transformed_tex_coords,
                             color,
                             attr_1,
                             attr_2,
                             attr_3);

          tile_start = -1;
        }

        if (fill_start == -1) {
          fill_start = x;
        }
      } else {
        if (fill_start > -1) {
          if (fill.rule == FillRule::NonZero ? (winding != 0) : (winding % 2 != 0)) {
            const dvec2 cell_min = dvec2(path_start_cell + ivec2(x + 1, y)) * m_cell_size;
            const dvec2 cell_max = cell_min + dvec2(fill_start - x, 1) * m_cell_size;

            const std::array<vec2, 4> transformed_tex_coords = reproject_texture_coords(
                bounding_rect, drect(cell_min, cell_max), texture_coords);

            drawable.push_fill(
                vec2(cell_min), vec2(cell_max), color, transformed_tex_coords, attr_1, attr_2);
          }

          fill_start = -1;
        }

        if (tile_start == -1) {
          tile_start = x;
          tile_start_winding = winding;
        }

        for (; intersection_index < m_cells.intersections(y).size(); intersection_index++) {
          const Intersection intersection = m_cells.intersections(y)[intersection_index];

          if (intersection.x <= (path_start_cell.x + x) * m_cell_size) {
            break;
          }

          winding += intersection.sign;
        }
      }
    }

    if (tile_start > -1) {
      const dvec2 cell_min = dvec2(path_start_cell + ivec2(0, y)) * m_cell_size;
      const dvec2 cell_max = cell_min + dvec2(tile_start + 1, 1) * m_cell_size;

      vec2 tex_coord_curves_min;
      vec2 tex_coord_curves_max;

      const std::array<vec2, 4> transformed_tex_coords = reproject_texture_coords(
          bounding_rect,
          drect(cell_min, cell_max),
          texture_coords,
          tex_coord_curves_min,
          tex_coord_curves_max);

      const uint32_t attr_1 = TileVertex::create_attr_1(0, fill.paint.type(), row_curves_offset);
      const uint32_t attr_3 = TileVertex::create_attr_3(tile_start_winding, row_curves_count);

      drawable.push_tile(vec2(cell_min),
                         vec2(cell_max),
                         tex_coord_curves_min,
                         tex_coord_curves_max,
                         transformed_tex_coords,
                         color,
                         attr_1,
                         attr_2,
                         attr_3);

      tile_start = -1;
    }
  }

  drawable.paints.push_back(
      {drawable.tiles.size(), drawable.fills.size(), fill.paint.type(), fill.paint.id()});
}

void TiledRenderer::setup(const ivec2 viewport_size,
                          const drect& visible,
                          const mat4& vp_matrix,
                          const uint8_t LOD,
                          const double base_cell_size)
{
  m_viewport_size = viewport_size;
  m_visible = visible;
  m_vp_matrix = vp_matrix;
  m_LOD = LOD;
  m_base_cell_size = base_cell_size;
  m_z_index = 1;

  m_cell_sizes[0] = m_base_cell_size * std::pow(0.5, m_LOD - 1);
  m_cell_sizes[1] = m_base_cell_size * std::pow(0.5, m_LOD);
  m_cell_sizes[2] = m_base_cell_size * std::pow(0.5, m_LOD + 1);

  m_cell_count = ivec2(math::ceil(visible.max / m_cell_sizes[1]) -
                       math::floor(visible.min / m_cell_sizes[1]));

  // m_culled.clear();
  // m_culled.resize(m_cell_count.x * m_cell_count.y, std::numeric_limits<uint32_t>::max());

  m_invalid.clear();
  m_invalid.resize(m_cell_count.x * m_cell_count.y, false);

  m_semivalid.clear();
  m_semivalid.resize(m_cell_count.x * m_cell_count.y, false);

  if (!m_framebuffers || m_framebuffers->size() != m_viewport_size) {
    delete m_framebuffers;
    m_framebuffers = new GPU::DoubleFramebuffer(m_viewport_size);
  } else {
    m_framebuffers->bind();
  }
}

void TiledRenderer::push_drawable(const Drawable* drawable)
{
  if (drawable->tiles.empty() && drawable->fills.empty()) {
    return;
  }

  m_front_stack.push_back(std::make_pair(drawable, m_z_index));
  m_z_index += drawable->paints.size();
}

void TiledRenderer::flush()
{
  GK_ASSERT(m_tile_program && m_fill_program && m_tile_vertex_array && m_fill_vertex_array,
            "Program and vertex array must be set through update_shader()!");

  populate_fills();

  TileBatchData& tiles = m_batch.tiles;
  FillBatchData& fills = m_batch.fills;
  BatchData& data = m_batch.data;

  GPU::RenderState render_state = GPU::RenderState().no_blend().default_depth().no_stencil();

  if (fills.vertices_count()) {
    m_framebuffers->bind();

    fills.vertex_buffer.upload(fills.vertices, fills.vertices_count() * sizeof(FillVertex));

    render_state.program = m_fill_program->program;
    render_state.vertex_array = &m_fill_vertex_array->vertex_array;
    render_state.primitive = fills.primitive;
    render_state.viewport = irect{ivec2::zero(), m_viewport_size};

    render_state.uniforms = {{m_fill_program->vp_uniform, m_vp_matrix}};
    render_state.texture_arrays = std::vector<GPU::TextureArrayBinding>{
        {m_fill_program->textures_uniform, {&data.gradients_texture}}};

    for (const auto& [texture_id, _] : m_binded_textures) {
      const auto it = m_textures->find(texture_id);

      if (it != m_textures->end()) {
        render_state.texture_arrays[0].second.push_back(&it->second);
      } else {
        // TODO: default texture
      }
    }

    GPU::Device::draw_elements(fills.indices_count(), render_state);
  }

  m_framebuffers->swap();

  render_tiles();

  m_z_index = 1;

  m_batch.clear();
  m_front_stack.clear();
  m_back_stack.clear();
}

void TiledRenderer::populate_fills()
{
  __debug_time_total();

  for (auto it = m_front_stack.rbegin(); it != m_front_stack.rend(); it++) {
    const Drawable& drawable = *(it->first);
    const uint32_t z_index = m_z_index - it->second;

    // TODO: limited batch size

    // TODO: should be non_color_paint
    uint32_t texture_index = 0;
    bool has_texture_paint = false;

    // TODO: extract logic
    for (const DrawablePaintBinding& binding : drawable.paints) {
      if (binding.paint_type != Paint::Type::TexturePaint) {
        continue;
      }

      const auto it = m_textures->find(binding.paint_id);
      const auto binded_it = std::find_if(
          m_binded_textures.begin(), m_binded_textures.end(), [binding](const auto& pair) {
            return pair.first == binding.paint_id;
          });

      if (it == m_textures->end()) {
        continue;
      }

      has_texture_paint = true;

      if (binded_it != m_binded_textures.end()) {
        texture_index = binded_it->second;
      } else {
        /* We add 1 to the index to avoid binding the gradients texture. */
        m_binded_textures.push_back(
            std::make_pair(binding.paint_id, m_binded_textures.size() + 1));
      }
    }

    const double cell_size = m_cell_sizes[1];

    // for (size_t i = 0; i < drawable.fills.size() / 4; i++) {
    //   const FillVertex& v1 = drawable.fills[i * 4];
    //   const FillVertex& v3 = drawable.fills[i * 4 + 2];

    //   const ivec2 cell_min = ivec2(math::ceil((dvec2(v1.position) - m_visible.min) /
    //   cell_size)); const ivec2 cell_max = ivec2(math::ceil((dvec2(v3.position) - m_visible.min)
    //   / cell_size));

    //   for (int y = std::max(cell_min.y, 0); y < std::min(cell_max.y, m_cell_count.y); y++) {
    //     for (int x = std::max(cell_min.x, 0); x < std::min(cell_max.x, m_cell_count.x); x++) {
    //       const int index = x + y * m_cell_count.x;

    //       if (m_culled[index] > z_index) {
    //         m_culled[index] = z_index;
    //       }
    //     }
    //   }
    // }

    if (!has_texture_paint && drawable.paints.size() == 1) {
      m_batch.fills.upload(drawable, z_index);
    } else {
      m_batch.fills.upload(drawable, z_index, m_binded_textures);
    }
  }

  // for (int y = 0; y < m_cell_count.y; y++) {
  //   for (int x = 0; x < m_cell_count.x; x++) {
  //     const int index = x + y * m_cell_count.x;

  //     if (m_culled[index] == std::numeric_limits<uint32_t>::max()) {
  //       Renderer::ui_rect(
  //           rect(math::floor(vec2(m_visible.min) / m_cell_sizes[1]) * m_cell_sizes[1] +
  //                    vec2(x, y) * m_cell_sizes[1],
  //                math::floor(vec2(m_visible.min) / m_cell_sizes[1]) * m_cell_sizes[1] +
  //                    vec2(x + 1, y + 1) * m_cell_sizes[1]),
  //           vec4(1.0, 0.0, 1.0, 0.5));
  //     }
  //   }
  // }
}

void TiledRenderer::render_tiles()
{
  __debug_time_total();

  while (true) {
    for (auto it = m_front_stack.begin(); it != m_front_stack.end(); it++) {
      const Drawable& drawable = *(it->first);
      const uint32_t z_index = it->second;

      const ivec2 cell_min = ivec2(math::ceil(drawable.bounding_rect.min - m_visible.min) /
                                   m_cell_sizes[1]);
      const ivec2 cell_max = ivec2(math::ceil(drawable.bounding_rect.max - m_visible.min) /
                                   m_cell_sizes[1]);

      bool semi_valid = false;
      bool invalid = false;

      for (int y = std::max(cell_min.y, 0); y <= std::min(cell_max.y, m_cell_count.y - 1); y++) {
        for (int x = std::max(cell_min.x, 0); x <= std::min(cell_max.x, m_cell_count.x - 1); x++) {
          const int index = x + y * m_cell_count.x;

          if (m_semivalid[index]) {
            semi_valid = true;
          }

          if (m_invalid[index]) {
            invalid = true;
          }
        }

        if (semi_valid && invalid) {
          break;
        }
      }

      if (invalid || (semi_valid && drawable.appearance.blending != BlendingMode::Normal)) {
        for (int y = std::max(cell_min.y, 0); y <= std::min(cell_max.y, m_cell_count.y - 1); y++) {
          for (int x = std::max(cell_min.x, 0); x <= std::min(cell_max.x, m_cell_count.x - 1); x++)
          {
            const int index = x + y * m_cell_count.x;

            m_invalid[index] = true;
          }
        }

        m_back_stack.push_back(std::make_pair(&drawable, z_index));

        continue;
      }

      for (int y = std::max(cell_min.y, 0); y <= std::min(cell_max.y, m_cell_count.y - 1); y++) {
        for (int x = std::max(cell_min.x, 0); x <= std::min(cell_max.x, m_cell_count.x - 1); x++) {
          const int index = x + y * m_cell_count.x;

          m_semivalid[index] = true;
        }
      }

      // TODO: limited batch size

      // TODO: should be non_color_paint
      uint32_t texture_index = 0;
      bool has_texture_paint = false;

      // TODO: extract logic
      for (const DrawablePaintBinding& binding : drawable.paints) {
        if (binding.paint_type != Paint::Type::TexturePaint) {
          continue;
        }

        const auto it = m_textures->find(binding.paint_id);
        const auto binded_it = std::find_if(
            m_binded_textures.begin(), m_binded_textures.end(), [binding](const auto& pair) {
              return pair.first == binding.paint_id;
            });

        if (it == m_textures->end()) {
          continue;
        }

        has_texture_paint = true;

        if (binded_it != m_binded_textures.end()) {
          texture_index = binded_it->second;
        } else {
          /* We add 1 to the index to avoid binding the gradients texture. */
          m_binded_textures.push_back(
              std::make_pair(binding.paint_id, m_binded_textures.size() + 1));
        }
      }

      if (!has_texture_paint && drawable.paints.size() == 1) {
        m_batch.tiles.upload(drawable, m_z_index - z_index);
      } else {
        m_batch.tiles.upload(drawable, m_z_index - z_index, m_binded_textures);
      }
    }

    TileBatchData& tiles = m_batch.tiles;
    BatchData& data = m_batch.data;

    GPU::RenderState render_state = GPU::RenderState().no_blend().default_depth().no_stencil();

    if (tiles.vertices_count()) {
      if (tiles.vertex_buffer.size <= tiles.vertices_count() * sizeof(TileVertex)) {
        console::log("bad bad stuff");
      }

      if (tiles.curves_texture.size.x * tiles.curves_texture.size.y * 4 * sizeof(float) <=
          tiles.max_curves * sizeof(vec2))
      {
        console::log("bad stuff");
      }

      m_framebuffers->blit_back_to_front();

      tiles.vertex_buffer.upload(tiles.vertices, tiles.vertices_count() * sizeof(TileVertex));
      tiles.curves_texture.upload(tiles.curves, tiles.curves_count());

      render_state.default_blend().no_depth_write().no_stencil();

      render_state.program = m_tile_program->program;
      render_state.vertex_array = &m_tile_vertex_array->vertex_array;
      render_state.primitive = tiles.primitive;
      render_state.viewport = irect{ivec2::zero(), m_viewport_size};

      render_state.uniforms = {{m_tile_program->vp_uniform, m_vp_matrix},
                               {m_tile_program->samples_uniform, 3}};
      render_state.textures = std::vector<GPU::TextureBinding>{
          {m_tile_program->curves_texture_uniform, &tiles.curves_texture}};
      render_state.texture_arrays = std::vector<GPU::TextureArrayBinding>{
          {m_tile_program->textures_uniform, {&data.gradients_texture}}};

      for (const auto& [texture_id, _] : m_binded_textures) {
        const auto it = m_textures->find(texture_id);

        if (it != m_textures->end()) {
          render_state.texture_arrays[0].second.push_back(&it->second);
        } else {
          // TODO: default texture
        }
      }

      GPU::Device::draw_elements(tiles.indices_count(), render_state);
    }

    m_batch.clear();

    m_invalid.clear();
    m_invalid.resize(m_cell_count.x * m_cell_count.y, false);

    m_semivalid.clear();
    m_semivalid.resize(m_cell_count.x * m_cell_count.y, false);

    if (m_back_stack.empty()) {
      break;
    }

    m_front_stack.clear();
    m_framebuffers->swap();

    std::swap(m_front_stack, m_back_stack);
  }

  m_framebuffers->blit();
  m_framebuffers->unbind();
}

}  // namespace graphick::renderer
