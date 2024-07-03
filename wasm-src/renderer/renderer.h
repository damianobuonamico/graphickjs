/**
 * @file renderer.h
 * @brief The file contains the definition of the main Graphick renderer.
 *
 * @todo remove using declarations when namespaces are unified
 */

#pragma once

#include "renderer_data.h"
#include "properties.h"

#include "gpu/shaders.h"

#include "../geom/quadratic_path.h"
#include "../geom/cubic_path.h"
#include "../geom/cubic_bezier.h"

#include "../math/mat2x3.h"
#include "../math/mat4.h"

#include "../utils/defines.h"

#include <unordered_set>
#include <optional>

namespace graphick::geom {
  template <typename T, typename>
  class Path;
}

namespace graphick::renderer {

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
    static void draw(const geom::quadratic_path& path, const Stroke& stroke, const Fill& fill, const mat2x3& transform, const rect* bounding_rect = nullptr);

    /**
     * @brief Draws a QuadraticPath with the provided Stroke properties.
     *
     * @param path The QuadraticPath to draw.
     * @param stroke The Stroke properties to use.
     * @param transform The transformation matrix to apply to the path.
     * @param bounding_rect The bounding rectangle of the path if known, default is nullptr.
     */
    static void draw(const geom::quadratic_path& path, const Stroke& stroke, const mat2x3& transform, const rect* bounding_rect = nullptr);

    /**
     * @brief Draws a QuadraticPath with the provided Fill properties.
     *
     * @param path The QuadraticPath to draw.
     * @param fill The Fill properties to use.
     * @param transform The transformation matrix to apply to the path.
     * @param bounding_rect The bounding rectangle of the path if known, default is nullptr.
     */
    static void draw(const geom::quadratic_path& path, const Fill& fill, const mat2x3& transform, const rect* bounding_rect = nullptr);

    /**
     * @brief Draws a CubicPath with the provided Fill properties.
     *
     * @param path The CubicPath to draw.
     * @param fill The Fill properties to use.
     * @param transform The transformation matrix to apply to the path.
     * @param bounding_rect The bounding rectangle of the path if known, default is nullptr.
     */
    static void draw(const geom::cubic_path& path, const Fill& fill, const mat2x3& transform, const rect* bounding_rect = nullptr);

    /**
     * @brief Draws the outline of a QuadraticPath.
     *
     * @param path The QuadraticPath to draw.
     * @param transform The transformation matrix to apply to the path.
     * @param tolerance The tolerance to use when approximating the path, default is 0.25.
     * @param stroke The Stroke properties to use, can be nullptr.
     * @param bounding_rect The bounding rectangle of the path if known, default is nullptr.
     */
    static void draw_outline(const geom::quadratic_path& path, const mat2x3& transform, const float tolerance = 0.25f, const Stroke* stroke = nullptr, const rect* bounding_rect = nullptr);

    /**
     * @brief Draws the outline of a Path.
     *
     * @param path The Path to draw.
     * @param transform The transformation matrix to apply to the path.
     * @param tolerance The tolerance to use when approximating the path, default is 0.25.
     * @param stroke The Stroke properties to use, can be nullptr.
     * @param bounding_rect The bounding rectangle of the path if known, default is nullptr.
     */
    static void draw_outline(const geom::Path<float, std::enable_if<true>>& path, const mat2x3& transform, const float tolerance = 0.25f, const Stroke* stroke = nullptr, const rect* bounding_rect = nullptr);

    /**
     * @brief Draws the vertices of a Path's outline.
     *
     * @param path The Path to draw the vertices of.
     * @param transform The transformation matrix to apply to the path.
     * @param selected_vertices The indices of the selected vertices, if nullptr all vertices are selected.
     * @param stroke The Stroke properties to use, can be nullptr.
     * @param bounding_rect The bounding rectangle of the path if known, default is nullptr.
     */
    static void draw_outline_vertices(const geom::Path<float, std::enable_if<true>>& path, const mat2x3& transform, const std::unordered_set<uint32_t>* selected_vertices = nullptr, const Stroke* stroke = nullptr, const rect* bounding_rect = nullptr);

    /**
     * @brief Draws a rectangle with the provided color.
     *
     * @param rect The rectangle to draw.
     * @param color The color to use.
     */
    static void draw_rect(const rect& rect, const std::optional<vec4> color = std::nullopt);

    /**
     * @brief Draws a rectangle with the provided color.
     *
     * @param center The center of the rectangle.
     * @param size The size of the rectangle.
     * @param color The color to use.
     */
    static void draw_rect(const vec2 center, const vec2 size, const std::optional<vec4> color = std::nullopt);

#ifdef GK_DEBUG
    static void draw_debug_overlays(const geom::cubic_bezier& cubic);
#endif
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
    GPU::Programs m_programs;                            /* The shader programs to use. */

    Viewport m_viewport;                                 /* The viewport to render to. */
    dmat4 m_vp_matrix;                                   /* The view-projection matrix. */

    std::vector<mat4> m_transforms;                      /* The model-view-projection matrices of the paths. */

    PathInstancedData m_path_instances;                  /* The path instances to render. */

    InstancedData<LineInstance> m_line_instances;        /* The line instances to render. */
    InstancedData<CircleInstance> m_circle_instances;    /* The handle instances to render. */
    InstancedData<RectInstance> m_rect_instances;        /* The rect instances to render. */
    // InstancedData<vec2> m_vertex_instances;          /* The vertex instances to render. */
    // InstancedData<vec2> m_white_vertex_instances;    /* The vertex instances to render. */

    UIOptions m_ui_options;                          /* The UI options (i.e. handle size, colors, etc.). */
  private:
    static Renderer* s_instance;    /* The singleton instance of the renderer. */
  };

}
