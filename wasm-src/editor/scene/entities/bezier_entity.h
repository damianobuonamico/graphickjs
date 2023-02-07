#pragma once

#include "../entity.h"
#include "vertex_entity.h"

#include <vector>

class BezierEntity: public Entity {
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
private:
  VertexEntity& m_start;
  VertexEntity& m_end;

  TransformComponent m_transform;
};
