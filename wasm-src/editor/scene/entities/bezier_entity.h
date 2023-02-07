#pragma once

#include "../entity.h"
#include "vertex_entity.h"

#include <vector>

// TOOD: Cache
class BezierEntity: public Entity {
public:
  enum class Type {
    Linear = 0,
    Cubic
  };
public:
  BezierEntity(VertexEntity& start, VertexEntity& end)
    : m_transform(TransformComponent{ this }), m_start(start), m_end(end) {
    console::log("BezierEntity created");
  };
  BezierEntity(const BezierEntity&) = default;
  BezierEntity(BezierEntity&&) = default;

  ~BezierEntity() {
    console::log("BezierEntity destroyed");
  }

  inline virtual TransformComponent& transform() override { return m_transform; }

  inline Type type() const {
    if (m_start.right() || m_end.left()) return Type::Cubic;
    return Type::Linear;
  }

  inline vec2 p0() const { return m_start.transform().position().get(); }
  inline vec2 p1() const {
    Vec2Value* right = m_start.transform().right();
    if (right) return p0() + right->get();
    return p0();
  }
  inline vec2 p2() const {
    Vec2Value* left = m_end.transform().left();
    if (left) return p3() + left->get();
    return p3();
  }
  inline vec2 p3() const { return m_end.transform().position().get(); }

  std::vector<vec2> extrema() const;
  Box bounding_box() const;
  vec2 size() const;

  bool clockwise(int resolution) const;

  vec2 get(float t) const;
  float closest_t_to(const vec2& position, int iterations);
  vec2 closest_point_to(const vec2& position, int iterations);
  float distance_from(const vec2& position, int iterations);

  std::vector<float> line_intersections(const Box& line);
  std::vector<vec2> line_intersection_points(const Box& line);
  bool intersects_line(const Box& line);

  std::vector<vec2> box_intersection_points(const Box& box);
  bool intersects_box(const Box& box);

  virtual void render(float zoom) override;

  virtual Entity* entity_at(const vec2& position, bool lower_level, float threshold) override;
private:
  struct BezierPointDistance {
    float t;
    vec2 point;
    float sq_distance;
  };
private:
  std::vector<float> linear_extrema() const;
  std::vector<float> cubic_extrema() const;

  vec2 linear_get(float t) const;
  vec2 cubic_get(float t) const;
  BezierPointDistance linear_closest_to(const vec2& position, int iterations);
  BezierPointDistance cubic_closest_to(const vec2& position, int iterations);

  std::vector<float> linear_line_intersections(const Box& line);
  std::vector<float> cubic_line_intersections(const Box& line);

  void linear_render(float zoom);
  void cubic_render(float zoom);
private:
  VertexEntity& m_start;
  VertexEntity& m_end;

  TransformComponent m_transform;
};
