#pragma once

#include "../math/vec2.h"
#include "../math/ivec2.h"

#include <vector>

namespace Graphick::Renderer {

  namespace Geometry {
    class Path;
  }

  class Rasterizer {
  public:
    // TODO: keep rasterizer for multiple paths 
    Rasterizer() = default;

    void rasterize(vec2 shift, ivec2 size, const Geometry::Path& path, uint8_t* buffer);
#ifndef OLD_RASTERIZER
#else
  private:
    void set_cell(int x, int y);

    void move_to(ivec2 to);
    void line_to(ivec2 to);
  private:
    struct Cell {
      int x;
      int cover;
      int area;
      int next;
    };

    struct RasterStorage {
      ivec2 min;
      ivec2 max;

      std::vector<Cell> cells;
      std::vector<int> indices;

      void reset(ivec2 min, ivec2 max);
      void set(int x, int y, int area, int cover);
    };
  private:
    RasterStorage m_storage;

    int m_xmin = 0;
    int m_xmax = 0;
    int m_ymin = 0;
    int m_ymax = 0;
    int m_height = 0;

    int m_x = 0;
    int m_y = 0;
    int m_px = 0;
    int m_py = 0;
    int m_cover = 0;
    int m_area = 0;

    vec2 m_shift = { 0,0 };
    vec2 m_current = { 0, 0 };
    ivec2 m_start = { 0, 0 };

    bool m_closed = false;
    bool m_invalid = false;
#endif
  };

}
