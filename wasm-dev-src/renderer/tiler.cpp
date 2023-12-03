/**
 * @file tiler.cpp
 * @brief Tiler implementation.
 *
 * @todo further optimize the tiler (i.e. Contour::push_segment() can definitely be faster).
 * @todo avoid useless float to double casts.
 * @todo simplify and move offset logic from DrawableTiler to Tiler.
 * @todo boundary checking or error handling in MemoryPool::emplace_segment().
 * @todo batch spans by row.
 * @todo reimplement clipping
 * @todo abstract away batching logic in tiler (bounds checking, etc.)
 * @todo can do better job tessellating only in frame curves, keeping the rest as linear
 * @todo fix precision issues on zoom + distance from origin
 * @todo fix border tiles being masked instead of filled
 */

#include "tiler.h"

#include "geometry/path.h"
#include "geometry/contour.h"

#include "../math/mat2x3.h"
#include "../math/matrix.h"
#include "../math/math.h"

 // TEMP
#include "renderer.h"
#include "geometry/internal.h"
#include "../utils/console.h"

#define SEGMENTS_MEMORY_POOL_SIZE 30
#define OVERLAP_CLIP_THRESHOLD 0.7f

namespace Graphick::Renderer {

  /**
   * @brief Calculates the tile coordinates of a point.
   *
   * @param p The point.
   * @return The tile coordinates.
   */
  static inline ivec2 tile_coords(const dvec2 p) {
    return {
      static_cast<int>(std::round(p.x / TILE_SIZE)),
      static_cast<int>(std::round(p.y / TILE_SIZE))
    };
  }

  /**
   * @brief Calculates the tile coordinates of a point and clamps them to the tiles count.
   *
   * @param p The point.
   * @param tiles_count The tiles count.
   * @return The tile coordinates.
   */
  static inline ivec2 tile_coords_clamp(const vec2 p, const ivec2 tiles_count) {
    return {
      std::clamp(static_cast<int>(std::floorf(p.x / TILE_SIZE)), 0, tiles_count.x - 1),
      std::clamp(static_cast<int>(std::floorf(p.y / TILE_SIZE)), 0, tiles_count.y - 1)
    };
  }

  /**
   * @brief Calculates the tile index of a tile.
   *
   * This function is slower than the (int16_t, int16_t, int16_t) variant.
   *
   * @param coords The tile coordinates.
   * @param tiles_count The tiles count.
   * @return The tile index.
   */
  static inline int tile_index(const ivec2 coords, const ivec2 tiles_count) {
    return coords.x + coords.y * tiles_count.x;
  }

  /**
   * @brief Calculates the tile index of a tile.
   *
   * This function is faster than the (ivec2, ivec2) variant.
   *
   * @param tile_x The x coordinate of the tile.
   * @param tile_y The y coordinate of the tile.
   * @param tiles_count_x The x dimension of the tiles count.
   * @return The tile index.
   */
  static inline size_t tile_index(const int16_t tile_x, const int16_t tile_y, const int16_t tiles_count_x) {
    return static_cast<size_t>(std::max(0, tile_x + tile_y * tiles_count_x));
  }

  /**
   * @brief Applies a transformation matrix to a vec2 point and returns the result as a f24x8x2.
   *
   * @param transform The transformation matrix.
   * @param point The point.
   * @param offset The offset to apply to the point.
   * @param subpixel The subpixel offset to apply to the point.
   * @param zoom The zoom to apply to the point.
   * @return The transformed point.
   */
  inline static f24x8x2 f_transform_point(const mat2x3& transform, const vec2 point, const dvec2 offset, const double zoom) {
    double x = (
      static_cast<double>(transform[0][0]) * static_cast<double>(point.x) +
      static_cast<double>(transform[0][1]) * static_cast<double>(point.y) +
      static_cast<double>(transform[0][2]) - offset.x
      ) * zoom;
    double y = (
      static_cast<double>(transform[1][0]) * static_cast<double>(point.x) +
      static_cast<double>(transform[1][1]) * static_cast<double>(point.y) +
      static_cast<double>(transform[1][2]) - offset.y
      ) * zoom;

    return Math::double_to_f24x8x2(x, y);
  }

  inline static dvec2 d_transform_point(const mat2x3& transform, const vec2 point, const dvec2 offset, const double zoom) {
    double x = (
      static_cast<double>(transform[0][0]) * static_cast<double>(point.x) +
      static_cast<double>(transform[0][1]) * static_cast<double>(point.y) +
      static_cast<double>(transform[0][2]) - offset.x
      ) * zoom;
    double y = (
      static_cast<double>(transform[1][0]) * static_cast<double>(point.x) +
      static_cast<double>(transform[1][1]) * static_cast<double>(point.y) +
      static_cast<double>(transform[1][2]) - offset.y
      ) * zoom;

    return { x, y };
  }

  static void clip_drawable(Drawable& drawable, const f24x8x4 clip) {
    for (auto& contour : drawable.contours) {
      Math::clip(contour.points, clip);
      contour.close();
    }

    drawable.bounds.x0 = std::max(drawable.bounds.x0, clip.x0);
    drawable.bounds.y0 = std::max(drawable.bounds.y0, clip.y0);
    drawable.bounds.x1 = std::min(drawable.bounds.x1, clip.x1);
    drawable.bounds.y1 = std::min(drawable.bounds.y1, clip.y1);
  }

  /* -- DrawableTiler::MemoryPool -- */

  DrawableTiler::MemoryPool::MemoryPool() : m_tiles(nullptr), m_segments(nullptr), m_size(0), m_capacity(0) {}

  DrawableTiler::MemoryPool::~MemoryPool() {
    delete[] m_tiles;
    delete[] m_segments;
  }

  void DrawableTiler::MemoryPool::resize(const size_t new_size) {
    if (new_size > m_capacity) {
      /* Reallocation needed, the old data is discarded. */

      delete[] m_tiles;
      delete[] m_segments;

      m_tiles = new Tile[new_size];
      m_segments = new f8x8x4[SEGMENTS_MEMORY_POOL_SIZE * new_size / 2];
      m_segments_ptr = m_segments;

      m_capacity = new_size;
      m_size = new_size;

      return;
    }

    /* Clear the old data. */

    for (size_t i = 0; i < m_size; i++) {
      Tile& tile = m_tiles[i];

      if (!tile.active) continue;

      tile.active = false;
      tile.winding = 0;
      tile.segments.clear();

      std::memset(tile.cover_table, 0, sizeof(tile.cover_table));
    }

    m_segments_ptr = m_segments;
    m_size = new_size;
  }

  void DrawableTiler::MemoryPool::emplace_segment(const f8x8x4 segment, const size_t tile_index) {
    Tile& tile = m_tiles[tile_index];

    if (!tile.segments.empty()) {
      auto& [segments, size] = tile.segments.back();

      if (size < SEGMENTS_MEMORY_POOL_SIZE) {
        std::memcpy(segments + size, &segment, sizeof(f8x8x4));
        size += 1;

        return;
      }
    }

    std::memcpy(m_segments_ptr, &segment, sizeof(f8x8x4));

    m_tiles[tile_index].segments.emplace_back(m_segments_ptr, 1);
    m_segments_ptr += SEGMENTS_MEMORY_POOL_SIZE;
  }

  /* -- DrawableTiler -- */

  DrawableTiler::DrawableTiler(const Drawable& drawable, const ivec2 position, const ivec2 tiles_count, MemoryPool* pool) {
    ivec2 min_coords = position + ivec2{ (drawable.bounds.x0 / TILE_SIZE) >> FRACBITS, (drawable.bounds.y0 / TILE_SIZE) >> FRACBITS };
    ivec2 max_coords = position + ivec2{ (drawable.bounds.x1 / TILE_SIZE) >> FRACBITS, (drawable.bounds.y1 / TILE_SIZE) >> FRACBITS };

    m_offset = min_coords;
    m_size = max_coords - min_coords;

    pool->resize(m_size.x * m_size.y);

    for (const Geometry::Contour& contour : drawable.contours) {
      if (contour.points.size() < 2) continue;

      f24x8 x_p = contour.points.front().x - drawable.bounds.x0;
      f24x8 y_p = contour.points.front().y - drawable.bounds.y0;

      move_to(x_p, y_p);

      for (size_t i = 1; i < contour.points.size(); i++) {
        x_p = contour.points[i].x - drawable.bounds.x0;
        y_p = contour.points[i].y - drawable.bounds.y0;

        line_to(x_p, y_p, pool);
      }
    }

    pack(drawable.paint.rule, tiles_count, pool);
  }

  void DrawableTiler::move_to(f24x8 x, f24x8 y) {
    m_x = x;
    m_y = y;

    m_tile_x = (x >> FRACBITS) / TILE_SIZE;
    m_tile_y = (y >> FRACBITS) / TILE_SIZE;
  }

  void DrawableTiler::line_to(f24x8 x, f24x8 y, MemoryPool* pool) {
    if (m_x == x && m_y == y) return;

    m_tile_y_prev = m_tile_y;

    int16_t to_tile_x = (x >> FRACBITS) / TILE_SIZE;
    int16_t to_tile_y = (y >> FRACBITS) / TILE_SIZE;

    if (m_tile_x == to_tile_x && m_tile_y == to_tile_y) {
      f24x8 tile_pos_x = (m_tile_x * TILE_SIZE) << FRACBITS;
      f24x8 tile_pos_y = (m_tile_y * TILE_SIZE) << FRACBITS;

      f8x8 x0 = static_cast<f8x8>(m_x - tile_pos_x);
      f8x8 y0 = static_cast<f8x8>(m_y - tile_pos_y);
      f8x8 x1 = static_cast<f8x8>(x - tile_pos_x);
      f8x8 y1 = static_cast<f8x8>(y - tile_pos_y);

      size_t index = tile_index(m_tile_x, m_tile_y, m_size.x);

      Tile& tile = pool->get(index);

      if (!tile.active) {
        tile.active = true;
        m_masks_num++;
      }

      if (y0 != y1) {
        pool->emplace_segment({ x0, y0, x1, y1 }, index);

        float cover = 1;

        if (y0 > y1) {
          cover = -1.0f;
          std::swap(y0, y1);
        }

        f8x8 y0_int = Math::int_bits(y0);
        f8x8 y1_int = std::min(Math::int_bits(y1) + FRACUNIT, TILE_SIZE << FRACBITS);

        int8_t i0 = static_cast<int8_t>(y0_int >> FRACBITS);
        int8_t i1 = static_cast<int8_t>(y1_int >> FRACBITS);

        tile.cover_table[i0] += cover * Math::f8x8_to_float(y0_int + FRACUNIT - y0);

        for (int j = i0 + 1; j < i1; j++) {
          tile.cover_table[j] += cover;
        }

        tile.cover_table[i1 - 1] -= cover * Math::f8x8_to_float(y1_int - y1);
      }

      m_x = x;
      m_y = y;

      return;
    }

    f24x8 vec_x = x - m_x;
    f24x8 vec_y = y - m_y;

    f24x8 dir_x = Math::sign(vec_x);
    f24x8 dir_y = Math::sign(vec_y);

    int16_t x_tile_dir = static_cast<int16_t>(std::max(0, dir_x) * TILE_SIZE);
    int16_t y_tile_dir = static_cast<int16_t>(std::max(0, dir_y) * TILE_SIZE);

    float fvec_x = Math::f24x8_to_float(vec_x);
    float fvec_y = Math::f24x8_to_float(vec_y);
    float t1_x = std::numeric_limits<float>::infinity();
    float t1_y = std::numeric_limits<float>::infinity();
    float dtdx = TILE_SIZE / fvec_x;
    float dtdy = TILE_SIZE / fvec_y;
    float step_x = std::abs(dtdx);
    float step_y = std::abs(dtdy);

    if (y != m_y) {
      f24x8 next_y = ((m_tile_y + (y > m_y ? 1 : 0)) * TILE_SIZE) << FRACBITS;
      t1_x = std::min(1.0f, Math::f24x8_to_float(next_y - m_y) / fvec_y);
    }

    if (x != m_x) {
      f24x8 next_x = ((m_tile_x + (x > m_x ? 1 : 0)) * TILE_SIZE) << FRACBITS;
      t1_y = std::min(1.0f, Math::f24x8_to_float(next_x - m_x) / fvec_x);
    }

    f24x8 from_x = m_x;
    f24x8 from_y = m_y;

    while (true) {
      float t1 = std::min(t1_x, t1_y);

      f24x8 to_x = m_x + Math::float_to_f24x8(t1 * fvec_x);
      f24x8 to_y = m_y + Math::float_to_f24x8(t1 * fvec_y);
      f24x8 tile_pos_x = (m_tile_x * TILE_SIZE) << FRACBITS;
      f24x8 tile_pos_y = (m_tile_y * TILE_SIZE) << FRACBITS;

      f8x8 x0 = static_cast<f8x8>(from_x - tile_pos_x);
      f8x8 y0 = static_cast<f8x8>(from_y - tile_pos_y);
      f8x8 x1 = static_cast<f8x8>(to_x - tile_pos_x);
      f8x8 y1 = static_cast<f8x8>(to_y - tile_pos_y);

      size_t index = tile_index(m_tile_x, m_tile_y, m_size.x);

      Tile& tile = pool->get(index);

      if (!tile.active) {
        tile.active = true;
        m_masks_num++;
      }

      if (y0 != y1) {
        pool->emplace_segment({ x0, y0, x1, y1 }, index);

        float cover = 1;

        if (y0 > y1) {
          cover = -1.0f;
          std::swap(y0, y1);
        }

        f8x8 y0_int = Math::int_bits(y0);
        f8x8 y1_int = std::min(Math::int_bits(y1) + FRACUNIT, TILE_SIZE << FRACBITS);

        int8_t i0 = static_cast<int8_t>(y0_int >> FRACBITS);
        int8_t i1 = static_cast<int8_t>(y1_int >> FRACBITS);

        tile.cover_table[i0] += cover * Math::f8x8_to_float(y0_int + FRACUNIT - y0);

        for (int j = i0 + 1; j < i1; j++) {
          tile.cover_table[j] += cover;
        }

        tile.cover_table[i1 - 1] -= cover * Math::f8x8_to_float(y1_int - y1);
      }


      bool fuzzy_equal;

      if (t1_x < t1_y) {
        fuzzy_equal = t1_x >= 1.0f - 0.0001f;
        t1_x = std::min(1.0f, t1_x + step_y);

        from_x = to_x;
        from_y = (m_tile_y * TILE_SIZE + y_tile_dir) << FRACBITS;

        m_tile_y += dir_y;
      } else {
        fuzzy_equal = t1_y >= 1.0f - 0.0001f;
        t1_y = std::min(1.0f, t1_y + step_x);

        from_x = (m_tile_x * TILE_SIZE + x_tile_dir) << FRACBITS;
        from_y = to_y;

        m_tile_x += dir_x;
      }

      if (fuzzy_equal) {
        m_tile_x = (x >> FRACBITS) / TILE_SIZE;
        m_tile_y = (y >> FRACBITS) / TILE_SIZE;
      }

      if (m_tile_y != m_tile_y_prev) {
        size_t sign_index = tile_index(m_tile_x, std::min(m_tile_y, m_tile_y_prev), m_size.x);

        Tile& sign_tile = pool->get(sign_index);

        if (!sign_tile.active) {
          sign_tile.active = true;
          m_masks_num++;
        }

        sign_tile.winding += (int8_t)(m_tile_y - m_tile_y_prev);
        m_tile_y_prev = m_tile_y;
      }

      if (fuzzy_equal) break;
    }

    m_x = x;
    m_y = y;

    m_tile_x = to_tile_x;
    m_tile_y = to_tile_y;
  }

  void DrawableTiler::pack(const FillRule rule, const ivec2 tiles_count, MemoryPool* pool) {
    float cover_table[TILE_SIZE] = { 0.0f };
    int winding = 0;

    m_masks.reserve(m_masks_num);

    for (int16_t y = 0; y < m_size.y; y++) {
      std::memset(cover_table, 0, TILE_SIZE * sizeof(float));
      winding = 0;

      for (int16_t x = 0; x < m_size.x; x++) {
        size_t index = tile_index(x, y, m_size.x);
        Tile& tile = pool->get(index);

        if (tile.active) {
          Mask& mask = m_masks.emplace_back();

          winding += tile.winding;

          mask.tile_x = x;
          mask.tile_y = y;

          std::memcpy(mask.cover_table, cover_table, TILE_SIZE * sizeof(float));

          if (tile.segments.empty()) continue;

          for (const auto& [_, size] : tile.segments) {
            mask.segments_size += size;
          }

          mask.segments = new f8x8x4[mask.segments_size];
          size_t start_pos = 0;

          for (const auto& [segments, size] : tile.segments) {
            std::memcpy(&mask.segments[start_pos], segments, size * sizeof(f8x8x4));
            start_pos += size;
          }

          for (int i = 0; i < TILE_SIZE; i++) {
            cover_table[i] += tile.cover_table[i];
          }
        } else if (
          (rule == FillRule::NonZero && winding != 0) ||
          (rule == FillRule::EvenOdd && winding % 2 != 0)
          ) {
          m_spans.push_back(Span{ x, y, 1 });
        }
      }
    }
  }

  /* -- Tiler -- */

  void Tiler::reset(const Viewport& viewport) {
    vec2 offset = {
      std::fmodf(viewport.position.x * viewport.zoom, TILE_SIZE),
      std::fmodf(viewport.position.y * viewport.zoom, TILE_SIZE)
    };

    m_zoom = static_cast<double>(viewport.zoom);

    m_position = {
      static_cast<int>(viewport.position.x > 0 ? std::floorf(viewport.position.x * viewport.zoom / TILE_SIZE) : std::ceilf(viewport.position.x * viewport.zoom / TILE_SIZE)),
      static_cast<int>(viewport.position.y > 0 ? std::floorf(viewport.position.y * viewport.zoom / TILE_SIZE) : std::ceilf(viewport.position.y * viewport.zoom / TILE_SIZE))
    };
    m_size = {
      static_cast<int>(std::ceilf(static_cast<float>(viewport.size.x) / TILE_SIZE)) + 2,
      static_cast<int>(std::ceilf(static_cast<float>(viewport.size.y) / TILE_SIZE)) + 2
    };

    m_subpixel = (viewport.position * viewport.zoom) % TILE_SIZE - offset;

    m_visible = {
      -viewport.position + m_subpixel / m_zoom,
      -viewport.position + vec2{
        static_cast<float>(viewport.size.x) / viewport.zoom,
        static_cast<float>(viewport.size.y) / viewport.zoom
      } + m_subpixel / m_zoom
    };
    m_visible_min = {
      static_cast<double>(m_visible.min.x),
      static_cast<double>(m_visible.min.y)
    };
    m_clipping_rect = {
      -(TILE_SIZE << FRACBITS),
      -(TILE_SIZE << FRACBITS),
      Math::double_to_f24x8((m_visible.max.x - m_visible.min.x) * m_zoom + TILE_SIZE),
      Math::double_to_f24x8((m_visible.max.y - m_visible.min.y) * m_zoom + TILE_SIZE)
    };

    m_memory_pool.resize(m_size.x * m_size.y);
    m_culled_tiles = std::vector<bool>(m_size.x * m_size.y, false);

    m_filled_batches = std::vector<FilledTilesBatch>(1);
    m_masked_batches = std::vector<MaskedTilesBatch>(1);
  }

  static constexpr float tolerance = 0.25;

  static dvec2 normal(const dvec2 v1, const dvec2 v2) {
    dvec2 n = { v2.y - v1.y, v1.x - v2.x };
    double l = std::hypot(n.x, n.y);

    return { n.x / l, n.y / l };
  }

  static dvec2 normalize(const dvec2 v) {
    double l = std::hypot(v.x, v.y);

    return { v.x / l, v.y / l };
  }

  struct OffsetSegment {
    bool is_linear;

    dvec2 start;
    dvec2 control1;
    dvec2 control2;
    dvec2 end;

    dvec2 start_normal;
    dvec2 end_normal;
    dvec2 end_pivot;
  };

  static OffsetSegment offset_segment(const dvec2 a, const dvec2 b, const double radius) {
    const dvec2 n = normal(a, b);
    const dvec2 nr = n * radius;
    const dvec2 start = a + nr;
    const dvec2 end = b + nr;

    return {
      true,
      start,
      start,
      end,
      end,
      n,
      n,
      b
    };
  }

  static constexpr double EPS = 0.5;

  static bool is_almost_equal(const dvec2 a, const dvec2 b, const double eps) {
    return std::abs(a.x - b.x) < eps && std::abs(a.y - b.y) < eps;
  }

  static OffsetSegment offset_segment(const dvec2 a, const dvec2 b, const dvec2 c, const dvec2 d, const double radius) {
    dvec2 normal_ab;
    dvec2 normal_bc;
    dvec2 normal_cd;

    double dot;

    if (is_almost_equal(a, b, EPS)) {
      if (is_almost_equal(a, c, EPS)) normal_ab = normal(a, d);
      else normal_ab = normal(a, c);
    } else normal_ab = normal(a, b);

    if (is_almost_equal(b, c, EPS)) {
      if (is_almost_equal(b, d, EPS)) normal_bc = normal(a, d);
      else normal_bc = normal(b, d);
    } else normal_bc = normal(b, c);

    if (is_almost_equal(c, d, EPS)) {
      if (is_almost_equal(b, d, EPS)) normal_cd = normal(a, d);
      else normal_cd = normal(b, d);
    } else normal_cd = normal(c, d);

    dvec2 normal_b = normal_ab + normal_bc;
    dvec2 normal_c = normal_bc + normal_cd;

    dot = normal_ab.x * normal_bc.x + normal_ab.y * normal_bc.y;
    normal_b = normalize(normal_b) * (radius / std::sqrt((1.0 + dot) * 0.5));

    dot = normal_bc.x * normal_cd.x + normal_bc.y * normal_cd.y;
    normal_c = normalize(normal_c) * (radius / std::sqrt((1.0 + dot) * 0.5));

    const dvec2 start = a + normal_ab * radius;
    const dvec2 end = d + normal_cd * radius;

    return {
      false,
      start,
      // b + normal_b,
      // c + normal_c,
      b,
      c,
      end,
      normal_ab,
      normal_cd,
      d
    };
  }

  static dvec2 bezier(const dvec2 p0, const dvec2 p1, const dvec2 p2, const dvec2 p3, const double t) {
    const double u = 1.0 - t;
    const double tt = t * t;
    const double uu = u * u;
    const double uuu = uu * u;
    const double ttt = tt * t;

    return uuu * p0 + 3.0 * uu * t * p1 + 3.0 * u * tt * p2 + ttt * p3;
  }

  static dvec2 bezier_derivative(const dvec2 p0, const dvec2 p1, const dvec2 p2, const dvec2 p3, const double t) {
    const double u = 1.0 - t;
    const double tt = t * t;
    const double uu = u * u;

    return 3.0 * uu * (p1 - p0) + 6.0 * u * t * (p2 - p1) + 3.0 * tt * (p3 - p2);
  }

  static dvec2 bezier_second_derivative(const dvec2 p0, const dvec2 p1, const dvec2 p2, const dvec2 p3, const double t) {
    const double u = 1.0 - t;

    return 6.0 * u * (p2 - 2.0 * p1 + p0) + 6.0 * t * (p3 - 2.0 * p2 + p1);
  }

  void Tiler::process_stroke(const Geometry::Path& path, const mat2x3& transform, const Stroke& stroke) {
    GK_TOTAL("Tiler::process_stroke");

    const float sw = 10.0f * stroke.width;

    const float radius_safe = 0.55f * sw * (stroke.join == LineJoin::Miter ? stroke.miter_limit : 1.0f);
    const double radius = 0.5f * sw * m_zoom;

    /* Expand path rect to include the width of the stroke */
    const rect path_rect = transform * path.bounding_rect() + rect{ vec2{ -radius_safe }, vec2{ radius_safe } };
    const float overlap = Math::rect_rect_intersection_area(path_rect, m_visible) / path_rect.area();

    if (overlap <= 0.0f) return;

    const dvec2 tile_offset = (m_visible_min * m_zoom) / TILE_SIZE;
    const dvec2 offset = TILE_SIZE / m_zoom * dvec2{ std::round(tile_offset.x), std::round(tile_offset.y) };
    const dvec2 subpixel_offset = offset + VEC2_TO_DVEC2(m_subpixel) / m_zoom;
    const dvec2 drawable_offset = offset * m_zoom;

    const bool clip_drawable = sw > std::min(m_visible.width(), m_visible.height());

    f24x8x4 bound = {
      Math::double_to_f24x8((path_rect.min.x - offset.x) * m_zoom - m_subpixel.x),
      Math::double_to_f24x8((path_rect.min.y - offset.y) * m_zoom - m_subpixel.y),
      Math::double_to_f24x8((path_rect.max.x - offset.x) * m_zoom - m_subpixel.x),
      Math::double_to_f24x8((path_rect.max.y - offset.y) * m_zoom - m_subpixel.y)
    };

#if 1
    const auto& segments = path.segments();
    const size_t len = segments.size();

    if (len == 0) return;

    Drawable drawable(1, Paint{ stroke.color, FillRule::NonZero, stroke.z_index }, bound);
    Geometry::Contour& contour = drawable.contours.front();

    std::vector<dvec2> points;
    Geometry::Internal::PathInternal lines;

    LineCap cap = LineCap::Square;
    LineJoin join = LineJoin::Miter;

    if (len == 1 && segments.front().is_point() && cap != LineCap::Butt) {
      auto& segment = segments.front();

      dvec2 from = d_transform_point(transform, segment.p0(), subpixel_offset, m_zoom);
      dvec2 n = { 0.0, 1.0 };
      dvec2 nr = n * radius;
      dvec2 start = from + nr;
      dvec2 rstart = from - nr;

      contour.move_to(start);

      contour.add_cap(start, rstart, n, radius, cap);
      contour.add_cap(rstart, start, -n, radius, cap);

      return process_drawable(drawable, drawable_offset, clip_drawable);
    }

    dvec2 last_dir = { 0.0, 0.0 };
    dvec2 first_point = { 0.0, 0.0 };
    dvec2 last_point = { 0.0, 0.0 };
    dvec2 pivot = { 0.0, 0.0 };

    if (path.closed()) {
      const auto& raw = segments.back();
      const auto segment = raw.is_linear() ?
        offset_segment(
          d_transform_point(transform, raw.p0(), subpixel_offset, m_zoom),
          d_transform_point(transform, raw.p3(), subpixel_offset, m_zoom),
          radius
        ) :
        offset_segment(
          d_transform_point(transform, raw.p0(), subpixel_offset, m_zoom),
          d_transform_point(transform, raw.p1(), subpixel_offset, m_zoom),
          d_transform_point(transform, raw.p2(), subpixel_offset, m_zoom),
          d_transform_point(transform, raw.p3(), subpixel_offset, m_zoom),
          radius
        );

      const dvec2 end_point = segment.end;
      const dvec2 out_dir = segment.end_normal;

      pivot = segment.end_pivot;
      last_dir = out_dir;
      last_point = end_point;
      first_point = end_point;

      contour.move_to(last_point);
    }

    /* Forward for the outer stroke. */

    bool is_first = !path.closed();
    double inv_miter_limit = 1.0 / stroke.miter_limit;

    for (const auto& raw : segments) {
      const dvec2 p0 = d_transform_point(transform, raw->p0(), subpixel_offset, m_zoom);
      const auto segment = raw->is_linear() ?
        offset_segment(
          p0,
          d_transform_point(transform, raw->p3(), subpixel_offset, m_zoom),
          radius
        ) :
        offset_segment(
          p0,
          d_transform_point(transform, raw->p1(), subpixel_offset, m_zoom),
          d_transform_point(transform, raw->p2(), subpixel_offset, m_zoom),
          d_transform_point(transform, raw->p3(), subpixel_offset, m_zoom),
          radius
        );

      /**
       * PARAMETERIZATION TEST
       */

#if 1
      std::vector<double> t_values;

      auto recursive = [&](double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4, unsigned int level, auto&& recursive) mutable {
        GK_TOTAL("recursive");

        const unsigned int curve_recursion_limit = 32;

        const double curve_angle_tolerance_epsilon = 0.01;
        const double curve_collinearity_epsilon = 1e-30;
        const double curve_distance_epsilon = 1e-30;
        const double angle_tolerance = 0.4;
        const double tolerance = 0.25;
        // const double approximation_scale = 1.0;
        const double distance_tolerance_square = tolerance * tolerance;
        const double cusp_limit = 0.0;

        auto calc_sq_distance = [](double x1, double y1, double x2, double y2) {
          double dx = x2 - x1;
          double dy = y2 - y1;

          return dx * dx + dy * dy;
          };


        if (level > curve_recursion_limit) return;

        /* Calculate all the mid-points of the line segments */

        double x12 = (x1 + x2) / 2.0;
        double y12 = (y1 + y2) / 2.0;
        double x23 = (x2 + x3) / 2.0;
        double y23 = (y2 + y3) / 2.0;
        double x34 = (x3 + x4) / 2.0;
        double y34 = (y3 + y4) / 2.0;
        double x123 = (x12 + x23) / 2.0;
        double y123 = (y12 + y23) / 2.0;
        double x234 = (x23 + x34) / 2.0;
        double y234 = (y23 + y34) / 2.0;
        double x1234 = (x123 + x234) / 2.0;
        double y1234 = (y123 + y234) / 2.0;

        /* Try to approximate the full cubic curve by a single straight line */

        double dx = x4 - x1;
        double dy = y4 - y1;

        double d2 = fabs(((x2 - x4) * dy - (y2 - y4) * dx));
        double d3 = fabs(((x3 - x4) * dy - (y3 - y4) * dx));
        double da1, da2, k;

        switch ((int(d2 > curve_collinearity_epsilon) << 1) +
          int(d3 > curve_collinearity_epsilon))
        {
        case 0:
          /*All collinear OR p1==p4 */

          k = dx * dx + dy * dy;
          if (k == 0)
          {
            d2 = calc_sq_distance(x1, y1, x2, y2);
            d3 = calc_sq_distance(x4, y4, x3, y3);
          } else
          {
            k = 1 / k;
            da1 = x2 - x1;
            da2 = y2 - y1;
            d2 = k * (da1 * dx + da2 * dy);
            da1 = x3 - x1;
            da2 = y3 - y1;
            d3 = k * (da1 * dx + da2 * dy);
            if (d2 > 0 && d2 < 1 && d3 > 0 && d3 < 1)
            {
              // Simple collinear case, 1---2---3---4
              // We can leave just two endpoints
              return;
            }
            if (d2 <= 0) d2 = calc_sq_distance(x2, y2, x1, y1);
            else if (d2 >= 1) d2 = calc_sq_distance(x2, y2, x4, y4);
            else             d2 = calc_sq_distance(x2, y2, x1 + d2 * dx, y1 + d2 * dy);

            if (d3 <= 0) d3 = calc_sq_distance(x3, y3, x1, y1);
            else if (d3 >= 1) d3 = calc_sq_distance(x3, y3, x4, y4);
            else             d3 = calc_sq_distance(x3, y3, x1 + d3 * dx, y1 + d3 * dy);
          }
          if (d2 > d3)
          {
            if (d2 < distance_tolerance_square)
            {
              points.push_back({ x2, y2 });
              return;
            }
          } else
          {
            if (d3 < distance_tolerance_square)
            {
              points.push_back({ x3, y3 });
              return;
            }
          }
          break;
        case 1:
          /* p1, p2, p4 are collinear, p3 is significant */

          if (d3 * d3 <= distance_tolerance_square * (dx * dx + dy * dy))
          {
            if (angle_tolerance < curve_angle_tolerance_epsilon)
            {
              points.push_back({ x23, y23 });
              return;
            }

            // Angle Condition
            //----------------------
            da1 = fabs(atan2(y4 - y3, x4 - x3) - atan2(y3 - y2, x3 - x2));
            if (da1 >= MATH_PI) da1 = MATH_TWO_PI - da1;

            if (da1 < angle_tolerance)
            {
              points.push_back({ x2, y2 });
              points.push_back({ x3, y3 });
              return;
            }

            if (cusp_limit != 0.0)
            {
              if (da1 > cusp_limit)
              {
                points.push_back({ x3, y3 });
                return;
              }
            }
          }
          break;
        case 2:
          /* p1, p3, p4 are collinear, p2 is significant */

          if (d2 * d2 <= distance_tolerance_square * (dx * dx + dy * dy))
          {
            if (angle_tolerance < curve_angle_tolerance_epsilon)
            {
              points.push_back({ x23, y23 });
              return;
            }

            // Angle Condition
            //----------------------
            da1 = fabs(atan2(y3 - y2, x3 - x2) - atan2(y2 - y1, x2 - x1));
            if (da1 >= MATH_PI) da1 = MATH_TWO_PI - da1;

            if (da1 < angle_tolerance)
            {
              points.push_back({ x2, y2 });
              points.push_back({ x3, y3 });
              return;
            }

            if (cusp_limit != 0.0)
            {
              if (da1 > cusp_limit)
              {
                points.push_back({ x2, y2 });
                return;
              }
            }
          }
          break;
        case 3:
          /* Regular case */

          if ((d2 + d3) * (d2 + d3) <= distance_tolerance_square * (dx * dx + dy * dy))
          {
            // If the curvature doesn't exceed the distance_tolerance value
            // we tend to finish subdivisions.
            //----------------------
            if (angle_tolerance < curve_angle_tolerance_epsilon)
            {
              points.push_back({ x23, y23 });
              // dvec2 n = normal({ x2, y2 }, { x3, y3 });
              // points.push_back(dvec2{ x23, y23 } + n * radius);
              return;
            }

            // Angle & Cusp Condition
            //----------------------
            k = atan2(y3 - y2, x3 - x2);
            da1 = fabs(k - atan2(y2 - y1, x2 - x1));
            da2 = fabs(atan2(y4 - y3, x4 - x3) - k);
            if (da1 >= MATH_PI) da1 = MATH_TWO_PI - da1;
            if (da2 >= MATH_PI) da2 = MATH_TWO_PI - da2;

            if (da1 + da2 < angle_tolerance)
            {
              // Finally we can stop the recursion
              //----------------------
              points.push_back({ x23, y23 });
              // dvec2 n = normal({ x2, y2 }, { x3, y3 });
              // points.push_back(dvec2{ x23, y23 } + n * radius);
              return;
            }

            if (cusp_limit != 0.0)
            {
              if (da1 > cusp_limit)
              {
                points.push_back({ x2, y2 });
                return;
              }

              if (da2 > cusp_limit)
              {
                points.push_back({ x3, y3 });
                return;
              }
            }
          }
          break;
        }

        recursive(x1, y1, x12, y12, x123, y123, x1234, y1234, level + 1, recursive);
        recursive(x1234, y1234, x234, y234, x34, y34, x4, y4, level + 1, recursive);
        };

      recursive(
        p0.x, p0.y,
        segment.control1.x, segment.control1.y,
        segment.control2.x, segment.control2.y,
        segment.end_pivot.x, segment.end_pivot.y,
        0, recursive
      );

      // points.clear();

      // for (double t : t_values) {
      //   dvec2 p = bezier(p0, segment.control1, segment.control2, segment.end_pivot, t);
      //   dvec2 p_prime = bezier_derivative(p0, segment.control1, segment.control2, segment.end_pivot, t);
      //   dvec2 normal = normalize({ p_prime.y, -p_prime.x });

      //   points.push_back(p + normal * radius);
      // }
#else
       // double t = 0.0;

       // while (t < 1.0) {
       //   const dvec2 p = bezier(p0, segment.control1, segment.control2, segment.end_pivot, t);
       //   const dvec2 p_prime = bezier_derivative(p0, segment.control1, segment.control2, segment.end_pivot, t);
       //   const dvec2 p_second = bezier_second_derivative(p0, segment.control1, segment.control2, segment.end_pivot, t);

       //   const double num = p_prime.x * p_second.y - p_prime.y * p_second.x;
       //   const double den = std::pow(p_prime.x * p_prime.x + p_prime.y * p_prime.y, 1.5);
       //   const double k = std::abs(num / den);

       //   double dt = std::min(std::max(std::sqrt(0.0001 * tolerance / k), 0.01), 0.1);

       //   //dt = 0.001 * MATH_PI * dt / k;
       //   // console::log("k", k);
       //   // console::log("dt", dt);

       //   points.push_back(p);

       //   t += dt;
       // }

       // console::log("points", points.size());

       // for (int i = 0; i < 10; i++) {
       //   double t = i / 10.0;

       //   const dvec2 p = bezier(p0, segment.control1, segment.control2, segment.end_pivot, t);
       //   const dvec2 p_prime = bezier_derivative(p0, segment.control1, segment.control2, segment.end_pivot, t);
       //   const dvec2 p_second = bezier_second_derivative(p0, segment.control1, segment.control2, segment.end_pivot, t);

       //   const double num = p_prime.x * p_second.y - p_prime.y * p_second.x;
       //   const double den = std::pow(p_prime.x * p_prime.x + p_prime.y * p_prime.y, 1.5);

       //   const double k = num / den;

       //   const dvec2 norm = normalize({ p_prime.y, -p_prime.x });
       //   const dvec2 rad = p + norm / k;

       //   // double dt = std::max(std::sqrt(0.5 * tolerance * std::abs(k)), 0.001);
       //   // console::log("k", k);
       //   // console::log("dt", dt);

       //   // points.push_back(p);
       //   // lines.rect(DVEC2_TO_VEC2(p) / m_zoom + DVEC2_TO_VEC2(subpixel_offset), vec2{ 1.0f / (float)m_zoom });
       //   lines.move_to(DVEC2_TO_VEC2(p) / m_zoom + DVEC2_TO_VEC2(subpixel_offset));
       //   lines.line_to(DVEC2_TO_VEC2(rad) / m_zoom + DVEC2_TO_VEC2(subpixel_offset));
       //   // points.push_back(rad);

       //   // while (true) {
       //   //   t += dt;
       //   //   if (t >= (i + 1) / 10.0) break;

       //   //   const dvec2 p = bezier(p0, segment.control1, segment.control2, segment.end_pivot, t);
       //   //   const dvec2 p_prime = bezier_derivative(p0, segment.control1, segment.control2, segment.end_pivot, t);
       //   //   const dvec2 p_second = bezier_second_derivative(p0, segment.control1, segment.control2, segment.end_pivot, t);

       //   //   const double num = p_prime.x * p_second.y - p_prime.y * p_second.x;
       //   //   const double den = std::pow(p_prime.x * p_prime.x + p_prime.y * p_prime.y, 1.5);

       //   //   const double k = num / den;

       //   //   const dvec2 norm = normalize({ p_prime.y, -p_prime.x });
       //   //   const dvec2 rad = p + norm / k;

       //   //   // dt = std::sqrt(0.5 * tolerance * std::abs(k));
       //   //   double dt = std::max(std::sqrt(0.5 * tolerance * std::abs(k)), 0.001);

       //   //   // points.push_back(p);
       //   //   points.push_back(p);
       //   //   // lines.move_to(DVEC2_TO_VEC2(p) / m_zoom + DVEC2_TO_VEC2(subpixel_offset));
       //   //   // lines.line_to(DVEC2_TO_VEC2(rad) / m_zoom + DVEC2_TO_VEC2(subpixel_offset));
       //   //   // points.push_back(rad);
       //   // }
       // }
#endif

      const dvec2 start = segment.start;

      if (is_first) {
        contour.move_to(start);
        first_point = start;
        is_first = false;
      } else {
        contour.add_join(last_point, start, pivot, last_dir, segment.start_normal, radius, inv_miter_limit, join);
      }

      last_dir = segment.end_normal;
      pivot = segment.end_pivot;
      last_point = segment.end;

      if (segment.is_linear) contour.line_to(last_point);
      // else contour.line_to(last_point);
      // else contour.cubic_to(segment.control1, segment.control2, last_point);
      else contour.offset_cubic(p0, segment.control1, segment.control2, pivot, last_dir, radius);
    }

    /* Backward for the inner stroke. */

    is_first = true;

    for (auto it = segments.rbegin(); it != segments.rend(); it++) {
      const dvec2 p0 = d_transform_point(transform, (*it)->p0(), subpixel_offset, m_zoom);
      const auto segment = (*it)->is_linear() ?
        offset_segment(
          d_transform_point(transform, (*it)->p3(), subpixel_offset, m_zoom),
          p0,
          radius
        ) :
        offset_segment(
          d_transform_point(transform, (*it)->p3(), subpixel_offset, m_zoom),
          d_transform_point(transform, (*it)->p2(), subpixel_offset, m_zoom),
          d_transform_point(transform, (*it)->p1(), subpixel_offset, m_zoom),
          p0,
          radius
        );

      const dvec2 start = segment.start;

      if (is_first) {
        if (path.closed()) {
          const auto raw = segments.front();
          const auto init = raw.is_linear() ?
            offset_segment(
              d_transform_point(transform, raw.p3(), subpixel_offset, m_zoom),
              d_transform_point(transform, raw.p0(), subpixel_offset, m_zoom),
              radius
            ) :
            offset_segment(
              d_transform_point(transform, raw.p3(), subpixel_offset, m_zoom),
              d_transform_point(transform, raw.p2(), subpixel_offset, m_zoom),
              d_transform_point(transform, raw.p1(), subpixel_offset, m_zoom),
              d_transform_point(transform, raw.p0(), subpixel_offset, m_zoom),
              radius
            );

          last_point = init.end;
          last_dir = init.end_normal;
          pivot = init.end_pivot;

          contour.line_to(last_point);
          contour.add_join(last_point, start, pivot, last_dir, segment.start_normal, radius, inv_miter_limit, join);
        } else {
          contour.add_cap(last_point, start, last_dir, radius, cap);
        }

        is_first = false;
      } else {
        contour.add_join(last_point, start, pivot, last_dir, segment.start_normal, radius, inv_miter_limit, join);
        // if (id != last_id) {
        //   contour.add_join(last_point, start, pivot, last_dir, segment.start_normal);
        // } else {
        //   contour.add_split_join(last_point, start, pivot, last_dir, segment.start_normal);
        // }
      }

      last_dir = segment.end_normal;
      pivot = segment.end_pivot;
      last_point = segment.end;

      if (segment.is_linear) contour.line_to(last_point);
      // else contour.line_to(last_point);
      // else contour.cubic_to(segment.control1, segment.control2, last_point);
      else contour.offset_cubic(p0, segment.control1, segment.control2, pivot, last_dir, radius);
    }

    if (!path.closed()) {
      contour.add_cap(last_point, first_point, last_dir, radius, cap);
    }

    contour.close();

    process_drawable(drawable, drawable_offset, clip_drawable);

    Geometry::Internal::PathInternal p;

    p.move_to(vec2{ Math::f24x8_to_float(contour.points.front().x) / (float)m_zoom, Math::f24x8_to_float(contour.points.front().y) / (float)m_zoom } + DVEC2_TO_VEC2(subpixel_offset));

    for (f24x8x2 point : contour.points) {
      p.line_to(vec2{ Math::f24x8_to_float(point.x) / (float)m_zoom, Math::f24x8_to_float(point.y) / (float)m_zoom } + DVEC2_TO_VEC2(subpixel_offset));
    }

    Renderer::draw_outline(p);

    console::log("recursive", points.size());

    for (dvec2 point : points) {
      Geometry::Internal::PathInternal points_geo;

      vec2 P = vec2{ (float)(point.x / m_zoom), (float)(point.y / m_zoom) } + DVEC2_TO_VEC2(subpixel_offset);

      points_geo.move_to(P - vec2{ 3.0f, 3.0f } / m_zoom);
      points_geo.line_to(P + vec2{ 3.0f, 3.0f } / m_zoom);
      points_geo.move_to(P - vec2{ 3.0f, -3.0f } / m_zoom);
      points_geo.line_to(P + vec2{ 3.0f, -3.0f } / m_zoom);

      Renderer::draw_outline(points_geo);
    }

    Renderer::draw_outline(lines);
#else
    const auto& segments = path.segments();

    if (true || overlap > OVERLAP_CLIP_THRESHOLD) {
      Drawable drawable(2, Paint{ stroke.color, FillRule::NonZero, stroke.z_index }, bound);
      Geometry::Contour& forward = drawable.contours.front();
      Geometry::Contour& backward = drawable.contours.back();

      f24x8x2 last = f_transform_point(transform, segments.front().p0(), subpixel_offset, m_zoom);
      f24x8x2 next = last;

      forward.begin(next, false);

      for (size_t i = 0; i < segments.size(); i++) {
        auto& raw_segment = segments[i];

        if (raw_segment.is_linear()) {
          next = f_transform_point(transform, raw_segment.p3(), subpixel_offset, m_zoom);

          vec2 normal = Math::normalize({ Math::f24x8_to_float(last.y - next.y), Math::f24x8_to_float(next.x - last.x) }) * radius;
          f24x8x2 n = { Math::float_to_f24x8(normal.x), Math::float_to_f24x8(normal.y) };

          forward.points.emplace_back(last.x - n.x, last.y - n.y);
          forward.points.emplace_back(next.x - n.x, next.y - n.y);

          backward.points.emplace_back(last.x + n.x, last.y + n.y);
          backward.points.emplace_back(next.x + n.x, next.y + n.y);

          last = next;
        } else {
          const vec2 A = raw_segment.p0();
          const vec2 B = raw_segment.p1();
          const vec2 C = raw_segment.p2();
          const vec2 D = raw_segment.p3();

          // const std::vector<float>& parameterization = raw_segment.parameterize(m_zoom);

          vec2 a = -A + 3.0f * B - 3.0f * C + D;
          vec2 b = 3.0f * A - 6.0f * B + 3.0f * C;
          vec2 c = -3.0f * A + 3.0f * B;
          vec2 p;

          float conc = std::max(Math::length(b), Math::length(a + b));
          float dt = std::sqrtf((std::sqrtf(8.0f) * tolerance / m_zoom) / conc);
          float t = dt;

          forward.points.reserve(static_cast<int>(1.0f / dt) + 1);
          backward.points.reserve(static_cast<int>(1.0f / dt) + 1);

          while (t <= 1.0f) {
            float t_sq = t * t;

            p = a * t_sq * t + b * t_sq + c * t + A;
            next = f_transform_point(transform, p, subpixel_offset, m_zoom);

            vec2 normal = Math::normalize({ Math::f24x8_to_float(last.y - next.y), Math::f24x8_to_float(next.x - last.x) }) * radius;
            f24x8x2 n = { Math::float_to_f24x8(normal.x), Math::float_to_f24x8(normal.y) };

            forward.points.emplace_back(last.x - n.x, last.y - n.y);
            forward.points.emplace_back(next.x - n.x, next.y - n.y);

            backward.points.emplace_back(last.x + n.x, last.y + n.y);
            backward.points.emplace_back(next.x + n.x, next.y + n.y);

            last = next;

            t += dt;
          }


          // for (const float t : parameterization) {
          //   // TODO: transform upfront
          //   next = f_transform_point(transform, Math::bezier(A, B, C, D, t), subpixel_offset, m_zoom);

          //   vec2 normal = Math::normalize({ Math::f24x8_to_float(last.y - next.y), Math::f24x8_to_float(next.x - last.x) }) * radius;
          //   f24x8x2 n = { Math::float_to_f24x8(normal.x), Math::float_to_f24x8(normal.y) };

          //   forward.points.emplace_back(last.x - n.x, last.y - n.y);
          //   forward.points.emplace_back(next.x - n.x, next.y - n.y);

          //   backward.points.emplace_back(last.x + n.x, last.y + n.y);
          //   backward.points.emplace_back(next.x + n.x, next.y + n.y);

          //   last = next;
          // }
        }
      }

      if (path.closed()) {
        backward.reverse();
        forward.close();
        backward.close();
      } else {
        forward.points.insert(forward.points.end(), backward.points.rbegin(), backward.points.rend());
        forward.close();

        drawable.contours.pop_back();
      }

      process_drawable(drawable, drawable_offset, clip_drawable);
    }
#endif
  }

  void Tiler::process_fill(const Geometry::Path& path, const mat2x3& transform, const Fill& fill) {
    GK_TOTAL("Tiler::process_fill");

    const rect path_rect = transform * path.bounding_rect();
    const float overlap = Math::rect_rect_intersection_area(path_rect, m_visible) / path_rect.area();

    if (overlap <= 0.0f) return;

    const dvec2 tile_offset = (m_visible_min * m_zoom) / TILE_SIZE;
    const dvec2 offset = TILE_SIZE / m_zoom * dvec2{ std::round(tile_offset.x), std::round(tile_offset.y) };
    const dvec2 subpixel_offset = offset + VEC2_TO_DVEC2(m_subpixel) / m_zoom;
    const dvec2 drawable_offset = offset * m_zoom;

    f24x8x4 bound = {
      Math::double_to_f24x8((path_rect.min.x - offset.x) * m_zoom - m_subpixel.x),
      Math::double_to_f24x8((path_rect.min.y - offset.y) * m_zoom - m_subpixel.y),
      Math::double_to_f24x8((path_rect.max.x - offset.x) * m_zoom - m_subpixel.x),
      Math::double_to_f24x8((path_rect.max.y - offset.y) * m_zoom - m_subpixel.y)
    };

    Drawable drawable(1, fill, bound);
    Geometry::Contour& contour = drawable.contours.front();

    const auto& segments = path.segments();
    const f24x8x2 first = f_transform_point(transform, segments.front().p0(), subpixel_offset, m_zoom);

    contour.move_to(first);

    for (size_t i = 0; i < segments.size(); i++) {
      auto& raw_segment = segments[i];

      if (raw_segment.is_linear()) {
        contour.line_to(f_transform_point(transform, raw_segment.p3(), subpixel_offset, m_zoom));
      } else {
        contour.cubic_to(
          f_transform_point(transform, raw_segment.p1(), subpixel_offset, m_zoom),
          f_transform_point(transform, raw_segment.p2(), subpixel_offset, m_zoom),
          f_transform_point(transform, raw_segment.p3(), subpixel_offset, m_zoom)
        );
      }
    }

    contour.close();

    process_drawable(drawable, drawable_offset, overlap < OVERLAP_CLIP_THRESHOLD);
  }

  void Tiler::process_drawable(Drawable& drawable, const dvec2 offset, const bool clip) {
    ivec2 tile_offset = tile_coords(offset);

    if (clip) {
      clip_drawable(drawable, m_clipping_rect);
    }

    f24x8x4 bounds = {
      (((drawable.bounds.x0 / TILE_SIZE - FRACUNIT) >> FRACBITS) << FRACBITS) * TILE_SIZE,
      (((drawable.bounds.y0 / TILE_SIZE - FRACUNIT) >> FRACBITS) << FRACBITS) * TILE_SIZE,
      ((((drawable.bounds.x1 / TILE_SIZE) >> FRACBITS) + 1) << FRACBITS) * TILE_SIZE,
      ((((drawable.bounds.y1 / TILE_SIZE) >> FRACBITS) + 1) << FRACBITS) * TILE_SIZE
    };

    drawable.bounds = bounds;

    DrawableTiler tiler(
      drawable,
      m_position + tile_offset,
      m_size,
      &m_memory_pool
    );

    const std::vector<DrawableTiler::Mask>& masks = tiler.masks();
    const std::vector<DrawableTiler::Span>& spans = tiler.spans();
    const ivec2 tiler_offset = tiler.offset();
    const ivec2 size = tiler.size();

    FilledTilesBatch& fills_batch = m_filled_batches.front();
    MaskedTilesBatch& masks_batch = m_masked_batches.front();

    for (const auto& mask : masks) {
      ivec2 coords = {
        mask.tile_x + tiler_offset.x + 1,
        mask.tile_y + tiler_offset.y + 1
      };

      if (coords.x < 0 || coords.y < 0 || coords.x >= m_size.x || coords.y >= m_size.y) continue;

      int absolute_index = tile_index(coords, m_size);
      if (m_culled_tiles[absolute_index]) continue;

      int offset = (int)(masks_batch.segments_ptr - masks_batch.segments) / 4;
      int cover_offset = (int)(masks_batch.cover_table_ptr - masks_batch.cover_table);

      masks_batch.segments_ptr[0] = (uint8_t)mask.segments_size;
      masks_batch.segments_ptr[1] = (uint8_t)(mask.segments_size >> 8);
      masks_batch.segments_ptr[2] = (uint8_t)(mask.segments_size >> 16);
      masks_batch.segments_ptr[3] = (uint8_t)(mask.segments_size >> 24);
      masks_batch.segments_ptr += 4;

      for (size_t i = 0; i < mask.segments_size; i++) {
        masks_batch.segments_ptr[0] = static_cast<uint8_t>(mask.segments[i].x0 >> FRACBITS);
        masks_batch.segments_ptr[1] = static_cast<uint8_t>(mask.segments[i].x0);
        masks_batch.segments_ptr[2] = static_cast<uint8_t>(mask.segments[i].y0 >> FRACBITS);
        masks_batch.segments_ptr[3] = static_cast<uint8_t>(mask.segments[i].y0);
        masks_batch.segments_ptr[4] = static_cast<uint8_t>(mask.segments[i].x1 >> FRACBITS);
        masks_batch.segments_ptr[5] = static_cast<uint8_t>(mask.segments[i].x1);
        masks_batch.segments_ptr[6] = static_cast<uint8_t>(mask.segments[i].y1 >> FRACBITS);
        masks_batch.segments_ptr[7] = static_cast<uint8_t>(mask.segments[i].y1);

        masks_batch.segments_ptr += 8;
      }

      // TODO: bounds check
      memcpy(masks_batch.cover_table_ptr, &mask.cover_table, TILE_SIZE * sizeof(float));
      masks_batch.cover_table_ptr += TILE_SIZE;

      masks_batch.tiles.push_back(MaskedTile{
        drawable.paint.color,
        absolute_index,
        { (uint16_t)(offset % SEGMENTS_TEXTURE_SIZE), (uint16_t)(offset / SEGMENTS_TEXTURE_SIZE) },
        { (uint16_t)(cover_offset % SEGMENTS_TEXTURE_SIZE), (uint16_t)(cover_offset / SEGMENTS_TEXTURE_SIZE) },
        drawable.paint.z_index
        });
    }

    for (auto& span : spans) {
      ivec2 coords = { span.tile_x + tiler_offset.x + 1, span.tile_y + tiler_offset.y + 1 };
      if (coords.x + span.width < 0 || coords.y < 0 || coords.x >= m_size.x || coords.y >= m_size.y) continue;

      int16_t width = coords.x < 0 ? span.width + coords.x : span.width;
      coords.x = std::max(coords.x, 0);

      for (int i = 0; i < width; i++) {
        if (coords.x + i >= m_size.x) break;

        size_t index = tile_index({ coords.x + i, coords.y }, m_size);
        if (!m_culled_tiles[index]/*&& color.a == 1.0f*/) {
          // TODO: abstract and avoid size_t to int conversion
          fills_batch.tiles.push_back({ drawable.paint.color, (int)index, drawable.paint.z_index });
          m_culled_tiles[index] = true;
        }
      }
    }
  }

#if 0
  Tiler::Tiler() {
    m_segments = new uint8_t[SEGMENTS_TEXTURE_SIZE * SEGMENTS_TEXTURE_SIZE * 4];
    m_cover_table = new float[SEGMENTS_TEXTURE_SIZE * SEGMENTS_TEXTURE_SIZE];
  }

  Tiler::~Tiler() {
    delete[] m_segments;
    delete[] m_cover_table;
  }

  void Tiler::reset(const Viewport& viewport) {
    m_tiles_count = { (int)(std::ceil((float)viewport.size.x / TILE_SIZE)) + 2, (int)(std::ceil((float)viewport.size.y / TILE_SIZE)) + 2 };
    m_position = {
      (int)(viewport.position.x > 0 ? std::floor(viewport.position.x * viewport.zoom / TILE_SIZE) : std::ceil(viewport.position.x * viewport.zoom / TILE_SIZE)),
      (int)(viewport.position.y > 0 ? std::floor(viewport.position.y * viewport.zoom / TILE_SIZE) : std::ceil(viewport.position.y * viewport.zoom / TILE_SIZE))
    };
    vec2 offset = {
      std::fmodf(viewport.position.x * viewport.zoom, TILE_SIZE),
      std::fmodf(viewport.position.y * viewport.zoom, TILE_SIZE)
    };
    m_subpixel = (viewport.position * viewport.zoom) % TILE_SIZE - offset;
    m_zoom = viewport.zoom;
    m_visible = { -viewport.position, vec2{(float)viewport.size.x / viewport.zoom, (float)viewport.size.y / viewport.zoom} - viewport.position };

    m_opaque_tiles.clear();
    m_masked_tiles.clear();

    m_segments_ptr = m_segments;
    m_cover_table_ptr = m_cover_table;

    m_culled_tiles = std::vector<bool>(m_tiles_count.x * m_tiles_count.y, false);
    m_memory_pool = std::vector<DrawableTiler::Tile>(m_tiles_count.x * m_tiles_count.y);
  }

  struct Segment {
    vec2 p0;
    vec2 p1;
    vec2 p2;
    vec2 p3;

    bool is_linear;

    Segment(vec2 p0, vec2 p3) : p0(p0), p3(p3), is_linear(true) {}
    Segment(vec2 p0, vec2 p1, vec2 p2, vec2 p3) : p0(p0), p1(p1), p2(p2), p3(p3), is_linear(false) {}
  };

  static Drawable clip_drawable(const Drawable& drawable, const rect& clip) {
    Drawable clipped(0, drawable.paint, { Math::max(drawable.bounds.min, clip.min), Math::min(drawable.bounds.max, clip.max) });

    for (auto& contour : drawable.contours) {
      Geometry::Contour& clipped_contour = clipped.contours.emplace_back();

      std::vector<vec2> points = contour.points;

      Math::clip(points, clip);

      clipped_contour.points = std::move(points);
      clipped_contour.close();
    }

    return clipped;
  }

  void Tiler::process_drawable(const Drawable& drawable, const rect& visible, const vec2 offset, const bool clip) {
    ivec2 tile_offset = tile_coords(offset);
    vec2 pixel_offset = offset - TILE_SIZE * vec2{ (float)tile_offset.x, (float)tile_offset.y };

    DrawableTiler tiler(
      clip ? clip_drawable(drawable, rect{ { -32.0f, -32.0f }, (visible.max - visible.min) * m_zoom + 32.0f }) : drawable,
      visible,
      m_zoom,
      m_position + tile_offset,
      m_subpixel - pixel_offset,
      m_tiles_count,
      m_memory_pool
    );

    const std::vector<DrawableTiler::Mask>& masks = tiler.masks();
    const std::vector<DrawableTiler::Span>& spans = tiler.spans();
    const ivec2 tiler_offset = tiler.offset();
    const ivec2 size = tiler.size();

    for (const auto& mask : masks) {
      ivec2 coords = {
        mask.tile_x + tiler_offset.x + 1,
        mask.tile_y + tiler_offset.y + 1
      };

      if (coords.x < 0 || coords.y < 0 || coords.x >= m_tiles_count.x || coords.y >= m_tiles_count.y) continue;

      int absolute_index = tile_index(coords, m_tiles_count);
      if (m_culled_tiles[absolute_index]) continue;

      int offset = (int)(m_segments_ptr - m_segments) / 4;
      int cover_offset = (int)(m_cover_table_ptr - m_cover_table);

      uint32_t segments_size = (uint32_t)mask.segments.size();

      m_segments_ptr[0] = (uint8_t)segments_size;
      m_segments_ptr[1] = (uint8_t)(segments_size >> 8);
      m_segments_ptr[2] = (uint8_t)(segments_size >> 16);
      m_segments_ptr[3] = (uint8_t)(segments_size >> 24);
      m_segments_ptr += 4;

      for (auto segment : mask.segments) {
        m_segments_ptr[0] = segment.x0;
        m_segments_ptr[1] = segment.y0;
        m_segments_ptr[2] = segment.x1;
        m_segments_ptr[3] = segment.y1;
        m_segments_ptr += 4;
      }

      // TODO: bounds check
      memcpy(m_cover_table_ptr, &mask.cover_table, TILE_SIZE * sizeof(float));
      m_cover_table_ptr += TILE_SIZE;

      m_masked_tiles.push_back(MaskedTile{
        drawable.paint.color,
        absolute_index,
        { (uint16_t)(offset % SEGMENTS_TEXTURE_SIZE), (uint16_t)(offset / SEGMENTS_TEXTURE_SIZE) },
        { (uint16_t)(cover_offset % SEGMENTS_TEXTURE_SIZE), (uint16_t)(cover_offset / SEGMENTS_TEXTURE_SIZE) },
        drawable.paint.z_index
        });
    }

    for (auto& span : spans) {
      ivec2 coords = { span.tile_x + tiler_offset.x + 1, span.tile_y + tiler_offset.y + 1 };
      if (coords.x + span.width < 0 || coords.y < 0 || coords.x >= m_tiles_count.x || coords.y >= m_tiles_count.y) continue;

      int16_t width = coords.x < 0 ? span.width + coords.x : span.width;
      coords.x = std::max(coords.x, 0);

      for (int i = 0; i < width; i++) {
        if (coords.x + i >= m_tiles_count.x) break;

        size_t index = tile_index({ coords.x + i, coords.y }, m_tiles_count);
        if (!m_culled_tiles[index]/*&& color.a == 1.0f*/) {
          m_opaque_tiles.push_back({ drawable.paint.color, index, drawable.paint.z_index });
          m_culled_tiles[index] = true;
        }
      }
    }
  }

  void Tiler::process_stroke(const Geometry::Path& path, const mat2x3& transform, const Stroke& stroke) {
    GK_TOTAL("Tiler::process_stroke");
    const float radius = 0.5f * stroke.width * m_zoom;

    rect path_rect = transform * path.bounding_rect();

    path_rect.min -= 1.1f * 0.5f * stroke.width * (stroke.join == LineJoin::Miter ? stroke.miter_limit : 1.0f);
    path_rect.max += 1.1f * 0.5f * stroke.width * (stroke.join == LineJoin::Miter ? stroke.miter_limit : 1.0f);

    const float overlap = Math::rect_rect_intersection_area(path_rect, m_visible) / path_rect.area();
    if (overlap <= 0.0f) return;

    const mat2x3 transform_zoom = transform * m_zoom;
    const auto& segments = path.segments();

    const bool clip_drawable = stroke.width > std::min(m_visible.size().x, m_visible.size().y);

    // TODO: test and tweak overlap threshold
    if (overlap > 0.7f) {
      Drawable drawable(path.closed() ? 2 : 1, Paint{ stroke.color, FillRule::NonZero, stroke.z_index }, (path_rect - m_visible.min) * m_zoom);
      Geometry::Contour* contour = &drawable.contours.front();

      contour->begin((transform * segments.front().p0() - m_visible.min) * m_zoom, false);

      for (size_t i = 0; i < segments.size(); i++) {
        auto& raw_segment = segments.at(i);

        if (raw_segment.is_linear()) {
          // TODO: test const reference vs copy
          contour->offset_segment((transform * raw_segment.p3() - m_visible.min) * m_zoom, radius);
        } else {
          contour->offset_segment(
            (transform * raw_segment.p1() - m_visible.min) * m_zoom,
            (transform * raw_segment.p2() - m_visible.min) * m_zoom,
            (transform * raw_segment.p3() - m_visible.min) * m_zoom,
            radius
          );
        }
      }

      if (path.closed()) {
        contour->close();
        contour = &drawable.contours.back();
      }

      contour->begin((transform * segments.back().p3() - m_visible.min) * m_zoom, false);

      for (int i = static_cast<int>(segments.size()) - 1; i >= 0; i--) {
        auto& raw_segment = segments.at(i);

        if (raw_segment.is_linear()) {
          contour->offset_segment(
            (transform * raw_segment.p0() - m_visible.min) * m_zoom,
            radius
          );
        } else {
          contour->offset_segment(
            (transform * raw_segment.p2() - m_visible.min) * m_zoom,
            (transform * raw_segment.p1() - m_visible.min) * m_zoom,
            (transform * raw_segment.p0() - m_visible.min) * m_zoom,
            radius
          );
        }
      }

      contour->close();

      return process_drawable(drawable, m_visible, m_visible.min * m_zoom, clip_drawable);
    }

    size_t len = segments.size();
    size_t i = 0;

    std::vector<std::vector<Segment>> clipped_contours(1);

    rect visible = m_visible;

    visible.min -= 1.1f * 0.5f * stroke.width * (stroke.join == LineJoin::Miter ? stroke.miter_limit : 1.0f);
    visible.max += 1.1f * 0.5f * stroke.width * (stroke.join == LineJoin::Miter ? stroke.miter_limit : 1.0f);

    visible.min -= 32.0f / m_zoom;
    visible.max += 32.0f / m_zoom;

    for (size_t i = 0; i < len; i++) {
      auto& raw_segment = segments.at(i);

      vec2 p0 = transform * raw_segment.p0() - visible.min;
      vec2 p3 = transform * raw_segment.p3() - visible.min;

      bool p0_in = Math::is_point_in_rect(p0, visible - visible.min);
      bool p3_in = Math::is_point_in_rect(p3, visible - visible.min);

      if (raw_segment.is_linear()) {

        if (p0_in && p3_in) {
          /* Entire segment is visible. */

          clipped_contours.back().emplace_back(p0, p3);
          continue;
        }

        std::vector<vec2> intersections = Math::line_rect_intersection_points(p0, p3, visible - visible.min);

        if (intersections.empty()) continue;

        for (int k = 0; k < (int)intersections.size(); k++) {
          if (k % 2 == 0) {
            if (p0_in) {
              clipped_contours.back().emplace_back((k < 1 ? p0 : intersections[k - 1]), intersections[k]);
            } else {
              if (!clipped_contours.back().empty()) clipped_contours.emplace_back();
              clipped_contours.back().emplace_back(intersections[k], (k >= intersections.size() - 1 ? p3 : intersections[k + 1]));
            }
          }
        }
      } else {
        vec2 p1 = transform * raw_segment.p1() - visible.min;
        vec2 p2 = transform * raw_segment.p2() - visible.min;

        // TODO: check if all inside or all outside with control points
        std::vector<float> intersections = Math::bezier_rect_intersections(p0, p1, p2, p3, visible - visible.min);

        if (intersections.empty()) {
          if (p0_in) {
            /* Entire segment is visible. */

            clipped_contours.back().emplace_back(p0, p1, p2, p3);
          }

          /* Segment is completely outside. */

          continue;
        }

        for (int k = 0; k < (int)intersections.size(); k++) {
          if (k % 2 == 0) {
            if (p0_in) {
              auto segment = Math::split_bezier(
                p0, p1, p2, p3,
                (k < 1 ? 0.0f : intersections[k - 1]),
                intersections[k]
              );

              clipped_contours.back().emplace_back(std::get<0>(segment), std::get<1>(segment), std::get<2>(segment), std::get<3>(segment));
            } else {
              if (!clipped_contours.back().empty()) clipped_contours.emplace_back();

              auto segment = Math::split_bezier(
                p0, p1, p2, p3,
                intersections[k],
                (k >= intersections.size() - 1 ? 1.0f : intersections[k + 1])
              );

              clipped_contours.back().emplace_back(std::get<0>(segment), std::get<1>(segment), std::get<2>(segment), std::get<3>(segment));
            }
          } else if (p3_in && k == (int)intersections.size() - 1) {
            if (!clipped_contours.back().empty()) clipped_contours.emplace_back();

            auto segment = Math::split_bezier(
              p0, p1, p2, p3,
              intersections[k],
              (k >= intersections.size() - 1 ? 1.0f : intersections[k + 1])
            );

            clipped_contours.back().emplace_back(std::get<0>(segment), std::get<1>(segment), std::get<2>(segment), std::get<3>(segment));
          } else {
            if (!clipped_contours.back().empty()) clipped_contours.emplace_back();
          }
        }
      }
    }

    Drawable drawable(0, Paint{ stroke.color, FillRule::NonZero, stroke.z_index }, rect{});

    for (auto& clipped_contour : clipped_contours) {
      if (clipped_contour.empty()) continue;

      Geometry::Contour* contour = &drawable.contours.emplace_back();

      vec2 p0 = clipped_contour.front().p0 * m_zoom;

      contour->begin(p0, false);

      Math::min(drawable.bounds.min, p0, drawable.bounds.min);
      Math::max(drawable.bounds.max, p0, drawable.bounds.max);

      for (size_t i = 0; i < clipped_contour.size(); i++) {
        auto& segment = clipped_contour.at(i);

        vec2 p3 = segment.p3 * m_zoom;

        if (segment.is_linear) {
          // TODO: test const reference vs copy
          contour->offset_segment(p3, radius);
        } else {
          vec2 p1 = segment.p1 * m_zoom;
          vec2 p2 = segment.p2 * m_zoom;

          contour->offset_segment(
            p1,
            p2,
            p3,
            radius
          );

          // TODO: optimize away this operations (perform intersection of bounds with visible rect)
          Math::min(drawable.bounds.min, p1, drawable.bounds.min);
          Math::max(drawable.bounds.max, p1, drawable.bounds.max);

          Math::min(drawable.bounds.min, p2, drawable.bounds.min);
          Math::max(drawable.bounds.max, p2, drawable.bounds.max);
        }

        Math::min(drawable.bounds.min, p3, drawable.bounds.min);
        Math::max(drawable.bounds.max, p3, drawable.bounds.max);
      }

      contour->begin(clipped_contour.back().p3 * m_zoom, false);

      for (int i = static_cast<int>(clipped_contour.size()) - 1; i >= 0; i--) {
        auto& segment = clipped_contour.at(i);

        vec2 p0 = segment.p0 * m_zoom;

        if (segment.is_linear) {
          contour->offset_segment(p0, radius);
        } else {
          vec2 p1 = segment.p1 * m_zoom;
          vec2 p2 = segment.p2 * m_zoom;

          contour->offset_segment(
            p2,
            p1,
            p0,
            radius
          );
        }
      }

      contour->close();
    }

    // drawable.bounds.min -= 1.1f * 0.5f * stroke.width * (stroke.join == LineJoin::Miter ? stroke.miter_limit : 1.0f);
    // drawable.bounds.max += 1.1f * 0.5f * stroke.width * (stroke.join == LineJoin::Miter ? stroke.miter_limit : 1.0f);
    drawable.bounds.min -= 1.1f * radius * (stroke.join == LineJoin::Miter ? stroke.miter_limit : 1.0f);
    drawable.bounds.max += 1.1f * radius * (stroke.join == LineJoin::Miter ? stroke.miter_limit : 1.0f);

    // drawable.bounds *= m_zoom;

    process_drawable(drawable, visible, visible.min * m_zoom, clip_drawable);
  }

  void Tiler::process_fill(const Geometry::Path& path, const mat2x3& transform, const Fill& fill) {
    GK_TOTAL("Tiler::process_fill");

    const rect path_rect = transform * path.bounding_rect();

    const float overlap = Math::rect_rect_intersection_area(path_rect, m_visible) / path_rect.area();
    if (overlap <= 0.0f) return;

    Drawable drawable(1, fill, (path_rect - m_visible.min) * m_zoom);
    Geometry::Contour& contour = drawable.contours.front();

    const auto& segments = path.segments();
    const vec2 first = (transform * segments.front().p0() - m_visible.min) * m_zoom;

    contour.begin(first);

    for (size_t i = 0; i < segments.size(); i++) {
      auto& raw_segment = segments.at(i);

      if (raw_segment.is_linear()) {
        // TODO: test const reference vs copy
        contour.push_segment((transform * raw_segment.p3() - m_visible.min) * m_zoom);
      } else {
        contour.push_segment(
          (transform * raw_segment.p1() - m_visible.min) * m_zoom,
          (transform * raw_segment.p2() - m_visible.min) * m_zoom,
          (transform * raw_segment.p3() - m_visible.min) * m_zoom
        );
      }
    }

    contour.close();

    process_drawable(drawable, m_visible, m_visible.min * m_zoom, overlap < 0.7f);
  }
#endif

}
