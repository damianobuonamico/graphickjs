#pragma once

#include "renderer_data.h"
#include "geometry/path.h"

#include <vector>

#define TILE_SIZE 32u

namespace Graphick::Render {

  class Tiler {
  public:
    Tiler(const Tiler&) = delete;
    Tiler(Tiler&&) = delete;

    Tiler();
    ~Tiler() = default;

    inline const std::vector<Fill>& opaque_tiles() const { return m_opaque_tiles; }
    inline const std::vector<Mask>& masks() const { return m_masks; }
    inline const std::vector<Tile>& tiles() const { return m_tiles; }

    void reset(const ivec2 size, const vec2 position, float zoom);
    void process_path(const Geometry::Path& path, const vec4& color);
  private:
    struct TileData {
      vec4 color = { 0.0f, 0.0f, 0.0f, 0.0f };
      int backdrop = 0;
      bool has_mask = false;
      std::vector<uint8_t> intersections;
      int bottom_intersections = 0;
    };

    struct FillData {
      int32_t index = 0;
      int32_t mask_index = 0;
    };

    struct PathTiler {
      PathTiler(ivec2 path_bounds_size, ivec2 path_bounds_offset);

      inline std::vector<TileData>& tiles() { return m_tiles; }
      inline const std::vector<Mask>& masks() const { return m_masks; }
      inline const std::vector<FillData>& fills() const { return m_fills; }

      void add_fill(const Box& segment, const ivec2 coords);
      int get_mask_tile_index(ivec2 coords);
      void intersection(float y, ivec2 coords);
      void bottom_intersection(ivec2 coords);
      void add_vertical_fill(uint8_t min, uint8_t max, ivec2 coords);
      void adjust_backdrop(const ivec2 coords, int8_t delta);
    private:
      ivec2 m_path_bounds_size;
      ivec2 m_path_bounds_offset;
      std::vector<TileData> m_tiles;
      std::vector<Mask> m_masks;
      std::vector<FillData> m_fills;
      std::vector<int> m_mask_tiles;
    };
  private:
    void process_linear_segment(const Geometry::Segment& segment, vec2 position, ivec2 bounds_size, int segment_index, PathTiler& path_tiler);
  private:
    ivec2 m_position = { 0, 0 };
    float m_zoom = 1.0f;
    Box m_visible;
    ivec2 m_tiles_count = { 0, 0 };
    std::vector<bool> m_processed_tiles;
    std::vector<Fill> m_opaque_tiles;
    std::vector<Mask> m_masks;
    std::vector<Tile> m_tiles;
    uint32_t m_mask_id = 0;
  };

}
