#include "mask.h"

#include "rasterizer.h"

#include "geometry/path.h"

namespace Graphick::Renderer {

  Mask::Mask(const Geometry::Path& path) {
    ensure_size(path);

    m_buffer = new uint8_t[m_size.x * m_size.y];
    std::fill(m_buffer, m_buffer + m_size.x * m_size.y, 0);

    render(path);
  }

  Mask::~Mask() {
    delete[] m_buffer;
  }

  void Mask::ensure_size(const Geometry::Path& path) {
    rect bounds = path.bounding_rect();
    vec2 size = bounds.size();

    m_size = {
      (int)std::ceil(size.x),
      (int)std::ceil(size.y)
    };
  }

  void Mask::render(const Geometry::Path& path) {
    vec2 shift = offset + render_offset;
    Rasterizer rasterizer;

    rasterizer.rasterize(shift, m_size, path, m_buffer);
  }

}
