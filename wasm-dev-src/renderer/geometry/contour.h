#pragma once

#include "../../math/f8x8.h"
#include "../../math/f24x8.h"
#include "../../math/dvec2.h"

#include "../properties.h"

#include <vector>

namespace Graphick::Renderer::Geometry {

  struct Contour {
    std::vector<f24x8x2> points;

    void move_to(const f24x8x2 p0);
    // TODO: test const reference vs value
    void move_to(const dvec2 p0);

    void line_to(const f24x8x2 p3);
    void line_to(const dvec2 p3);

    void cubic_to(const f24x8x2 p1, const f24x8x2 p2, const f24x8x2 p3);
    void cubic_to(const dvec2 p1, const dvec2 p2, const dvec2 p3);

    void add_cap(const dvec2 from, const dvec2 to, const dvec2 n, const double radius, const LineCap cap);
    // void offset_segment(const f24x8x2 p3, const f24x8 radius);
    // void offset_segment(const f24x8x2 p1, const f24x8x2 p2, const f24x8x2 p3, const f24x8 radius);

    void close();
    void reverse();
  private:
    void arc(const dvec2 from, const double radius, const dvec2 to);
  private:
    f24x8x2 m_p0 = { 0, 0 };
    dvec2 m_d_p0 = { 0, 0 };
  };

}
