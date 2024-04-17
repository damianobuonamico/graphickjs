/**
 * @file renderer.h
 * @brief The file contains the definition of the main Graphick renderer.
 *
 * @todo remove using declarations when namespaces are unified
 */

#pragma once

#include "renderer_data_new.h"
#include "properties.h"

#include "gpu/shaders_new.h"

#include "geometry/quadratic_path.h"

#include "../math/mat2x3.h"
#include "../math/mat4.h"

#include <unordered_set>

namespace graphick::Renderer::Geometry {
  class Path;
};

namespace graphick::renderer {

  // TEMP
  using Stroke = graphick::Renderer::Stroke;
  using Fill = graphick::Renderer::Fill;

  /**
   * @brief The main Graphick renderer.
   *
   * The renderer takes QuadraticPaths as input and draws them on the screen based on the provided Stroke and Fill properties.
   *
   * @class Renderer
   */
  class Renderer {
  public:
    /**
     * @brief Deleted copy and move constructors.
     */
    Renderer(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;

    /**
     * @brief Initializes the renderer.
     *
     * This function should be called before any other renderer functions.
     */
    static void init();

    /**
     * @brief Shuts down the renderer.
     */
    static void shutdown();

    /**
     * @brief Begins a new frame.
     *
     * This function should be called at the beginning of each frame before issuing any draw calls.
     *
     * @param viewport The viewport to render to.
     */
    static void begin_frame(const Viewport& viewport);

    /**
     * @brief Ends the current frame.
     *
     * This function should be called at the end of each frame after all draw calls have been issued.
     */
    static void end_frame();

    /**
     * @brief Draws a QuadraticPath with the provided Stroke and Fill properties.
     *
     * @param path The QuadraticPath to draw.
     * @param stroke The Stroke properties to use.
     * @param fill The Fill properties to use.
     * @param transform The transformation matrix to apply to the path.
     * @param bounding_rect The bounding rectangle of the path if known, default is nullptr.
     */
    static void draw(const geometry::QuadraticPath& path, const Stroke& stroke, const Fill& fill, const mat2x3& transform, const rect* bounding_rect = nullptr);

    /**
     * @brief Draws a QuadraticPath with the provided Stroke properties.
     *
     * @param path The QuadraticPath to draw.
     * @param stroke The Stroke properties to use.
     * @param transform The transformation matrix to apply to the path.
     * @param bounding_rect The bounding rectangle of the path if known, default is nullptr.
     */
    static void draw(const geometry::QuadraticPath& path, const Stroke& stroke, const mat2x3& transform, const rect* bounding_rect = nullptr);

    /**
     * @brief Draws a QuadraticPath with the provided Fill properties.
     *
     * @param path The QuadraticPath to draw.
     * @param fill The Fill properties to use.
     * @param transform The transformation matrix to apply to the path.
     * @param bounding_rect The bounding rectangle of the path if known, default is nullptr.
     */
    static void draw(const geometry::QuadraticPath& path, const Fill& fill, const mat2x3& transform, const rect* bounding_rect = nullptr);

    /**
     * @brief Draws the outline of a QuadraticPath.
     *
     * @param path The QuadraticPath to draw.
     * @param transform The transformation matrix to apply to the path.
     * @param tolerance The tolerance to use when approximating the path, default is 0.25.
     * @param stroke The Stroke properties to use, can be nullptr.
     * @param bounding_rect The bounding rectangle of the path if known, default is nullptr.
     */
    static void draw_outline(const geometry::QuadraticPath& path, const mat2x3& transform, const float tolerance = 0.25f, const Stroke* stroke = nullptr, const rect* bounding_rect = nullptr);

    /**
     * @brief Draws the outline of a Path.
     *
     * @param path The Path to draw.
     * @param transform The transformation matrix to apply to the path.
     * @param tolerance The tolerance to use when approximating the path, default is 0.25.
     * @param stroke The Stroke properties to use, can be nullptr.
     * @param bounding_rect The bounding rectangle of the path if known, default is nullptr.
     */
    static void draw_outline(const graphick::Renderer::Geometry::Path& path, const mat2x3& transform, const float tolerance = 0.25f, const Stroke* stroke = nullptr, const rect* bounding_rect = nullptr);

    /**
     * @brief Draws the vertices of a Path's outline.
     *
     * @param path The Path to draw the vertices of.
     * @param transform The transformation matrix to apply to the path.
     * @param selected_vertices The indices of the selected vertices, if nullptr all vertices are selected.
     * @param stroke The Stroke properties to use, can be nullptr.
     * @param bounding_rect The bounding rectangle of the path if known, default is nullptr.
     */
    static void draw_outline_vertices(const graphick::Renderer::Geometry::Path& path, const mat2x3& transform, const std::unordered_set<size_t>* selected_vertices = nullptr, const Stroke* stroke = nullptr, const rect* bounding_rect = nullptr);
  private:
    /**
     * @brief Default constructor and destructor.
     */
    Renderer();
    ~Renderer() = default;

    /**
     * @brief Returns the singleton instance of the renderer.
     *
     * @return The singleton instance of the renderer.
     */
    static inline Renderer* get() { return s_instance; }

    /**
     * @brief Flushes the renderer.
     *
     * This function issues the draw calls to the GPU.
     */
    void flush_meshes();
  private:
    GPU::Programs m_programs;                        /* The shader programs to use. */

    Viewport m_viewport;                             /* The viewport to render to. */
    dmat4 m_vp_matrix;                               /* The view-projection matrix. */

    std::vector<mat4> m_transforms;                  /* The model-view-projection matrices of the paths. */

    PathInstancedData m_path_instances;              /* The path instances to render. */

    InstancedData<vec4> m_line_instances;            /* The line instances to render. */
    InstancedData<vec2> m_handle_instances;          /* The handle instances to render. */
    InstancedData<vec2> m_vertex_instances;          /* The vertex instances to render. */
    InstancedData<vec2> m_white_vertex_instances;    /* The vertex instances to render. */
  private:
    static Renderer* s_instance;    /* The singleton instance of the renderer. */
  };

}
