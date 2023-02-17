#include "geometry.h"

#include "../../math/math.h"

/* -- Geometry -- */

Geometry::Geometry(unsigned int primitive)
  : m_primitive(primitive), m_vertices(), m_indices() {}

Geometry::Geometry(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
  : m_vertices(vertices), m_indices(indices), m_offset((uint32_t)vertices.size()) {}

void Geometry::reserve(size_t vertices, size_t indices) {
  reserve_vertices(vertices);
  reserve_indices(indices);
}

void Geometry::push_vertex(const Vertex& vertex) {
  m_vertices.push_back(vertex);
  m_offset++;
}

void Geometry::push_vertices(std::initializer_list<Vertex> vertices) {
  m_vertices.insert(m_vertices.end(), vertices);
  m_offset += (uint32_t)vertices.size();
}

void Geometry::push_quad(const vec2& v1, const vec2& v2, const vec2& v3, const vec2& v4, const vec4& color) {
  push_vertices({ { v1, color }, { v2, color }, { v3, color }, { v4, color } });
  push_indices({ m_offset - 4, m_offset - 3, m_offset - 2, m_offset - 4, m_offset - 2, m_offset - 1 });
}

void Geometry::push_quad(const vec2& position, float radius, const vec4& color) {
  vec2 offset_d1{ radius, radius };
  vec2 offset_d2{ -radius, radius };

  push_quad(position - offset_d1, position + offset_d2, position + offset_d1, position - offset_d2, color);
}

void Geometry::push_circle(const vec2& position, float radius, const vec4& color, uint32_t segments) {
  const float step = MATH_TWO_PI / segments;
  const uint32_t center = m_offset;

  reserve(segments + 1, segments * 3);
  push_vertex({ position, color });
  push_vertex({ position + vec2{ radius, 0.0f }, color });

  for (uint32_t i = 1; i < segments; i++) {
    float angle = i * step;

    push_vertex({ position + radius * vec2{ std::cosf(angle), std::sinf(angle) }, color });
    push_indices({ center, m_offset - 2, m_offset - 1 });
  }

  push_indices({ center, center + 1, m_offset - 1 });
}

/* -- InstancedGeometry -- */

void InstancedGeometry::reserve(size_t vertices, size_t indices, size_t instances) {
  reserve_vertices(vertices);
  reserve_indices(indices);
  reserve_instances(instances);
}
