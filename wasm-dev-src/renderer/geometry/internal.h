#pragma once 

#include "../../math/vec2.h"
#include "../../math/rect.h"

#include <vector>
#include <memory>

namespace Graphick::Renderer::Geometry::Internal {

  class PathInternal;

  class SegmentInternal {
  public:
    using ControlPoint = std::shared_ptr<vec2>;
  public:
    enum class Kind {
      // A line segment
      Linear,
      // A cubic bezier segment
      Cubic
    };
  public:
    SegmentInternal(vec2 p0, vec2 p3);
    SegmentInternal(vec2 p0, vec2 p1, vec2 p3, bool is_p1 = true);
    SegmentInternal(vec2 p0, vec2 p1, vec2 p2, vec2 p3);

    SegmentInternal(ControlPoint p0, ControlPoint p3);
    SegmentInternal(ControlPoint p0, vec2 p1, ControlPoint p3, bool is_p1 = true);
    SegmentInternal(ControlPoint p0, vec2 p1, vec2 p2, ControlPoint p3);

    SegmentInternal(const SegmentInternal& other);
    SegmentInternal(SegmentInternal&& other) noexcept;

    SegmentInternal& operator=(const SegmentInternal& other);
    SegmentInternal& operator=(SegmentInternal&& other) noexcept;

    inline Kind kind() const { return m_kind; }
    inline bool is_linear() const { return m_kind == Kind::Linear; }
    inline bool is_cubic() const { return m_kind == Kind::Cubic; }

    inline vec2 p0() const { return *m_p0; }
    inline vec2 p1() const { return m_p1 ? *m_p1 : p0(); }
    inline vec2 p2() const { return m_p2 ? *m_p2 : p3(); }
    inline vec2 p3() const { return *m_p3; }

    inline bool has_p1() const { return m_p1 != nullptr; }
    inline bool has_p2() const { return m_p2 != nullptr; }

    inline std::weak_ptr<vec2> p0_ptr() const { return m_p0; }
    inline std::weak_ptr<vec2> p1_ptr() const { return m_p1; }
    inline std::weak_ptr<vec2> p2_ptr() const { return m_p2; }
    inline std::weak_ptr<vec2> p3_ptr() const { return m_p3; }

    vec2 get(const float t) const;

    rect bounding_rect() const;
    vec2 size() const;
  private:
    vec2 linear_get(const float t) const;
    vec2 cubic_get(const float t) const;

    std::vector<vec2> extrema() const;

    std::vector<float> linear_extrema() const;
    std::vector<float> cubic_extrema() const;
  private:
    // The type of segment: linear, quadratic, or cubic bezier.
    Kind m_kind;
    // The start, end, and control points of the segment.
    //
    // For a line segment, p1 and p2 are ignored. For a quadratic bezier, only p1 is used. 
    ControlPoint m_p0;
    ControlPoint m_p1;
    ControlPoint m_p2;
    ControlPoint m_p3;
  private:
    friend class PathInternal;
  };

  class PathInternal {
  public:
    inline bool empty() const { return m_segments.empty(); }
    inline bool closed() const { return m_closed; }
    inline const std::vector<SegmentInternal>& segments() const { return m_segments; }

    void move_to(vec2 p);
    void line_to(vec2 p);
    void cubic_to(vec2 p1, vec2 p2, vec2 p3);
    void cubic_to(vec2 p, vec2 p3, bool is_p1 = true);

    void ellipse(vec2 c, vec2 radius);
    void circle(vec2 c, float radius);
    void rect(vec2 p, vec2 size, bool centered = false);
    void round_rect(vec2 p, vec2 size, float radius, bool centered = false);

    void close();

    Math::rect bounding_rect() const;
  private:
    bool m_closed = false;
    SegmentInternal::ControlPoint m_last_point;
    std::vector<SegmentInternal> m_segments;
  };


}