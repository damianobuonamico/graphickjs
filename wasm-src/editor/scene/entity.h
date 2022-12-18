#pragma once

#include "../../renderer/renderer.h"

class Entity {
public:
  Entity() = default;
  Entity(const Entity&) = default;
  Entity(Entity&&) = default;

  ~Entity() = default;

  void render() const {
    std::vector<Vertex> vertices = { {{0.0f, 0.0f}}, {{100.0f, 0.0f}}, {{100.0f, 100.0f}}, {{0.0f, 100.0f}} };
    std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0 };

    const Geometry geometry = { vertices, indices };
    Renderer::get()->draw(geometry);
  };
};
