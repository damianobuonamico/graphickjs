#pragma once

#include "control_point.h"

#include "../../math/rect.h"
#include "../../math/mat2x3.h"

#include <vector>
#include <memory>
#include <unordered_set>
#include <optional>

namespace Graphick::History {
  class CreateHandleCommand;
  class RemoveHandleCommand;
  class InsertInSegmentsVectorCommand;
  class EraseFromSegmentsVectorCommand;
}

namespace Graphick::Renderer::Geometry {

  class Segment {
  public:
    using ControlPointVertex = std::shared_ptr<ControlPoint>;
    using ControlPointHandle = std::shared_ptr<History::Vec2Value>;
  public:
    enum class Kind {
      // A line segment.
      Linear,
      // A quadratic bezier segment.
      Quadratic,
      // A cubic bezier segment.
      Cubic
    };

    struct SegmentPointDistance {
      float t;
      vec2 point;
      float sq_distance;
    };
  public:
    Segment(vec2 p0, vec2 p3);
    Segment(vec2 p0, vec2 p1, vec2 p3, bool is_quadratic = true, bool is_p1 = true);
    Segment(vec2 p0, vec2 p1, vec2 p2, vec2 p3);

    Segment(ControlPointVertex p0, ControlPointVertex p3);
    Segment(ControlPointVertex p0, vec2 p, ControlPointVertex p3, bool is_quadratic = true, bool is_p1 = true);
    Segment(ControlPointVertex p0, vec2 p1, vec2 p2, ControlPointVertex p3);

    Segment(ControlPointVertex p0, std::optional<vec2> p1, std::optional<vec2> p2, ControlPointVertex p3);

    Segment(const Segment& other);
    Segment(Segment&& other) noexcept;

    Segment& operator=(const Segment& other);
    Segment& operator=(Segment&& other) noexcept;

    bool operator==(const Segment& other) const;

    inline Kind kind() const { return m_kind; }
    inline bool is_linear() const { return m_kind == Kind::Linear; }
    inline bool is_quadratic() const { return m_kind == Kind::Quadratic; }
    inline bool is_cubic() const { return m_kind == Kind::Cubic; }

    inline vec2 p0() const { return m_p0->get(); }
    inline vec2 p1() const { return m_p1 ? m_p1->get() : p0(); }
    inline vec2 p2() const { return m_p2 ? m_p2->get() : p3(); }
    inline vec2 p3() const { return m_p3->get(); }

    inline bool has_p1() const { return m_p1 != nullptr; }
    inline bool has_p2() const { return m_p2 != nullptr; }

    inline std::weak_ptr<ControlPoint> p0_ptr() const { return m_p0; }
    inline std::weak_ptr<History::Vec2Value> p1_ptr() const { return m_p1; }
    inline std::weak_ptr<History::Vec2Value> p2_ptr() const { return m_p2; }
    inline std::weak_ptr<ControlPoint> p3_ptr() const { return m_p3; }

    inline uuid p0_id() const { return m_p0->id; }
    inline uuid p3_id() const { return m_p3->id; }

    vec2 get(const float t) const;

    rect bounding_rect() const;
    rect bounding_rect(const mat2x3& transform) const;
    rect approx_bounding_rect() const;
    vec2 size() const;

    SegmentPointDistance closest_to(const vec2 position, int iterations = 4) const;

    bool is_inside(const vec2 position, bool deep_search = false, vec2 threshold = vec2{ 0.0f }) const;
    bool intersects(const Math::rect& rect) const;
    bool intersects(const Math::rect& rect, const mat2x3& transform) const;
    bool intersects(const Math::rect& rect, const bool found, std::unordered_set<uuid>& vertices) const;
    bool intersects(const Math::rect& rect, const bool found, const mat2x3& transform, std::unordered_set<uuid>& vertices) const;
    bool intersects_line(const Math::rect& line) const;
    bool intersects_line(const Math::rect& line, const mat2x3& transform) const;

    void create_p1(const vec2 position);
    void create_p2(const vec2 position);
    void remove_p1();
    void remove_p2();

    const std::vector<float>& parameterize(const double zoom) const;

    bool is_masquerading_linear() const;
    bool is_masquerading_quadratic(vec2& new_p1) const;

    bool rehydrate_cache() const;
  public:
    static std::shared_ptr<Segment> reverse(const Segment& segment);
    static void transform(Segment& segment, const mat2x3& matrix, bool transform_p3 = true);
  private:
    void recalculate_kind();

    vec2 linear_get(const float t) const;
    vec2 quadratic_get(const float t) const;
    vec2 cubic_get(const float t) const;

    std::vector<vec2> extrema() const;
    std::vector<vec2> extrema(const mat2x3& transform) const;

    std::vector<float> linear_extrema() const;
    std::vector<float> linear_extrema(const mat2x3& transform) const;
    std::vector<float> quadratic_extrema() const;
    std::vector<float> quadratic_extrema(const mat2x3& transform) const;
    std::vector<float> cubic_extrema() const;
    std::vector<float> cubic_extrema(const mat2x3& transform) const;

    SegmentPointDistance linear_closest_to(const vec2 position, int iterations = 4) const;
    SegmentPointDistance quadratic_closest_to(const vec2 position, int iterations = 4) const;
    SegmentPointDistance cubic_closest_to(const vec2 position, int iterations = 4) const;

    std::optional<std::vector<vec2>> line_intersection_points(const rect& line, const float threshold = 1e-5f) const;
    std::optional<std::vector<vec2>> line_intersection_points(const rect& line, const mat2x3& transform, const float threshold = 1e-5f) const;
    std::vector<float> line_intersections(const rect& line) const;
    std::vector<float> line_intersections(const rect& line, const mat2x3& transform) const;

    std::vector<float> linear_line_intersections(const rect& line) const;
    std::vector<float> linear_line_intersections(const rect& line, const mat2x3& transform) const;
    std::vector<float> quadratic_line_intersections(const rect& line) const;
    std::vector<float> quadratic_line_intersections(const rect& line, const mat2x3& transform) const;
    std::vector<float> cubic_line_intersections(const rect& line) const;
    std::vector<float> cubic_line_intersections(const rect& line, const mat2x3& transform) const;
  private:
    // The type of segment: linear, quadratic, or cubic bezier.
    Kind m_kind;
    // The start, end, and control points of the segment.
    //
    // For a line segment, p1 and p2 are ignored. For a quadratic bezier, only p1 is used. 
    ControlPointVertex m_p0 = nullptr;
    ControlPointHandle m_p1 = nullptr;
    ControlPointHandle m_p2 = nullptr;
    ControlPointVertex m_p3 = nullptr;

    mutable int m_hash = 0;

    mutable std::optional<Math::rect> m_bounding_rect_cache = std::nullopt;
    mutable std::optional<std::vector<float>> m_parameterization = std::nullopt;
  private:
    // TEMP: remove
    friend class Path;
    friend class History::CreateHandleCommand;
    friend class History::RemoveHandleCommand;
    friend class History::InsertInSegmentsVectorCommand;
    friend class History::EraseFromSegmentsVectorCommand;
  };

}
