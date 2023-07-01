#pragma once

#include "renderer_data.h"

#include "../math/rect.h"

#include <vector>

#define TILE_SIZE 32

namespace Graphick::Renderer {

  namespace Geometry {

    class Path;
    class Segment;

  }

  class PathTiler {
  public:
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

      TileMask(int16_t tile_x, int16_t tile_y, uint8_t* data) : tile_x(tile_x), tile_y(tile_y) {
        memcpy(this->data, data, TILE_SIZE * TILE_SIZE);
      }
    };
    struct TileSpan {
      int16_t tile_x;
      int16_t tile_y;
      int16_t width;
      vec4 color;
    };
    struct Bin {
      int16_t tile_x;
      int16_t tile_y;
      size_t start;
      size_t end;
    };
  public:
    PathTiler(const PathTiler&) = default;
    PathTiler(PathTiler&&) = default;

    PathTiler(const Geometry::Path& path, const vec4& color, const rect& visible, float zoom, ivec2 position);
    ~PathTiler() = default;

    inline const std::vector<TileMask>& masks() const { return m_masks; }
    inline const std::vector<TileSpan>& spans() const { return m_spans; }

    inline ivec2 offset() const { return m_offset; }
    inline ivec2 size() const { return m_bounds_size; }
  private:
    void process_linear_segment(const Geometry::Segment& segment, vec2 offset);
    void finish();
  private:
    std::vector<Increment> m_increments;
    std::vector<TileIncrement> m_tile_increments;

    std::vector<TileMask> m_masks;
    std::vector<TileSpan> m_spans;

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

    inline const std::vector<OpaqueTile>& opaque_tiles() const { return m_opaque_tiles; }
    inline const std::vector<MaskedTile>& masked_tiles() const { return m_masked_tiles; }
    inline const uint8_t* masks_texture_data() const { return m_masks; }

    void reset(const Viewport& viewport);
    void process_path(const Geometry::Path& path, const vec4& color);
  private:
    std::vector<MaskedTile> m_masked_tiles;
    std::vector<OpaqueTile> m_opaque_tiles;
    std::vector<bool> m_culled_tiles;

    float m_zoom;
    ivec2 m_position;
    ivec2 m_tiles_count;

    rect m_visible;

    int m_masks_offset = 0;
    uint8_t* m_masks = nullptr;
  };

}
