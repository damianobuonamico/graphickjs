#pragma once

#include "../entity.h"
#include "bezier_entity.h"

#include <unordered_map>

class CubicSplineEntity: public Entity {
public:
  CubicSplineEntity(): m_transform(TransformComponent{ this }) {
    console::log("CubicSplineEntity created");
  };
  CubicSplineEntity(const CubicSplineEntity&) = default;
  CubicSplineEntity(CubicSplineEntity&&) = default;

  ~CubicSplineEntity() {
    console::log("CubicSplineEntity destroyed");
  }

  inline virtual TransformComponent& transform() { return m_transform; }
private:
  vec2 m_position;
  std::unordered_map<UUID, BezierEntity> m_entities;

  TransformComponent m_transform;
};
