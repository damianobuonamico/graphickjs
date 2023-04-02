#pragma once

#include "../entity.h"
#include "../../input/wobble_smoother.h"
#include "../../../renderer/geometry/stroker.h"
#include "../../../renderer/geometry/bezier_fitter.h"
#include "../../../renderer/geometry/path_simplify.h"
#include "../../../math/models/path_point.h"
#include "element_entity.h"
#include "bezier_entity.h"

#include <vector>
#include <memory>

class FreehandEntity: public Entity {
public:
  FreehandEntity(const vec2& position, float pressure, double time);
  FreehandEntity(const FreehandEntity&) = default;
  FreehandEntity(FreehandEntity&&) = default;

  ~FreehandEntity() {
    console::log("FreehandEntity destroyed");
  }

  inline virtual TransformComponent* transform() override { return &m_transform; }
  inline virtual const TransformComponent* transform() const override { return &m_transform; }

  void add_point(const vec2& position, float pressure, double time);

  virtual void tessellate_outline(const vec4& color, float zoom, Geometry& geo) const override;
  virtual void render(float zoom) const override;

  std::shared_ptr<ElementEntity> to_element() const;
private:
  struct CreationState {
    CreationState(const vec2& position, float pressure, double time)
      : distance(0.0f), time(time),
      points({ { position, pressure} }), can_modify(true) {}

    bool can_modify;

    float distance;
    double time;
    size_t deleted_offset = 0;

    int last_committed_curve = -1;
    int last_committed_vertex = -1;
    std::vector<PathPoint> points;
  };
private:
  vec2 m_position;
  std::vector<FreehandPathPoint> m_points;
  std::vector<float> m_pressures;
  std::vector<PathPoint> m_input_points;

  // TODO: make pointer
  CreationState m_creation_state;

  std::vector<BezierEntity*> m_curves;
  std::vector<VertexEntity*> m_vertices;
  std::vector<size_t> m_curves_indices;

  TransformComponent m_transform;
};
