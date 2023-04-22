#pragma once

#include "vertex.h"
#include "../../math/box.h"
#include "../../math/vec4.h"

#include <vector>
#include <initializer_list>

#ifndef GL_LINES
#define GL_LINES 0x0001
#endif
#ifndef GL_TRIANGLES
#define GL_TRIANGLES 0x0004
#endif
#ifndef GL_TRIANGLE_STRIP
#define GL_TRIANGLE_STRIP 0x0005
#endif
#ifndef GL_TRIANGLE_FAN
#define GL_TRIANGLE_FAN 0x0006
#endif

struct Geometry {
private:
  using Vertices = std::vector<Vertex>;
  using Indices = std::vector<uint32_t>;
public:
  template <typename T>
  struct Uniform {
    T value;
    uint32_t start_index;
    uint32_t end_index = std::numeric_limits<uint32_t>::max();
  };
public:
  Geometry(unsigned int primitive = GL_TRIANGLES);

  inline uint32_t offset() const { return m_offset; }
  inline unsigned int primitive() const { return m_primitive; }

  inline uint32_t vertex_count() const { return (uint32_t)m_vertices.size(); }
  inline uint32_t index_count() const { return (uint32_t)m_indices.size(); }
  inline uint32_t uniform_count() const { return (uint32_t)std::max(m_colors.size(), m_z_indices.size()); }

  inline const Vertex* vertex_data() const { return m_vertices.data(); }
  inline const uint32_t* index_data() const { return m_indices.data(); }

  inline const Vertices& vertices() const { return m_vertices; }
  inline const Indices& indices() const { return m_indices; }
  inline const std::vector<Uniform<vec4>>& colors() const { return m_colors; }
  inline const std::vector<Uniform<float>>& z_indices() const { return m_z_indices; }

  inline void reserve_vertices(uint32_t count) { m_vertices.reserve(count); }
  inline void reserve_indices(uint32_t count) { m_indices.reserve(count); }
  void reserve(uint32_t vertices, uint32_t indices);

  void push_vertex(const Vertex& vertex);
  void push_vertices(std::initializer_list<Vertex> vertices);
  void push_vertices(const Vertices& vertices);

  inline void push_index(uint32_t index) { m_indices.push_back(index); }
  inline void push_indices(std::initializer_list<uint32_t> indices) { m_indices.insert(m_indices.end(), indices); }
  inline void push_indices(const Indices& indices) { m_indices.insert(m_indices.end(), indices.begin(), indices.end()); }

  /**
   * @color: the color of the next vertices
   * Changes the color of the next vertices, offsets are automatically handled
   */
  void push_color(const vec4& color);
  /**
   * @z_index: the z-index of the next vertices
   * Changes the z-index of the next vertices, offsets are automatically handled
   */
  void push_z_index(float z_index);

  void push_quad(const vec2 v1, const vec2 v2, const vec2 v3, const vec2 v4);
  void push_quad(const vec2 v1, const vec2 v2, const vec2 v3, const vec2 v4, const vec4& color);
  void push_quad(const vec2 position, float radius);
  void push_quad(const vec2 position, float radius, const vec4& color);
  void push_quad(const Box& box);
  void push_quad(const Box& box, const vec4& color);

  void push_quad_outline(const Box& box, float line_dash = 0.0f);
  void push_quad_outline(const Box& box, const vec4& color, float line_dash = 0.0f);

  void push_circle(const vec2 position, float radius, uint32_t segments = 32);
  void push_circle(const vec2 position, float radius, const vec4& color, uint32_t segments = 32);

  void push_line(const vec2 v1, const vec2 v2);
  void push_line(const vec2 v1, const vec2 v2, const vec4& color);
  void push_line_strip(const std::vector<vec2>& vertices);
  void push_line_strip(std::initializer_list<vec2> vertices);

  /**
   * Returns a wireframe version of the geometry
   */
  Geometry wireframe() const;
  /**
   * @color: the wireframe color
   * Returns a wireframe version of the geometry and overrides all colors with @color
   */
  Geometry wireframe(const vec4& color) const;
private:
  void create_wireframe(Geometry& geo) const;
protected:
  Vertices m_vertices;
  Indices m_indices;

  std::vector<Uniform<vec4>> m_colors;
  std::vector<Uniform<float>> m_z_indices;

  uint32_t m_offset = 0;
  unsigned int m_primitive;
};

// struct StencilGeometry: public Geometry<Vertex> {
//   StencilGeometry(unsigned int primitive = GL_TRIANGLES);
// };

// struct TextureGeometry: public Geometry<TextureVertex> {
//   TextureGeometry(unsigned int primitive = GL_TRIANGLES);
// };

// struct InstanceGeometry: public Geometry<Vertex> {
//   InstanceGeometry(unsigned int primitive = GL_TRIANGLES);
// };

#if 0
struct Geometry {
  Geometry(unsigned int primitive = GL_TRIANGLES);
  Geometry(const std::vector<Vertex>& vertices, unsigned int primitive = GL_TRIANGLES);
  Geometry(const std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, unsigned int primitive = GL_TRIANGLES);

  inline const unsigned int primitive() const { return m_primitive; }
  inline std::vector<Vertex>& vertices() { return m_vertices; }
  inline const std::vector<Vertex>& vertices() const { return m_vertices; }
  inline std::vector<uint32_t>& indices() { return m_indices; }
  inline const std::vector<uint32_t>& indices() const { return m_indices; }
  inline uint32_t offset() const { return m_offset; }

  inline void reserve_vertices(size_t count) { m_vertices.reserve(count); }
  void reserve(size_t vertices, size_t indices);
  inline void reserve_indices(size_t count) { m_indices.reserve(count); }

  void push_vertex(const Vertex& vertex);
  void push_vertices(std::initializer_list<Vertex> vertices);
  inline void push_index(uint32_t index) { m_indices.push_back(index); }
  inline void push_indices(std::initializer_list<uint32_t> indices) { m_indices.insert(m_indices.end(), indices); }
  inline void push_indices(const std::vector<uint32_t>& indices) { m_indices.insert(m_indices.end(), indices.begin(), indices.end()); }

  void push_quad(const vec2& v1, const vec2& v2, const vec2& v3, const vec2& v4, const vec4& color = vec4(0.5f, 0.5f, 0.5f, 1.0f));
  void push_quad(const vec2& position, float radius, const vec4& color = vec4(0.5f, 0.5f, 0.5f, 1.0f));
  void push_quad(const Box& box, const vec4& color = vec4(0.5f, 0.5f, 0.5f, 1.0f));
  void push_circle(const vec2& position, float radius, const vec4& color = vec4(0.5f, 0.5f, 0.5f, 1.0f), uint32_t segments = 32);
  void push_line(const vec2& v1, const vec2& v2, const vec4& color = vec4(0.5f, 0.5f, 0.5f, 1.0f));
  void push_line_strip(std::initializer_list<Vertex> vertices);
  void push_line_strip(const std::vector<Vertex>& vertices);

  void push_quad_outline(const Box& box, const vec4& color = vec4(0.5f, 0.5f, 0.5f, 1.0f), float line_dash = 0.0f);

  Geometry wireframe() const;
private:
  std::vector<Vertex> m_vertices;
  std::vector<uint32_t> m_indices;
  uint32_t m_offset = 0;
  unsigned int m_primitive = GL_TRIANGLES;
};

struct InstancedGeometry: public Geometry {
  inline const std::vector<vec2>& translations() const { return m_translations; }
  inline uint32_t instances() const { return (uint32_t)m_translations.size(); }

  inline void reserve_instances(size_t count) { m_translations.reserve(count); }
  void reserve(size_t vertices, size_t indices, size_t instances);

  inline void push_instance(const vec2& translation) { m_translations.push_back(translation); }
  inline void push_instances(std::initializer_list<vec2> translations) { m_translations.insert(m_translations.end(), translations); }
  inline void push_instances(const std::vector<vec2>& translations) { m_translations.insert(m_translations.end(), translations.begin(), translations.end()); }
private:
  std::vector<vec2> m_translations;
};

#endif
