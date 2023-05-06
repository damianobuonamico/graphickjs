#include "element_entity.h"

void ElementEntity::add_vertex(const std::shared_ptr<VertexEntity>& vertex) {
  m_vertices.insert({ vertex->id, vertex });
  vertex->parent = this;
  regenerate();
}

void ElementEntity::tessellate_outline(const vec4& color, const RenderingOptions& options, Geometry& geo) const {
  TessellationParams params = {
    m_transform.position().get(), options,
    1.0f, color,
    JoinType::Bevel, CapType::Butt, 10.0f,
    false, false, false, false, true,
    { vec2{}, vec2{}, 0 }
  };

  for (auto& curve : m_curves) {
    curve.tessellate_outline(params, geo);
  }
}

void ElementEntity::render(const RenderingOptions& options) const {
  if (m_curves.empty()) {
    return;
  }

  Geometry geo{};


  bool is_closed = m_closed.get();
  TessellationParams params = {
    m_transform.position().get(), options,
    1.0f, vec4(0.5f, 0.5f, 0.5f, 1.0f),
    JoinType::Round, CapType::Round, 10.0f,
    false, false, !is_closed, false, true,
    { vec2{}, vec2{}, 0 }
  };

  params.rendering_options.facet_angle /= std::sqrtf(5.0f);

  for (int i = 0; i < m_curves.size() - 1; i++) {
    m_curves[i].tessellate(params, geo);

    params.start_join = true;
    params.start_cap = false;
    params.is_first_segment = false;
  }

  if (is_closed) {
    params.end_join = true;
  } else {
    params.end_cap = true;
  }

  m_curves[m_curves.size() - 1].tessellate(params, geo);

  Renderer::draw(geo);

  // for (const auto& [id, vertex] : m_vertices) {
  //   vertex->position()->render(zoom);
  // }

  // Box box = transform().large_bounding_box();
  // Geometry box_geometry{};
  // box_geometry.push_quad(box, vec4{ 0.0f, 1.0f, 0.5f, 0.2f });
  // Renderer::draw(box_geometry);
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

const Box ElementEntity::closing_line() const {
  return {
    last_vertex().transform()->position().get(),
    first_vertex().transform()->position().get()
  };
}

bool ElementEntity::intersects_box(const Box& box) const {
  size_t num_vertices = vertex_count();

  if (num_vertices < 1) {
    return false;
  } else if (num_vertices == 1) {
    return is_point_in_box(first_vertex().transform()->position().get(), box);
  }

  if (!does_box_intersect_box(box, transform()->bounding_box())) {
    return false;
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
  if (!is_point_in_box(position, lower_level ? m_transform.large_bounding_box() : m_transform.bounding_box(), threshold)) {
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
      intersections += (int)line_line_intersections(line, closing_line()).size();
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
      Box closing = closing_line();

      for (float t : line_line_intersections(line, closing)) {
        if (lerp(closing.min, closing.max, t).x < position.x) continue;

        if (
          lerp(closing.min, closing.max, t - GEOMETRY_MAX_INTERSECTION_ERROR).x <=
          lerp(closing.min, closing.max, t + GEOMETRY_MAX_INTERSECTION_ERROR).x
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
