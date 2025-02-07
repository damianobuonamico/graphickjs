/**
 * @file renderer/instances.h
 * @brief The file contains the definition of the renderer instances.
 */

#pragma once

#include "../math/mat4.h"
#include "../math/vec2.h"
#include "../math/vec4.h"

#include "gpu/render_state.h"

#include <vector>

namespace graphick::renderer::GPU {

struct PrimitiveProgram;
struct PrimitiveVertexArray;

}  // namespace graphick::renderer::GPU

namespace graphick::renderer {

struct PrimitiveInstance {
  vec2 attr1;      // position.xy (or start.xy)
  vec2 attr2;      // size.xy (or end.xy)
  uint32_t attr3;  // primitive_attr | type (1 byte)
  uvec4 color;     // color.rgba

  /**
   * @brief Constructs a new LinePrimitive instance.
   */
  PrimitiveInstance(const vec2 start, const vec2 end, const float width, const vec4& color)
      : attr1(start), attr2(end), color(color * 255.0f)
  {
    uint32_t u_primitive_type = 0;
    uint32_t u_primitive_attr = static_cast<uint32_t>(width * 1024.0f) << 8 >> 8;

    attr3 = (u_primitive_attr << 8) | (u_primitive_type);
  }

  /**
   * @brief Constructs a new RectPrimitive instance.
   */
  PrimitiveInstance(const vec2 position, const vec2 size, const vec4& color)
      : attr1(position), attr2(size), color(color * 255.0f)
  {
    uint32_t u_primitive_type = 1;

    attr3 = (u_primitive_type);
  }

  /**
   * @brief Constructs a new CirclePrimitive instance.
   */
  PrimitiveInstance(const vec2 position, const float radius, const vec4& color)
      : attr1(position), attr2(vec2(radius)), color(color * 255.0f)
  {
    uint32_t u_primitive_type = 2;

    attr3 = (u_primitive_type);
  }
};

/**
 * @brief Represents a buffer of instances.
 */
template<typename T>
struct InstanceBuffer {
  std::vector<std::vector<T>> batches;  // The instances.

  uint32_t max_instances_per_batch;     // The maximum number of instances for each batch.

  /**
   * @brief Constructs a new InstanceBuffer object.
   *
   * @param max_instances_per_batch The maximum number of instances for each batch.
   */
  InstanceBuffer(const uint32_t max_instances_per_batch)
      : max_instances_per_batch(max_instances_per_batch)
  {
    batches.resize(1);
    batches[0].reserve(max_instances_per_batch);
  }

  /**
   * @brief Clears the instance batches.
   */
  inline void clear()
  {
    batches.resize(1);
    batches[0].clear();
  }

  /**
   * @brief Adds a new instance to the buffer.
   *
   * @param instance The instance to add.
   */
  inline void push_back(T&& instance)
  {
    if (batches.back().size() >= max_instances_per_batch) {
      batches.push_back({});
      batches.back().reserve(max_instances_per_batch);
    }

    batches.back().push_back(std::move(instance));
  }
};

/**
 * @brief Represents a mesh to be rendered using instancing.
 */
template<typename T>
struct InstancedData {
  InstanceBuffer<T> instances;  // The per-instance data.

  GPU::Primitive primitive;     // The primitive type of the mesh.

  GPU::Buffer instance_buffer;  // The GPU instance buffer.
  GPU::Buffer vertex_buffer;    // The GPU vertex buffer.

  size_t vertex_size;           // The size of a vertex in bytes.

  /**
   * @brief Initializes the instance data.
   *
   * @param buffer_size The maximum buffer size in bytes.
   * @param primitive The primitive type of the mesh.
   */
  InstancedData(const size_t buffer_size,
                const std::vector<vec2>& vertices,
                const GPU::Primitive primitive = GPU::Primitive::Triangles)
      : primitive(primitive),
        instances(static_cast<uint32_t>(buffer_size / sizeof(T))),
        instance_buffer(GPU::BufferTarget::Vertex, GPU::BufferUploadMode::Dynamic, buffer_size),
        vertex_buffer(GPU::BufferTarget::Vertex,
                      GPU::BufferUploadMode::Static,
                      vertices.size() * sizeof(vec2),
                      vertices.data()),
        vertex_size(sizeof(vec2))
  {
  }

  /**
   * @brief Initializes the instance data.
   *
   * @param buffer_size The maximum buffer size in bytes.
   * @param primitive The primitive type of the mesh.
   */
  InstancedData(const size_t buffer_size,
                const std::vector<uvec2>& vertices,
                const GPU::Primitive primitive = GPU::Primitive::Triangles)
      : primitive(primitive),
        instances(static_cast<uint32_t>(buffer_size / sizeof(T))),
        instance_buffer(GPU::BufferTarget::Vertex, GPU::BufferUploadMode::Dynamic, buffer_size),
        vertex_buffer(GPU::BufferTarget::Vertex,
                      GPU::BufferUploadMode::Static,
                      vertices.size() * sizeof(uvec2),
                      vertices.data()),
        vertex_size(sizeof(uvec2))
  {
  }

  InstancedData(const InstancedData&) = delete;
  InstancedData(InstancedData&&) = delete;

  InstancedData& operator=(const InstancedData&) = delete;
  InstancedData& operator=(InstancedData&&) = delete;

  /**
   * @brief Gets the maximum number of instances for each batch.
   *
   * @return The maximum number of instances for each batch.
   */
  inline uint32_t max_instances() const
  {
    return instances.max_instances_per_batch;
  }

  /**
   * @brief Checks if the instance data is empty.
   *
   * @return Whether the instance data is empty.
   */
  inline bool empty() const
  {
    return instances.batches.empty() || instances.batches[0].empty();
  }

  /**
   * @brief Clears the instance data.
   */
  virtual inline void clear()
  {
    instances.clear();
  }
};

/**
 * @brief The InstancedRenderer takes care of drawing the UI.
 */
class InstancedRenderer {
 public:
  /**
   * @brief Constructs a new InstancedRenderer object.
   */
  InstancedRenderer(const uint32_t max_instances_per_batch)
      : m_instances(
            max_instances_per_batch,
            {uvec2(0, 0), uvec2(1, 0), uvec2(1, 1), uvec2(1, 1), uvec2(0, 1), uvec2(0, 0)}),
        m_program(nullptr),
        m_vertex_array(nullptr)
  {
  }

  /**
   * @brief Updates the shader and vertex array to use.
   *
   * @param program The program to use.
   * @param vertex_array The vertex array to use.
   */
  inline void update_shader(GPU::PrimitiveProgram* program,
                            GPU::PrimitiveVertexArray* vertex_array)
  {
    m_program = program;
    m_vertex_array = vertex_array;
  }

  /**
   * @brief Returns the instance buffer.
   *
   * @return The instance buffer.
   */
  inline const GPU::Buffer& instance_buffer() const
  {
    return m_instances.instance_buffer;
  }

  /**
   * @brief Returns the vertex buffer.
   *
   * @return The vertex buffer.
   */
  inline const GPU::Buffer& vertex_buffer() const
  {
    return m_instances.vertex_buffer;
  }

  /**
   * @brief Adds a new line instance to the buffer.
   *
   * @param start The start position of the line.
   * @param end The end position of the line.
   * @param width The width of the line.
   * @param color The color of the line.
   */
  inline void push_line(const vec2 start,
                        const vec2 end,
                        const vec4& color,
                        const float width = 1.0f)
  {
    m_instances.instances.push_back({start, end, width, color});
  }

  /**
   * @brief Adds a new rect instance to the buffer.
   *
   * @param position The position of the rect.
   * @param size The size of the rect.
   * @param color The color of the rect.
   */
  inline void push_rect(const vec2 position, const vec2 size, const vec4& color)
  {
    m_instances.instances.push_back({position, size, color});
  }

  /**
   * @brief Adds a new circle instance to the buffer.
   *
   * @param position The position of the circle.
   * @param radius The radius of the circle.
   * @param color The color of the circle.
   */
  inline void push_circle(const vec2 position, const float radius, const vec4& color)
  {
    m_instances.instances.push_back({position, radius, color});
  }

  /**
   * @brief Flushes the instanced data to the GPU.
   *
   * Here the GPU draw calls are actually issued.
   *
   * @param viewport_size The size of the viewport.
   * @param vp_matrix The view projection matrix.
   * @param zoom The zoom level of the viewport.
   */
  void flush(const ivec2 viewport_size, const mat4& vp_matrix, const float zoom);

 private:
  InstancedData<PrimitiveInstance> m_instances;  // The instance buffer.

  GPU::PrimitiveProgram* m_program;              // The program to use.
  GPU::PrimitiveVertexArray* m_vertex_array;     // The vertex array to use.
};

}  // namespace graphick::renderer
