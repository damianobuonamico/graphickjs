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
#include "renderer.h"

#include <algorithm>
#include <unordered_set>

namespace graphick::renderer {

Tiler::Tiler() : m_zoom(1.0), m_base_cell_size(512.0), m_max_LOD(0) {}

void Tiler::setup(const double zoom, const drect& visible)
{
  const double raw_log = -std::log2(13.0 / (m_base_cell_size * zoom));

  m_visible = visible;
  m_zoom = zoom;
  m_max_LOD = static_cast<uint8_t>(math::clamp(std::round(raw_log), 0.0, 24.0));

  m_max_LOD = std::min(m_max_LOD, uint8_t(4));

  m_cell_size = m_base_cell_size * std::pow(0.5, m_max_LOD);
  m_cell_count = ivec2(math::ceil(visible.max / m_cell_size) -
                       math::floor(visible.min / m_cell_size));

  m_culled.clear();
  m_culled.resize(m_cell_count.x * m_cell_count.y, false);
}

void Tiler::tile(const geom::dcubic_path& path,
                 const drect& bounding_rect,
                 const Fill& fill,
                 Drawable& drawable)
{
  /* A segment is considered in a cell if within the tolerance of the cell boundary. */

  const double tolerance = 2.0 / m_zoom;

  const uvec4 color = fill.paint.is_color() ? uvec4(fill.paint.color() * 255.0f) :
                                              uvec4(255, 255, 255, 255);

  const uint32_t attr_1 = TileVertex::create_attr_1(0, fill.paint.type(), drawable.curves.size());
  const uint32_t attr_2 = TileVertex::create_attr_2(0, false, fill.rule == FillRule::EvenOdd, 0);

  const dvec2 bounds_size = bounding_rect.size();

  /* Setting up the workspace, a 1 cell padding in all directions is applied. */

  const ivec2 path_start_cell = ivec2(math::floor(bounding_rect.min / m_cell_size)) - 1;
  const ivec2 path_end_cell = ivec2(math::ceil(bounding_rect.max / m_cell_size)) - 1;
  const ivec2 path_cell_count = path_end_cell - path_start_cell + 2;

  m_cells.clear();
  m_cells.resize(path_cell_count.x * path_cell_count.y);

  m_curves.clear();
  m_curves.resize(path_cell_count.x * path_cell_count.y * 8);

  m_extra_curves.clear();
  m_curves_map.clear();

  m_intersections.clear();
  m_intersections.resize(path_cell_count.y);

  for (uint16_t i = 0; i < path.size(); i++) {
    /* Being monotonic, it is straightforward to determine which cells the curve intersects. */

    const dvec2 p0 = path[i * 3];
    const dvec2 p1 = path[i * 3 + 1];
    const dvec2 p2 = path[i * 3 + 2];
    const dvec2 p3 = path[i * 3 + 3];

    if (math::squared_distance(p0, p3) < math::geometric_epsilon<double>) {
      continue;
    }

    // Renderer::ui_circle(vec2(p0), 5.0, vec4(0.0, 0.0, 1.0, 1.0));

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

      const uint16_t curves_count =
          m_cells[start_cell.y * path_cell_count.x + start_cell.x].curves_count;

      m_curves[(start_cell.y * path_cell_count.x + start_cell.x) * 8 + curves_count] = i;
      m_cells[start_cell.y * path_cell_count.x + start_cell.x].curves_count++;

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

    const uint16_t curves_count = m_cells[y * path_cell_count.x + x].curves_count;

    m_curves[(y * path_cell_count.x + x) * 8 + curves_count] = i;
    m_cells[y * path_cell_count.x + x].curves_count++;

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
          const uint16_t curves_count = m_cells[y * path_cell_count.x + xc].curves_count;

          m_curves[(y * path_cell_count.x + xc) * 8 + curves_count] = i;
          m_cells[y * path_cell_count.x + xc].curves_count++;
        }

        if (x_cell_alt_tol != x_cell) {
          const uint16_t curves_count =
              m_cells[(y - y_inc) * path_cell_count.x + x_cell_alt_tol].curves_count;

          m_curves[((y - y_inc) * path_cell_count.x + x_cell_alt_tol) * 8 + curves_count] = i;
          m_cells[(y - y_inc) * path_cell_count.x + x_cell_alt_tol].curves_count++;
        }

        if ((up && t0 >= -math::epsilon<double> && t0 < 1.0 - math::epsilon<double>) ||
            (!up && t0 > math::epsilon<double> && t0 <= 1.0 + math::epsilon<double>))
        {
          // TODO: avoid push_backs, maybe preallocate
          m_intersections[y - (y_inc - 1) / 2].push_back({x0, up ? int8_t(1) : int8_t(-1)});
        }

        x = x_cell;
      }
    } else {
      const auto& [a, b, c, d] = geom::cubic_coefficients(p0, p1, p2, p3);

      for (; y_inc * y > y_inc * end_cell.y; y -= y_inc) {
        const double y0 = (y + path_start_cell.y - (y_inc - 1) / 2) * m_cell_size;
        const double t0 = (y0 - p0.y) / (p3.y - p0.y);
        const double clamped_t0 = math::clamp(t0, 0.0, 1.0);

        const double t = math::is_almost_equal(clamped_t0, 1.0) ||
                                 math::is_almost_equal(clamped_t0, 0.0) ?
                             clamped_t0 :
                             geom::cubic_line_intersect_approx(a.y, b.y, c.y, d.y, y0, t0);
        const double t_sq = t * t;
        const double x0 = math::clamp(
            a.x * t_sq * t + b.x * t_sq + c.x * t + d.x, bounding_rect.min.x, bounding_rect.max.x);

        const int x_cell = int(std::floor(x0 / m_cell_size)) - path_start_cell.x;
        const int x_cell_tol = int(std::floor((x0 + x_tol) / m_cell_size)) - path_start_cell.x;
        const int x_cell_alt_tol = int(std::floor((x0 - x_tol) / m_cell_size)) - path_start_cell.x;

        for (int xc = std::min(x, x_cell_tol); xc <= std::max(x, x_cell_tol); xc++) {
          const uint16_t curves_count = m_cells[y * path_cell_count.x + xc].curves_count;

          m_curves[(y * path_cell_count.x + xc) * 8 + curves_count] = i;
          m_cells[y * path_cell_count.x + xc].curves_count++;
        }

        if (x_cell_alt_tol != x_cell) {
          const uint16_t curves_count =
              m_cells[(y - y_inc) * path_cell_count.x + x_cell_alt_tol].curves_count;

          m_curves[((y - y_inc) * path_cell_count.x + x_cell_alt_tol) * 8 + curves_count] = i;
          m_cells[(y - y_inc) * path_cell_count.x + x_cell_alt_tol].curves_count++;
        }

        if ((up && t0 >= -math::epsilon<double> && t0 < 1.0 - math::epsilon<double>) ||
            (!up && t0 > math::epsilon<double> && t0 <= 1.0 + math::epsilon<double>))
        {
          m_intersections[y - (y_inc - 1) / 2].push_back({x0, up ? int8_t(1) : int8_t(-1)});
        }

        x = x_cell;
      }
    }

    if (x != end_cell.x) {
      for (int xc = std::min(x, end_cell.x); xc <= std::max(x, end_cell.x); xc++) {
        const uint16_t curves_count = m_cells[y * path_cell_count.x + xc].curves_count;

        m_curves[(y * path_cell_count.x + xc) * 8 + curves_count] = i;
        m_cells[y * path_cell_count.x + xc].curves_count++;
      }
    }

    const uint16_t end_curves_count = m_cells[end_cell.y * path_cell_count.x + end_cell.x].curves_count;

    m_curves[(end_cell.y * path_cell_count.x + end_cell.x) * 8 + end_curves_count] = i;
    m_cells[end_cell.y * path_cell_count.x + end_cell.x].curves_count++;
  }

  /* Create tiles and fills. */

  for (int y = 0; y < path_cell_count.y; y++) {
    int intersection_index = 0;
    int winding = 0;

    int fill_start = -1;
    int tile_start = -1;
    int tile_start_winding = 0;

    std::sort(m_intersections[y].begin(),
              m_intersections[y].end(),
              [](const Intersection a, const Intersection b) { return a.x > b.x; });

    std::unordered_set<uint16_t> tile_curves_indices;
    uint32_t tile_curves_offset = drawable.curves.size() / 2;
    uint16_t tile_curves_count = 0;

    for (int x = path_cell_count.x - 1; x >= 0; x--) {
      // {
      //   const dvec2 cell_min = dvec2(path_start_cell + ivec2(x, y)) * m_cell_size;
      //   const dvec2 cell_max = cell_min + m_cell_size;

      //   const drect cell_rect = drect(cell_min, cell_max);

      //   Renderer::ui_rect(cell_rect, vec4(0.0, 0.0, 1.0, 0.5));
      // }

      const int cell_index = y * path_cell_count.x + x;
      const uint16_t curves_count = m_cells[cell_index].curves_count;

#if 1
      if (curves_count == 0) {
        if (tile_start > -1) {
          const dvec2 cell_min = dvec2(path_start_cell + ivec2(x + 1, y)) * m_cell_size;
          const dvec2 cell_max = cell_min + dvec2(tile_start - x, 1) * m_cell_size;

          const vec2 tex_coord_curves_min = vec2((cell_min - bounding_rect.min) / bounds_size);
          const vec2 tex_coord_curves_max = vec2((cell_max - bounding_rect.min) / bounds_size);

          const uint32_t attr_1 = TileVertex::create_attr_1(
              0, fill.paint.type(), tile_curves_offset);
          const uint32_t attr_3 = TileVertex::create_attr_3(tile_start_winding, tile_curves_count);

          drawable.push_tile(vec2(cell_min),
                             vec2(cell_max),
                             tex_coord_curves_min,
                             tex_coord_curves_max,
                             color,
                             attr_1,
                             attr_2,
                             attr_3);

          tile_curves_offset = drawable.curves.size() / 2;
          tile_curves_indices.clear();
          tile_curves_count = 0;

          tile_start = -1;
        }

        if (fill_start == -1) {
          fill_start = x;
        }
      } else {
        if (curves_count <= 8) {
          for (uint16_t i = 0; i < curves_count; i++) {
            const uint16_t curve_index = m_curves[cell_index * 8 + i];

            if (tile_curves_indices.find(curve_index) == tile_curves_indices.end()) {
              tile_curves_indices.insert(curve_index);

              const vec2 p0 = vec2((path[curve_index * 3] - bounding_rect.min) / bounds_size);
              const vec2 p1 = vec2((path[curve_index * 3 + 1] - bounding_rect.min) / bounds_size);
              const vec2 p2 = vec2((path[curve_index * 3 + 2] - bounding_rect.min) / bounds_size);
              const vec2 p3 = vec2((path[curve_index * 3 + 3] - bounding_rect.min) / bounds_size);

              drawable.curves.insert(drawable.curves.end(), {p0, p1, p2, p3});

              tile_curves_count++;
            }
          }
        }

        if (fill_start > -1) {
          if (fill.rule == FillRule::NonZero ? (winding != 0) : (winding % 2 != 0)) {
            const vec2 cell_min = vec2(path_start_cell + ivec2(x + 1, y)) * m_cell_size;
            const vec2 cell_max = cell_min + vec2(fill_start - x, 1) * m_cell_size;

            drawable.push_fill(cell_min,
                               cell_max,
                               color,
                               {vec2::zero(), vec2::zero(), vec2::zero(), vec2::zero()},
                               attr_1,
                               attr_2);
          }

          fill_start = -1;
        }

        if (tile_start == -1) {
          tile_start = x;
          tile_start_winding = winding;
        }

        for (; intersection_index < m_intersections[y].size(); intersection_index++) {
          const Intersection intersection = m_intersections[y][intersection_index];

          if (intersection.x <= (path_start_cell.x + x) * m_cell_size) {
            break;
          }

          winding += intersection.sign;
        }
      }
#else

      // if (!m_cells[cell_index].curves.empty()) {
      if (curves_count != 0) {
        const dvec2 cell_min = dvec2(path_start_cell + ivec2(x, y)) * m_cell_size;
        const dvec2 cell_max = cell_min + m_cell_size;

        const vec2 tex_coord_curves_min = vec2((cell_min - bounding_rect.min) / bounds_size);
        const vec2 tex_coord_curves_max = vec2((cell_max - bounding_rect.min) / bounds_size);

        uint32_t curves_offset = drawable.curves.size() / 2;

        __debug_text(std::to_string(curves_count),
                     vec2(200.0 + (cell_min + cell_max) / 2),
                     vec4::identity());

        if (curves_count <= 8) {
          const int hash = math::hash(m_curves, cell_index * 8, curves_count);
          const auto it = m_curves_map.find(hash);

          if (it == m_curves_map.end()) {
            for (uint16_t i = 0; i < 8; i++) {
              const uint16_t curve_index = m_curves[cell_index * 8 + i];

              const vec2 p0 = vec2((path[curve_index * 3] - bounding_rect.min) / bounds_size);
              const vec2 p1 = vec2((path[curve_index * 3 + 1] - bounding_rect.min) / bounds_size);
              const vec2 p2 = vec2((path[curve_index * 3 + 2] - bounding_rect.min) / bounds_size);
              const vec2 p3 = vec2((path[curve_index * 3 + 3] - bounding_rect.min) / bounds_size);

              drawable.curves.insert(drawable.curves.end(), {p0, p1, p2, p3});
            }

            m_curves_map.insert(std::make_pair(hash, curves_offset));
          } else {
            curves_offset = m_curves_map[hash];
          }
        } else {
          // TODO: implement
        }

        for (; intersection_index < m_intersections[y].size(); intersection_index++) {
          const Intersection intersection = m_intersections[y][intersection_index];

          if (intersection.x >= cell_min.x) {
            break;
          }

          winding += intersection.sign;
        }

        const uint32_t attr_1 = TileVertex::create_attr_1(0, fill.paint.type(), curves_offset);
        const uint32_t attr_3 = TileVertex::create_attr_3(winding, curves_count);

        drawable.push_tile(vec2(cell_min),
                           vec2(cell_max),
                           tex_coord_curves_min,
                           tex_coord_curves_max,
                           color,
                           attr_1,
                           attr_2,
                           attr_3);

        // const drect cell_rect = drect(cell_min, cell_max);

        // if (curves_count == 8) {
        //   Renderer::ui_rect(cell_rect, vec4(0.0, 1.0, 0.0, 1.0));
        // } else {
        //   Renderer::ui_rect(cell_rect,
        //                     vec4(1.0, 0.0, 1.0, 1.0 / 8 * curves_count));
        // }

        if (fill_start > -1) {
          // int winding = 0;

          // for (const Intersection intersection : m_intersections[y]) {
          //   if (intersection.x < (x + path_start_cell.x) * m_cell_size) {
          //     winding += intersection.sign;
          //   }
          // }

          if (fill.rule == FillRule::NonZero ? (winding != 0) : (winding % 2 != 0)) {
            const vec2 cell_min = vec2(path_start_cell + ivec2(fill_start, y)) * m_cell_size;
            const vec2 cell_max = cell_min + vec2(x - fill_start, 1) * m_cell_size;

            drawable.push_fill(cell_min,
                               cell_max,
                               color,
                               {vec2::zero(), vec2::zero(), vec2::zero(), vec2::zero()},
                               attr_1,
                               attr_2);

            // for (int xf = fill_start; xf < x; xf++) {
            // const dvec2 cell_min = dvec2(path_start_cell + ivec2(xf, y)) * m_cell_size;
            // const dvec2 cell_max = cell_min + m_cell_size;

            // const drect cell_rect = drect(cell_min, cell_max);

            // Renderer::ui_rect(cell_rect, vec4(0.0, 1.0, 0.0, 0.5));
            // }
          }

          fill_start = -1;
        }
      } else if (fill_start == -1) {
        fill_start = x;
      }

      // if (!m_cells[cell_index].curves.empty()) {
      //   const dvec2 cell_min = dvec2(path_start_cell + ivec2(x, y)) * m_cell_size;
      //   const dvec2 cell_max = cell_min + m_cell_size;

      //   const drect cell_rect = drect(cell_min, cell_max);

      //   Renderer::ui_rect(cell_rect, vec4(1.0, 1.0, 0.0, 0.5));
      // }
#endif
    }

    if (tile_start > -1) {
      const dvec2 cell_min = dvec2(path_start_cell + ivec2(0, y)) * m_cell_size;
      const dvec2 cell_max = cell_min + dvec2(tile_start + 1, 1) * m_cell_size;

      const vec2 tex_coord_curves_min = vec2((cell_min - bounding_rect.min) / bounds_size);
      const vec2 tex_coord_curves_max = vec2((cell_max - bounding_rect.min) / bounds_size);

      const uint32_t attr_1 = TileVertex::create_attr_1(0, fill.paint.type(), tile_curves_offset);
      const uint32_t attr_3 = TileVertex::create_attr_3(tile_start_winding, tile_curves_count);

      drawable.push_tile(vec2(cell_min),
                         vec2(cell_max),
                         tex_coord_curves_min,
                         tex_coord_curves_max,
                         color,
                         attr_1,
                         attr_2,
                         attr_3);

      tile_curves_offset = drawable.curves.size() / 2;
      tile_curves_indices.clear();
      tile_curves_count = 0;

      tile_start = -1;
    }
  }

  // for (int y = 0; y < path_cell_count.y; y++) {
  //   const double y0 = double(path_start_cell.y + y) * m_cell_size;

  //   for (const Intersection intersection : m_intersections[y]) {
  //     Renderer::ui_circle(vec2(intersection.x, y0), 2.0, vec4(1.0, 1.0, 1.0, 0.5));
  //   }
  // }
}

void TiledRenderer::push_drawable(const Drawable& drawable)
{
  // // TODO: check for gradients, textures, etc.
  // if (!m_batch.fills.can_handle_quads(drawable.fills.size() / 4) ||
  //     !m_batch.tiles.can_handle_quads(drawable.tiles.size() / 4) ||
  //     !m_batch.tiles.can_handle_curves(drawable.curves.size() / 4) ||
  //     !m_batch.tiles.can_handle_bands(drawable.bands.size()))
  // {
  //   flush_meshes();
  // }

  // uint32_t texture_index = 0;
  // // TODO: should be non_color_paint
  // bool has_texture_paint = false;

  // for (const DrawablePaintBinding& binding : drawable.paints) {
  //   if (binding.paint_type != Paint::Type::TexturePaint) {
  //     continue;
  //   }

  //   has_texture_paint = true;

  //   const auto it = m_textures.find(binding.paint_id);
  //   const auto binded_it = m_binded_textures.find(binding.paint_id);

  //   if (it == m_textures.end()) {
  //     continue;
  //   }

  //   if (binded_it != m_binded_textures.end()) {
  //     texture_index = binded_it->second;
  //   } else {
  //     /* We add 1 to the index to avoid binding the gradients texture. */
  //     m_binded_textures.insert(std::make_pair(binding.paint_id, m_binded_textures.size() + 1));
  //   }
  // }

  // if (!has_texture_paint && drawable.paints.size() == 1) {
  m_batch.fills.upload(drawable, m_z_index);
  m_batch.tiles.upload(drawable, m_z_index);
  // } else {
  //   m_batch.fills.upload(drawable, m_z_index, m_binded_textures);
  //   m_batch.tiles.upload(drawable, m_z_index, m_binded_textures);
  // }

  m_z_index += drawable.paints.size();
}

void TiledRenderer::flush(const ivec2 viewport_size, const mat4& vp_matrix, const float zoom)
{
  GK_ASSERT(m_tile_program && m_fill_program && m_tile_vertex_array && m_fill_vertex_array,
            "Program and vertex array must be set through update_shader()!");

  TileBatchData& tiles = m_batch.tiles;
  FillBatchData& fills = m_batch.fills;
  BatchData& data = m_batch.data;

  GPU::RenderState render_state = GPU::RenderState().no_blend().default_depth().no_stencil();

  if (fills.vertices_count()) {
    fills.vertex_buffer.upload(fills.vertices, fills.vertices_count() * sizeof(FillVertex));

    render_state.program = m_fill_program->program;
    render_state.vertex_array = &m_fill_vertex_array->vertex_array;
    render_state.primitive = fills.primitive;
    render_state.viewport = irect{ivec2::zero(), viewport_size};

    render_state.uniforms = {{m_fill_program->vp_uniform, vp_matrix}};
    // render_state.texture_arrays = std::vector<GPU::TextureArrayBinding>{
    //     {m_programs.tile_program.textures_uniform, {&data.gradients_texture}}};

    GPU::Device::draw_elements(fills.indices_count(), render_state);
  }

  if (tiles.vertices_count()) {
    tiles.vertex_buffer.upload(tiles.vertices, tiles.vertices_count() * sizeof(TileVertex));

    // TODO: upload only necessary data
    tiles.curves_texture.upload(tiles.curves, tiles.max_curves * sizeof(vec2));

    render_state.default_blend().no_depth_write().no_stencil();

    render_state.program = m_tile_program->program;
    render_state.vertex_array = &m_tile_vertex_array->vertex_array;
    render_state.primitive = tiles.primitive;
    render_state.viewport = irect{ivec2::zero(), viewport_size};

    render_state.uniforms = {{m_tile_program->vp_uniform, vp_matrix},
                             {m_tile_program->samples_uniform, 3}};
    render_state.textures = std::vector<GPU::TextureBinding>{
        {m_tile_program->bands_texture_uniform, &tiles.bands_texture},
        {m_tile_program->curves_texture_uniform, &tiles.curves_texture}};

    GPU::Device::draw_elements(tiles.indices_count(), render_state);
  }

  m_batch.clear();
}

}  // namespace graphick::renderer
