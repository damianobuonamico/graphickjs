#pragma once

#include "../entity.h"
#include "../../input/wobble_smoother.h"

#include <vector>

class FreehandEntity: public Entity {
public:
  struct FreehandPoint {
    vec2 position;
    float pressure;
  };
public:
  FreehandEntity(const vec2& position, float pressure, float time)
    : m_position(position), m_points({ {{ 0.0f, 0.0f }, pressure} }) {
    console::log("FreehandEntity created");
    WobbleSmoother::reset({ 0.0f, 0.0f }, time);
  };
  FreehandEntity(const FreehandEntity&) = default;
  FreehandEntity(FreehandEntity&&) = default;

  ~FreehandEntity() {
    console::log("FreehandEntity destroyed");
  }

  void add_point(const vec2& position, float pressure, float time) {
    vec2 smoothed_position = WobbleSmoother::update(position, time);

    m_points[m_points.size() - 1].position = smoothed_position;
    m_points.push_back({ position, pressure });
  }

  virtual void render() const override {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    unsigned int offset = 0;

    for (auto point : m_points) {
      vec2 position = m_position + point.position;

      vertices.push_back({ position - vec2{point.pressure * 1.0f, point.pressure * 1.0f} });
      vertices.push_back({ position + vec2{point.pressure * 1.0f, -point.pressure * 1.0f} });
      vertices.push_back({ position + vec2{point.pressure * 1.0f, point.pressure * 1.0f} });
      vertices.push_back({ position + vec2{-point.pressure * 1.0f, point.pressure * 1.0f} });

      indices.insert(indices.end(), { offset + 0, offset + 1, offset + 2,offset + 2, offset + 3, offset + 0 });

      offset += 4;
    }

    const Geometry geometry = { vertices, indices };
    Renderer::get()->draw(geometry);
  };
private:
  vec2 m_position;
  std::vector<FreehandPoint> m_points;
};
