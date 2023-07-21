#include "rasterizer.h"

#include "geometry/path.h"

#include "../lib/stb/stb_truetype.h"

#include <numeric>

#define PIXEL_BITS 8
#define ONE_PIXEL (1 << PIXEL_BITS)

namespace Graphick::Renderer {

  enum class Fill {
    NonZero,
    EvenOdd
  };

#ifndef OLD_RASTERIZER

  void Rasterizer::rasterize(vec2 shift, ivec2 size, const Geometry::Path& path, uint8_t* buffer) {
    const std::vector<Geometry::Segment> segments = path.segments();
    const rect box = path.bounding_rect();
    const int num_verts = segments.size() + (path.closed() ? 1 : 2);

    stbtt_vertex* vertices = new stbtt_vertex[num_verts];
    stbtt__bitmap bitmap = { size.x, size.y, size.x, buffer };

    vertices[0] = { stbtt_vertex_type((segments.front().p0().x - box.min.x) * 1.00f), stbtt_vertex_type((segments.front().p0().y - box.min.y) * 1.00f), 0, 0, 0, 0, STBTT_vmove, 0 };

    for (size_t i = 1; i <= segments.size(); i++) {
      const Geometry::Segment& segment = segments[i - 1];

      switch (segment.kind()) {
      case Geometry::Segment::Kind::Quadratic:
        vertices[i] = {
          stbtt_vertex_type((segment.p3().x - box.min.x) * 1.00f),
          stbtt_vertex_type((segment.p3().y - box.min.y) * 1.00f),
          stbtt_vertex_type((segment.p1().x - box.min.x) * 1.00f),
          stbtt_vertex_type((segment.p1().y - box.min.y) * 1.00f),
          0, 0, STBTT_vcurve, 0
        };
        break;
      case Geometry::Segment::Kind::Cubic:
        vertices[i] = {
          stbtt_vertex_type((segment.p3().x - box.min.x) * 1.00f),
          stbtt_vertex_type((segment.p3().y - box.min.y) * 1.00f),
          stbtt_vertex_type((segment.p1().x - box.min.x) * 1.00f),
          stbtt_vertex_type((segment.p1().y - box.min.y) * 1.00f),
          stbtt_vertex_type((segment.p2().y - box.min.y) * 1.00f),
          stbtt_vertex_type((segment.p2().y - box.min.y) * 1.00f),
          STBTT_vcubic, 0
        };
        break;
      default:
      case Geometry::Segment::Kind::Linear:
        vertices[i] = {
          stbtt_vertex_type((segment.p3().x - box.min.x) * 1.00f),
          stbtt_vertex_type((segment.p3().y - box.min.y) * 1.00f),
          0, 0, 0, 0, STBTT_vline, 0
        };
        break;
      }
    }

    if (!path.closed()) {
      vertices[num_verts - 1] = { vertices[0].x, vertices[0].y, 0, 0, 0, 0, STBTT_vline, 0 };
    }

    stbtt_Rasterize(&bitmap, 0.35f, vertices, num_verts, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0, nullptr);

    delete[] vertices;
  }

#elif false

  struct Edge {
    vec2 p0;
    vec2 p3;

    float min_y;
    float max_y;
  };

  static bool is_in_span(int y, float min, float max) {
    return y >= min && y <= max;
  }

  static void recalculate_edges(int y, const std::vector<Edge>& edges, std::vector<int>& possible_edges, std::vector<int>& active_edges) {
    active_edges.clear();
    std::vector<int> new_possible_edges{};

    for (auto i : possible_edges) {
      const Edge& edge = edges[i];

      if (y <= edge.max_y) {
        if (y >= edge.min_y) {
          active_edges.push_back(i);
        }
        new_possible_edges.push_back(i);
      }
    }

    possible_edges = new_possible_edges;
  }

  void Rasterizer::rasterize(vec2 shift, ivec2 size, const Geometry::Path& path, uint8_t* buffer) {
    const std::vector<Geometry::Segment> segments = path.segments();
    rect box = path.bounding_rect();
    vec2 min = box.min;

    std::vector<float> vertices(segments.size() + path.closed() ? 1 : 2);
    std::vector<Edge> edges(segments.size() + path.closed() ? 0 : 1);

    for (const auto& segment : segments) {
      vec2 p0 = segment.p0() - min;
      vec2 p3 = segment.p3() - min;

      vertices.push_back(p0.y);

      if (!Math::is_almost_equal(p0.y, p3.y)) {
        if (p0.x >= p3.y) {
          edges.push_back({ p3, p0, std::min(p0.y, p3.y), std::max(p0.y, p3.y) });
        } else {
          edges.push_back({ p0, p3, std::min(p0.y, p3.y), std::max(p0.y, p3.y) });
        }
      }
    }

    vertices.push_back(std::numeric_limits<float>::max());
    if (!path.closed()) {
      vec2 p0 = segments.back().p3() - min;
      vec2 p3 = segments.front().p0() - min;

      vertices.push_back(p0.y);

      if (!Math::is_almost_equal(p0.y, p3.y)) {
        edges.push_back({ p0, p3, std::min(p0.y, p3.y), std::max(p0.y, p3.y) });
      }
    }

    std::sort(vertices.begin(), vertices.end());

    uint8_t color = 255;
    int vertex_index = 0;
    bool needs_recalculation = true;

    std::vector<int> active_edges{};
    std::vector<int> possible_edges(edges.size());

    std::iota(possible_edges.begin(), possible_edges.end(), 0);

    for (int y = 0; y < size.y; y++) {
      while (y >= vertices[vertex_index]) {
        needs_recalculation = true;
        vertex_index++;
      }

      float y_float = (float)y + 0.5f;

      if (needs_recalculation) {
        needs_recalculation = false;
        recalculate_edges(y_float, edges, possible_edges, active_edges);
      }

      // precompute intersections
      // std::vector<float> intersections(active_edges.size());
      // for (int i = 0; i < intersections.size(); i++) {
      //   const Edge& edge = edges[active_edges[i]];

      //   if (is_in_span(y_float, edge.min_y, edge.max_y)) {
      //     float t = (y_float - edge.p0.y) / (edge.p3.y - edge.p0.y);
      //     float x0 = edge.p0.x + t * (edge.p3.x - edge.p0.x);
      //     intersections[i] = x0;
      //   } else {
      //     intersections[i] = std::numeric_limits<float>::max();
      //   }
      // }

      for (int x = 0; x < size.x; x++) {
        int index = y * size.x + x;
        float alpha = 0.0f;

        for (int i : active_edges) {
          const Edge& edge = edges[i];

          float px0 = (float)x;
          float px1 = (float)x + 1.0f;
          float py0 = (float)y;
          float py1 = (float)y + 1.0f;


        }

        buffer[index] = uint8_t(alpha * 255.0f);
      }
    }
  }

#else

  static int to_fixed(float value) {
    return (int)(value * 256.0f);
  }

  static int trunc(int value) {
    return value >> PIXEL_BITS;
  }

  static int fract(int value) {
    return value & (ONE_PIXEL - 1);
  }

  static int udiv(int a, int b) {
    return (int)(((uint64_t)a * (uint64_t)b) >> (4 * 8 - PIXEL_BITS));
  }

  static ivec2 into(vec2 v) {
    return {
      to_fixed(v.x),
      to_fixed(v.y)
    };
  }

  static uint8_t coverage(Fill fill, int coverage) {
    coverage >>= PIXEL_BITS * 2 + 1 - 8;

    if (fill == Fill::EvenOdd) {
      coverage &= 511;
      if (coverage >= 256) {
        coverage = 511 - coverage;
      }
    } else {
      if (coverage < 0) {
        coverage = std::abs(coverage) - 1;
      }
      if (coverage >= 256) {
        coverage = 255;
      }
    }

    return (uint8_t)coverage;
  }

  // TODO: maybe create a lightweight intermediate representation of path
  void Rasterizer::rasterize(vec2 shift, ivec2 size, const Geometry::Path& path, uint8_t* buffer) {
    m_storage.reset({ 0, 0 }, size);

    m_xmin = 0;
    m_xmax = size.x;
    m_ymin = 0;
    m_ymax = size.y;
    m_height = size.y;

    m_x = 0;
    m_y = 0;
    m_px = 0;
    m_py = 0;
    m_cover = 0;
    m_area = 0;

    m_shift = shift;
    m_current = { 0, 0 };
    m_start = into(path.bounding_rect().min);

    m_closed = path.closed();
    m_invalid = true;

    // TEMP: just render a pixel grid
    // for (int y = 0; y < size.y; y++) {
    //   for (int x = 0; x < size.x; x++) {
    //     int index = y * size.x + x;
    //     buffer[index] = (x + y) % 2 == 0 ? 255 : 50;
    //   }
    // }

    move_to(into(path.segments().front().p0()));

    for (const auto& segment : path.segments()) {
      line_to(into(segment.p3()));
    }

    if (!m_closed) {
      line_to(m_start);
    }
    if (m_invalid) {
      m_storage.set(m_x, m_y, m_area, m_cover);
    }

    std::vector<int>& indices = m_storage.indices;
    std::vector<Cell>& cells = m_storage.cells;

    ivec2 min = { m_xmin, m_ymin };
    ivec2 max = { m_xmax, m_ymax };

    for (int i = 0; i < indices.size(); i++) {
      int index = indices[i];

      if (index != -1) {
        int y = i - min.y;
        int row_offset = size.x * (m_height - 1 - y);

        uint8_t* row = buffer + row_offset;

        int x = min.x;
        int cover = 0;
        int area;

        while (true) {
          Cell& cell = cells[index];

          if (cover != 0 && cell.x > x) {
            int count = cell.x - x;
            int xi = x;

            uint8_t c = coverage(Fill::NonZero, cover);

            for (int b = xi; b < xi + count; b++) {
              *(row + b) = c;
            }
          }

          cover = cover + cell.cover * (ONE_PIXEL * 2);
          area = cover - cell.area;

          if (area != 0 && cell.x >= min.x) {
            int count = 1;
            int xi = cell.x;

            uint8_t c = coverage(Fill::NonZero, area);

            for (int b = xi; b < xi + count; b++) {
              *(row + b) = c;
            }
          }

          x = cell.x + 1;
          index = cell.next;

          if (index == -1) break;
        }

        if (cover != 0) {
          int count = max.x - x;
          int xi = x;

          uint8_t c = coverage(Fill::NonZero, cover);

          for (int b = xi; b < xi + count; b++) {
            *(row + b) = c;
          }
        }
      }
    }
  }

  void Rasterizer::set_cell(int x, int y) {
    if (!m_invalid && (m_area != 0 || m_cover != 0)) {
      m_storage.set(m_x, m_y, m_area, m_cover);
    }

    m_area = 0;
    m_cover = 0;
    m_x = std::max(x, m_xmin - 1);
    m_y = y;
    m_invalid = y >= m_ymax || y < m_ymin || x >= m_xmax;
  }

  void Rasterizer::move_to(ivec2 to) {
    set_cell(trunc(to.x - m_start.x), trunc(to.y - m_start.y));

    m_px = to.x;
    m_py = to.y;
  }

  void Rasterizer::line_to(ivec2 to) {
    int to_x = to.x - m_start.x;
    int to_y = to.y - m_start.y;
    int ey1 = trunc(m_py);
    int ey2 = trunc(to_y);

    if ((ey1 >= m_ymax && ey2 >= m_ymax) || (ey1 < m_ymin && ey2 < m_ymin)) {
      m_px = to_x;
      m_py = to_y;
      return;
    }

    int ex1 = trunc(m_px);
    int ex2 = trunc(to_x);
    int fx1 = fract(m_px);
    int fy1 = fract(m_py);
    int dx = to_x - m_px;
    int dy = to_y - m_py;

    if (ex1 == ex2 && ey1 == ey2) {
      // empty
    } else if (dy == 0) {
      set_cell(ex2, ey2);
      m_px = to_x;
      m_py = to_y;
      return;
    } else if (dx == 0) {
      if (dy > 0) {
        while (true) {
          int fy2 = ONE_PIXEL;

          m_cover += fy2 - fy1;
          m_area += (fy2 - fy1) * fx1 * 2;

          fy1 = 0;
          ey1 += 1;

          set_cell(ex1, ey1);

          if (ey1 == ey2) break;
        }
      } else {
        while (true) {
          int fy2 = 0;
          m_cover += fy2 - fy1;
          m_area += (fy2 - fy1) * fx1 * 2;

          fy1 = ONE_PIXEL;
          ey1 -= 1;

          set_cell(ex1, ey1);

          if (ey1 == ey2) break;
        }
      }
    } else {
      int prod = dx * fy1 - dy * fx1;
      int dx_r = ex1 != ex2 ? 0x00FFFFFF / dx : 0;
      int dy_r = ey1 != ey2 ? 0x00FFFFFF / dy : 0;

      while (true) {
        if (prod <= 0 && prod - dx * ONE_PIXEL > 0) {
          int fx2 = 0;
          int fy2 = udiv(-prod, -dx_r);

          prod -= dy * ONE_PIXEL;
          m_cover += fy2 - fy1;
          m_area += (fy2 - fy1) * (fx1 + fx2);
          fx1 = ONE_PIXEL;
          fy1 = fy2;
          ex1 -= 1;
        } else if (prod - dx * ONE_PIXEL <= 0 && prod - dx * ONE_PIXEL + dy * ONE_PIXEL > 0) {
          prod -= dx * ONE_PIXEL;

          int fx2 = udiv(-prod, dy_r);
          int fy2 = ONE_PIXEL;

          m_cover += fy2 - fy1;
          m_area += (fy2 - fy1) * (fx1 + fx2);

          fx1 = fx2;
          fy1 = 0;
          ey1 += 1;
        } else if (prod - dx * ONE_PIXEL + dy * ONE_PIXEL <= 0 && prod + dy * ONE_PIXEL >= 0) {
          prod += dy * ONE_PIXEL;

          int fx2 = ONE_PIXEL;
          int fy2 = udiv(prod, dx_r);

          m_cover += fy2 - fy1;
          m_area += (fy2 - fy1) * (fx1 + fx2);

          fx1 = 0;
          fy1 = fy2;
          ex1 += 1;
        } else {
          int fx2 = udiv(prod, -dy_r);
          int fy2 = 0;

          prod += dx * ONE_PIXEL;

          m_cover += fy2 - fy1;
          m_area += (fy2 - fy1) * (fx1 + fx2);

          fx1 = fx2;
          fy1 = ONE_PIXEL;
          ey1 -= 1;
        }

        set_cell(ex1, ey1);
        if (ex1 == ex2 && ey1 == ey2) break;
      }
    }

    int fx2 = fract(to_x);
    int fy2 = fract(to_y);

    m_cover += fy2 - fy1;
    m_area += (fy2 - fy1) * (fx1 + fx2);
    m_px = to_x;
    m_py = to_y;
  }

  void Rasterizer::RasterStorage::reset(ivec2 min, ivec2 max) {
    this->min = min;
    this->max = max;

    cells.clear();
    indices.clear();

    indices.resize(max.y - min.y, -1);
  }

  void Rasterizer::RasterStorage::set(int x, int y, int area, int cover) {
    int y_index = y - min.y;
    int cell_index = indices[y_index];
    int last_index = -1;

    while (cell_index != -1) {
      Cell& cell = cells[cell_index];

      if (cell.x > x) {
        break;
      } else if (cell.x == x) {
        cell.area += area;
        cell.cover += cover;
        return;
      }

      last_index = cell_index;
      cell_index = cell.next;
    }

    int new_index = (int)cells.size();
    Cell cell = { x, area, cover, cell_index };

    if (last_index != -1) {
      cells[last_index].next = new_index;
    } else {
      indices[y_index] = new_index;
    }

    cells.push_back(cell);
  }

#endif

}
