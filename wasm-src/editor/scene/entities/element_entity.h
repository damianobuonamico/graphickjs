#pragma once

#include "../entity.h"
#include "bezier_entity.h"
#include "vertex_entity.h"
#include "handle_entity.h"
#include "../../../values/ordered_map.h"

#include <vector>
#include <memory>

class ElementEntity: public Entity {
public:
  ElementEntity(const vec2& position): m_transform(TransformComponent{ this }), m_position(position) {
    add_vertex(std::make_shared<VertexEntity>(vec2{ 0.0f, 0.0f }));
    add_vertex(std::make_shared<VertexEntity>(vec2{ 100.0f, 0.0f }));
    add_vertex(std::make_shared<VertexEntity>(vec2{ 100.0f, 100.0f }));
    add_vertex(std::make_shared<VertexEntity>(vec2{ 0.0f, 100.0f }));

    console::log("ElementEntity created");
  };
  ElementEntity(const ElementEntity&) = default;
  ElementEntity(ElementEntity&&) = default;

  ~ElementEntity() {
    console::log("ElementEntity destroyed");
  }

  inline virtual TransformComponent& transform() { return m_transform; }

  void add_vertex(const std::shared_ptr<VertexEntity>& vertex);

  // TODO: const
  virtual void render(float zoom) override;
private:
  vec2 m_position;
  OrderedMap<UUID, std::shared_ptr<VertexEntity>> m_vertices;
  std::unordered_map<UUID, BezierEntity> m_entities;

  TransformComponent m_transform;
};
