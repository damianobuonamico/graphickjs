#include "tiler.h"

#include "geometry/path.h"

#include "../math/math.h"

#include "../utils/console.h"

#include <algorithm>

namespace Graphick::Renderer {

#define OPAQUE_AND_MASKED 0

  static constexpr float tolerance = 0.25f;

  static inline ivec2 tile_coords(const vec2 p) {
    return { (int)std::floor(p.x / TILE_SIZE), (int)std::floor(p.y / TILE_SIZE) };
  }

  static inline ivec2 tile_coords_clamp(const vec2 p, const ivec2 tiles_count) {
    return { std::clamp((int)std::floor(p.x / TILE_SIZE), 0, tiles_count.x - 1), std::clamp((int)std::floor(p.y / TILE_SIZE), 0, tiles_count.y - 1) };
  }

  static inline int tile_index(const ivec2 coords, const ivec2 tiles_count) {
    return coords.x + coords.y * tiles_count.x;
  }

  static inline int tile_index(const int16_t tile_x, const int16_t tile_y, const int16_t tiles_count_x) {
    return tile_x + tile_y * tiles_count_x;
  }

  static inline float x_intersect(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4) {
    float num = (x1 * y2 - y1 * x2) * (x3 - x4) -
      (x1 - x2) * (x3 * y4 - y3 * x4);
    float den = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    return num / den;
  }

  static inline float x_intersect(float one_over_m, float q, float y) {
    return (y - q) * one_over_m;
  }

  static inline float y_intersect(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4) {
    float num = (x1 * y2 - y1 * x2) * (y3 - y4) -
      (y1 - y2) * (x3 * y4 - y3 * x4);
    float den = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    return num / den;
  }

  static inline float y_intersect(float m, float q, float x) {
    return m * x + q;
  }

  static void clip_to_left(std::vector<vec2>& points, float x) {
    if (points.empty()) return;
    std::vector<vec2> new_points;

    for (int i = 0; i < points.size() - 1; i++) {
      vec2 point = points[i];

      if (point.x < x) {
        if (points[i + 1].x > x) {
          new_points.push_back({ x, y_intersect(x, -1000.0f, x, 1000.0f, point.x, point.y, points[i + 1].x, points[i + 1].y) });
        }
      } else {
        new_points.push_back(point);

        if (points[i + 1].x < x) {
          new_points.push_back({ x, y_intersect(x, -1000.0f, x, 1000.0f, point.x, point.y, points[i + 1].x, points[i + 1].y) });
        }
      }
    }

    if (new_points.size() > 2 && new_points.front() != new_points.back()) {
      new_points.push_back(new_points.front());
    }

    points = new_points;
  }

  static void clip_to_right(std::vector<vec2>& points, float x) {
    if (points.empty()) return;
    std::vector<vec2> new_points;

    for (int i = 0; i < points.size() - 1; i++) {
      vec2 point = points[i];

      if (point.x > x) {
        if (points[i + 1].x < x) {
          new_points.push_back({ x, y_intersect(x, -1000.0f, x, 1000.0f, point.x, point.y, points[i + 1].x, points[i + 1].y) });
        }
      } else {
        new_points.push_back(point);

        if (points[i + 1].x > x) {
          new_points.push_back({ x, y_intersect(x, -1000.0f, x, 1000.0f, point.x, point.y, points[i + 1].x, points[i + 1].y) });
        }
      }
    }

    if (new_points.size() > 2 && new_points.front() != new_points.back()) {
      new_points.push_back(new_points.front());
    }

    points = new_points;
  }

  static void clip_to_top(std::vector<vec2>& points, float y) {
    if (points.empty()) return;
    std::vector<vec2> new_points;

    for (int i = 0; i < points.size() - 1; i++) {
      vec2 point = points[i];

      if (point.y < y) {
        if (points[i + 1].y > y) {
          new_points.push_back({ x_intersect(-1000.0f, y, 1000.0f, y, point.x, point.y, points[i + 1].x, points[i + 1].y), y });
        }
      } else {
        new_points.push_back(point);

        if (points[i + 1].y < y) {
          new_points.push_back({ x_intersect(-1000.0f, y, 1000.0f, y, point.x, point.y, points[i + 1].x, points[i + 1].y), y });
        }
      }
    }

    if (new_points.size() > 2 && new_points.front() != new_points.back()) {
      new_points.push_back(new_points.front());
    }

    points = new_points;
  }

  static vec2 clip_to_bottom(std::vector<vec2>& points, float y) {
    vec2 min = std::numeric_limits<vec2>::max();

    if (points.empty()) return min;
    std::vector<vec2> new_points;

    for (int i = 0; i < points.size() - 1; i++) {
      vec2 point = points[i];

      if (point.y > y) {
        if (points[i + 1].y < y) {
          new_points.push_back({ x_intersect(-1000.0f, y, 1000.0f, y, point.x, point.y, points[i + 1].x, points[i + 1].y), y });
          min = Math::min(min, new_points.back());
        }
      } else {
        new_points.push_back(point);
        min = Math::min(min, new_points.back());

        if (points[i + 1].y > y) {
          new_points.push_back({ x_intersect(-1000.0f, y, 1000.0f, y, point.x, point.y, points[i + 1].x, points[i + 1].y), y });
          min = Math::min(min, new_points.back());
        }
      }
    }

    if (new_points.size() > 2 && new_points.front() != new_points.back()) {
      new_points.push_back(new_points.front());
      min = Math::min(min, new_points.back());
    }

    points = new_points;

    return min;
  }

  static vec2 clip(std::vector<vec2>& points, rect visible) {
    // OPTICK_EVENT();

    clip_to_left(points, visible.min.x);
    clip_to_right(points, visible.max.x);
    clip_to_top(points, visible.min.y);
    return clip_to_bottom(points, visible.max.y);
  }

  PathTiler::PathTiler(const Geometry::Path& path, const vec2 translation, const vec4& color, const rect& visible, float zoom, ivec2 position, const std::vector<bool>& culled, const ivec2 tiles_count) :
    m_zoom(zoom),
    m_position(position),
    m_tile_y_prev(0)
  {
    // OPTICK_EVENT();

    // TODO: cache bounding_rects
    rect rect = path.bounding_rect() + translation;

    float intersection_overlap = Math::rect_rect_intersection_area(rect, visible) / rect.area();
    if (intersection_overlap <= 0.0f) return;

    float zoom_factor = m_zoom / TILE_SIZE;

    rect.min = floor(rect.min * zoom_factor) * TILE_SIZE;
    rect.max = ceil(rect.max * zoom_factor) * TILE_SIZE;

    ivec2 min_coords = tile_coords(rect.min) + m_position;
    ivec2 max_coords = tile_coords(rect.max) + m_position;

    m_offset = min_coords;
    m_bounds_size = max_coords - min_coords;

    const auto& segments = path.segments();

    m_prev = (segments.front().p0() + translation) * m_zoom - rect.min;

    if (intersection_overlap < 0.7f) {
      std::vector<vec2> points;
      points.reserve(segments.size() + 1);

      vec2 first_point = (segments.front().p0() + translation) * m_zoom/* - rect.min*/;
      points.push_back(first_point);

      Math::rect vis = visible * zoom;

      vis.min = floor(vis.min / TILE_SIZE) * TILE_SIZE - 1;
      vis.max = ceil(vis.max / TILE_SIZE) * TILE_SIZE + TILE_SIZE + 1;

      for (const auto& segment : segments) {
        vec2 p0 = (segment->p0() + translation) * m_zoom;
        vec2 p3 = (segment->p3() + translation) * m_zoom;

        if (segment->is_cubic()) {
          vec2 p1 = (segment->p1() + translation) * m_zoom;
          vec2 p2 = (segment->p2() + translation) * m_zoom;

          Math::rect segment_rect = segment->bounding_rect() + translation;
          if (Math::does_rect_intersect_rect(segment_rect, visible)) {
            vec2 a = -1.0f * p0 + 3.0f * p1 - 3.0f * p2 + p3;
            vec2 b = 3.0f * (p0 - 2.0f * p1 + p2);

            float conc = std::max(Math::length(b), Math::length(a + b));
            float dt = std::sqrtf((std::sqrtf(8.0f) * tolerance) / conc);
            float t = 0.0f;

            while (t < 1.0f) {
              t = std::min(t + dt, 1.0f);

              vec2 p01 = Math::lerp(p0, p1, t);
              vec2 p12 = Math::lerp(p1, p2, t);
              vec2 p23 = Math::lerp(p2, p3, t);
              vec2 p012 = Math::lerp(p01, p12, t);
              vec2 p123 = Math::lerp(p12, p23, t);

              points.push_back(Math::lerp(p012, p123, t));
            }
          } else {
            points.push_back(p3);
          }
        } else if (segment->is_quadratic()) {
          vec2 p1 = (segment->p1() + translation) * m_zoom;

          float dt = std::sqrtf((4.0f * tolerance) / Math::length(p0 - 2.0f * p1 + p3));
          float t = 0.0f;

          while (t < 1.0f) {
            t = std::min(t + dt, 1.0f);

            vec2 p01 = Math::lerp(p0, p1, t);
            vec2 p12 = Math::lerp(p1, p3, t);

            points.push_back(Math::lerp(p01, p12, t));
          }
        } else {
          points.push_back(p3);
        }
      }

      if (points.size() > 1 && points.front() != points.back()) {
        points.push_back(points.front());
      }

      vec2 min = clip(points, vis);
      if (points.empty()) return;

      min = floor(min / TILE_SIZE) * TILE_SIZE;

      for (int i = 0; i < points.size() - 1; i++) {
        process_linear_segment(points[i] - min, points[i + 1] - min);
      }

      m_offset = tile_coords(min) + m_position;
    } else {
      for (const auto& segment : segments) {
        vec2 p0 = (segment->p0() + translation) * m_zoom - rect.min;
        vec2 p3 = (segment->p3() + translation) * m_zoom - rect.min;

        if (segment->is_cubic()) {
          vec2 p1 = (segment->p1() + translation) * m_zoom - rect.min;
          vec2 p2 = (segment->p2() + translation) * m_zoom - rect.min;

          process_cubic_segment(p0, p1, p2, p3);
        } else if (segment->is_quadratic()) {
          vec2 p1 = (segment->p1() + translation) * m_zoom - rect.min;

          process_quadratic_segment(p0, p1, p3);
        } else {
          process_linear_segment(p0, p3);
        }
      }

      if (!path.closed()) {
        vec2 p0 = (segments.back().p3() + translation) * m_zoom - rect.min;
        vec2 p3 = (segments.front().p0() + translation) * m_zoom - rect.min;

        process_linear_segment(p0, p3);
      }
    }

    finish(tiles_count);
  }

  void PathTiler::push_segment(const uvec4 segment, int16_t tile_x, int16_t tile_y) {
    if (tile_x >= m_bounds_size.x || tile_y >= m_bounds_size.y) return;

    int index = tile_index(tile_x, tile_y, m_bounds_size.x);

    if (m_masks.find(index) == m_masks.end()) {
      if (segment.y0 == segment.y1) {
        m_masks.insert({ index, { { } } });
      } else {
        m_masks.insert({ index, { { segment } } });
      }
    } else {
      if (segment.y0 != segment.y1) {
        m_masks.at(index).segments.push_back(segment);
      }
    }
  }

  void PathTiler::process_linear_segment(const vec2 p0, const vec2 p3) {
    // OPTICK_EVENT();

    m_prev = p3;
    m_tile_y_prev = (int16_t)std::floor(p0.y) / TILE_SIZE;

    if (Math::is_almost_equal(p0, p3)) return;

    float x_vec = p3.x - p0.x;
    float y_vec = p3.y - p0.y;

    int16_t x_dir = (int16_t)std::copysign(1, x_vec);
    int16_t y_dir = (int16_t)std::copysign(1, y_vec);

    float dtdx = (float)TILE_SIZE / (x_vec);
    float dtdy = (float)TILE_SIZE / (y_vec);

    int16_t x = (int16_t)std::floor(p0.x);
    int16_t y = (int16_t)std::floor(p0.y);

    float row_t1 = std::numeric_limits<float>::infinity();
    float col_t1 = std::numeric_limits<float>::infinity();

    if (p0.y != p3.y) {
      float next_y = (float)(y / TILE_SIZE + (p3.y > p0.y ? 1 : 0)) * TILE_SIZE;
      row_t1 = std::min(1.0f, (next_y - p0.y) / (y_vec));
    }
    if (p0.x != p3.x) {
      float next_x = (float)(x / TILE_SIZE + (p3.x > p0.x ? 1 : 0)) * TILE_SIZE;
      col_t1 = std::min(1.0f, (next_x - p0.x) / (x_vec));
    }

    float x_step = std::abs(dtdx);
    float y_step = std::abs(dtdy);

    vec2 from = p0;

    while (true) {
      float t1 = std::min(row_t1, col_t1);

      vec2 to = lerp(p0, p3, t1);

      int16_t tile_x = x / TILE_SIZE;
      int16_t tile_y = y / TILE_SIZE;

      if (tile_x != m_bin.tile_x || tile_y != m_bin.tile_y) {
        m_bins.push_back(m_bin);
        m_bin = { tile_x, tile_y };
      }

      vec2 tile_pos = TILE_SIZE * vec2{ (float)tile_x, (float)tile_y };
      vec2 from_delta = from - tile_pos;
      vec2 to_delta = to - tile_pos;

      push_segment({
        (uint8_t)std::round(from_delta.x / TILE_SIZE * 255),
        (uint8_t)std::round(from_delta.y / TILE_SIZE * 255),
        (uint8_t)std::round(to_delta.x / TILE_SIZE * 255),
        (uint8_t)std::round(to_delta.y / TILE_SIZE * 255),
        }, tile_x, tile_y);

      bool fuzzy_equal;

      if (row_t1 < col_t1) {
        fuzzy_equal = Math::is_almost_equal(row_t1, 1.0f, 0.0001f);
        row_t1 = std::min(1.0f, row_t1 + y_step);
        y += y_dir * TILE_SIZE;
      } else {
        fuzzy_equal = Math::is_almost_equal(col_t1, 1.0f, 0.0001f);
        col_t1 = std::min(1.0f, col_t1 + x_step);
        x += x_dir * TILE_SIZE;
      }

      if (fuzzy_equal) {
        x = (int16_t)std::floor(p3.x);
        y = (int16_t)std::floor(p3.y);
      }

      tile_y = y / TILE_SIZE;
      from = to;

      if (tile_y != m_tile_y_prev) {
        m_tile_increments.push_back(Increment{ (int16_t)(x / TILE_SIZE), std::min(tile_y, m_tile_y_prev), (int8_t)(tile_y - m_tile_y_prev) });
        m_tile_y_prev = tile_y;
      }

      if (fuzzy_equal) break;
    }
  }

  void PathTiler::process_quadratic_segment(const vec2 p0, const vec2 p1, const vec2 p3) {
    float dt = std::sqrtf((4.0f * tolerance) / Math::length(p0 - 2.0f * p1 + p3));
    float t = 0.0f;

    while (t < 1.0f) {
      t = std::min(t + dt, 1.0f);

      vec2 p01 = Math::lerp(p0, p1, t);
      vec2 p12 = Math::lerp(p1, p3, t);

      process_linear_segment(m_prev, Math::lerp(p01, p12, t));
    }
  }

  void PathTiler::process_cubic_segment(const vec2 p0, const vec2 p1, const vec2 p2, const vec2 p3) {
    // OPTICK_EVENT();

    vec2 a = -1.0f * p0 + 3.0f * p1 - 3.0f * p2 + p3;
    vec2 b = 3.0f * (p0 - 2.0f * p1 + p2);

    float conc = std::max(Math::length(b), Math::length(a + b));
    float dt = std::sqrtf((std::sqrtf(8.0f) * tolerance) / conc);
    float t = 0.0f;

    while (t < 1.0f) {
      t = std::min(t + dt, 1.0f);

      vec2 p01 = Math::lerp(p0, p1, t);
      vec2 p12 = Math::lerp(p1, p2, t);
      vec2 p23 = Math::lerp(p2, p3, t);
      vec2 p012 = Math::lerp(p01, p12, t);
      vec2 p123 = Math::lerp(p12, p23, t);

      process_linear_segment(m_prev, Math::lerp(p012, p123, t));
    }
  }

  void PathTiler::finish(const ivec2 tiles_count) {
    // OPTICK_EVENT();

    m_bins.push_back(m_bin);

    {
      OPTICK_EVENT("Sort Bins");
      std::sort(m_bins.begin(), m_bins.end(), [](const Bin& a, const Bin& b) {
        return a.tile_y < b.tile_y || (a.tile_y == b.tile_y && a.tile_x < b.tile_x);
        });
    }

    {
      OPTICK_EVENT("Sort Increments");
      std::sort(m_tile_increments.begin(), m_tile_increments.end(), [](const Increment& a, const Increment& b) {
        return a.tile_y < b.tile_y || (a.tile_y == b.tile_y && a.tile_x < b.tile_x);
        });
    }

    int tile_increments_i = 0;
    int winding = 0;
    size_t bins_len = m_bins.size();
    ivec2 prev_coords = { -1, -1 };
    float cover_table[TILE_SIZE] = { 0.0f };

    for (size_t i = 0; i < bins_len; i++) {
      const Bin& bin = m_bins[i];
      ivec2 coords = { bin.tile_x, bin.tile_y };

      if (coords != prev_coords) {
        OPTICK_EVENT("Generate Cover Table");

        if (coords.y != prev_coords.y) {
          memset(cover_table, 0, TILE_SIZE * sizeof(float));
        }

        int index = tile_index({ bin.tile_x, bin.tile_y }, m_bounds_size);
        const auto& it = m_masks.find(index);

        if (it != m_masks.end()) {
          Mask& mask = it->second;

          for (int j = 0; j < TILE_SIZE; j++) {
            // TODO: implement different workflow for non-zero winding rule (maybe separate cover table)
            // mask.cover_table[j] = (uint8_t)std::round(127.5f + cover_table[j] * 127.5f);

            mask.cover_table[j] = (uint8_t)Math::wrap((int)std::round(127.0f + cover_table[j] * 127.0f), 0, 254);
          }

          {
            // GK_TOTAL("Calculate Cover");
            // TODO: Optimize further if possible, maybe use SIMD

            float tile_size_over_255 = (float)TILE_SIZE / 255.0f;

            for (auto& segment : mask.segments) {
              float p0_y = tile_size_over_255 * (float)segment.y0;
              float p1_y = tile_size_over_255 * (float)segment.y1;

              // Segment is always on the left of the tile so we don't need to check x
              for (int j = 0; j < TILE_SIZE; j++) {
                float y0 = (float)j;
                float y1 = y0 + 1.0f;

                cover_table[j] += std::clamp(p1_y, y0, y1) - std::clamp(p0_y, y0, y1);
              }
            }
          }

          prev_coords = coords;
        }
      }

      if (i + 1 == bins_len || m_bins[i + 1].tile_x != bin.tile_x || m_bins[i + 1].tile_y != bin.tile_y) {
        uint8_t tile[TILE_SIZE * TILE_SIZE] = { 0 };

        ivec2 coords = { bin.tile_x + m_offset.x + 1, bin.tile_y + m_offset.y + 1 };
        int index = tile_index(coords, tiles_count);

        if (i + 1 < m_bins.size() && m_bins[i + 1].tile_y == bin.tile_y && m_bins[i + 1].tile_x > bin.tile_x + 1) {
          OPTICK_EVENT("Generate Span");

          while (tile_increments_i < m_tile_increments.size()) {
            Increment& tile_increment = m_tile_increments[tile_increments_i];
            if (std::tie(tile_increment.tile_y, tile_increment.tile_x) > std::tie(bin.tile_y, bin.tile_x)) break;

            winding += tile_increment.sign;
            tile_increments_i++;
          }

          // Non-zero winding rule
          // if (winding != 0) {
            // Even-odd winding rule
          if (winding % 2 != 0) {
            int16_t width = m_bins[i + 1].tile_x - bin.tile_x - 1;
            m_spans.push_back(Span{ (int16_t)(bin.tile_x + 1), bin.tile_y, width });
          }
        }
      }
    }
  }

  Tiler::Tiler() {
    m_segments = new uint8_t[SEGMENTS_TEXTURE_SIZE * SEGMENTS_TEXTURE_SIZE * 4];
  }

  Tiler::~Tiler() {
    delete[] m_segments;
  }

  void Tiler::reset(const Viewport& viewport) {
    m_tiles_count = { (int)(std::ceil((float)viewport.size.x / TILE_SIZE)) + 2, (int)(std::ceil((float)viewport.size.y / TILE_SIZE)) + 2 };
    m_position = {
      (int)(viewport.position.x > 0 ? std::floor(viewport.position.x * viewport.zoom / TILE_SIZE) : std::ceil(viewport.position.x * viewport.zoom / TILE_SIZE)),
      (int)(viewport.position.y > 0 ? std::floor(viewport.position.y * viewport.zoom / TILE_SIZE) : std::ceil(viewport.position.y * viewport.zoom / TILE_SIZE))
    };
    m_zoom = viewport.zoom;
    m_visible = { -viewport.position, vec2{(float)viewport.size.x / viewport.zoom, (float)viewport.size.y / viewport.zoom} - viewport.position };

    m_opaque_tiles.clear();
    m_masked_tiles.clear();

    m_segments_ptr = m_segments;

    m_culled_tiles = std::vector<bool>(m_tiles_count.x * m_tiles_count.y, false);
  }

  void Tiler::process_path(const Geometry::Path& path, const vec2 translation, const vec4& color, const float z_index) {
    // OPTICK_EVENT();

    PathTiler tiler(path, translation, color, m_visible, m_zoom, m_position, m_culled_tiles, m_tiles_count);

    const std::vector<PathTiler::Span>& spans = tiler.spans();
    const std::unordered_map<int, PathTiler::Mask>& masks = tiler.masks();
    const ivec2 offset = tiler.offset();
    const ivec2 size = tiler.size();

    for (const auto& [index, mask] : masks) {
      ivec2 coords = {
        index % size.x + offset.x + 1,
        index / size.x + offset.y + 1
      };

      if (coords.x < 0 || coords.y < 0 || coords.x >= m_tiles_count.x || coords.y >= m_tiles_count.y) continue;

      int absolute_index = tile_index(coords, m_tiles_count);
      if (m_culled_tiles[absolute_index]) continue;

      int offset = (int)(m_segments_ptr - m_segments) / 4;
      uint32_t segments_size = (uint32_t)mask.segments.size();

      m_segments_ptr[0] = (uint8_t)segments_size;
      m_segments_ptr[1] = (uint8_t)(segments_size >> 8);
      m_segments_ptr[2] = (uint8_t)(segments_size >> 16);
      m_segments_ptr[3] = (uint8_t)(segments_size >> 24);
      m_segments_ptr += 4;

      memcpy(m_segments_ptr, &mask.cover_table, TILE_SIZE);
      m_segments_ptr += TILE_SIZE;

      for (auto segment : mask.segments) {
        m_segments_ptr[0] = segment.x0;
        m_segments_ptr[1] = segment.y0;
        m_segments_ptr[2] = segment.x1;
        m_segments_ptr[3] = segment.y1;
        m_segments_ptr += 4;
      }

      m_masked_tiles.push_back(MaskedTile{
        color,
        absolute_index,
        ivec2{ offset % SEGMENTS_TEXTURE_SIZE, offset / SEGMENTS_TEXTURE_SIZE },
        z_index
        });
    }

    for (auto& span : spans) {
      ivec2 coords = { span.tile_x + offset.x + 1, span.tile_y + offset.y + 1 };
      if (coords.x + span.width < 0 || coords.y < 0 || coords.x >= m_tiles_count.x || coords.y >= m_tiles_count.y) continue;

      int16_t width = coords.x < 0 ? span.width + coords.x : span.width;
      coords.x = std::max(coords.x, 0);

      for (int i = 0; i < width; i++) {
        if (coords.x + i >= m_tiles_count.x) break;

        int index = tile_index({ coords.x + i, coords.y }, m_tiles_count);
        if (!m_culled_tiles[index]/*&& color.a == 1.0f*/) {
          m_opaque_tiles.push_back({ color, index, z_index });
          m_culled_tiles[index] = true;
        }
      }
    }
  }

}
