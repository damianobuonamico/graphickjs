/**
 * @file renderer.h
 * @brief The file contains the definition of the main Graphick renderer.
 *
 * @todo remove using declarations when namespaces are unified
 */

#pragma once

#include <optional>
#include <unordered_set>

#include "../geom/cubic_bezier.h"
#include "../geom/cubic_path.h"
#include "../geom/quadratic_path.h"

#include "../math/mat2x3.h"
#include "../math/mat4.h"

#include "../utils/defines.h"

#include "gpu/shaders.h"

#include "renderer_data.h"

namespace graphick::geom {
template<typename T, typename>
class Path;
using path = geom::Path<float, std::enable_if<true>>;
}  // namespace graphick::geom

namespace graphick::renderer {

/**
 * @brief The main Graphick renderer.
 *
 * The renderer takes Paths as input and draws them on the screen based on the provided Stroke,
 * Fill and outline properties.
 *
 * Takes floats as input, but uses doubles internally for better precision.
 */
class Renderer {
 public:
 public:
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
   * @param options The RenderOptions to use for the frame.
   */
  static void begin_frame(const RenderOptions& options);

  /**
   * @brief Ends the current frame.
   *
   * This function should be called at the end of each frame after all draw calls have been issued.
   */
  static void end_frame();

  /**
   * @brief Draws a Path with the provided Fill and Stroke properties.
   *
   * @param path The Path to draw.
   * @param transform The transformation matrix to apply to the path.
   * @param options The DrawingOptions to use.
   * @return true if the path was visible and drawn, false otherwise.
   */
  static bool draw(const geom::path& path, const mat2x3& transform, const DrawingOptions& options);

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
  static void draw_rect(const vec2 center,
                        const vec2 size,
                        const std::optional<vec4> color = std::nullopt);

  /**
   * @brief Draws a line with the provided color.
   *
   * @param start The start of the line.
   * @param end The end of the line.
   * @param color The color to use.
   */
  static void draw_line(const vec2 start,
                        const vec2 end,
                        const std::optional<vec4> color = std::nullopt);

  /**
   * @brief Draws a circle with the provided color.
   *
   * @param center The center of the circle.
   * @param radius The radius of the circle.
   * @param color The color to use.
   */
  static void draw_circle(const vec2 center,
                          const float radius,
                          const std::optional<vec4> color = std::nullopt);

 private:
  struct PathDrawable {
    const rect& bounding_rect;
    const vec2 bounds_size;

    const Fill& fill;

    const size_t curves_start_index;
    const size_t bands_start_index;
    const size_t length;

    const bool culling;

    std::vector<vec2> min;
    std::vector<vec2> max;

    uint8_t horizontal_bands;
    float band_delta;
  };

  struct PathCullingData {
    std::vector<Band> bands;
    std::vector<std::vector<Intersection>> bottom_intersections;

    size_t accumulator = 0;

    inline PathCullingData(const size_t length) : bands(length), bottom_intersections(length) {}
  };

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
  static inline Renderer* get()
  {
    return s_instance;
  }

  /**
   * @brief Draws a transformed Path with the provided Fill and Stroke properties.
   *
   * @param path The transformed Path to draw.
   * @param fill The Fill properties to use.
   * @param stroke The Stroke properties to use.
   * @param bounding_rect The bounding rectangle of the path.
   */
  void draw_transformed(const geom::Path<float, std::enable_if<true>>& path,
                        const rect& bounding_rect,
                        const Fill* fill = nullptr,
                        const Stroke* stroke = nullptr);

  bool draw_cubic_path_impl(PathDrawable& drawable);

  void draw_cubic_path_cull(PathDrawable& drawable,
                            PathCullingData& data,
                            const geom::CubicPath<float, std::enable_if<true>>& path);

  void draw_cubic_path_cull_commit(PathDrawable& drawable, PathCullingData& data);

  /**
   * @brief Draws a cubic_path with the provided Fill properties.
   *
   * @param path The transformed CubicPath to draw.
   * @param bounding_rect The bounding rectangle of the path.
   * @param fill The Fill properties to use.
   * @param culling Whether to perform culling (calculate smaller tiles and fills).
   */
  void draw_cubic_path(const geom::CubicPath<float, std::enable_if<true>>& path,
                       const rect& bounding_rect,
                       const Fill& fill,
                       const bool culling = false);

  /**
   * @brief Draws multiple cubic_paths with the provided Fill properties.
   *
   * @param paths The transformed CubicPaths to draw.
   * @param bounding_rect The bounding rectangle of the path.
   * @param fill The Fill properties to use.
   * @param culling Whether to perform culling (calculate smaller tiles and fills).
   */
  void draw_cubic_paths(
      const std::vector<const geom::CubicPath<float, std::enable_if<true>>*>& paths,
      const rect& bounding_rect,
      const Fill& fill,
      const bool culling = false);

  /**
   * @brief Draws the individual vertices of a path.
   *
   * @param path The Path to draw.
   * @param transform The transformation matrix to apply to the path.
   * @param selected_vertices The indices of the selected vertices, if nullptr all vertices are
   * selected.
   * @param stroke The Stroke properties to use.
   */
  void draw_outline_vertices(const geom::Path<float, std::enable_if<true>>& path,
                             const mat2x3& transform,
                             const std::unordered_set<uint32_t>* selected_vertices,
                             const Stroke* stroke);

  /**
   * @brief Flushes the cached elements to the framebuffer.
   */
  void flush_cache();

  /**
   * @brief Flushes the renderer.
   *
   * This function issues the draw calls to the GPU.
   */
  void flush_meshes();

  /**
   * @brief Flushes the overlay renderer.
   *
   * This function issues the draw calls to the GPU for UI and overlay elements.
   */
  void flush_overlay_meshes();

 private:
  GPU::Programs m_programs;           // The shader programs to use.
  GPU::VertexArrays m_vertex_arrays;  // The vertex arrays to use.

  GPU::Framebuffer
      m_framebuffer;         // The framebuffer to render elements to, it is used for caching.

  Viewport m_last_viewport;  // The last viewport, used for diffing.
  Viewport m_viewport;       // The viewport to render to.

  std::vector<rect> m_invalid_rects;  // The invalid rects to clip against.
  editor::Cache* m_cache;             // The cache to use.
  rect m_safe_clip_rect;    // The largest clip rect that can be used with cached rendering.
  bool m_cached_rendering;  // Whether to use cached rendering.

  dmat4 m_vp_matrix;        // The view-projection matrix.

  InstancedData<LineInstance> m_line_instances;      // The line instances to render.
  InstancedData<CircleInstance> m_circle_instances;  // The handle instances to render.
  InstancedData<RectInstance> m_rect_instances;      // The rect instances to render.
  InstancedData<ImageInstance> m_image_instances;    // The image instances to render.

  Batch m_batch;                                     // The tile/fill batch to render.

  UIOptions m_ui_options;       // The UI options (i.e. handle size, colors, etc.).
 private:
  static Renderer* s_instance;  // The singleton instance of the renderer.
};

}  // namespace graphick::renderer
