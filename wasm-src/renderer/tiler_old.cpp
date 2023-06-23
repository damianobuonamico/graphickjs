#include "tiler.h"

#include "../utils/console.h"
#include "../math/math.h"

#include <algorithm>

#define CLAMP_ROUND_COORD(x) (uint8_t)std::round(std::clamp((x) * 255.0f, 0.0f, 255.0f))

namespace Graphick::Render {

  static inline ivec2 tile_coords(const vec2 p) {
    return { (int)std::floor(p.x / TILE_SIZE), (int)std::floor(p.y / TILE_SIZE) };
  }

  static inline ivec2 tile_coords_clamp(const vec2 p, const ivec2 bounds_size) {
    return { std::clamp((int)std::floor(p.x / TILE_SIZE), 0, bounds_size.x - 1), std::clamp((int)std::floor(p.y / TILE_SIZE), 0, bounds_size.y - 1) };
  }

  static inline int tile_index(const ivec2 coords, const ivec2 tiles_count) {
    return coords.x + coords.y * tiles_count.x;
  }

  PathTiler::PathTiler(const Geometry::Path& path, const vec4& color, const Box& visible, float zoom, ivec2 position) :
    m_zoom(zoom),
    m_position(position)
  {
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
    m_tiles = std::vector<TileData>(m_bounds_size.x * m_bounds_size.y);

    for (const auto& segment : path.segments()) {
      // TODO: Check segment's kind and handle other kind of segments
      process_linear_segment(segment, box.min);
    }

    finish();
  }

  enum class StepDirection {
    None,
    X,
    Y
  };

#define LARGE_TILE_SIZE TILE_SIZE + 2 * TILE_OVERLAP

  void PathTiler::process_linear_segment(const Geometry::Segment& line, vec2 offset) {
    vec2 p0 = line.p0() * m_zoom - offset;
    vec2 p3 = line.p3() * m_zoom - offset;

    ivec2 from_coords = tile_coords_clamp(p0, m_bounds_size);
    ivec2 to_coords = tile_coords_clamp(p3, m_bounds_size);

#if 1
    if (is_almost_equal(p0, p3)) return;

    int16_t x_dir = (int16_t)sign(p3.x - p0.x);
    int16_t y_dir = (int16_t)sign(p3.y - p0.y);

    float dtdx = 1.0 / (p3.x - p0.x);
    float dtdy = 1.0 / (p3.y - p0.y);

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

      int16_t tile_y = y / TILE_SIZE;
      if (tile_y != m_tile_y_prev) {
        m_tile_increments.push_back({ x / TILE_SIZE, std::min(tile_y, m_tile_y_prev), (int8_t)(tile_y - m_tile_y_prev) });
        m_tile_y_prev = tile_y;
      }

      if (row_t0 == 1.0f || col_t0 == 1.0f) break;
    }

#elif 0
    int dx = std::abs(to_coords.x - from_coords.x);
    int dy = std::abs(to_coords.y - from_coords.y);

    int p = 2 * dy - dx;

    int two_dy = 2 * dy;
    int two_dy_dx = 2 * (dy - dx);

    int x, y, x_end;

    if (from_coords.x > to_coords.x) {
      x = to_coords.x;
      y = to_coords.y;
      x_end = from_coords.x;
    } else {
      x = from_coords.x;
      y = from_coords.y;
      x_end = to_coords.x;
    }

    while (x < x_end) {
      x++;

      if (p < 0) {
        p += two_dy;
        // int index = tile_index({ x, y }, m_bounds_size);
        // m_tiles[index].segments.push_back({ 0, 0, 0, 255 });
        // p += 2 * dy - 2 * dx;
        // y++;
      } else {
        y++;
        // int index = tile_index({ x, y }, m_bounds_size);
        // m_tiles[index].segments.push_back({ 0, 0, 0, 255 });
        // p += 2 * dy;
        p += two_dy_dx;
      }

      if (y < m_bounds_size.y) {
        int index = tile_index({ x, y }, m_bounds_size);
        m_tiles[index].segments.push_back({ 0, 0, 0, 255 });
      }
    }
#elif 0
    bool steep = std::abs(to_coords.y - from_coords.y) > std::abs(to_coords.x - from_coords.x);

    // Swap the coordinates if slope > 1
    if (steep) {
      std::swap(from_coords.x, from_coords.y);
      std::swap(to_coords.x, to_coords.y);
    }
    if (from_coords.x > to_coords.x) {
      std::swap(from_coords.x, to_coords.x);
      std::swap(from_coords.y, to_coords.y);
    }

    // Compute the slope
    float dx = to_coords.x - from_coords.x;
    float dy = to_coords.y - from_coords.y;
    float gradient = dy / dx;

    if (dx == 0.0f) {
      gradient = 1.0f;
    }

    int xpxl1 = from_coords.x;
    int xpxl2 = to_coords.x;
    float intersect_y = from_coords.y;

    // Main loop
    if (steep) {
      for (int x = xpxl1; x <= xpxl2; ++x) {
        int index = tile_index({ (int)intersect_y, x }, m_bounds_size);
        m_tiles[index].segments.push_back({ 255, 0, 0, 255 });

        if (intersect_y >= 1.0f) {
          index = tile_index({ (int)intersect_y - 1, x }, m_bounds_size);
          m_tiles[index].segments.push_back({ 0, 0, 255, 255 });
        }

        intersect_y += gradient;
      }
    } else {
      for (int x = xpxl1; x <= xpxl2; ++x) {
        int index = tile_index({ x, (int)intersect_y }, m_bounds_size);
        m_tiles[index].segments.push_back({ 0, 0, 0, 255 });

        if (intersect_y >= 1.0f) {
          index = tile_index({ x, (int)intersect_y - 1 }, m_bounds_size);
          m_tiles[index].segments.push_back({ 0, 0, 255, 255 });
        }

        intersect_y += gradient;
      }
    }

#else
    // if (is_almost_equal(p3.y, (float)(to_coords.y * TILE_SIZE))) {
    //   to_coords.y--;
    // }

    vec2 vector = p3 - p0;
    vec2 step = { vector.x < 0 ? -1.0f : 1.0f, vector.y < 0 ? -1.0f : 1.0f };

    ivec2 vector_is_negative = { vector.x < 0 ? -1 : 0, vector.y < 0 ? -1 : 0 };
    vec2 first_crossing = TILE_SIZE * (vec2{ (float)from_coords.x, (float)from_coords.y } + vec2{ vector.x >= 0 ? 1.0f : 0.0f, vector.y >= 0 ? 1.0f : 0.0f });

    vec2 t_max = (first_crossing - p0) / vector;
    vec2 t_delta = abs(TILE_SIZE / vector);

    vec2 current_position = p0;
    ivec2 coords = from_coords;
    StepDirection last_step_direction = StepDirection::None;

    while (true) {
      StepDirection next_step_direction;

      if (t_max.x < t_max.y) {
        next_step_direction = StepDirection::X;
      } else if (t_max.x > t_max.y) {
        next_step_direction = StepDirection::Y;
      } else {
        // Line's destinetion is exactly on the tile's corner
        next_step_direction = step.y > 0 ? StepDirection::Y : StepDirection::X;

        // int index = tile_index({ coords.x + 1, coords.y }, m_bounds_size);
        // m_tiles[index].segments.push_back({ 0, 0, 0, 255 });
      }

      float next_t = std::min(1.0f, next_step_direction == StepDirection::X ? t_max.x : t_max.y);

      // Reached the end tile
      if (coords == to_coords) {
        next_step_direction = StepDirection::None;
      }

      vec2 next_position = p0 + next_t * vector;
      Box clipped_line_segment = { current_position, next_position };

      add_line(clipped_line_segment, coords);

      if (step.x < 0 && next_step_direction == StepDirection::X) {
        // Leaves through left boundary.
        int index = tile_index(coords, m_bounds_size);
        m_tiles[index].segments.push_back({ 0, 0, 0, CLAMP_ROUND_COORD(clipped_line_segment.max.y / TILE_SIZE - coords.y) });
      } else if (step.x > 0 && last_step_direction == StepDirection::X) {
        // Enters through left boundary.
        int index = tile_index(coords, m_bounds_size);
        m_tiles[index].segments.push_back({ 0, 0, 0, CLAMP_ROUND_COORD(clipped_line_segment.min.y / TILE_SIZE - coords.y) });
      }

      if (step.y > 0 && next_step_direction == StepDirection::Y) {
        // Leaves through bottom boundary.
        int index = tile_index(coords, m_bounds_size);
        m_tiles[index].bottom_intersections++;
      } else if (step.y < 0 && last_step_direction == StepDirection::Y) {
        // Enters through bottom boundary.
        int index = tile_index(coords, m_bounds_size);
        m_tiles[index].bottom_intersections++;
      }

      if (next_step_direction == StepDirection::None) {
        break;
      } else if (next_step_direction == StepDirection::X) {
        if (coords.x == to_coords.x) break;

        t_max.x += t_delta.x;
        coords.x += step.x;
      } else {
        if (coords.y == to_coords.y) break;

        t_max.y += t_delta.y;
        coords.y += step.y;
      }

      current_position = next_position;
      last_step_direction = next_step_direction;
    }
#endif
  }

  struct Bin {
    int16_t tile_x;
    int16_t tile_y;
    size_t start;
    size_t end;
  };

  bool compare_bins(const Bin& a, const Bin& b) {
    return a.tile_y < b.tile_y || (a.tile_y == b.tile_y && a.tile_x < b.tile_x);
    // return (a.tile_y < b.tile_y && a.tile_x <= b.tile_x) || (a.tile_x < b.tile_x && a.tile_y <= b.tile_y);
  }

  bool compare_tile_increments(const PathTiler::TileIncrement& a, const PathTiler::TileIncrement& b) {
    return a.tile_y < b.tile_y || (a.tile_y == b.tile_y && a.tile_x < b.tile_x);
    // return (a.tile_y < b.tile_y && a.tile_x <= b.tile_x) || (a.tile_x < b.tile_x && a.tile_y <= b.tile_y);
  }

  void PathTiler::finish() {
    // Close path if needed

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

    std::sort(bins.begin(), bins.end(), compare_bins);
    std::sort(m_tile_increments.begin(), m_tile_increments.end(), compare_tile_increments);

    float areas[TILE_SIZE * TILE_SIZE] = { 0.0f };
    float heights[TILE_SIZE * TILE_SIZE] = { 0.0f };
    float prev[TILE_SIZE] = { 0.0f };
    float next[TILE_SIZE] = { 0.0f };

    int tile_increments_i = 0;
    int winding = 0;

    size_t bins_len = bins.size();
    size_t tile_increments_len = m_tile_increments.size();

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

        for (int y = 0; y < TILE_SIZE; y++) {
          float accum = prev[y];
          for (int x = 0; x < TILE_SIZE; x++) {
            int index = x + y * TILE_SIZE;
            tile[index] = (uint8_t)std::round(std::min(std::abs(accum + areas[index]) * 256.0f, 255.0f));
            accum += heights[index];
          }
          next[y] = accum;
        }

        // tile
        // int index = tile_index({ bin.tile_x, bin.tile_y }, m_bounds_size);
        // if (index >= 0 && index < m_tiles.size()) {
        //   m_tiles[index].segments.push_back({ 0, 0, 0, 255 });
        // }

        m_masks.push_back({ bin.tile_x, bin.tile_y, tile });

        memset(areas, 0, sizeof(areas));
        memset(heights, 0, sizeof(heights));

        if (i + 1 < bins_len && bins[i + 1].tile_y == bin.tile_y) {
          memcpy(prev, next, sizeof(prev));
        } else {
          memset(prev, 0, sizeof(prev));
        }

        memset(next, 0, sizeof(next));

        if (i + 1 < bins_len && bins[i + 1].tile_y == bin.tile_y && bins[i + 1].tile_x > bin.tile_x + 1) {
          while (tile_increments_i < tile_increments_len) {
            TileIncrement& tile_increment = m_tile_increments[tile_increments_i];
            if (tile_increment.tile_y > bin.tile_y || (tile_increment.tile_y == bin.tile_y && tile_increment.tile_x > bin.tile_x)) break;
            // if (tile_increment.tile_x > bin.tile_x || (tile_increment.tile_x == bin.tile_x && tile_increment.tile_y > bin.tile_y)) break;
            // if (tile_increment.tile_y > bin.tile_y && tile_increment.tile_x > bin.tile_x) break;
            // if ((tile_increment.tile_y > bin.tile_y && tile_increment.tile_x >= bin.tile_x) || (tile_increment.tile_x > bin.tile_x && tile_increment.tile_y >= bin.tile_y)) break;

            winding += (int)tile_increment.sign;
            tile_increments_i++;
          }

          if (winding != 0) {
            int16_t width = bins[i + 1].tile_x - bin.tile_x - 1;
            m_spans.push_back({ bin.tile_x + 1, bin.tile_y, width });
            // for (int k = 0; k < width; ++k) {
            //   int index = tile_index({ bin.tile_x + k, bin.tile_y }, m_bounds_size);
            //   if (index >= 0 && index < m_tiles.size()) {
            //     m_tiles[index].segments.push_back({ 0, 0, 0, 255 });
            //   }
            // }
            // span
          }
        }
      }
    }

    // for (auto& bin : bins) {
    //   int index = tile_index({ bin.tile_x, bin.tile_y }, m_bounds_size);
    //   if (index >= 0 && index < m_tiles.size()) {
    //     m_tiles[index].segments.push_back({ 0, 0, 0, 255 });
    //   }
    // }

  }

  void PathTiler::add_line(const Box& line, const ivec2 coords) {
    if (coords.x < 0 || coords.x >= m_bounds_size.x || coords.y < 0 || coords.y >= m_bounds_size.y) return;

    vec2 float_coords = { (float)coords.x, (float)coords.y };
    Box normalized_line = { line.min / TILE_SIZE - float_coords, line.max / TILE_SIZE - float_coords };

    Line clipped_line = {
      CLAMP_ROUND_COORD(normalized_line.min.x),
      CLAMP_ROUND_COORD(normalized_line.min.y),
      CLAMP_ROUND_COORD(normalized_line.max.x),
      CLAMP_ROUND_COORD(normalized_line.max.y),
    };

    if (clipped_line.y1 == clipped_line.y2) return;

    int index = tile_index(coords, m_bounds_size);
    m_tiles[index].segments.push_back(clipped_line);
  }

  Tiler::Tiler() : m_segments(SEGMENTS_TEXTURE_SIZE* SEGMENTS_TEXTURE_SIZE) {}

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
    m_segments = std::vector<uint8_t>(SEGMENTS_TEXTURE_SIZE * SEGMENTS_TEXTURE_SIZE, 0);

    m_segments[1] = 1;
    m_segments[2] = 0;
    m_segments[3] = 0;
    m_segments[4] = 255;
    m_segments[5] = 255;

    m_segments_offset = 6;

    delete[] m_masks;
    m_masks = new uint8_t[TILE_SIZE * TILE_SIZE * SEGMENTS_TEXTURE_SIZE * SEGMENTS_TEXTURE_SIZE];
    m_masks_offset = 1;

    for (int y = 0; y < TILE_SIZE; ++y) {
      for (int x = 0; x < TILE_SIZE; ++x) {
        m_masks[y * SEGMENTS_TEXTURE_SIZE + x] = 255;
      }
    }
  }

  void Tiler::process_path(const Geometry::Path& path, const vec4& color) {
    PathTiler tiler(path, color, m_visible, m_zoom, m_position);

    const std::vector<PathTiler::TileMask>& masks = tiler.masks();
    const std::vector<PathTiler::Span>& spans = tiler.spans();
    const ivec2 offset = tiler.offset();

    for (auto& mask : masks) {
      ivec2 coords = { mask.tile_x + offset.x + 1, mask.tile_y + offset.y + 1 };
      if (coords.x < 0 || coords.y < 0 || coords.x >= m_tiles_count.x || coords.y >= m_tiles_count.y) continue;

      m_tiles.push_back({ color, tile_index(coords, m_tiles_count), m_masks_offset });
      ivec2 mask_offset = { m_masks_offset % (SEGMENTS_TEXTURE_SIZE / TILE_SIZE) * TILE_SIZE, m_masks_offset / (SEGMENTS_TEXTURE_SIZE / TILE_SIZE) * TILE_SIZE };

      for (int y = 0; y < TILE_SIZE; ++y) {
        for (int x = 0; x < TILE_SIZE; ++x) {
          m_masks[(mask_offset.y + y) * SEGMENTS_TEXTURE_SIZE + mask_offset.x + x] = mask.data[y * TILE_SIZE + x];
        }
      }

      m_masks_offset++;
    }

    for (auto& span : spans) {
      for (int i = 0; i < span.width; i++) {
        ivec2 coords = { span.tile_x + offset.x + 1 + i, span.tile_y + offset.y + 1 };
        if (coords.x < 0 || coords.y < 0 || coords.x >= m_tiles_count.x || coords.y >= m_tiles_count.y) continue;

        m_tiles.push_back({ color, tile_index(coords, m_tiles_count), 0 });
      }
    }
#if 0
    std::vector<PathTiler::TileData> tiles = tiler.tiles();
    ivec2 size = tiler.size();
    ivec2 offset = tiler.offset();

    for (int y = 0; y < size.y; ++y) {
      int intersections = 0;

      for (int x = size.x - 1; x >= 0; x--) {
        PathTiler::TileData& tile = tiles[y * size.x + x];
        intersections += tile.bottom_intersections;

        if (intersections % 2 != 0) {
          tile.segments.push_back({ 0, 0, 0, 255 });
        }
      }
    }

    for (int y = 0; y < size.y; ++y) {
      for (int x = 0; x < size.x; ++x) {
        ivec2 coords = { x + offset.x + 1, y + offset.y + 1 };
        if (coords.x < 0 || coords.y < 0 || coords.x >= m_tiles_count.x || coords.y >= m_tiles_count.y) continue;

        int index = tile_index(coords, m_tiles_count);
        int i = y * size.x + x;

        PathTiler::TileData& tile = tiles[i];
        if (tile.segments.empty()) {
          m_tiles.push_back({ color, index, 0 });
          continue;
        }

        // TODO: Handle more than 255 segments per tile
        m_segments[m_segments_offset] = (uint8_t)tile.segments.size();

        for (int j = 0; j < tile.segments.size(); ++j) {
          m_segments[m_segments_offset + 1 + j * 4 + 0] = tile.segments[j].x1;
          m_segments[m_segments_offset + 1 + j * 4 + 1] = tile.segments[j].y1;
          m_segments[m_segments_offset + 1 + j * 4 + 2] = tile.segments[j].x2;
          m_segments[m_segments_offset + 1 + j * 4 + 3] = tile.segments[j].y2;
        }

        m_tiles.push_back({ color, index, m_segments_offset });
        // m_tiles.push_back({ tile.bottom_intersections > 0 ? vec4{ 0.8f, 0.2f, 0.2f, 1.0f } : color, index, m_segments_offset });
        m_segments_offset += 1 + (int)tile.segments.size() * 4;
      }
    }
#endif
  }

}
