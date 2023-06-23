#pragma once

#include "renderer_data.h"
#include "geometry/path.h"

#include <vector>

#define TILE_SIZE 16
#define TILE_OVERLAP 2u
#define TILE_SIZE_OVERLAP (TILE_SIZE + TILE_OVERLAP)

namespace Graphick::Render {

  class PathTiler {
  public:
    struct TileData {
      std::vector<Line> segments;
      uint16_t bottom_intersections = 0;
    };
    struct Increment {
      int16_t x;
      int16_t y;
      float area;
      float height;
    };
    struct TileIncrement {
      int16_t tile_x;
      int16_t tile_y;
      int8_t sign;
    };
    struct TileMask {
      int16_t tile_x;
      int16_t tile_y;
      uint8_t data[TILE_SIZE * TILE_SIZE];

      TileMask::TileMask(int16_t tile_x, int16_t tile_y, uint8_t* data) : tile_x(tile_x), tile_y(tile_y) {
        memcpy(this->data, data, TILE_SIZE * TILE_SIZE);
      }
    };
    struct Span {
      int16_t tile_x;
      int16_t tile_y;
      int16_t width;
    };
  public:
    PathTiler(const PathTiler&) = default;
    PathTiler(PathTiler&&) = default;

    PathTiler(const Geometry::Path& path, const vec4& color, const Box& visible, float zoom, ivec2 position);
    ~PathTiler() = default;

    inline const std::vector<TileMask>& masks() { return m_masks; }
    inline const std::vector<Span>& spans() { return m_spans; }
    inline const std::vector<TileData>& tiles() { return m_tiles; }
    inline ivec2 offset() const { return m_offset; }
    inline ivec2 size() const { return m_bounds_size; }
  private:
    void process_linear_segment(const Geometry::Segment& segment, vec2 offset);

    void add_line(const Box& line, const ivec2 coords);
    void finish();
  private:
    std::vector<TileData> m_tiles;
    std::vector<Increment> m_increments;
    std::vector<TileIncrement> m_tile_increments;
    std::vector<TileMask> m_masks;
    std::vector<Span> m_spans;

    int16_t m_tile_y_prev = 0;

    float m_zoom;
    ivec2 m_position;
    ivec2 m_offset;
    ivec2 m_bounds_size;
  };

  class Tiler {
  public:
    Tiler(const Tiler&) = delete;
    Tiler(Tiler&&) = delete;

    Tiler();
    ~Tiler();

    inline const std::vector<Tile>& tiles() const { return m_tiles; }
    inline const std::vector<uint8_t>& segments() const { return m_segments; }
    inline const uint8_t* masks_texture_data() const { return m_masks; }

    void reset(const ivec2 size, const vec2 position, float zoom);
    void process_path(const Geometry::Path& path, const vec4& color);
  private:
    std::vector<Tile> m_tiles;
    std::vector<uint8_t> m_segments;

    int m_segments_offset = 0;

    float m_zoom;
    ivec2 m_position;
    ivec2 m_tiles_count;

    Box m_visible;

    int m_masks_offset = 0;
    uint8_t* m_masks = nullptr;
  };

}
