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

#if 1

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

    // std::vector<TileData> tiles(bounds_size.x * bounds_size.y);

    PathTiler path_tiler(bounds_size, min_coords);

    for (int i = 0; i < path.segments().size(); i++) {
      // TODO: Check segment's kind and handle other kind of segments
      process_linear_segment(path.segments()[i], box.min, bounds_size, i, path_tiler);
    }

    auto& tiles = path_tiler.tiles();

    // for (int i = 0; i < tiles.size(); i++) {
    //   ivec2 coords = ivec2{ i % bounds_size.x + 1, i / bounds_size.x + 1 } + min_coords;
    //   if (coords.x < 0 || coords.y < 0 || coords.x >= m_tiles_count.x || coords.y >= m_tiles_count.y) continue;

    //   int index = tile_index(coords, m_tiles_count);

    //   // if (tiles[i].color != vec4{ 0.0f, 0.0f, 0.0f, 0.0f }) {
    //   if (tiles[i].backdrop > 0) {
    //     // m_opaque_tiles.push_back({ vec4{ 0.3f * tiles[i].backdrop, 0.2f, 0.2f, 1.0f }, index });
    //     // }
    //     // else if (tiles[i].backdrop == 1) {
    //     //   m_opaque_tiles.push_back({ vec4{ 0.2f, 0.7f, 0.2f, 1.0f }, index });
    //     // } else {
    //     //   // m_opaque_tiles.push_back({ tiles[i].color, index });
    //     // }
    //   } else if (tiles[i].color == vec4{ 0.2f, 0.2f, 0.7f, 1.0f}) {
    //     // m_opaque_tiles.push_back({ tiles[i].color, index });
    //   }
    //   // else if (tiles[i].backdrop == 0) {
    //   //   m_opaque_tiles.push_back({ tiles[i].color, index });
    //   // }
    // }

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
          // RangeLayer result = { { { 0, tiles[i].intersections[0] } } };

          // for (int j = 1; j < tiles[i].intersections.size(); i++) {
          //   // uint8_t y = tiles[i].intersections[j];
          //   Range temp = { 0, tiles[i].intersections[j] };

          //   for (auto range : result) {
          //     if (temp.max > range.min) {
          //       if (temp.min < range.min) {
          //         range.min = min()
          //       }
          //     } else {
          //       // add directly
          //     }
          //   }
          // }
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

    // for (int x = 0; x < bounds_size.x; x++) {
    //   int backdrop = 0;
    //   for (int y = bounds_size.y - 1; y >= 0; y--) {
    //     // for (int y = 0; y < bounds_size.y; y++) {
    //     int i = y * bounds_size.x + x;
    //     int tile_backdrop = tiles[i].backdrop;

    //     if (backdrop == -1 && tile_backdrop == 0 && tiles[i].color == vec4{ 0.0f, 0.0f, 0.0f, 0.0f }) {
    //       ivec2 coords = ivec2{ i % bounds_size.x + 1, i / bounds_size.x + 1 } + min_coords;
    //       if (coords.x < 0 || coords.y < 0 || coords.x >= m_tiles_count.x || coords.y >= m_tiles_count.y) continue;
    //       int index = tile_index(coords, m_tiles_count);

    //       m_opaque_tiles.push_back({ color, index });
    //     }

    //     if (backdrop * tile_backdrop <= 0) {
    //       backdrop += tile_backdrop;
    //     }
    //   }
    // }
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

    // bool is_horizontal = from_coords.y == to_coords.y;

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

    // TODO: Check if it starts at the top boundary
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
        // if (step.x < 0 && next_step_direction == StepDirection::X) {
          // Leaves through left boundary.
          // Box auxiliary_segment = { clipped_line_segment.max, vec2{ (float)(coords.x * TILE_SIZE), (float)(coords.y * TILE_SIZE) } };

        // Box auxiliary_segment = { clipped_line_segment.max, vec2{ clipped_line_segment.max.x, (float)((coords.y + 1) * TILE_SIZE) } };
        // path_tiler.add_fill(auxiliary_segment, coords);

        // Box auxiliary_segment = { vec2{ clipped_line_segment.max.x, (float)(coords.y * TILE_SIZE) }, clipped_line_segment.max };
        path_tiler.intersection(std::ceil(clipped_line_segment.max.y), coords);
        // path_tiler.add_fill(auxiliary_segment, coords);
      } else if (step.x > 0 && last_step_direction == StepDirection::X) {
        // Enters through left boundary.
        // console::log("enters through left boundary");
        // Box auxiliary_segment = { vec2{ (float)(coords.x * TILE_SIZE), (float)(coords.y * TILE_SIZE) }, clipped_line_segment.min };
        // Box auxiliary_segment = { vec2{ (float)(coords.x * TILE_SIZE), (float)(coords.y * TILE_SIZE + TILE_SIZE) }, vec2{ (float)(coords.x * TILE_SIZE + TILE_SIZE), (float)(coords.y * TILE_SIZE) } };
        // Box auxiliary_segment = { vec2{ clipped_line_segment.min.x, (float)(coords.y * TILE_SIZE) }, clipped_line_segment.min };

        // Box auxiliary_segment = { vec2{ clipped_line_segment.min.x, std::max(clipped_line_segment.min.y, clipped_line_segment.max.y) }, { clipped_line_segment.min.x, (float)(coords.y * TILE_SIZE) } };
        // path_tiler.add_fill(auxiliary_segment, coords);

        // Box auxiliary_segment = { vec2{ clipped_line_segment.min.x, (float)(coords.y * TILE_SIZE) }, clipped_line_segment.min };
        // path_tiler.add_fill(auxiliary_segment, coords);
        path_tiler.intersection(std::ceil(clipped_line_segment.min.y), coords);
      }
      if (step.y > 0 && next_step_direction == StepDirection::Y) {
        // Leaves through bottom boundary.
        // Box auxiliary_segment = { clipped_line_segment.max, vec2{ (float)(coords.x * TILE_SIZE), (float)(coords.y * TILE_SIZE) } };
        // path_tiler.add_fill(auxiliary_segment, coords);
        path_tiler.bottom_intersection(coords);
      } else if (step.y < 0 && last_step_direction == StepDirection::Y) {
        // Enters through bottom boundary.
        path_tiler.bottom_intersection(coords);
        // Box auxiliary_segment = { vec2{ (float)(coords.x * TILE_SIZE), (float)(coords.y * TILE_SIZE) }, clipped_line_segment.min };
        // path_tiler.add_fill(auxiliary_segment, coords);
      }


      // // Adjust backdrop if necessary.
      // if (step.x < 0 && last_step_direction == StepDirection::X) {
      //   // Entered through right boundary.
      //   path_tiler.adjust_backdrop(coords, 1);
      // } else if (step.x > 0 && next_step_direction == StepDirection::X) {
      //   // Leaving through right boundary.
      //   path_tiler.adjust_backdrop(coords, -1);
      // }

      // Adjust backdrop if necessary.
      if (step.y < 0 && next_step_direction == StepDirection::Y) {
        // Leaving through bottom boundary.
        path_tiler.adjust_backdrop(coords, 1);
      } else if (step.y > 0 && last_step_direction == StepDirection::Y) {
        // Entering through bottom boundary.
        path_tiler.adjust_backdrop(coords, 1);
      }
      // else if (step.y > 0 && next_step_direction == StepDirection::X) {
      //   // Leaving through right boundary.
      //   path_tiler.adjust_backdrop(coords, -1);
      // }


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

    // if (segment_index % 2 == 0) {
    //   // if (std::fmod(p0.y, (float)TILE_SIZE) < 1.0f) {
    //   //   console::log("fmod", std::fmod(p0.y, (float)TILE_SIZE));
    //   // }
    //   if (is_almost_zero(std::fmod(p0.y, (float)TILE_SIZE), 0.2f)) {
    //     path_tiler.adjust_backdrop(from_coords, -1);
    //     console::log("almost_zero");
    //   }
    //   if (is_almost_zero(std::fmod(p3.y, (float)TILE_SIZE), 0.2f)) {
    //     path_tiler.adjust_backdrop(to_coords, -1);
    //     console::log("almost_zero");
    //   }
    // }

    // if (is_horizontal) {
    //   path_tiler.adjust_backdrop(from_coords, 1);
    //   path_tiler.adjust_backdrop(to_coords, 1);
    // }

    // tiles[tile_index(from_coords, bounds_size)].color = { 0.7f, 0.2f, 0.2f, 1.0f };
    // tiles[tile_index(first_crossing / TILE_SIZE, bounds_size)].color = { 0.2f, 0.7f, 0.2f, 1.0f };
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
    // Box line = { (segment.min - tile_upper_left) * 256.0f, (segment.max - tile_upper_left) * 256.0f };
    // vec2 min = { 0.0f, 0.0f }, max = { TILE_SIZE * 256.0f - 1.0f, TILE_SIZE * 256.0f - 1.0f };
    Line clipped_line = {
      (uint8_t)std::round(std::clamp(line.min.x * 255.0f, min, max)),
      (uint8_t)std::round(std::clamp(line.min.y * 255.0f, min, max)),
      (uint8_t)std::round(std::clamp(line.max.x * 255.0f, min, max)),
      (uint8_t)std::round(std::clamp(line.max.y * 255.0f, min, max)),
    };

    // Line clipped_line = {
    //   (uint8_t)255,
    //   (uint8_t)255,
    //   (uint8_t)0,
    //   (uint8_t)0,
    // };

    if (clipped_line.y1 == clipped_line.y2) return;

    int index = get_mask_tile_index(coords);
    if (index < 0) return;

    m_masks.push_back({ clipped_line, index });
    // int index = tile_index(coords, m_path_bounds_size);


    // m_tiles[index].color = { 0.2f, 0.2f, 0.7f, 1.0f };
  }

  void Tiler::PathTiler::add_vertical_fill(uint8_t min, uint8_t max, ivec2 coords) {
    if (coords.x < 0 || coords.x >= m_path_bounds_size.x || coords.y < 0 || coords.y >= m_path_bounds_size.y) return;

    vec2 tile_upper_left = vec2{ (float)coords.x, (float)coords.y } *(float)TILE_SIZE;

    // Box line = { (segment.min - tile_upper_left) * 256.0f, (segment.max - tile_upper_left) * 256.0f };
    // vec2 min = { 0.0f, 0.0f }, max = { TILE_SIZE * 256.0f - 1.0f, TILE_SIZE * 256.0f - 1.0f };
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
    // ivec2 tile_offset = coords - m_path_bounds_offset;
    if (coords.x < 0 || coords.x >= m_path_bounds_size.x || coords.y >= m_path_bounds_size.y) return;

    if (coords.y < 0) {
      m_tiles[coords.x].backdrop += (int)delta;
      // m_tiles[coords.x].color = delta > 0 ? vec4{ 0.2f, 0.7f, 0.2f, 1.0f } : vec4{ 0.7f, 0.2f, 0.2f, 1.0f };
      return;
    }

    int index = tile_index(coords, m_path_bounds_size);

    m_tiles[index].backdrop += (int)delta;
    // TEMP
    // m_tiles[index].color = delta > 0 ? vec4{ 0.2f, 0.7f, 0.2f, 1.0f } : vec4{ 0.7f, 0.2f, 0.2f, 1.0f };
  }

#else

  struct TileData {
    ivec2 coords = { 0, 0 };
    std::vector<Line> lines;
    std::vector<uint8_t> right_intersections;
    std::vector<uint8_t> bottom_intersections;
    vec4 color = { 0.5f, 0.5f, 0.5f, 0.1f };
    bool filled = false;
  };

  void Tiler::process_linear_segment(const Geometry::Segment& segment, vec2 position, ivec2 bounds_size, std::vector<TileData>& tiles) {}

  void Tiler::process_path(const Geometry::Path& path, const vec4& color, bool) {

    Box box = path.bounding_box();
    // TODO: maybe can optimize this check
    if (!does_box_intersect_box(box, m_visible)) return;

    box.min = {
      std::floor(box.min.x * m_zoom / TILE_SIZE) * TILE_SIZE,
      std::floor(box.min.y * m_zoom / TILE_SIZE) * TILE_SIZE
    };
    box.max = {
      std::ceil(box.max.x * m_zoom / TILE_SIZE) * TILE_SIZE,
      std::ceil(box.max.y * m_zoom / TILE_SIZE) * TILE_SIZE
    };

    ivec2 min_coords = tile_coords(box.min) + m_position;
    ivec2 max_coords = tile_coords(box.max) + m_position;

    ivec2 path_bounds = { max_coords.x - min_coords.x, max_coords.y - min_coords.y };
    ivec2 box_tiles = path_bounds;

    std::vector<TileData> tiles(box_tiles.x * box_tiles.y);

    for (const auto& segment : path.segments()) {
      vec2 p0 = segment.p0() * m_zoom - box.min;
      vec2 p3 = segment.p3() * m_zoom - box.min;

      ivec2 p0_coords = tile_coords_clamp(p0, box_tiles);
      ivec2 p3_coords = tile_coords_clamp(p3, box_tiles);

      ivec2 increment = { p3_coords.x > p0_coords.x ? 1 : -1, p3_coords.y > p0_coords.y ? 1 : -1 };
      ivec2 delta = { std::abs(p3_coords.x - p0_coords.x), std::abs(p3_coords.y - p0_coords.y) };

      int index = tile_index(p0_coords, box_tiles);
      tiles[index].color = color;

      if (delta.x == 0) {
        for (int i = 0; i < delta.y; i++) {
          p0_coords.y += increment.y;
          index = tile_index(p0_coords, box_tiles);
          tiles[index].color = color;

          // TODO: Calculate intersections
          if (increment.y > 0) {
            tiles[index].bottom_intersections.push_back((uint8_t)std::round(0));
          } else {
            tiles[tile_index({ p0_coords.x, p0_coords.y - increment.y }, box_tiles)].bottom_intersections.push_back((uint8_t)std::round(0));
          }
        }

        continue;
      } else if (delta.y == 0) {
        uint8_t last[2] = { 0, 0 };

        for (int i = 0; i < delta.x; i++) {
          p0_coords.x += increment.x;
          index = tile_index(p0_coords, box_tiles);
          tiles[index].color = color;

          float boundary = (p0_coords.x) * TILE_SIZE;
          float m = (p3.y - p0.y) / (p3.x - p0.x);
          float y = m * boundary + p0.y;

          // if (i == 0) {
          // } else if (i == delta.x - 1) {
          // } else {
          //   uint8_t inter = (uint8_t)std::round((y - p0_coords.y) / TILE_SIZE * 255);
          //   tiles[index].lines.push_back({ 0, last[1], 255, inter });
          //   tiles[index].lines.push_back({ 0, last[1], 255, last[1] });
          //   last[1] = inter;

          // }

          if (increment.x < 0) {
            tiles[index].right_intersections.push_back((uint8_t)std::round(0));
          } else {
            tiles[tile_index({ p0_coords.x - increment.x, p0_coords.y }, box_tiles)].right_intersections.push_back((uint8_t)std::round(0));
          }
        }

        continue;
      }

      float m = (p3.y - p0.y) / (p3.x - p0.x);
      float m_inv = 1.0f / m;
      float q = p0.y - m * p0.x;

      // vec2 last = p0;
      ivec2 last_coords = p0_coords;
      ivec2 tile_increment = { increment.x > 0 ? increment.x : 0, increment.y > 0 ? increment.y : 0 };
      ivec2 tile_error_increment = { increment.x > 0 ? 0 : increment.x, increment.y > 0 ? 0 : increment.y };

      int n = delta.x + delta.y;
      uint8_t last_inter = 0;

      TileData& last_tile = tiles[index];
      uint8_t last_intersection = 0;

      //while (p0_coords != p3_coords && n > 0) {
      for (int n = delta.x + delta.y; n > 0; n--) {
        ivec2 boundary = (p0_coords + tile_increment) * TILE_SIZE;

        float x = (boundary.y - q) * m_inv;
        float y = m * boundary.x + q;

        float error = std::abs(y - (last_coords.y - tile_error_increment.y) * TILE_SIZE);
        TileData tile = { { 0, 0 }, {}, {}, {}, color / 2 };

        if (error < TILE_SIZE) {
          p0_coords.x += increment.x;
          // last = { (float)boundary.x, y };
          uint8_t intersection = (uint8_t)std::round((y - boundary.y) / TILE_SIZE * 255);

          if (increment.x < 0) {
            // tile.lines.push_back({ 0, 0, 255, 255 });
            tile.right_intersections.push_back(intersection);
            tile.lines.push_back({ 255, last_intersection, 0, intersection });
            tile.lines.push_back({ 0, (uint8_t)std::round(std::floor(y - boundary.y) / TILE_SIZE * 255), 0, 0 });
          } else {
            // tile.lines.push_back({ 255, last_inter, 0, inter });
            // tile.lines.push_back({ 0, inter, 0, 255 });
            // tile.lines.push_back({ 255, intersection, 0, last_intersection });
            // console::log(last_inter - inter);
            // if (n % 2 == 0) {
            //   tile.lines.push_back({ 0, 5, 255, 0 });
            //   tile.lines.push_back({ 0, (uint8_t)std::round(1.0 / TILE_SIZE * 255), 0, 0 });
            // } else {
            //   tile.lines.push_back({ 0, 5, 0, 0 });
            // }
            last_tile.right_intersections.push_back(intersection);
            tile.lines.push_back({ 0, last_intersection, 255, intersection });
            tile.lines.push_back({ 0, (uint8_t)std::round(std::floor(y - boundary.y) / TILE_SIZE * 255), 0, 0 });
          }

          last_intersection = intersection;
        } else if (error > TILE_SIZE) {
          p0_coords.y += increment.y;
          // last = { x, (float)boundary.y };

          if (increment.y > 0) {
            // tile.lines.push_back({ 0, 150, 255, 255 });
            tile.bottom_intersections.push_back((uint8_t)std::round(x - boundary.x));
          } else {
            // tile.lines.push_back({ 0, 0, 150, 255 });
            tiles[tile_index(last_coords, box_tiles)].bottom_intersections.push_back((uint8_t)std::round(x - boundary.x));
          }
        } else {
          p0_coords.x += increment.x;
          p0_coords.y += increment.y;
          n--;
          // last = { (float)boundary.x, (float)boundary.y };

          Line line = { 0, 0, 0, 0 };

          if (increment.x < 0 && increment.y > 0) {
            tile.right_intersections.push_back(255);
            tile.bottom_intersections.push_back(255);
            tile.lines.push_back({ 255, 255, 0, 0 });
          } else if (increment.x > 0 && increment.y < 0) {
            last_tile.right_intersections.push_back(255);
            tile.bottom_intersections.push_back(255);
            tile.lines.push_back({ 0, 0, 255, 255 });
          }

#if 0
          if (increment.x < 0) {
            // tile.right_intersections.push_back((uint8_t)std::round(y - boundary.y));
            tile.right_intersections.push_back(255);
            line.x1 = 255;
            line.x2 = 0;
            // tile.lines.push_back({ 0, 0, 255, 255 });

            // last_tile.lines.push_back({ 0, (uint8_t)std::round(std::floor(y - boundary.y) / TILE_SIZE * 255), 0, 0 });
          } else {
            last_tile.right_intersections.push_back(255);
            line.x1 = 0;
            line.x2 = 255;
          }

          if (increment.y > 0) {
            tile.bottom_intersections.push_back(0);
            line.y1 = 0;
            line.y2 = 255;
          } else {
            last_tile.bottom_intersections.push_back((uint8_t)std::round(x - boundary.x));
            line.y1 = 255;
            line.y2 = 0;
          }

          tile.lines.push_back(line);
#endif
        }

        // if (p0_coords.x < 0 || p0_coords.y < 0) {
        //   tile.color *= 4;
        // }
        tile.coords = p0_coords;
        last_coords = p0_coords;
        tiles[tile_index(p0_coords, box_tiles)].color = tile.color;
        tiles[tile_index(p0_coords, box_tiles)].right_intersections.insert(tiles[tile_index(p0_coords, box_tiles)].right_intersections.end(), tile.right_intersections.begin(), tile.right_intersections.end());
        tiles[tile_index(p0_coords, box_tiles)].bottom_intersections.insert(tiles[tile_index(p0_coords, box_tiles)].bottom_intersections.end(), tile.bottom_intersections.begin(), tile.bottom_intersections.end());
        tiles[tile_index(p0_coords, box_tiles)].lines.insert(tiles[tile_index(p0_coords, box_tiles)].lines.end(), tile.lines.begin(), tile.lines.end());

        last_tile = tiles[tile_index(p0_coords, box_tiles)];
        //n--;
      }
    }

    // for (int x = 0; x < box_tiles.x; x++) {
    //   int winding = 0;
    //   for (int y = 0; y < box_tiles.y; y++) {
    //     int index = tile_index({ x, y }, box_tiles);
    //     if (!tiles[index].right_intersections.empty()) {
    //       winding++;
    //     }

    //     if (tiles[index].color == vec4{ 0.5f, 0.5f, 0.5f, 0.1f } && (winding % 2) != 0) {
    //       tiles[index].color = vec4{ 0.7f, 0.2f, 0.2f, 0.5f };
    //     }
    //   }
    // }

    for (int y = 0; y < box_tiles.y; y++) {
      int intersections = 0;
      // std::vector<uint8_t> last_intersections;
      for (int x = box_tiles.x - 1; x >= 0; x--) {
        int index = tile_index({ x, y }, box_tiles);
        if ((intersections % 2) != 0) {
          if (tiles[index].color == vec4{0.5f, 0.5f, 0.5f, 0.1f}) {
            tiles[index].color = color;
            tiles[index].filled = true;
          } else if (!tiles[index].lines.empty()) {
            // for (uint8_t inter : tiles[index - 1].right_intersections) {
              // tile.lines.push_back({ 0, (uint8_t)std::round(std::floor(y - boundary.y) / TILE_SIZE * 255), 0, 0 });
              // tiles[index].lines.push_back({ 0, inter, 0, 0 });
            // }

            tiles[index].color = vec4{ 0.7f, 0.2f, 0.2f, 1.0f };
          }
        }

        // if (tiles[index].bottom_intersections.size()) {
        //   tiles[index].color = vec4{ 0.2f, 0.2f, 0.7f, 1.0f };
        // }

        intersections += tiles[index].bottom_intersections.size();
        // last_intersections = {};
      }
    }

    for (int i = 0; i < tiles.size(); i++) {
      ivec2 coords = ivec2{ i % box_tiles.x + 1, i / box_tiles.x + 1 } + min_coords;
      if (coords.x < 0 || coords.y < 0 || coords.x >= m_tiles_count.x || coords.y >= m_tiles_count.y) continue;
      if (tiles[i].color == vec4{ 0.5f, 0.5f, 0.5f, 0.1f }) continue;

      int index = tile_index(coords, m_tiles_count);

      if (m_processed_tiles[index]) continue;

      if (!tiles[i].lines.empty()) {
        for (Line line : tiles[i].lines) {
          m_masks.push_back({ line, (int)m_mask_id });
        }
        if (tiles[i].color.a = 0.0f) tiles[i].color.a = 1.0f;
        m_tiles.push_back({ color, tile_index(coords, m_tiles_count), (int)m_mask_id });

        m_mask_id++;
      } else if (tiles[i].filled) {
        m_opaque_tiles.push_back({ color, tile_index(coords, m_tiles_count) });
        m_processed_tiles[index] = true;
      }
    }

    return;
  }

#endif

}
