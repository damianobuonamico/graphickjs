#pragma once

#include "segment.h"

#include <unordered_set>

namespace Graphick::Renderer::Geometry {

  class Path {
  public:
    inline bool vacant() const { return m_segments.empty() && m_last_point == nullptr; }
    inline bool empty() const { return m_segments.empty(); }
    inline bool closed() const { return m_closed; }

    inline const std::weak_ptr<ControlPoint> last() const { return m_last_point; }
    inline const std::vector<Segment>& segments() const { return m_segments; }
    inline std::vector<Segment>& segments() { return m_segments; }

    const std::vector<ControlPoint*> vertices() const;
    const std::vector<uuid> vertices_ids() const;

    inline std::weak_ptr<History::Vec2Value> in_handle_ptr() const { return m_in_handle; }
    inline std::weak_ptr<History::Vec2Value> out_handle_ptr() const { return m_out_handle; }

    void move_to(vec2 p);
    void line_to(vec2 p);
    void quadratic_to(vec2 p1, vec2 p2);
    void cubic_to(vec2 p1, vec2 p2, vec2 p3);
    void cubic_to(vec2 p, vec2 p3, bool is_p1 = true);
    void arc_to(vec2 c, vec2 radius, float x_axis_rotation, bool large_arc_flag, bool sweep_flag, vec2 p);

    void ellipse(vec2 c, vec2 radius);
    void circle(vec2 c, float radius);
    void rect(vec2 p, vec2 size, bool centered = false);
    void round_rect(vec2 p, vec2 size, float radius, bool centered = false);

    void close();

    Math::rect bounding_rect() const;
    Math::rect approx_bounding_rect() const;
    Math::rect large_bounding_rect() const;

    bool is_inside(const vec2 position, bool deep_search = false, float threshold = 0.0f) const;
    bool intersects(const Math::rect& rect) const;
    bool intersects(const Math::rect& rect, std::unordered_set<uuid>& vertices) const;

    void create_in_handle(const vec2 position);
    void create_out_handle(const vec2 position);
    void clear_in_handle();
    void clear_out_handle();

    void rehydrate_cache() const;
  private:
    bool m_closed = false;
    Segment::ControlPointVertex m_last_point;
    std::vector<Segment> m_segments;

    Segment::ControlPointHandle m_in_handle = nullptr;
    Segment::ControlPointHandle m_out_handle = nullptr;

    mutable int m_hash = 0;

    mutable std::optional<Math::rect> m_bounding_rect_cache = std::nullopt;
    mutable std::optional<Math::rect> m_approx_bounding_rect_cache = std::nullopt;
  };

}
