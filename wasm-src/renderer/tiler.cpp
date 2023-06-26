#include "tiler.h"

#include "../utils/console.h"
#include "../math/math.h"

#include <algorithm>

namespace Graphick::Render {

  static inline ivec2 tile_coords(const vec2 p) {
    return { (int)std::floor(p.x / TILE_SIZE), (int)std::floor(p.y / TILE_SIZE) };
  }

  static inline int tile_index(const ivec2 coords, const ivec2 tiles_count) {
    return coords.x + coords.y * tiles_count.x;
  }

  static inline bool compare_bins(const PathTiler::Bin& a, const PathTiler::Bin& b) {
    return a.tile_y < b.tile_y || (a.tile_y == b.tile_y && a.tile_x < b.tile_x);
  }

  PathTiler::PathTiler(const Geometry::Path& path, const vec4& color, const Box& visible, float zoom, ivec2 position) :
    m_zoom(zoom),
    m_position(position)
  {
    OPTICK_EVENT();

    Box box = path.bounding_box();

    // TODO: Maybe optimize this check
    if (!does_box_intersect_box(box, visible)) return;

    float zoom_factor = m_zoom / TILE_SIZE;

    box.min = floor(box.min * zoom_factor) * TILE_SIZE;
    box.max = ceil(box.max * zoom_factor) * TILE_SIZE;

    ivec2 min_coords = tile_coords(box.min) + m_position;
    ivec2 max_coords = tile_coords(box.max) + m_position;

    m_offset = min_coords;
    m_bounds_size = max_coords - min_coords;

    const std::vector<Geometry::Segment>& segments = path.segments();

    for (const auto& segment : segments) {
      // TODO: Check segment's kind and handle other kind of segments
      process_linear_segment(segment, box.min);
    }

    if (!path.closed()) {
      Geometry::Segment closing_segment = { segments.back().p3(), segments.front().p0() };
      process_linear_segment(closing_segment, box.min);
    }

    finish();
  }

  void PathTiler::process_linear_segment(const Geometry::Segment& line, vec2 offset) {
    OPTICK_EVENT();

    vec2 p0 = line.p0() * m_zoom - offset;
    vec2 p3 = line.p3() * m_zoom - offset;

    ivec2 from_coords = tile_coords(p0);
    ivec2 to_coords = tile_coords(p3);

    if (is_almost_equal(p0, p3)) return;

    int16_t x_dir = (int16_t)sign(p3.x - p0.x);
    int16_t y_dir = (int16_t)sign(p3.y - p0.y);

    float dtdx = 1.0f / (p3.x - p0.x);
    float dtdy = 1.0f / (p3.y - p0.y);

    int16_t x = (int16_t)std::floor(p0.x);
    int16_t y = (int16_t)std::floor(p0.y);

    float row_t0 = 0.0f;
    float col_t0 = 0.0f;
    float row_t1 = std::numeric_limits<float>::infinity();
    float col_t1 = std::numeric_limits<float>::infinity();

    if (p0.y != p3.y) {
      float next_y = p3.y > p0.y ? (float)y + 1.0f : (float)y;
      row_t1 = std::min(1.0f, dtdy * (next_y - p0.y));
    }
    if (p0.x != p3.x) {
      float next_x = p3.x > p0.x ? (float)x + 1.0f : (float)x;
      col_t1 = std::min(1.0f, dtdx * (next_x - p0.x));
    }

    float x_step = std::abs(dtdx);
    float y_step = std::abs(dtdy);

    while (true) {
      float t0 = std::max(row_t0, col_t0);
      float t1 = std::min(row_t1, col_t1);

      vec2 from = lerp(p0, p3, t0);
      vec2 to = lerp(p0, p3, t1);

      float height = to.y - from.y;
      float right = (float)x + 1.0f;
      float area = 0.5f * height * ((right - from.x) + (right - to.x));

      m_increments.push_back({ x, y, area, height });

      if (row_t1 < col_t1) {
        row_t0 = row_t1;
        row_t1 = std::min(1.0f, row_t1 + y_step);
        y += y_dir;
      } else {
        col_t0 = col_t1;
        col_t1 = std::min(1.0f, col_t1 + x_step);
        x += x_dir;
      }

      if (row_t0 == 1.0f && col_t0 == 1.0f) {
        x = (int16_t)std::floor(p3.x);
        y = (int16_t)std::floor(p3.y);
      }

      if (row_t0 == 1.0f || col_t0 == 1.0f) break;
    }
  }

  void PathTiler::finish() {
    OPTICK_EVENT();

    std::vector<Bin> bins;
    Bin bin = { 0, 0, 0, 0 };

    if (!m_increments.empty()) {
      Increment& first = m_increments.front();
      bin.tile_x = first.x / TILE_SIZE;
      bin.tile_y = first.y / TILE_SIZE;
    }

    for (size_t i = 0; i < m_increments.size(); ++i) {
      Increment& increment = m_increments[i];

      int16_t tile_x = increment.x / TILE_SIZE;
      int16_t tile_y = increment.y / TILE_SIZE;

      if (tile_x != bin.tile_x || tile_y != bin.tile_y) {
        bins.push_back(bin);
        bin = { tile_x, tile_y, i, i };
      }

      bin.end++;
    }

    bins.push_back(bin);

    {
      OPTICK_EVENT("Sort Bins");
      std::sort(bins.begin(), bins.end(), compare_bins);
    }

    float areas[TILE_SIZE * TILE_SIZE] = { 0.0f };
    float heights[TILE_SIZE * TILE_SIZE] = { 0.0f };
    float prev[TILE_SIZE] = { 0.0f };
    float next[TILE_SIZE] = { 0.0f };

    size_t bins_len = bins.size();

    m_masks.clear();

    for (size_t i = 0; i < bins_len; i++) {
      const Bin& bin = bins[i];

      for (size_t j = bin.start; j < bin.end; ++j) {
        Increment& increment = m_increments[j];

        int x = (size_t)increment.x % TILE_SIZE;
        int y = (size_t)increment.y % TILE_SIZE;
        int index = x + y * TILE_SIZE;

        areas[index] += increment.area;
        heights[index] += increment.height;
      }

      if (i + 1 == bins_len || bins[i + 1].tile_x != bin.tile_x || bins[i + 1].tile_y != bin.tile_y) {
        uint8_t tile[TILE_SIZE * TILE_SIZE] = { 0 };

        {
          OPTICK_EVENT("Generate Mask");

          for (int y = 0; y < TILE_SIZE; y++) {
            float accum = prev[y];
            for (int x = 0; x < TILE_SIZE; x++) {
              int index = x + y * TILE_SIZE;
              tile[index] = (uint8_t)std::round(std::min(std::abs(accum + areas[index]) * 256.0f, 255.0f));
              accum += heights[index];
            }
            next[y] = accum;
          }
        }

        m_masks.push_back({ bin.tile_x, bin.tile_y, tile });

        {
          OPTICK_EVENT("Memset");

          memset(areas, 0, sizeof(areas));
          memset(heights, 0, sizeof(heights));

          if (i + 1 < bins_len && bins[i + 1].tile_y == bin.tile_y) {
            memcpy(prev, next, sizeof(prev));
          } else {
            memset(prev, 0, sizeof(prev));
          }

          memset(next, 0, sizeof(next));
        }

        {
          OPTICK_EVENT("Generate Spans");

          if (i + 1 < bins_len && bins[i + 1].tile_y == bin.tile_y && bins[i + 1].tile_x > bin.tile_x + 1) {
            int16_t width = bins[i + 1].tile_x - bin.tile_x - 1;
            TileMask& last_mask = m_masks.back();

            if (last_mask.tile_y == bin.tile_y) {
              int16_t winding = 0;

              for (int j = 1; j <= TILE_SIZE; j += 2) {
                uint8_t current = last_mask.data[TILE_SIZE * j - 1];
                if (current == 0) {
                  winding--;
                } else if (current == 255 || current >= last_mask.data[TILE_SIZE * j - 2]) {
                  winding++;
                } else {
                  winding--;
                }
              }

              if (winding >= 0) {
                m_spans.push_back(TileSpan{ (int16_t)(bin.tile_x + 1), bin.tile_y, width, vec4{0.7f, 0.5f, 0.5f, 1.0f} });
              }
            }
          }
        }
      }
    }
  }

  Tiler::Tiler() {}

  Tiler::~Tiler() {
    delete[] m_masks;
  }

  void Tiler::reset(const ivec2 size, const vec2 position, float zoom) {
    m_tiles_count = { (int)(std::ceil((float)size.x / TILE_SIZE)) + 2, (int)(std::ceil((float)size.y / TILE_SIZE)) + 2 };
    m_position = {
      (int)(position.x > 0 ? std::floor(position.x * zoom / TILE_SIZE) : std::ceil(position.x * zoom / TILE_SIZE)),
      (int)(position.y > 0 ? std::floor(position.y * zoom / TILE_SIZE) : std::ceil(position.y * zoom / TILE_SIZE))
    };
    m_zoom = zoom;
    m_visible = { -position, vec2{(float)size.x / zoom, (float)size.y / zoom} - position };

    m_tiles.clear();
    m_spans.clear();

    delete[] m_masks;

    m_masks = new uint8_t[SEGMENTS_TEXTURE_SIZE * SEGMENTS_TEXTURE_SIZE];
    m_masks_offset = 0;

    m_opaque_tiles = std::vector<bool>(m_tiles_count.x * m_tiles_count.y, false);
  }

  void Tiler::process_path(const Geometry::Path& path, const vec4& color) {
    OPTICK_EVENT();

    PathTiler tiler(path, color, m_visible, m_zoom, m_position);

    const std::vector<PathTiler::TileMask>& masks = tiler.masks();
    const std::vector<PathTiler::TileSpan>& spans = tiler.spans();
    const ivec2 offset = tiler.offset();

    for (auto& mask : masks) {
      ivec2 coords = { mask.tile_x + offset.x + 1, mask.tile_y + offset.y + 1 };
      if (coords.x < 0 || coords.y < 0 || coords.x >= m_tiles_count.x || coords.y >= m_tiles_count.y) continue;

      int index = tile_index(coords, m_tiles_count);
      if (m_opaque_tiles[index]) continue;

      m_tiles.push_back({ color, index, m_masks_offset });
      ivec2 mask_offset = { m_masks_offset % (SEGMENTS_TEXTURE_SIZE / TILE_SIZE) * TILE_SIZE, m_masks_offset / (SEGMENTS_TEXTURE_SIZE / TILE_SIZE) * TILE_SIZE };

      for (int y = 0; y < TILE_SIZE; ++y) {
        for (int x = 0; x < TILE_SIZE; ++x) {
          m_masks[(mask_offset.y + y) * SEGMENTS_TEXTURE_SIZE + mask_offset.x + x] = mask.data[y * TILE_SIZE + x];
        }
      }

      m_masks_offset++;
    }

    for (auto& span : spans) {
      ivec2 coords = { span.tile_x + offset.x + 1, span.tile_y + offset.y + 1 };
      if (coords.x + span.width < 0 || coords.y < 0 || coords.x >= m_tiles_count.x || coords.y >= m_tiles_count.y) continue;

      int16_t width = coords.x < 0 ? span.width + coords.x : span.width;
      coords.x = std::max(coords.x, 0);

      for (int i = 0; i < width; i++) {
        if (coords.x + i >= m_tiles_count.x) break;

        int index = tile_index({ coords.x + i, coords.y }, m_tiles_count);
        if (!m_opaque_tiles[index] && color.a == 1.0f) {
          m_spans.push_back({ color, index, 1 });
          m_opaque_tiles[index] = true;
        }
      }
    }
  }

}
