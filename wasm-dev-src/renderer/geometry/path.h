#pragma once

#include "segment.h"

#include "../../history/values.h"

#include <unordered_set>

namespace Graphick::History {
  class InsertInSegmentsVectorCommand;
  class EraseFromSegmentsVectorCommand;
}

namespace Graphick::Renderer::Geometry {

  class Path {
  public:
    const uuid id;
  public:
    class SegmentsVector {
    public:
      using iterator = typename std::vector<std::shared_ptr<Segment>>::iterator;
      using const_iterator = typename std::vector<std::shared_ptr<Segment>> ::const_iterator;
    public:
      SegmentsVector(Path* path, const std::vector<std::shared_ptr<Segment>>& vector) : m_value(vector), m_path(path) {}
      SegmentsVector(Path* path, const SegmentsVector& vector) : m_value(vector.m_value), m_path(path ? path : vector.m_path) {}
      SegmentsVector(Path* path) : m_path(path) {};

      ~SegmentsVector() = default;

      inline void set_path(Path* path) { m_path = path; }

      inline iterator begin() { return m_value.begin(); }
      inline iterator end() { return m_value.end(); }
      inline const_iterator begin() const { return m_value.begin(); }
      inline const_iterator end() const { return m_value.end(); }

      inline size_t size() const { return m_value.size(); }
      inline bool empty() const { return m_value.empty(); }

      inline Segment& operator[](size_t index) { return *m_value[index]; }
      inline Segment& front() { return *m_value.front(); }
      inline Segment& back() { return *m_value.back(); }
      inline const Segment& front() const { return *m_value.front(); }
      inline const Segment& back() const { return *m_value.back(); }

      void push_back(const std::shared_ptr<Segment>& value);
      void insert(const std::shared_ptr<Segment>& value, int index);
      void erase(int index);
    private:
      std::vector<std::shared_ptr<Segment>> m_value;
      Path* m_path;
    };

    struct RelativeHandles {
      History::Vec2Value* in_handle = nullptr;
      History::Vec2Value* out_handle = nullptr;

      Segment* in_segment = nullptr;
      Segment* out_segment = nullptr;
    };
  public:
    Path(const uuid id);
    Path(const uuid id, const Path& path);
    Path(const Path& path);
    Path(Path&& path) noexcept;

    inline bool vacant() const { return m_segments.empty() && m_last_point == nullptr; }
    inline bool empty() const { return m_segments.empty(); }
    inline bool closed() const { return m_closed; }
    inline bool reversed() const { return m_reversed; }

    inline const std::weak_ptr<ControlPoint> last() const { return m_last_point; }
    inline const SegmentsVector& segments() const { return m_segments; }
    inline SegmentsVector& segments() { return m_segments; }

    const std::vector<ControlPoint*> vertices() const;
    const std::vector<uuid> vertices_ids() const;

    std::optional<Segment::ControlPointHandle> in_handle_ptr() const;
    std::optional<Segment::ControlPointHandle> out_handle_ptr() const;

    RelativeHandles relative_handles(const uuid id);

    bool is_open_end(const uuid id) const;

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
    void reverse(bool reversed = true);
    std::optional<std::weak_ptr<ControlPoint>> split(Segment& segment, float t);

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
    History::BoolValue m_reversed = false;
    bool m_closed = false;

    Segment::ControlPointVertex m_last_point = nullptr;
    SegmentsVector m_segments;

    Segment::ControlPointHandle m_in_handle;
    Segment::ControlPointHandle m_out_handle;

    mutable int m_hash = 0;

    mutable std::optional<Math::rect> m_bounding_rect_cache = std::nullopt;
    mutable std::optional<Math::rect> m_approx_bounding_rect_cache = std::nullopt;
  private:
    friend class History::InsertInSegmentsVectorCommand;
    friend class History::EraseFromSegmentsVectorCommand;
  };

}
