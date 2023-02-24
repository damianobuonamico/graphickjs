#pragma once

#include "../entity.h"
#include "../../input/wobble_smoother.h"
#include "../../../renderer/geometry/stroker.h"
// #include "../../../renderer/geometry/path_fitter.h"
#include "../../../renderer/geometry/bezier_fitter.h"
#include "../../../renderer/geometry/corners_detection.h"
#include "../../../renderer/geometry/path_simplify.h"
#include "bezier_entity.h"

#include <vector>

class FreehandEntity: public Entity {
public:
  FreehandEntity(const vec2& position, float pressure, double time)
    : m_transform(TransformComponent{ this }), m_position(position), m_points({ {m_position, pressure} }) {
    console::log("FreehandEntity created");
    WobbleSmoother::reset(m_position, time);
  };
  FreehandEntity(const FreehandEntity&) = default;
  FreehandEntity(FreehandEntity&&) = default;

  ~FreehandEntity() {
    console::log("FreehandEntity destroyed");
  }

  inline virtual TransformComponent& transform() override { return m_transform; }
  inline virtual const TransformComponent& transform() const override { return m_transform; }

  void add_point(const vec2& position, float pressure, double time);

  virtual void render(float zoom) const override;
private:
  vec2 m_position;
  std::vector<FreehandPathPoint> m_points;
  // TODO: remove mutable
  mutable std::vector<BezierEntity> m_curves;
  mutable std::vector<VertexEntity> m_vertices;

  TransformComponent m_transform;
};
