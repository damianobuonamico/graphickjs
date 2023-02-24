#include "geometry.h"

#include "../../math/math.h"

/* -- Geometry -- */

Geometry::Geometry(unsigned int primitive)
  : m_primitive(primitive), m_vertices(), m_indices() {}

Geometry::Geometry(const std::vector<Vertex>& vertices, unsigned int primitive)
  : m_vertices(vertices), m_indices(), m_offset((uint32_t)vertices.size()), m_primitive(primitive) {}

Geometry::Geometry(const std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, unsigned int primitive)
  : m_vertices(vertices), m_indices(indices), m_offset((uint32_t)vertices.size()), m_primitive(primitive) {}

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

void Geometry::push_quad(const Box& box, const vec4& color) {
  push_quad(box.min, vec2{ box.max.x, box.min.y }, box.max, vec2{ box.min.x, box.max.y }, color);
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

void Geometry::push_line(const vec2& v1, const vec2& v2, const vec4& color) {
  push_vertices({ { v1, color }, { v2, color } });
  push_indices({ m_offset - 2, m_offset - 1 });
}

void Geometry::push_line_strip(std::initializer_list<Vertex> vertices) {
  m_vertices.insert(m_vertices.end(), vertices);
  m_indices.reserve(vertices.size() * 2 - 2);

  for (uint32_t i = m_offset; i < vertices.size() - 1 + m_offset; i++) {
    m_indices.push_back(i);
    m_indices.push_back(i + 1);
  }

  m_offset += (uint32_t)vertices.size();
}

void Geometry::push_line_strip(const std::vector<Vertex>& vertices) {
  m_vertices.insert(m_vertices.end(), vertices.begin(), vertices.end());
  m_indices.reserve(vertices.size() * 2 - 2);

  for (uint32_t i = m_offset; i < vertices.size() - 1 + m_offset; i++) {
    m_indices.push_back(i);
    m_indices.push_back(i + 1);
  }

  m_offset += (uint32_t)vertices.size();
}

void Geometry::push_quad_outline(const Box& box, const vec4& color, float line_dash) {
  if (line_dash <= 0.0f) {
    push_line_strip({
      { box.min, color }, { vec2{ box.max.x, box.min.y }, color },
      { box.max, color }, { vec2{ box.min.x, box.max.y}, color }
      });
    push_indices({ m_offset - 1, m_offset - 4 });

    return;
  }

  for (Box& line : lines_from_box(box)) {
    float length = distance(line.min, line.max);
    if (line_dash >= length) {
      push_line(line.min, line.max, color);
      continue;
    }

    vec2 direction = (line.max - line.min) / length;
    int segments = (int)((length - line_dash) / line_dash);
    float half_dash = line_dash / 2.0f;
    vec2 start = line.min + direction * half_dash;

    push_line(line.min, start, color);

    for (int i = 1; i < segments; i += 2) {
      push_line(start + direction * line_dash * (float)i, start + direction * line_dash * (float)(i + 1), color);
    }

    if (segments % 2 != 0) {
      push_line(start + direction * (line_dash * (segments)), line.max, color);
    }

    push_line(line.max, line.max - direction * half_dash, color);
  }
}

Geometry Geometry::wireframe() const {
  Geometry geo{ m_vertices, GL_LINES };

  if (m_primitive == GL_LINES) {
    geo.push_indices(m_indices);
    return geo;
  }


  if (m_primitive == GL_TRIANGLES) {
    geo.reserve_indices(m_indices.size() * 2);

    for (uint32_t i = 0; i < m_indices.size(); i += 3) {
      geo.push_indices({ m_indices[i], m_indices[i + 1], m_indices[i + 1], m_indices[i + 2], m_indices[i + 2], m_indices[i] });
    }
  }

  return geo;
}

/* -- InstancedGeometry -- */

void InstancedGeometry::reserve(size_t vertices, size_t indices, size_t instances) {
  reserve_vertices(vertices);
  reserve_indices(indices);
  reserve_instances(instances);
}
