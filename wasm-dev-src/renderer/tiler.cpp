#include "tiler.h"

#include "geometry/path.h"
#include "geometry/contour.h"

#include "../math/mat2x3.h"
#include "../math/matrix.h"
#include "../math/math.h"

#include "../utils/console.h"

// TODO: zoom and transform operations should use doubles
// TODO: fix right border of tiger (near min_y)

namespace Graphick::Renderer {

  /**
   * @brief Calculates the tile coordinates of a point.
   *
   * @param p The point.
   * @return The tile coordinates.
   */
  static inline ivec2 tile_coords(const vec2 p) {
    return {
      static_cast<int>(std::floorf(p.x / TILE_SIZE)),
      static_cast<int>(std::floorf(p.y / TILE_SIZE))
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
  static inline int tile_index(const int16_t tile_x, const int16_t tile_y, const int16_t tiles_count_x) {
    return tile_x + tile_y * tiles_count_x;
  }

  /**
   * @brief Calculates the sign of a float.
   *
   * This method is designed and optimized to return an int16_t value (-1, 0, 1).
   *
   * @param x The float.
   * @return The sign of the float.
   */
  inline static int16_t sign(float x) {
    return (0 < x) - (x < 0);
  }

  /* -- DrawableTiler -- */

  DrawableTiler::DrawableTiler(const Drawable& drawable, const rect& visible, const float zoom, const ivec2 position, const vec2 subpixel, const ivec2 tiles_count, std::vector<Tile>& pool) {
    rect bounds = {
      Math::floor((drawable.bounds.min - subpixel - 1.0f) / TILE_SIZE) * TILE_SIZE,
      Math::ceil((drawable.bounds.max - subpixel + 1.0f) / TILE_SIZE) * TILE_SIZE
    };

    ivec2 min_coords = tile_coords(bounds.min) + position;
    ivec2 max_coords = tile_coords(bounds.max) + position;

    bounds += subpixel;

    m_offset = min_coords;
    m_size = max_coords - min_coords;

    m_memory_pool = &pool;

    m_memory_pool->clear();
    m_memory_pool->resize(m_size.x * m_size.y);

    for (const Geometry::Contour& contour : drawable.contours) {
      if (contour.points.size() < 2) continue;

#ifndef USE_F8x8
      move_to(contour.points.front() - bounds.min);

      for (size_t i = 1; i < contour.points.size(); i++) {
        line_to(contour.points[i] - bounds.min);
      }
#endif
    }

    pack(drawable.paint.rule, tiles_count);

    m_memory_pool = nullptr;
  }

  void DrawableTiler::move_to(const vec2 p0) {
    m_p0 = p0;
  }

  static constexpr float max_over_tile_size = 255.0f / (float)TILE_SIZE;
  static constexpr float tile_size_over_max = (float)TILE_SIZE / 255.0f;
  static constexpr float one_shifted = static_cast<float>(1 << 8);

  void DrawableTiler::line_to(const vec2 p3) {
    if (Math::is_almost_equal(m_p0, p3)) return;

    vec2 p0 = m_p0;
    vec2 vec = p3 - p0;

    int16_t x_dir = sign(vec.x);
    int16_t y_dir = sign(vec.y);
    int16_t x = (int16_t)std::floor(p0.x);
    int16_t y = (int16_t)std::floor(p0.y);
    int16_t x_tile_dir = x_dir * TILE_SIZE;
    int16_t y_tile_dir = y_dir * TILE_SIZE;
    int16_t tile_x = x / TILE_SIZE;
    int16_t tile_y = y / TILE_SIZE;

    m_p0 = p3;
    m_tile_y_prev = tile_y;

    float row_t1 = std::numeric_limits<float>::infinity();
    float col_t1 = std::numeric_limits<float>::infinity();
    float dtdx = (float)TILE_SIZE / (vec.x);
    float dtdy = (float)TILE_SIZE / (vec.y);

    if (p0.y != p3.y) {
      float next_y = (float)(tile_y + (p3.y > p0.y ? 1 : 0)) * TILE_SIZE;
      row_t1 = std::min(1.0f, (next_y - p0.y) / vec.y);
    }

    if (p0.x != p3.x) {
      float next_x = (float)(tile_x + (p3.x > p0.x ? 1 : 0)) * TILE_SIZE;
      col_t1 = std::min(1.0f, (next_x - p0.x) / vec.x);
    }

    vec2 step = { std::abs(dtdx), std::abs(dtdy) };
    vec2 from = p0;

    while (true) {
      float t1 = std::min(row_t1, col_t1);

      vec2 to = p0 + vec * t1;
      vec2 tile_pos = TILE_SIZE * vec2{ (float)tile_x, (float)tile_y };
      vec2 from_delta = from - tile_pos;
      vec2 to_delta = to - tile_pos;

      int index = tile_index(tile_x, tile_y, m_size.x);

      Tile& tile = (*m_memory_pool)[index];

      if (!tile.active) {
        // TODO: test again
        tile.segments.reserve(25);
        tile.active = true;
      }

      if (from_delta.y != to_delta.y) {
        /* Direct cast to uint8_t without passing to std::roundf() is 10x faster in this case. */
        uint8_t y0 = static_cast<uint8_t>(from_delta.y * max_over_tile_size + 0.5f);
        uint8_t y1 = static_cast<uint8_t>(to_delta.y * max_over_tile_size + 0.5f);

        if (y0 != y1) {
          uint8_t x0 = static_cast<uint8_t>(from_delta.x * max_over_tile_size + 0.5f);
          uint8_t x1 = static_cast<uint8_t>(to_delta.x * max_over_tile_size + 0.5f);

          int icover = 1;
          int ify0 = (((y0 << 8) * 32) / 255);
          int ify1 = (((y1 << 8) * 32) / 255);

          if (y0 > y1) {
            icover = -1;
            std::swap(ify0, ify1);
          }

          int ii0 = ify0 >> 8;
          int ii1 = (ify1 >> 8) + 1;

          int iiy0 = ii0 << 8;
          int iiy1 = ii1 << 8;

          tile.cover_table[ii0] += icover * (iiy0 + (1 << 8) - ify0) / one_shifted;

          for (int j = ii0 + 1; j < ii1; j++) {
            tile.cover_table[j] += icover;
          }

          tile.cover_table[ii1 - 1] -= icover * (iiy1 - ify1) / one_shifted;

          tile.segments.emplace_back(x0, y0, x1, y1);
        }
      }

      bool fuzzy_equal;

      if (row_t1 < col_t1) {
        fuzzy_equal = row_t1 >= 1.0f - 0.0001f;
        row_t1 = std::min(1.0f, row_t1 + step.y);

        y += y_tile_dir;
        tile_y += y_dir;
      } else {
        fuzzy_equal = col_t1 >= 1.0f - 0.0001f;
        col_t1 = std::min(1.0f, col_t1 + step.x);
        x += x_tile_dir;
        tile_x += x_dir;
      }

      if (fuzzy_equal) {
        x = (int16_t)std::floorf(p3.x);
        y = (int16_t)std::floorf(p3.y);

        tile_x = x / TILE_SIZE;
        tile_y = y / TILE_SIZE;
      }

      if (tile_y != m_tile_y_prev) {
        int sign_index = tile_index(tile_x, std::min(tile_y, m_tile_y_prev), m_size.x);

        Tile& sign_tile = (*m_memory_pool)[sign_index];

        if (!sign_tile.active) {
          sign_tile.active = true;
        }

        sign_tile.winding += (int8_t)(tile_y - m_tile_y_prev);
        m_tile_y_prev = tile_y;
      }

      from = to;

      if (fuzzy_equal) break;
    }
  }

  void DrawableTiler::pack(const FillRule rule, const ivec2 tiles_count) {
    float cover_table[TILE_SIZE] = { 0.0f };
    int winding = 0;

    for (int16_t y = 0; y < m_size.y; y++) {
      std::memset(cover_table, 0, TILE_SIZE * sizeof(float));
      winding = 0;

      for (int16_t x = 0; x < m_size.x; x++) {
        int index = tile_index(x, y, m_size.x);
        Tile& tile = (*m_memory_pool)[index];

        if (tile.active) {
          Mask& mask = m_masks.emplace_back();

          winding += tile.winding;

          mask.tile_x = x;
          mask.tile_y = y;

          std::memcpy(mask.cover_table, cover_table, TILE_SIZE * sizeof(float));

          if (tile.segments.empty()) continue;

          mask.segments = std::move(tile.segments);

          for (int i = 0; i < TILE_SIZE; i++) {
            cover_table[i] += tile.cover_table[i];
          }
        } else if (
          (rule == FillRule::NonZero && winding != 0) ||
          (rule == FillRule::EvenOdd && winding % 2 != 0)
          ) {
          // TODO: batch spans together
          m_spans.push_back(Span{ x, y, 1 });
        }
      }
    }
  }

  /* -- Tiler -- */

#ifdef USE_F8x8

  inline static f24x8x2 transform_point(const mat2x3& transform, const vec2 point, const dvec2 offset, const vec2 subpixel, const double zoom) {
    double x = (
      static_cast<double>(transform[0][0]) * static_cast<double>(point.x) +
      static_cast<double>(transform[0][1]) * static_cast<double>(point.y) +
      static_cast<double>(transform[0][2]) - offset.x + (double)subpixel.x
      ) * zoom;
    double y = (
      static_cast<double>(transform[1][0]) * static_cast<double>(point.x) +
      static_cast<double>(transform[1][1]) * static_cast<double>(point.y) +
      static_cast<double>(transform[1][2]) - offset.y + (double)subpixel.y
      ) * zoom;

    return Math::double_to_f24x8x2(x, y);
  }

  inline static f24x8 sign(const f24x8 x) {
    return (0 < x) - (x < 0);
  }

  struct Drawable8x8Tiler {
    struct Tile {
      bool active = false;
      int8_t winding = 0;

      float cover_table[TILE_SIZE] = { 0.0f };
      std::vector<f8x8x4> segments;

      Tile() = default;
    };

    struct Span {
      int16_t tile_x;
      int16_t tile_y;

      int16_t width;

      Span(int16_t x, int16_t y, int16_t w) : tile_x(x), tile_y(y), width(w) {}
    };

    struct Mask {
      int16_t tile_x;
      int16_t tile_y;

      float cover_table[TILE_SIZE] = { 0.0f };
      std::vector<f8x8x4> segments;
    };

    inline const std::vector<Mask>& masks() const { return m_masks; }
    inline const std::vector<Span>& spans() const { return m_spans; }
    inline const ivec2 offset() const { return m_offset; }
    inline const ivec2 size() const { return m_size; }

    Drawable8x8Tiler(
      const Drawable& drawable,
      const ivec2 position,
      const ivec2 tiles_count,
      std::vector<Tile>& pool
    ) {
      rect bounds = {
        Math::floor((drawable.bounds.min - 32.0f) / TILE_SIZE) * TILE_SIZE,
        Math::ceil((drawable.bounds.max + 32.0f) / TILE_SIZE) * TILE_SIZE
      };

      ivec2 min_coords = tile_coords(bounds.min) + position;
      ivec2 max_coords = tile_coords(bounds.max) + position;

      // TODO: move this offset in tiler
      f24x8 x_offset = Math::float_to_f24x8(bounds.min.x);
      f24x8 y_offset = Math::float_to_f24x8(bounds.min.y);

      m_offset = min_coords;
      m_size = max_coords - min_coords;

      m_memory_pool = &pool;

      m_memory_pool->clear();
      m_memory_pool->resize(m_size.x * m_size.y);

      for (const Geometry::Contour& contour : drawable.contours) {
        if (contour.points.size() < 2) continue;

        f24x8 x_p = contour.points.front().x - x_offset;
        f24x8 y_p = contour.points.front().y - y_offset;

        move_to(x_p, y_p);

        for (size_t i = 1; i < contour.points.size(); i++) {
          x_p = contour.points[i].x - x_offset;
          y_p = contour.points[i].y - y_offset;


          line_to(x_p, y_p);
        }
      }

      pack(drawable.paint.rule, tiles_count);

      m_memory_pool = nullptr;
    }

    void move_to(f24x8 x, f24x8 y) {
      m_x = x;
      m_y = y;
    }

    enum class StepDirection {
      None,
      X,
      Y
    };

    void line_to(f24x8 x, f24x8 y) {
      if (m_x == x && m_y == y) return;

      f24x8 vec_x = x - m_x;
      f24x8 vec_y = y - m_y;

      f24x8 dir_x = sign(vec_x);
      f24x8 dir_y = sign(vec_y);

      int16_t x_tile_dir = static_cast<int16_t>(dir_x * TILE_SIZE);
      int16_t y_tile_dir = static_cast<int16_t>(dir_y * TILE_SIZE);

      int16_t tile_x = (m_x >> FRACBITS) / TILE_SIZE;
      int16_t tile_y = (m_y >> FRACBITS) / TILE_SIZE;

      m_tile_y_prev = tile_y;

      float fvec_x = Math::f24x8_to_float(vec_x);
      float fvec_y = Math::f24x8_to_float(vec_y);
      float t1_x = std::numeric_limits<float>::infinity();
      float t1_y = std::numeric_limits<float>::infinity();
      float dtdx = TILE_SIZE / fvec_x;
      float dtdy = TILE_SIZE / fvec_y;
      float step_x = std::abs(dtdx);
      float step_y = std::abs(dtdy);

      if (y != m_y) {
        f24x8 next_y = ((tile_y + (y > m_y ? 1 : 0)) * TILE_SIZE) << FRACBITS;
        t1_x = std::min(1.0f, Math::f24x8_to_float(next_y - m_y) / fvec_y);
      }

      if (x != m_x) {
        f24x8 next_x = ((tile_x + (x > m_x ? 1 : 0)) * TILE_SIZE) << FRACBITS;
        t1_y = std::min(1.0f, Math::f24x8_to_float(next_x - m_x) / fvec_x);
      }

      f24x8 from_x = m_x;
      f24x8 from_y = m_y;

      while (true) {
        float t1 = std::min(t1_x, t1_y);

        f24x8 to_x = m_x + Math::float_to_f24x8(t1 * fvec_x);
        f24x8 to_y = m_y + Math::float_to_f24x8(t1 * fvec_y);
        f24x8 tile_pos_x = (tile_x * TILE_SIZE) << FRACBITS;
        f24x8 tile_pos_y = (tile_y * TILE_SIZE) << FRACBITS;

        f8x8 x0 = static_cast<f8x8>(from_x - tile_pos_x);
        f8x8 y0 = static_cast<f8x8>(from_y - tile_pos_y);
        f8x8 x1 = static_cast<f8x8>(to_x - tile_pos_x);
        f8x8 y1 = static_cast<f8x8>(to_y - tile_pos_y);

        int index = tile_index(tile_x, tile_y, m_size.x);

        Tile& tile = (*m_memory_pool)[index];

        // console::log("size", tile.segments.size());

        if (!tile.active) {
          // TODO: test again
          // tile.segments.reserve(25);
          tile.active = true;
          m_masks_num++;
        } else {
          // console::log("size active", tile.segments.size());
        }

        if (y0 != y1) {
          tile.segments.emplace_back(x0, y0, x1, y1);
          // console::log("size pushed", tile.segments.size());

          float cover = 1;

          if (y0 > y1) {
            cover = -1.0f;
            std::swap(y0, y1);
          }

          f8x8 y0_int = Math::int_bits(y0);
          f8x8 y1_int = std::min(Math::int_bits(y1) + FRACUNIT, TILE_SIZE << FRACBITS);

          int8_t i0 = static_cast<int8_t>(y0_int >> FRACBITS);
          int8_t i1 = static_cast<int8_t>(y1_int >> FRACBITS);

          // console::log("i0", (int)i0);
          // console::log("i1", (int)i1);

          tile.cover_table[i0] += cover * Math::f8x8_to_float(y0_int + FRACUNIT - y0);

          for (int j = i0 + 1; j < i1; j++) {
            tile.cover_table[j] += cover;
          }

          tile.cover_table[i1 - 1] -= cover * Math::f8x8_to_float(y1_int - y1);
        }

        from_x = to_x;
        from_y = to_y;

        bool fuzzy_equal;

        if (t1_x < t1_y) {
          fuzzy_equal = t1_x >= 1.0f - 0.0001f;
          t1_x = std::min(1.0f, t1_x + step_y);

          // TODO: cache max
          from_y = ((tile_y + std::max(0, dir_y)) * TILE_SIZE) << FRACBITS;

          tile_y += dir_y;
        } else {
          fuzzy_equal = t1_y >= 1.0f - 0.0001f;
          t1_y = std::min(1.0f, t1_y + step_x);

          // TODO: cache max
          from_x = ((tile_x + std::max(0, dir_x)) * TILE_SIZE) << FRACBITS;

          tile_x += dir_x;
        }

        if (fuzzy_equal) {
          tile_x = (x >> FRACBITS) / TILE_SIZE;
          tile_y = (y >> FRACBITS) / TILE_SIZE;
        }

        if (tile_y != m_tile_y_prev) {
          int sign_index = tile_index(tile_x, std::min(tile_y, m_tile_y_prev), m_size.x);

          Tile& sign_tile = (*m_memory_pool)[sign_index];

          if (!sign_tile.active) {
            sign_tile.active = true;
            m_masks_num++;
          }

          sign_tile.winding += (int8_t)(tile_y - m_tile_y_prev);
          m_tile_y_prev = tile_y;
        }

        if (fuzzy_equal) break;
      }

      m_x = x;
      m_y = y;
    }

    void pack(const FillRule rule, const ivec2 tiles_count) {
      float cover_table[TILE_SIZE] = { 0.0f };
      int winding = 0;

      m_masks.reserve(m_masks_num);

      for (int16_t y = 0; y < m_size.y; y++) {
        std::memset(cover_table, 0, TILE_SIZE * sizeof(float));
        winding = 0;

        for (int16_t x = 0; x < m_size.x; x++) {
          int index = tile_index(x, y, m_size.x);
          Tile& tile = (*m_memory_pool)[index];

          if (tile.active) {
            Mask& mask = m_masks.emplace_back();

            winding += tile.winding;

            mask.tile_x = x;
            mask.tile_y = y;

            std::memcpy(mask.cover_table, cover_table, TILE_SIZE * sizeof(float));

            // console::log("size", tile.segments.size());

            if (tile.segments.empty()) continue;

            mask.segments = std::move(tile.segments);

            for (int i = 0; i < TILE_SIZE; i++) {
              cover_table[i] += tile.cover_table[i];
            }
          } else if (
            (rule == FillRule::NonZero && winding != 0) ||
            (rule == FillRule::EvenOdd && winding % 2 != 0)
            ) {
            // TODO: batch spans together
            m_spans.push_back(Span{ x, y, 1 });
          }
        }
      }
    }
  private:
    f24x8 m_x;
    f24x8 m_y;

    ivec2 m_offset;
    ivec2 m_size;

    std::vector<Mask> m_masks;
    std::vector<Span> m_spans;

    // TODO: convert to uint8_t or uint16_t
    int16_t m_tile_y_prev;
    uint16_t m_masks_num = 0;

    std::vector<Tile>* m_memory_pool;
  };

  static std::vector<Drawable8x8Tiler::Tile> s_pool;

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
      -viewport.position,
      -viewport.position + vec2{
        static_cast<float>(viewport.size.x) / viewport.zoom,
        static_cast<float>(viewport.size.y) / viewport.zoom
      }
    };
    m_visible_min = {
      static_cast<double>(m_visible.min.x + m_subpixel.x),
      static_cast<double>(m_visible.min.y + m_subpixel.y)
    };

    s_pool = std::vector<Drawable8x8Tiler::Tile>(m_size.x * m_size.y);
    m_culled_tiles = std::vector<bool>(m_size.x * m_size.y, false);

    m_filled_batches = std::vector<FilledTilesBatch>(1);
    m_masked_batches = std::vector<MaskedTilesBatch>(1);
  }

  void Tiler::process_stroke(const Geometry::Path& path, const mat2x3& transform, const Stroke& stroke) {}

  void Tiler::process_fill(const Geometry::Path& path, const mat2x3& transform, const Fill& fill) {
    GK_TOTAL("Tiler::process_fill");

    const rect path_rect = transform * path.bounding_rect();
    const float overlap = Math::rect_rect_intersection_area(path_rect, m_visible) / path_rect.area();

    if (overlap <= 0.0f) return;

    // TODO: convert to ivec2 to make clear that we are rounding down
    dvec2 offset = {
      (std::round(m_visible_min.x * m_zoom / TILE_SIZE) * TILE_SIZE) / m_zoom,
      (std::round(m_visible_min.y * m_zoom / TILE_SIZE) * TILE_SIZE) / m_zoom
    };

    Drawable drawable(1, fill, (path_rect - vec2{ (float)offset.x, (float)offset.y }) * m_zoom + m_subpixel);
    Geometry::Contour& contour = drawable.contours.front();

    const auto& segments = path.segments();
    const f24x8x2 first = transform_point(transform, segments.front().p0(), offset, m_subpixel / m_zoom, m_zoom);

    contour.begin(first);

    for (size_t i = 0; i < segments.size(); i++) {
      auto& raw_segment = segments[i];

      if (raw_segment.is_linear()) {
        contour.push_segment(transform_point(transform, raw_segment.p3(), offset, m_subpixel / m_zoom, m_zoom));
      } else {
        contour.push_segment(
          transform_point(transform, raw_segment.p1(), offset, m_subpixel / m_zoom, m_zoom),
          transform_point(transform, raw_segment.p2(), offset, m_subpixel / m_zoom, m_zoom),
          transform_point(transform, raw_segment.p3(), offset, m_subpixel / m_zoom, m_zoom)
        );
      }
    }

    contour.close();

    process_drawable(drawable, m_visible, { (float)(offset.x * m_zoom), (float)(offset.y * m_zoom) }, overlap < 0.7f);
  }

  void Tiler::process_drawable(const Drawable& drawable, const rect& visible, const vec2 offset, const bool clip) {
    ivec2 tile_offset = tile_coords(offset);
    vec2 pixel_offset = offset - TILE_SIZE * IVEC2_TO_VEC2(tile_offset);

    Drawable8x8Tiler tiler(
      drawable,
      m_position + tile_offset,
      m_size,
      s_pool
    );

    const std::vector<Drawable8x8Tiler::Mask>& masks = tiler.masks();
    const std::vector<Drawable8x8Tiler::Span>& spans = tiler.spans();
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

      uint32_t segments_size = (uint32_t)mask.segments.size();

      // masks_batch.segments_ptr[0] = (uint8_t)segments_size;
      // masks_batch.segments_ptr[1] = 0;
      // masks_batch.segments_ptr[2] = 0;
      // masks_batch.segments_ptr[3] = 0;
      masks_batch.segments_ptr[0] = (uint8_t)segments_size;
      masks_batch.segments_ptr[1] = (uint8_t)(segments_size >> 8);
      masks_batch.segments_ptr[2] = (uint8_t)(segments_size >> 16);
      masks_batch.segments_ptr[3] = (uint8_t)(segments_size >> 24);
      masks_batch.segments_ptr += 4;

      for (auto segment : mask.segments) {
        masks_batch.segments_ptr[0] = static_cast<uint8_t>(segment.x0 >> FRACBITS);
        masks_batch.segments_ptr[1] = static_cast<uint8_t>(segment.x0);
        masks_batch.segments_ptr[2] = static_cast<uint8_t>(segment.y0 >> FRACBITS);
        masks_batch.segments_ptr[3] = static_cast<uint8_t>(segment.y0);
        masks_batch.segments_ptr[4] = static_cast<uint8_t>(segment.x1 >> FRACBITS);
        masks_batch.segments_ptr[5] = static_cast<uint8_t>(segment.x1);
        masks_batch.segments_ptr[6] = static_cast<uint8_t>(segment.y1 >> FRACBITS);
        masks_batch.segments_ptr[7] = static_cast<uint8_t>(segment.y1);

        // memcpy(masks_batch.segments_ptr, &segment, sizeof(f8x8x4));
        // masks_batch.segments_ptr[0] = (uint8_t)std::clamp(Math::f8x8_to_float(segment.x0) * 255 / TILE_SIZE, 0.0f, 255.0f);
        // masks_batch.segments_ptr[1] = (uint8_t)std::clamp(Math::f8x8_to_float(segment.y0) * 255 / TILE_SIZE, 0.0f, 255.0f);
        // masks_batch.segments_ptr[2] = (uint8_t)std::clamp(Math::f8x8_to_float(segment.x1) * 255 / TILE_SIZE, 0.0f, 255.0f);
        // masks_batch.segments_ptr[3] = (uint8_t)std::clamp(Math::f8x8_to_float(segment.y1) * 255 / TILE_SIZE, 0.0f, 255.0f);
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

        int index = tile_index({ coords.x + i, coords.y }, m_size);
        if (!m_culled_tiles[index]/*&& color.a == 1.0f*/) {
          // TODO: abstract
          fills_batch.tiles.push_back({ drawable.paint.color, index, drawable.paint.z_index });
          m_culled_tiles[index] = true;
        }
      }
    }
  }

#else
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

        int index = tile_index({ coords.x + i, coords.y }, m_tiles_count);
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
