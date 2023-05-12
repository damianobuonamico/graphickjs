#include "tiler.h"

#include "../utils/console.h"
#include "../math/math.h"

namespace Graphick::Render {

  Tiler::Tiler() {}

  void Tiler::reset(const ivec2 size, const vec2 position, float zoom) {
    m_tiles_count = { (int)(std::ceil((float)size.x / TILE_SIZE)) + 2, (int)(std::ceil((float)size.y / TILE_SIZE)) + 2 };
    m_processed_tiles = std::vector<bool>(m_tiles_count.x * m_tiles_count.y, false);
    m_opaque_tiles.clear();

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

  struct Line {
    uint8_t x1;
    uint8_t y1;
    uint8_t x2;
    uint8_t y2;
  };

  struct Tile {
    ivec2 coords = { 0, 0 };
    std::vector<Line> lines;
    std::vector<uint8_t> right_intersections;
    std::vector<uint8_t> bottom_intersections;
    vec4 color = { 0.5f, 0.5f, 0.5f, 0.1f };
    bool filled = false;
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

  void Tiler::process_path(const Geometry::Path& path, const vec4& color) {

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

    std::vector<Tile> tiles(box_tiles.x * box_tiles.y);

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
        for (int i = 0; i < delta.x; i++) {
          p0_coords.x += increment.x;
          index = tile_index(p0_coords, box_tiles);
          tiles[index].color = color;

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

      //while (p0_coords != p3_coords && n > 0) {
      for (int n = delta.x + delta.y; n > 0; n--) {
        ivec2 boundary = (p0_coords + tile_increment) * TILE_SIZE;

        float x = (boundary.y - q) * m_inv;
        float y = m * boundary.x + q;

        // float error = std::abs(x - last_coords.x * TILE_SIZE) / delta.x - std::abs(y - last_coords.y * TILE_SIZE) / delta.y;
        float error = std::abs(y - (last_coords.y - tile_error_increment.y) * TILE_SIZE);
        Tile tile = { { 0, 0 }, {}, {}, {}, color / 2 };

        if (error < TILE_SIZE) {
          p0_coords.x += increment.x;
          // last = { (float)boundary.x, y };

          if (increment.x < 0) {
            tile.right_intersections.push_back((uint8_t)std::round(y - boundary.y));
          } else {
            tiles[tile_index(last_coords, box_tiles)].right_intersections.push_back((uint8_t)std::round(y - boundary.y));
          }
        } else if (error > TILE_SIZE) {
          p0_coords.y += increment.y;
          // last = { x, (float)boundary.y };

          if (increment.y > 0) {
            tile.bottom_intersections.push_back((uint8_t)std::round(x - boundary.x));
          } else {
            tiles[tile_index(last_coords, box_tiles)].bottom_intersections.push_back((uint8_t)std::round(x - boundary.x));
          }
        } else {
          p0_coords.x += increment.x;
          p0_coords.y += increment.y;
          n--;
          // last = { (float)boundary.x, (float)boundary.y };

          if (increment.x < 0) {
            tile.right_intersections.push_back((uint8_t)std::round(y - boundary.y));
          } else {
            tiles[tile_index(last_coords, box_tiles)].right_intersections.push_back((uint8_t)std::round(y - boundary.y));
          }

          if (increment.y > 0) {
            tile.bottom_intersections.push_back((uint8_t)std::round(x - boundary.x));
          } else {
            tiles[tile_index(last_coords, box_tiles)].bottom_intersections.push_back((uint8_t)std::round(x - boundary.x));
          }
        }

        // if (p0_coords.x < 0 || p0_coords.y < 0) {
        //   tile.color *= 4;
        // }
        tile.coords = p0_coords;
        last_coords = p0_coords;
        tiles[tile_index(p0_coords, box_tiles)].color = tile.color;
        tiles[tile_index(p0_coords, box_tiles)].right_intersections.insert(tiles[tile_index(p0_coords, box_tiles)].right_intersections.end(), tile.right_intersections.begin(), tile.right_intersections.end());
        tiles[tile_index(p0_coords, box_tiles)].bottom_intersections.insert(tiles[tile_index(p0_coords, box_tiles)].bottom_intersections.end(), tile.bottom_intersections.begin(), tile.bottom_intersections.end());

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
      for (int x = box_tiles.x - 1; x >= 0; x--) {
        int index = tile_index({ x, y }, box_tiles);
        if ((intersections % 2) != 0 && tiles[index].color == vec4{ 0.5f, 0.5f, 0.5f, 0.1f }) {
          // tiles[index].color = vec4{ 0.7f, 0.2f, 0.2f, 0.5f };
          tiles[index].color = color;
          tiles[index].filled = true;
        }

        // if (tiles[index].bottom_intersections.size()) {
        //   tiles[index].color = vec4{ 0.2f, 0.2f, 0.7f, 1.0f };
        // }

        intersections += tiles[index].bottom_intersections.size();
      }
    }

    for (int i = 0; i < tiles.size(); i++) {
      ivec2 coords = ivec2{ i % box_tiles.x + 1, i / box_tiles.x + 1 } + min_coords;
      if (coords.x < 0 || coords.y < 0 || coords.x >= m_tiles_count.x || coords.y >= m_tiles_count.y) continue;
      if (tiles[i].color == vec4{ 0.5f, 0.5f, 0.5f, 0.1f }) continue;

      int index = tile_index(coords, m_tiles_count);

      if (m_processed_tiles[index]) continue;

      if (tiles[i].filled) {
        m_opaque_tiles.push_back({ tiles[i].color, tile_index(coords, m_tiles_count) });
        m_processed_tiles[index] = true;
      }
    }

    return;
  }
}
