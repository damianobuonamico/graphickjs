#if 0

#include "tiler.h"

#include "../utils/console.h"
#include "../math/math.h"

namespace Graphick::Render {

  Tiler::Tiler() {}

  void Tiler::reset(const ivec2 size, const vec2 position, float zoom) {
    m_tiles_count = { (int)(std::ceil((float)size.x / TILE_SIZE)) + 2, (int)(std::ceil((float)size.y / TILE_SIZE)) + 2 };
    m_processed_tiles = std::vector<bool>(m_tiles_count.x * m_tiles_count.y, false);
    m_opaque_tiles.clear();
    m_masks.clear();
    m_tiles.clear();
    m_mask_id = 0;

    m_position = {
      (int)(position.x > 0 ? std::floor(position.x * zoom / TILE_SIZE) : std::ceil(position.x * zoom / TILE_SIZE)),
      (int)(position.y > 0 ? std::floor(position.y * zoom / TILE_SIZE) : std::ceil(position.y * zoom / TILE_SIZE))
    };
    m_zoom = zoom;

    m_visible = { -position, vec2{(float)size.x / zoom, (float)size.y / zoom} - position };
  }

  struct TileIntersection {
    int segment_index;
    vec2 position;
  };

  ivec2 tile_coords(const vec2 p) {
    return { (int)std::floor(p.x / TILE_SIZE), (int)std::floor(p.y / TILE_SIZE) };
  }

  ivec2 tile_coords_clamp(const vec2 p, const ivec2 tiles_count) {
    return { std::clamp((int)std::floor(p.x / TILE_SIZE), 0, tiles_count.x - 1), std::clamp((int)std::floor(p.y / TILE_SIZE), 0, tiles_count.y - 1) };
  }

  int tile_index(const ivec2 coords, const ivec2 tiles_count) {
    return coords.x + coords.y * tiles_count.x;
  }

  struct Range {
    uint8_t min;
    uint8_t max;
  };

  using RangeLayer = std::vector<Range>;

  void Tiler::process_path(const Geometry::Path& path, const vec4& color) {
    Box box = path.bounding_box();
    // TODO: maybe can optimize this check
    if (!does_box_intersect_box(box, m_visible)) return;

    float zoom_factor = m_zoom / TILE_SIZE;

    box.min = floor(box.min * zoom_factor) * TILE_SIZE;
    box.max = ceil(box.max * zoom_factor) * TILE_SIZE;

    ivec2 min_coords = tile_coords(box.min) + m_position;
    ivec2 max_coords = tile_coords(box.max) + m_position;
    ivec2 bounds_size = max_coords - min_coords;

    PathTiler path_tiler(bounds_size, min_coords);

    for (int i = 0; i < path.segments().size(); i++) {
      // TODO: Check segment's kind and handle other kind of segments
      process_linear_segment(path.segments()[i], box.min, bounds_size, i, path_tiler);
    }

    auto& tiles = path_tiler.tiles();

    // TODO: Move into path tiler
    for (int y = 0; y < bounds_size.y; y++) {
      int backdrop = 0;
      int intersections = 0;

      for (int x = bounds_size.x - 1; x >= 0; x--) {
        int i = y * bounds_size.x + x;

        if (backdrop % 2 != 0 && !tiles[i].has_mask) {
          ivec2 coords = ivec2{ i % bounds_size.x + 1, i / bounds_size.x + 1 } + min_coords;
          if (coords.x < 0 || coords.y < 0 || coords.x >= m_tiles_count.x || coords.y >= m_tiles_count.y) continue;
          int index = tile_index(coords, m_tiles_count);

          m_opaque_tiles.push_back({ color, index });
        }

        intersections += tiles[i].bottom_intersections;
        backdrop += tiles[i].backdrop;

        if (intersections % 2 != 0) {
          tiles[i].intersections.push_back((uint8_t)255);
        }

        if (tiles[i].has_mask && !tiles[i].intersections.empty()) {
          std::vector<bool> coverage(256, false);

          for (auto y : tiles[i].intersections) {
            for (int j = 0; j <= y; j++) {
              coverage[j] = coverage[j] != true;
            }
          }

          bool last = false;
          for (int j = 0; j < coverage.size(); j++) {
            if (!coverage[j]) continue;

            int min_y = j;
            while (j < coverage.size() && coverage[j] == true) j++;
            path_tiler.add_vertical_fill(min_y, j - 1, { x, y });
          }
        }
      }
    }

    int next_mask_index = m_masks.size();
    int next_index = m_tiles.size();

    for (auto& mask : path_tiler.masks()) {
      m_masks.push_back({ mask.line_segment, mask.index + next_index });
    }

    for (auto& fill : path_tiler.fills()) {
      ivec2 coords = ivec2{ fill.index % bounds_size.x + 1, fill.index / bounds_size.x + 1 } + min_coords;
      if (coords.x < 0 || coords.y < 0 || coords.x >= m_tiles_count.x || coords.y >= m_tiles_count.y) continue;
      int index = tile_index(coords, m_tiles_count);
      m_tiles.push_back({ color, index, fill.mask_index + next_index });
    }
  }

  enum class StepDirection {
    None,
    X,
    Y
  };

  void Tiler::process_linear_segment(const Geometry::Segment& segment, vec2 position, ivec2 bounds_size, int segment_index, PathTiler& path_tiler) {
    vec2 p0 = segment.p0() * m_zoom - position;
    vec2 p3 = segment.p3() * m_zoom - position;

    ivec2 from_coords = tile_coords_clamp(p0, bounds_size);
    ivec2 to_coords = tile_coords_clamp(p3, bounds_size);

    if (is_almost_equal(p3.y, (float)(to_coords.y * TILE_SIZE))) {
      to_coords.y--;
    }

    vec2 vector = p3 - p0;
    vec2 step = { vector.x < 0 ? -1.0f : 1.0f, vector.y < 0 ? -1.0f : 1.0f };

    ivec2 vector_is_negative = { vector.x < 0 ? -1 : 0, vector.y < 0 ? -1 : 0 };
    vec2 first_crossing = TILE_SIZE * (vec2{ (float)from_coords.x, (float)from_coords.y } + vec2{ vector.x >= 0 ? 1.0f : 0.0f, vector.y >= 0 ? 1.0f : 0.0f });

    vec2 t_max = (first_crossing - p0) / vector;
    vec2 t_delta = abs(TILE_SIZE / vector);

    vec2 current_position = p0;
    ivec2 coords = from_coords;
    StepDirection last_step_direction = StepDirection::None;

    if (is_almost_equal(p0.y, (float)(from_coords.y * TILE_SIZE)) && p0.y >= (float)(from_coords.y * TILE_SIZE)) {
      last_step_direction = StepDirection::Y;
    }

    while (true) {
      StepDirection next_step_direction;

      if (t_max.x < t_max.y) {
        next_step_direction = StepDirection::X;
      } else if (t_max.x > t_max.y) {
        next_step_direction = StepDirection::Y;
      } else {
        // Line's destinetion is exactly on the tile's corner
        next_step_direction = step.y > 0 ? StepDirection::Y : StepDirection::X;
      }

      float next_t = std::min(1.0f, next_step_direction == StepDirection::X ? t_max.x : t_max.y);

      // Reached the end tile
      if (coords == to_coords) {
        next_step_direction = StepDirection::None;
      }

      vec2 next_position = p0 + next_t * vector;
      Box clipped_line_segment = { current_position, next_position };
      path_tiler.add_fill(clipped_line_segment, coords);

      if (step.x < 0 && next_step_direction == StepDirection::X) {
        // Leaves through left boundary.
        path_tiler.intersection(std::ceil(clipped_line_segment.max.y), coords);
      } else if (step.x > 0 && last_step_direction == StepDirection::X) {
        // Enters through left boundary.
        path_tiler.intersection(std::ceil(clipped_line_segment.min.y), coords);
      }
      if (step.y > 0 && next_step_direction == StepDirection::Y) {
        // Leaves through bottom boundary.
        path_tiler.bottom_intersection(coords);
      } else if (step.y < 0 && last_step_direction == StepDirection::Y) {
        // Enters through bottom boundary.
        path_tiler.bottom_intersection(coords);
      }

      // Adjust backdrop if necessary.
      if (step.y < 0 && next_step_direction == StepDirection::Y) {
        // Leaving through bottom boundary.
        path_tiler.adjust_backdrop(coords, 1);
      } else if (step.y > 0 && last_step_direction == StepDirection::Y) {
        // Entering through bottom boundary.
        path_tiler.adjust_backdrop(coords, 1);
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
  }

  Tiler::PathTiler::PathTiler(ivec2 path_bounds_size, ivec2 path_bounds_offset) :
    m_path_bounds_size(path_bounds_size),
    m_path_bounds_offset(path_bounds_offset),
    m_tiles(path_bounds_size.x* path_bounds_size.y),
    m_mask_tiles(path_bounds_size.x* path_bounds_size.y, -1) {}

  void Tiler::PathTiler::add_fill(const Box& segment, const ivec2 coords) {
    if (coords.x < 0 || coords.x >= m_path_bounds_size.x || coords.y < 0 || coords.y >= m_path_bounds_size.y) return;

    vec2 tile_upper_left = vec2{ (float)coords.x, (float)coords.y } *(float)TILE_SIZE;

    Box line = { (segment.min - tile_upper_left) / TILE_SIZE, (segment.max - tile_upper_left) / TILE_SIZE };
    float min = 0.0f, max = 255.0f;

    Line clipped_line = {
      (uint8_t)std::round(std::clamp(line.min.x * 255.0f, min, max)),
      (uint8_t)std::round(std::clamp(line.min.y * 255.0f, min, max)),
      (uint8_t)std::round(std::clamp(line.max.x * 255.0f, min, max)),
      (uint8_t)std::round(std::clamp(line.max.y * 255.0f, min, max)),
    };

    if (clipped_line.y1 == clipped_line.y2) return;

    int index = get_mask_tile_index(coords);
    if (index < 0) return;

    m_masks.push_back({ clipped_line, index });
  }

  void Tiler::PathTiler::add_vertical_fill(uint8_t min, uint8_t max, ivec2 coords) {
    if (coords.x < 0 || coords.x >= m_path_bounds_size.x || coords.y < 0 || coords.y >= m_path_bounds_size.y) return;

    vec2 tile_upper_left = vec2{ (float)coords.x, (float)coords.y } *(float)TILE_SIZE;

    Line clipped_line = {
      0,
      min,
      0,
      max,
    };

    if (clipped_line.y1 == clipped_line.y2) return;

    int index = get_mask_tile_index(coords);
    if (index < 0) return;

    m_masks.push_back({ clipped_line, index });
  }

  int Tiler::PathTiler::get_mask_tile_index(ivec2 coords) {
    int index = tile_index(coords, m_path_bounds_size);
    if (index < 0 || index >= m_mask_tiles.size()) return -1;

    int mask_index = m_mask_tiles[index];

    if (mask_index < 0) {
      mask_index = m_fills.size();
      m_fills.push_back({ index, mask_index });
      m_tiles[index].has_mask = true;
      m_mask_tiles[index] = mask_index;
    }

    return mask_index;
  }

  void Tiler::PathTiler::intersection(float y, ivec2 coords) {
    if (coords.x < 0 || coords.x >= m_path_bounds_size.x || coords.y < 0 || coords.y >= m_path_bounds_size.y) return;

    uint8_t y_intersection = (uint8_t)std::clamp(std::round((y / TILE_SIZE - coords.y) * 255.0f), 0.0f, 255.0f);
    if (y_intersection == 0) return;

    int index = tile_index(coords, m_path_bounds_size);

    m_tiles[index].intersections.push_back(y_intersection);
  }

  void Tiler::PathTiler::bottom_intersection(ivec2 coords) {
    if (coords.x < 0 || coords.x >= m_path_bounds_size.x || coords.y < 0 || coords.y >= m_path_bounds_size.y) return;
    int index = tile_index(coords, m_path_bounds_size);
    m_tiles[index].bottom_intersections++;
  }

  void Tiler::PathTiler::adjust_backdrop(const ivec2 coords, int8_t delta) {
    if (coords.x < 0 || coords.x >= m_path_bounds_size.x || coords.y >= m_path_bounds_size.y) return;

    if (coords.y < 0) {
      m_tiles[coords.x].backdrop += (int)delta;
      return;
    }

    int index = tile_index(coords, m_path_bounds_size);

    m_tiles[index].backdrop += (int)delta;
  }

}

#endif