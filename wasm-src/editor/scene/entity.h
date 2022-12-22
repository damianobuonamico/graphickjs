#pragma once

#include "../../renderer/renderer.h"
#include "../../utils/uuid.h"

class Entity {
public:
  const UUID id;
public:
  Entity() = default;
  Entity(const Entity&) = default;
  Entity(Entity&&) = default;

  ~Entity() = default;

  virtual void render(float zoom) const {
    std::vector<Vertex> vertices = { {{0.0f, 0.0f}}, {{100.0f, 0.0f}}, {{100.0f, 100.0f}}, {{0.0f, 100.0f}} };
    std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0 };

    const Geometry geometry = { vertices, indices };
    Renderer::get()->draw(geometry);
  };
};
