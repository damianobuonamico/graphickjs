#include "element_entity.h"

void ElementEntity::add_vertex(const std::shared_ptr<VertexEntity>& vertex) {
  m_vertices.insert({ vertex->id, vertex });
  vertex->parent = this;
  regenerate();
}

void ElementEntity::render(float zoom) const {
  vec2 position = transform().position().get();

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

  for (const auto& [id, vertex] : m_vertices) {
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

const BezierEntity ElementEntity::closing_curve() const {
  if (m_closed.get()) {
    return m_curves.back();
  }

  VertexEntity start_vertex{ last_vertex().transform().position().get() };
  VertexEntity end_vertex{ first_vertex().transform().position().get() };

  return { start_vertex, end_vertex };
}

bool ElementEntity::intersects_box(const Box& box) const {
  size_t num_vertices = vertex_count();

  if (num_vertices < 1) {
    return false;
  } else if (num_vertices == 1) {
    return is_point_in_box(first_vertex().transform().position().get(), box);
  }

  vec2 position = m_transform.position().get();
  Box translated_box = { box.min - position, box.max - position };

  for (auto& curve : m_curves) {
    if (curve.intersects_box(translated_box)) {
      return true;
    }
  }

  return false;
}

Entity* ElementEntity::entity_at(const vec2& position, bool lower_level, float threshold) {
  if (!is_point_in_box(position, lower_level ? m_transform.large_bounding_box() : m_transform.bounding_box())) {
    return nullptr;
  }

  vec2 pos = position - m_transform.position().get();
  Entity* entity = nullptr;

  for (const auto& [id, vertex] : m_vertices) {
    entity = vertex->entity_at(pos, lower_level, threshold);
    if (entity) return entity;
  }

  for (auto& curve : m_curves) {
    entity = curve.entity_at(pos, lower_level, threshold);
    if (entity) return entity;
  }

  // TODO: implement fill
  // if (!m_filled) return nullptr;

  Box line = { pos, { std::numeric_limits<float>::max(), pos.y } };

  // TODO: implement fill algorithm (even-odd/non-zero)
  // even-odd
  if (true) {
    int intersections = 0;

    for (auto& curve : m_curves) {
      intersections += (int)curve.line_intersection_points(line).size();
    }

    if (!m_closed.get() && vertex_count() > 1) {
      intersections += (int)closing_curve().line_intersection_points(line).size();
    }

    if (intersections % 2 != 0) {
      return this;
    }
  } else {
    int count = 0;

    for (auto& curve : m_curves) {
      for (float t : curve.line_intersections(line)) {
        if (curve.get(t).x < position.x) continue;

        if (
          curve.get(t - GEOMETRY_MAX_INTERSECTION_ERROR).x <=
          curve.get(t + GEOMETRY_MAX_INTERSECTION_ERROR).x
          ) {
          count++;
        } else {
          count--;
        }
      }
    }

    if (!m_closed.get() && vertex_count() > 1) {
      BezierEntity curve = closing_curve();

      for (float t : curve.line_intersections(line)) {
        if (curve.get(t).x < position.x) continue;

        if (
          curve.get(t - GEOMETRY_MAX_INTERSECTION_ERROR).x <=
          curve.get(t + GEOMETRY_MAX_INTERSECTION_ERROR).x
          ) {
          count++;
        } else {
          count--;
        }
      }
    }

    if (count != 0) {
      return this;
    }
  }

  return nullptr;
}

void ElementEntity::entities_in(const Box& box, std::vector<Entity*>& entities, bool lower_level) {
  if (lower_level) {
    vec2 position = m_transform.position().get();
    Box translated_box = { box.min - position, box.max - position };

    for (const auto& [id, vertex] : m_vertices) {
      vertex->entities_in(translated_box, entities, lower_level);
    }
  } else if (intersects_box(box)) {
    entities.push_back(this);
  }
}
