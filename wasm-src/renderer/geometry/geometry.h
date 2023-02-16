#pragma once

#include "vertex.h"

#include <vector>
#include <initializer_list>

struct Geometry {
  Geometry();
  Geometry(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);

  inline std::vector<Vertex>& vertices() { return m_vertices; }
  inline const std::vector<Vertex>& vertices() const { return m_vertices; }
  inline std::vector<uint32_t>& indices() { return m_indices; }
  inline const std::vector<uint32_t>& indices() const { return m_indices; }
  inline uint32_t offset() const { return m_offset; }

  inline void reserve_vertices(size_t count) { m_vertices.reserve(count); }
  inline void reserve_indices(size_t count) { m_indices.reserve(count); }
  void reserve(size_t vertices, size_t indices);

  void push_vertex(const Vertex& vertex);
  void push_vertices(std::initializer_list<Vertex> vertices);
  inline void push_index(uint32_t index) { m_indices.push_back(index); }
  inline void push_indices(std::initializer_list<uint32_t> indices) { m_indices.insert(m_indices.end(), indices); }

  void push_quad(const vec2& v1, const vec2& v2, const vec2& v3, const vec2& v4, const vec4& color = vec4(0.5f, 0.5f, 0.5f, 1.0f));
  void push_quad(const vec2& position, float radius, const vec4& color = vec4(0.5f, 0.5f, 0.5f, 1.0f));
  void push_circle(const vec2& position, float radius, const vec4& color = vec4(0.5f, 0.5f, 0.5f, 1.0f), uint32_t segments = 32);
private:
  std::vector<Vertex> m_vertices;
  std::vector<uint32_t> m_indices;
  uint32_t m_offset = 0;
};

struct InstancedGeometry: public Geometry {
  inline const std::vector<vec2>& translations() const { return m_translations; }
  inline uint32_t instances() const { return (uint32_t)m_translations.size(); }

  inline void reserve_instances(size_t count) { m_translations.reserve(count); }
  void reserve(size_t vertices, size_t indices, size_t instances);

  inline void push_instance(const vec2& translation) { m_translations.push_back(translation); }
  inline void push_instances(std::initializer_list<vec2> translations) { m_translations.insert(m_translations.end(), translations); }
private:
  std::vector<vec2> m_translations;
};
