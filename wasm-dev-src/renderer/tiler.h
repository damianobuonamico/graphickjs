#pragma once

#include "renderer_data.h"

#include "../math/rect.h"

#include <vector>

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
    struct TempBin {
      int16_t tile_x;
      int16_t tile_y;
    };
    struct TempMask {
      std::vector<uvec4> segments;
      uint8_t cover_table[TILE_SIZE] = { 0 };
    };
  public:
    PathTiler(const PathTiler&) = default;
    PathTiler(PathTiler&&) = default;

    PathTiler(const Geometry::Path& path, const vec4& color, const rect& visible, float zoom, ivec2 position, const std::vector<bool>& culled, const ivec2 tiles_count);
    ~PathTiler() = default;

    inline const std::unordered_map<int, TempMask>& temp_masks() const { return m_temp_masks; }
    inline const std::vector<TileMask>& masks() const { return m_masks; }
    inline const std::vector<TileSpan>& spans() const { return m_spans; }

    inline ivec2 offset() const { return m_offset; }
    inline ivec2 size() const { return m_bounds_size; }
  private:
    void process_linear_segment(const vec2 p0, const vec2 p3);
    void process_linear_segment_old(const vec2 p0, const vec2 p3);
    void process_cubic_segment(const vec2 p0, const vec2 p1, const vec2 p2, const vec2 p3);
    void process_linear_segment_clipped(const vec2 p0, const vec2 p3, rect visible);
    void process_cubic_segment_clipped(const vec2 p0, const vec2 p1, const vec2 p2, const vec2 p3, rect visible);

    void push_segment(const uvec4 segment, int16_t tile_x, int16_t tile_y);
    void adjust_alpha_tile_backdrop(const ivec2 tile_coords, int delta);

    void finish(const std::vector<bool>& culled, const ivec2 tiles_count);
    void finish_old(const std::vector<bool>& culled, const ivec2 tiles_count);
  private:
    std::vector<Increment> m_increments;
    std::vector<TileIncrement> m_tile_increments;
    std::vector<int> m_backdrops;
    std::vector<TempBin> m_bins;
    TempBin m_bin = { 0, 0 };


    std::unordered_map<int, TempMask> m_temp_masks;
    std::vector<TileMask> m_masks;
    std::vector<TileSpan> m_spans;

    int16_t m_tile_y_prev = 0;
    vec2 m_prev = { 0.0f, 0.0f };

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

    const std::vector<uint8_t*> masks_textures_data() const;
    inline const std::vector<uint8_t> segments() const { return m_segments; };

    void reset(const Viewport& viewport);
    void process_path(const Geometry::Path& path, const vec4& color);
  private:
    void push_mask(const PathTiler::TileMask& tile, int index, const vec4& color);
    void resize_masks_textures(const int textures);
  private:
    std::vector<MaskedTile> m_masked_tiles;
    std::vector<OpaqueTile> m_opaque_tiles;
    std::vector<bool> m_culled_tiles;

    float m_zoom;
    ivec2 m_position;
    ivec2 m_tiles_count;

    rect m_visible;

    int m_masks_offset = 0;
    int m_masks_texture_index = 0;
    int m_masks_textures_count = 0;

    int m_segments_offset = 0;

    uint8_t* m_masks_textures = nullptr;
    // TODO: replace with array of textures
    std::vector<uint8_t> m_segments;
  };

}
