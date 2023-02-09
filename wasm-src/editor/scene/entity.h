#pragma once

#include "../../renderer/renderer.h"
#include "../../utils/uuid.h"
#include "../../math/box.h"
#include "components/transform_component.h"

class Entity {
public:
  const UUID id;
  Entity* parent = nullptr;
public:
  Entity() {};
  Entity(const Entity&) = default;
  Entity(Entity&&) = default;

  ~Entity() = default;

  virtual TransformComponent& transform() = 0;
  virtual const TransformComponent& transform() const = 0;

  virtual void render(float zoom) const {
    std::vector<Vertex> vertices = { {{0.0f, 0.0f}}, {{100.0f, 0.0f}}, {{100.0f, 100.0f}}, {{0.0f, 100.0f}} };
    std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0 };

    const Geometry geometry = { vertices, indices };
    Renderer::draw(geometry);
  };

  virtual Entity* entity_at(const vec2& position, bool lower_level, float threshold) { return nullptr; };
  virtual void entities_in(const Box& box, std::vector<Entity*>& entities, bool lower_level) {};
};
