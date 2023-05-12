#pragma once

#include "segment.h"
#include "geometry.h"

#include "../../math/vec4.h"

namespace Graphick::Render::Geometry {

  class Path {
  public:
    inline bool empty() const { return m_segments.empty(); }
    inline const std::vector<Segment>& segments() const { return m_segments; }

    void move_to(vec2 p);
    void line_to(vec2 p);
    void quadratic_to(vec2 p1, vec2 p2);
    void cubic_to(vec2 p1, vec2 p2, vec2 p3);
    void arc_to(vec2 c, vec2 radius, float x_axis_rotation, bool large_arc_flag, bool sweep_flag, vec2 p);

    void ellipse(vec2 c, vec2 radius);
    void circle(vec2 c, float radius);
    void rect(vec2 p, vec2 size, bool centered = false);
    void round_rect(vec2 p, vec2 size, float radius, bool centered = false);

    void close();

    Box bounding_box() const;

    // TEMP
    Temp::Geo outline_geo() const;
  private:
    bool m_closed = false;
    Segment::Point m_last_point;
    std::vector<Segment> m_segments;
  };

}
