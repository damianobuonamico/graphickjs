#pragma once

#include "../entity.h"
#include "vertex_entity.h"
#include "../../../renderer/geometry/stroker.h"

#include <vector>

// TODO: Cache
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
  BezierEntity(VertexEntity& start, VertexEntity& end, Entity* parent)
    : BezierEntity(start, end) {
    this->parent = parent;
    console::log("BezierEntity created");
  };
  BezierEntity(const BezierEntity&) = default;
  BezierEntity(BezierEntity&&) = default;

  ~BezierEntity() {
    console::log("BezierEntity destroyed");
  }

  inline virtual TransformComponent& transform() override { return m_transform; }
  inline virtual const TransformComponent& transform() const override { return m_transform; }

  inline Type type() const {
    if (m_start.right() || m_end.left()) return Type::Cubic;
    return Type::Linear;
  }

  inline VertexEntity& start() const { return m_start; }
  inline VertexEntity& end() const { return m_end; }

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
  std::vector<float> inflections() const;
  std::vector<vec2> turning_angles() const;
  std::vector<float> triangulation_params(float zoom, float facet_angle) const;
  Box bounding_box() const;
  Box large_bounding_box() const;
  vec2 size() const;

  bool clockwise(int resolution) const;

  vec2 get(float t) const;
  float closest_t_to(const vec2& position, int iterations) const;
  vec2 closest_point_to(const vec2& position, int iterations) const;
  float distance_from(const vec2& position, int iterations) const;

  std::vector<float> line_intersections(const Box& line) const;
  std::vector<vec2> line_intersection_points(const Box& line) const;
  bool intersects_line(const Box& line) const;

  std::vector<vec2> box_intersection_points(const Box& box) const;
  bool intersects_box(const Box& box) const;

  void tessellate(TessellationParams& params, Geometry& geo) const;
  virtual void render(float zoom) const override;

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

  inline std::vector<float> linear_inflections() const { return { 0.0f, 1.0f }; }
  std::vector<float> cubic_inflections() const;

  inline std::vector<vec2> linear_turning_angles() const { return { vec2{ 0.0f }, vec2{ 1.0f } }; };
  std::vector<vec2> cubic_turning_angles() const;

  vec2 cubic_t_from_theta(float theta) const;
  inline std::vector<float> linear_triangulation_params(float zoom, float facet_angle) const { return { 0.0f, 1.0f }; }
  std::vector<float> cubic_triangulation_params(float zoom, float facet_angle) const;

  vec2 linear_get(float t) const;
  vec2 cubic_get(float t) const;
  vec2 cubic_gradient(float t) const;
  vec2 cubic_curvature(float t) const;

  BezierPointDistance linear_closest_to(const vec2& position, int iterations) const;
  BezierPointDistance cubic_closest_to(const vec2& position, int iterations) const;

  std::vector<float> linear_line_intersections(const Box& line) const;
  std::vector<float> cubic_line_intersections(const Box& line) const;

  void linear_tessellate(TessellationParams& params, Geometry& geo) const;
  void cubic_tessellate(TessellationParams& params, Geometry& geo) const;

  void linear_render(float zoom) const;
  void cubic_render(float zoom) const;
private:
  VertexEntity& m_start;
  VertexEntity& m_end;

  TransformComponent m_transform;
};
