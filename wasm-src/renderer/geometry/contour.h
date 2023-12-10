#pragma once

#include "../../math/f8x8.h"
#include "../../math/f24x8.h"
#include "../../math/dvec2.h"

#include "../../utils/defines.h"

#include "../properties.h"

#include <vector>
#include <memory>

namespace Graphick::Renderer::Geometry {

  class Contour {
  public:
    using Parameterization = std::vector<std::pair<dvec2, dvec2>>;
  public:
    Contour(const double tolerance = GK_PATH_TOLERANCE) : m_tolerance(tolerance) {}

    std::vector<f24x8x2> points;

    void move_to(const f24x8x2 p0);
    // TODO: test const reference vs value
    void move_to(const dvec2 p0);

    void line_to(const f24x8x2 p3);
    void line_to(const dvec2 p3);

    void cubic_to(const f24x8x2 p1, const f24x8x2 p2, const f24x8x2 p3);
    void cubic_to(const dvec2 p1, const dvec2 p2, const dvec2 p3);

    std::unique_ptr<Parameterization> offset_cubic(const dvec2 p0, const dvec2 p1, const dvec2 p2, const dvec2 p3, const dvec2 end_normal, const double radius);
    void offset_cubic(const Parameterization& parameterization, const dvec2 end_point, const double radius);

    void add_cap(const dvec2 from, const dvec2 to, const dvec2 n, const double radius, const LineCap cap);
    void add_join(const dvec2 from, const dvec2 to, const dvec2 pivot, const dvec2 from_normal, const dvec2 to_normal, const double radius, const double inv_miter_limit, LineJoin join);
    // void offset_segment(const f24x8x2 p3, const f24x8 radius);
    // void offset_segment(const f24x8x2 p1, const f24x8x2 p2, const f24x8x2 p3, const f24x8 radius);

    void close();
    void reverse();

    int winding_of(const f24x8x2 point);
  private:
    void arc(const dvec2 center, const dvec2 from, const double radius, const dvec2 to);
    void recursive_cubic_offset(const dvec2 p0, const dvec2 p1, const dvec2 p2, const dvec2 p3, const unsigned int level, const double angular_tolerance, Parameterization& parameterization);
  private:
    f24x8x2 m_p0 = { 0, 0 };
    dvec2 m_d_p0 = { 0, 0 };
    double m_tolerance;
  };

}
