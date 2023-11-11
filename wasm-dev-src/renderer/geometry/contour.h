#pragma once

#include "../../math/f8x8.h"
#include "../../math/f24x8.h"

#include <vector>

namespace Graphick::Renderer::Geometry {

#ifdef USE_F8x8
  struct Contour {
    std::vector<f24x8x2> points;

    void begin(const f24x8x2 p0, const bool push = true);

    void push_segment(const f24x8x2 p3);
    void push_segment(const f24x8x2 p1, const f24x8x2 p2, const f24x8x2 p3);

    void offset_segment(const f24x8x2 p3, const f24x8 radius);
    void offset_segment(const f24x8x2 p1, const f24x8x2 p2, const f24x8x2 p3, const f24x8 radius);

    void close();
  private:
    f24x8x2 m_p0 = 0;
  };
#else
  struct Contour {
    std::vector<vec2> points;

    void begin(const vec2 p0, const bool push = true);

    void push_segment(const vec2 p3);
    void push_segment(const vec2 p1, const vec2 p2, const vec2 p3);

    void offset_segment(const vec2 p3, const float radius);
    void offset_segment(const vec2 p1, const vec2 p2, const vec2 p3, const float radius);

    void close();
  private:
    vec2 m_p0;
  };
#endif

}
