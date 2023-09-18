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
      int16_t tile_x;
      int16_t tile_y;
      int8_t sign;
    };
    struct Span {
      int16_t tile_x;
      int16_t tile_y;
      int16_t width;
    };
    struct Bin {
      int16_t tile_x;
      int16_t tile_y;
    };
    struct Mask {
      std::vector<uvec4> segments;
      uint8_t cover_table[TILE_SIZE] = { 0 };
    };

    enum class StepDirection {
      None = 0,
      X,
      Y
    };
  public:
    PathTiler(const PathTiler&) = default;
    PathTiler(PathTiler&&) = default;

    PathTiler(
      const Geometry::Path& path,
      const vec2 translation,
      const vec4& color,
      const rect& visible,
      float zoom,
      ivec2 position,
      const std::vector<bool>& culled,
      const ivec2 tiles_count
    );
    ~PathTiler() = default;

    inline const std::unordered_map<int, Mask>& masks() const { return m_masks; }
    inline const std::vector<Span>& spans() const { return m_spans; }

    inline ivec2 offset() const { return m_offset; }
    inline ivec2 size() const { return m_bounds_size; }
  private:
    void process_linear_segment(const vec2 p0, const vec2 p3);
    void process_quadratic_segment(const vec2 p0, const vec2 p1, const vec2 p3);
    void process_cubic_segment(const vec2 p0, const vec2 p1, const vec2 p2, const vec2 p3);
    void push_segment(const uvec4 segment, int16_t tile_x, int16_t tile_y);

    void finish(const ivec2 tiles_count);
  private:
    std::vector<Increment> m_tile_increments;
    std::vector<Bin> m_bins;
    Bin m_bin = { 0, 0 };

    std::unordered_map<int, Mask> m_masks;
    std::vector<Span> m_spans;

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

    inline const uint8_t* segments() const { return m_segments; };
    inline const size_t segments_size() const { return m_segments_ptr - m_segments; };

    void reset(const Viewport& viewport);
    void process_path(const Geometry::Path& path, const vec2 translation, const vec4& color, const float z_index);
  private:
    void push_mask(const PathTiler::Mask& tile, int index, const vec4& color);
  private:
    std::vector<MaskedTile> m_masked_tiles;
    std::vector<OpaqueTile> m_opaque_tiles;
    std::vector<bool> m_culled_tiles;

    float m_zoom = 1.0f;
    ivec2 m_position = { 0, 0 };
    ivec2 m_tiles_count = { 0, 0 };
    rect m_visible;

    // TODO: replace with array of textures
    uint8_t* m_segments = nullptr;
    uint8_t* m_segments_ptr = nullptr;
  };

}
