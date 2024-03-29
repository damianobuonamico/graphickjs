/**
 * @file renderer_data.h
 * @brief The file contains data structures used by the renderer.
 *
 * @todo maybe replace vectors with preallocated arrays
 */

#pragma once

#include "gpu/gpu_data.h"

#include "../math/vec4.h"
#include "../math/rect.h"
#include "../math/mat2x3.h"

#include "../utils/defines.h"
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
   * @brief Represents a path instance, the main building block of the renderer.
   *
   * @struct PathInstance
   */
  struct PathInstance {
    vec4 attrib_1;           /* transform[0][0] transform[0][1] transform[0][2] transform[1][0] */
    vec2 attrib_2;           /* transform[1][1] transform[1][2] */
    vec2 position;           /* position.xy */
    vec2 size;               /* size.xy */
    uvec4 color;             /* color.rgba */
    uint32_t curves_data;    /* start_x start_y */
    uint32_t bands_data;     /* h_count v_count start_x start_y */

    /**
     * @brief Constructs a new PathInstance object.
     *
     * @param transform The transformation matrix of the path.
     * @param position The position of the path's bounding rect before transformation, used to normalize the curves.
     * @param size The size of the path's bounding rect before transformation, used to normalize the curves.
     * @param color The color of the path.
     */
    PathInstance(
      const mat2x3& transform, const vec2 position, const vec2 size, const vec4& color,
      const size_t curves_start_index, const size_t bands_start_index,
      const uint8_t horizontal_bands, const uint8_t vertical_bands
    ) :
      attrib_1(transform[0][0], transform[0][1], transform[0][2], transform[1][0]),
      attrib_2(transform[1][1], transform[1][2]),
      position(position), size(size), color(color * 255.0f)
    {
      const uint32_t curves_x = static_cast<uint32_t>(curves_start_index % GK_CURVES_TEXTURE_SIZE);
      const uint32_t curves_y = static_cast<uint32_t>(curves_start_index / GK_CURVES_TEXTURE_SIZE);
      const uint32_t bands_x = static_cast<uint32_t>(bands_start_index % GK_BANDS_TEXTURE_SIZE);
      const uint32_t bands_y = static_cast<uint32_t>(bands_start_index / GK_BANDS_TEXTURE_SIZE);
      const uint32_t bands_h = static_cast<uint32_t>(horizontal_bands - 1);
      const uint32_t bands_v = static_cast<uint32_t>(vertical_bands - 1);

      curves_data = ((curves_x << 20) >> 8) | ((curves_y << 20) >> 20);
      bands_data = (bands_h << 28) | ((bands_v << 28) >> 4) | ((bands_x << 20) >> 8) | ((bands_y << 20) >> 20);
    }
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
    virtual inline void clear() {
      instances.clear();
    }
  };

  /**
   * @brief Represents the data of the path instances to render.
   *
   * @struct PathInstancedData
   */
  struct PathInstancedData : public InstancedData<PathInstance> {
    std::vector<vec2> curves;               /* The control points of the curves. */
    std::vector<uint16_t> bands;            /* The bands of the mesh. */
    std::vector<uint16_t> bands_data;       /* The indices of each curve in the bands. */

    uuid curves_texture_id = uuid::null;    /* The ID of the curves texture. */
    uuid bands_texture_id = uuid::null;     /* The ID of the curves texture. */

    /**
     * @brief Constructs a new PathInstanceData object.
     *
     * @param buffer_size The maximum buffer size.
     */
    PathInstancedData(const size_t buffer_size) : InstancedData<PathInstance>(buffer_size) {}

    /**
     * @brief Clears the instance data.
     */
    virtual inline void clear() override {
      InstancedData<PathInstance>::clear();

      curves.clear();
      bands.clear();
      bands_data.clear();
    }
  };

}
