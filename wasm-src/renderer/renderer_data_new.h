/**
 * @file renderer_data.h
 * @brief The file contains data structures used by the renderer.
 *
 * @todo maybe replace vectors with preallocated arrays
 */

#pragma once

#include "gpu/gpu_data.h"

#include "../utils/uuid.h"

#include <vector>

namespace Graphick::renderer {

  /**
   * @brief Represents the viewport of the renderer.
   *
   * The viewport is the area of the screen where the renderer will draw.
   *
   * @struct Viewport
   */
  struct Viewport {
    vec2 size;          /* The size of the viewport. */
    vec2 position;      /* The position of the viewport. */

    double zoom;        /* The zoom level of the viewport (it is pre-multiplied by the dpr). */
    double dpr;         /* The device pixel ratio. */

    vec4 background;    /* The background color to clear the viewport with. */
  };

  /**
   * @brief Represents a mesh to be rendered using instancing.
   *
   * @struct InstancedData
   */
  template<typename T>
  struct InstancedData {
    std::vector<T> instances;                /* The per-instance data. */

    std::vector<vec2> vertices;              /* The vertices of the mesh. */

    Graphick::Renderer::GPU::Primitive primitive;                /* The primitive type of the mesh. */

    uuid instance_buffer_id = uuid::null;    /* The ID of the instance buffer. */
    uuid vertex_buffer_id = uuid::null;      /* The ID of the vertex buffer. */

    uint32_t max_instances;                  /* The maximum number of instances. */

    /**
     * @brief Initializes the instance data.
     *
     * @param buffer_size The maximum buffer size.
     * @param primitive The primitive type of the mesh.
     */
    InstancedData(const size_t buffer_size, const Graphick::Renderer::GPU::Primitive primitive = Graphick::Renderer::GPU::Primitive::Triangles) :
      primitive(primitive),
      max_instances(static_cast<uint32_t>(buffer_size / sizeof(T)))
    {
      instances.reserve(max_instances);
    }

    InstancedData(const InstancedData&) = delete;
    InstancedData(InstancedData&&) = delete;

    InstancedData& operator=(const InstancedData&) = delete;
    InstancedData& operator=(InstancedData&&) = delete;

    /**
     * @brief Clears the instance data.
     */
    inline void clear() {
      instances.clear();
    }
  };

  /**
   * @brief Represents a path instance, the main building block of the renderer.
   *
   * @struct PathInstance
   */
  struct PathInstance {
    vec2 size;             /* The size of the quad. */
    uint32_t mvp_index;    /* The index of the model-view-projection matrix in the uniform buffer. */
  };

}
