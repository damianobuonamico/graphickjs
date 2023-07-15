#pragma once

#include "../math/vec2.h"
#include "../math/ivec2.h"

namespace Graphick::Renderer {

  namespace Geometry {
    class Path;
  }

  class Mask {
  public:
    vec2 offset = { 0, 0 };
    vec2 render_offset = { 0, 0 };
    vec2 bounds_offset = { 0, 0 };
  public:
    Mask(const Geometry::Path& path);
    ~Mask();

    inline ivec2 size() const { return m_size; }
    inline uint8_t* data() const { return m_buffer; }
  private:
    void ensure_size(const Geometry::Path& path);
    void render(const Geometry::Path& path);
  private:
    ivec2 m_size = { 0, 0 };

    uint8_t* m_buffer = nullptr;
  };

}
