#pragma once

#include "../../math/vec2.h"

#include <vector>

namespace Graphick::Renderer::Geometry {

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

}
