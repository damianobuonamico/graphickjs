#pragma once

#include "../entity.h"
#include "../../input/wobble_smoother.h"
#include "../../../renderer/geometry/stroker.h"

#include <vector>

class FreehandEntity: public Entity {
public:
  FreehandEntity(const vec2& position, float pressure, double time)
    : m_position(position), m_points({ {m_position, pressure} }) {
    console::log("FreehandEntity created");
    WobbleSmoother::reset(m_position, time);
  };
  FreehandEntity(const FreehandEntity&) = default;
  FreehandEntity(FreehandEntity&&) = default;

  ~FreehandEntity() {
    console::log("FreehandEntity destroyed");
  }

  void add_point(const vec2& position, float pressure, double time) {
    vec2 smoothed_position = WobbleSmoother::update(m_position + position, time);

    m_points[m_points.size() - 1].position = smoothed_position;
    m_points.push_back({ m_position + position, pressure });
  }

  virtual void render(float zoom) const override {
    std::vector<FreehandPathPoint> points = smooth_freehand_path(m_points, 4);
    Geometry geometry = stroke_freehand_path(points, 8.0f, zoom);

    Renderer::get()->draw(geometry);
  };
private:
  vec2 m_position;
  std::vector<FreehandPathPoint> m_points;
};
