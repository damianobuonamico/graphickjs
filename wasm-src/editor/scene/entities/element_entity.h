#pragma once

#include "../entity.h"
#include "vertex_entity.h"
#include "handle_entity.h"
#include "cubic_spline_entity.h"
#include "../../../values/ordered_map.h"

#include <vector>
#include <memory>

class ElementEntity: public Entity {
public:
  ElementEntity(const vec2& position): m_transform(TransformComponent{ this }), m_position(position) {
    console::log("ElementEntity created");
  };
  ElementEntity(const ElementEntity&) = default;
  ElementEntity(ElementEntity&&) = default;

  ~ElementEntity() {
    console::log("ElementEntity destroyed");
  }

  inline virtual TransformComponent& transform() { return m_transform; }

  virtual void render(float zoom) const override {
    std::vector<Vertex> vertices = { {{0.0f, 0.0f}}, {{100.0f, 0.0f}}, {{100.0f, 100.0f}}, {{0.0f, 100.0f}} };
    std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0 };

    const Geometry geometry = { vertices, indices };
    Renderer::get()->draw(geometry);

    HandleEntity handle({ 0.0f, 0.0f });
    handle.render(zoom);
  }
private:
  vec2 m_position;
  OrderedMap<UUID, std::shared_ptr<VertexEntity>> m_vertices;
  CubicSplineEntity m_spline;

  TransformComponent m_transform;
};
