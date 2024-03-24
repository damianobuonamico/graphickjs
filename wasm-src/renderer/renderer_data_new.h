/**
 * @file renderer_data.h
 * @brief The file contains data structures used by the renderer.
 *
 * @todo maybe replace vectors with preallocated arrays
 */

#pragma once

#include "gpu/gpu_data.h"

#include "../math/rect.h"

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

    /**
     * @brief Default constructor.
     */
    Viewport() = default;

    /**
     * @brief Constructs a new Viewport object.
     *
     * @param size The size of the viewport.
     * @param position The position of the viewport.
     * @param zoom The zoom level of the viewport, pre-multiplied by the dpr.
     * @param dpr The device-pixel-ratio of the screen.
     * @param background The clear color of the viewport.
     */
    Viewport(const vec2 size, const vec2 position, const double zoom, const double dpr, const vec4& background) :
      size(size), position(position), zoom(zoom), dpr(dpr), background(background),
      m_visible({ -position, size / static_cast<float>(zoom) - position }) {}

    /**
     * @brief Returns the scene-space visible area.
     *
     * @return The scene-space rectangle that is visible in the viewport.
    */
    inline rect visible() const {
      return m_visible;
    }
  private:
    rect m_visible;     /* The visible area of the viewport in scene-space coordinates. */
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
  };

}
