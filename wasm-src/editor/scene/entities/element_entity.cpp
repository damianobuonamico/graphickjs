#include "element_entity.h"

void ElementEntity::add_vertex(const std::shared_ptr<VertexEntity>& vertex) {
  m_vertices.insert({ vertex->id, vertex });
  vertex->parent = this;
  regenerate();
}

void ElementEntity::render(float zoom) {
  // Geometry geometry{};
  // uint32_t offset = 0;

  for (auto& curve : m_curves) {
    curve.render(zoom);
  }

  for (const auto& [id, vertex] : m_vertices) {
    vertex->position()->render(zoom);
  }
}

// TODO: diff algorithm
void ElementEntity::regenerate() {
  m_curves.clear();
  if (m_vertices.size() < 2) return;

  VertexEntity* first_vertex = nullptr;
  VertexEntity* last_vertex = nullptr;

  for (auto& [id, vertex] : m_vertices) {
    VertexEntity* current_vertex = vertex.get();

    if (last_vertex) {
      m_curves.push_back({ *last_vertex, *current_vertex, this });
    }

    last_vertex = current_vertex;

    if (!first_vertex) {
      first_vertex = current_vertex;
    }
  }

  if (m_closed.get() && first_vertex && last_vertex) {
    m_curves.push_back({ *last_vertex, *first_vertex, this });
  }
}
