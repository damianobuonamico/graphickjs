#include "element_entity.h"

void ElementEntity::add_vertex(const std::shared_ptr<VertexEntity>& vertex) {
  m_vertices.insert({ vertex->id, vertex });
  vertex->parent = this;
}

void ElementEntity::render(float zoom) {
  std::vector<Vertex> vertices = { {{0.0f, 0.0f}}, {{100.0f, 0.0f}}, {{100.0f, 100.0f}}, {{0.0f, 100.0f}} };
  std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0 };

  const Geometry geometry = { vertices, indices };
  Renderer::get()->draw(geometry);

  for (const auto& [id, vertex] : m_vertices) {
    vertex->position()->render(zoom);
  }
}
