#pragma once

#include "renderer_data.h"
#include "geometry/path.h"

#include <vector>

#define TILE_SIZE 16u

namespace Graphick::Render {

  class Tiler {
  public:
    Tiler(const Tiler&) = delete;
    Tiler(Tiler&&) = delete;

    Tiler();
    ~Tiler() = default;

    inline const std::vector<Fill>& opaque_tiles() const { return m_opaque_tiles; }

    void reset(const ivec2 size, const vec2 position, float zoom);
    void process_path(const Geometry::Path& path, const vec4& color);
  private:
    ivec2 m_position = { 0, 0 };
    float m_zoom = 1.0f;
    Box m_visible;
    ivec2 m_tiles_count = { 0, 0 };
    std::vector<bool> m_processed_tiles;
    std::vector<Fill> m_opaque_tiles;
  };

}
