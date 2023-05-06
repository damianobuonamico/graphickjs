#pragma once

#include "../../math/box.h"

#include "../../values/vec2_value.h"

namespace Graphick::Render::Geometry {

  class Segment {
  public:
    using Point = std::shared_ptr<Vec2Value>;
    using ControlPoint = std::unique_ptr<Vec2Value>;
  public:
    enum class Kind {
      // A line segment.
      Linear,
      // A quadratic bezier segment.
      Quadratic,
      // A cubic bezier segment.
      Cubic
    };
  public:
    Segment(vec2 p0, vec2 p3);
    Segment(vec2 p0, vec2 p1, vec2 p3, bool is_quadratic = true);
    Segment(vec2 p0, vec2 p1, vec2 p2, vec2 p3);

    Segment(Point p0, Point p3);
    Segment(Point p0, vec2 p1, Point p3, bool is_quadratic = true);
    Segment(Point p0, vec2 p1, vec2 p2, Point p3);

    Segment(const Segment& other);

    inline Kind kind() const { return m_kind; }
    inline bool is_linear() const { return m_kind == Kind::Linear; }
    inline bool is_quadratic() const { return m_kind == Kind::Quadratic; }
    inline bool is_cubic() const { return m_kind == Kind::Cubic; }

    inline vec2 p0() const { return m_p0->get(); }
    inline vec2 p1() const { return m_p1 ? m_p1->get() : p0(); }
    inline vec2 p2() const { return m_p2 ? m_p2->get() : p3(); }
    inline vec2 p3() const { return m_p3->get(); }

    vec2 get(const float t) const;

    Box bounding_box() const;
    Box large_bounding_box() const;
    vec2 size() const;
  private:
    bool is_masquerading_linear() const;
    bool is_masquerading_quadratic(vec2& new_p1) const;

    vec2 linear_get(const float t) const;
    vec2 quadratic_get(const float t) const;
    vec2 cubic_get(const float t) const;

    std::vector<vec2> extrema() const;

    std::vector<float> linear_extrema() const;
    std::vector<float> quadratic_extrema() const;
    std::vector<float> cubic_extrema() const;
  private:
    // The type of segment: linear, quadratic, or cubic bezier.
    Kind m_kind;
    // The start, end, and control points of the segment.
    //
    // For a line segment, p1 and p2 are ignored. For a quadratic bezier, only p1 is used. 
    Point m_p0;
    ControlPoint m_p1;
    ControlPoint m_p2;
    Point m_p3;
  private:
    // TEMP: remove
    friend class Path;
  };

}
