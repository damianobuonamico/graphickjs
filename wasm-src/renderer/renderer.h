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

#include "properties.h"
#include "renderer_data.h"

namespace graphick::geom {
template <typename T, typename>
class Path;
}

namespace graphick::renderer {

/**
 * @brief The main Graphick renderer.
 *
 * The renderer takes QuadraticPaths as input and draws them on the screen based on the provided Stroke and Fill
 * properties.
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
   * @brief Draws a CubicPath with the provided Fill properties.
   *
   * Takes ownership of the path and caches it for accelerated rendering.
   *
   * @param path The CubicPath to draw.
   * @param fill The Fill properties to use.
   * @param transform The transformation matrix to apply to the path.
   * @param bounding_rect The bounding rectangle of the path if known, default is nullptr.
   * @param pretransformed_rect Whether the bounding rectangle is already transformed, default is false.
   */
  static void draw(
    geom::cubic_path&& path,
    const Fill& fill,
    const mat2x3& transform,
    const rect* bounding_rect = nullptr,
    const bool pretransformed_rect = false
  );

  /**
   * @brief Draws the outline of a Path.
   *
   * @param path The Path to draw.
   * @param transform The transformation matrix to apply to the path.
   * @param tolerance The tolerance to use when approximating the path, default is 0.25.
   * @param draw_vertices Whether to draw the vertices of the path, default is false.
   * @param selected_vertices The indices of the selected vertices, if nullptr all vertices are selected.
   * @param stroke The Stroke properties to use, can be nullptr.
   * @param bounding_rect The bounding rectangle of the path if known, default is nullptr.
   * @param pretransformed_rect Whether the bounding rectangle is already transformed, default is false.
   */
  static void draw_outline(
    const geom::Path<float, std::enable_if<true>>& path,
    const mat2x3& transform,
    const float tolerance = 0.25f,
    const bool draw_vertices = false,
    const std::unordered_set<uint32_t>* selected_vertices = nullptr,
    const Stroke* stroke = nullptr,
    const rect* bounding_rect = nullptr,
    const bool pretransformed_rect = false
  );

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
   * @brief Pushes a new transformation matrix to the renderer if needed.
   *
   * If an identical transformation matrix is already present, the index of the existing matrix is returned.
   *
   * @param transform The transformation matrix to push.
   * @return The index of the transformation matrix.
   */
  uint32_t push_transform(const mat2x3& transform);

  /**
   * @brief Draws a CubicPath with the provided Fill properties without performing clipping.
   *
   * Takes ownership of the path and caches it for accelerated rendering.
   * If the screen-space footpring is small, no culling is performed.
   * This method should be used for elements that are almost completely visible.
   *
   * @param path The CubicPath to draw.
   * @param fill The Fill properties to use.
   * @param transform The transformation matrix to apply to the path.
   * @param bounding_rect The bounding rectangle of the path.
   * @param transformed_bounding_rect The bounding rectangle of the transformed path.
   * @param culling Whether to perform culling.
   */
  void draw_no_clipping(
    geom::cubic_path&& path,
    const Fill& fill,
    const mat2x3& transform,
    const rect& bounding_rect,
    const rect& transformed_bounding_rect,
    const bool culling
  );

  /**
   * @brief Draws a CubicPath with the provided Fill properties with clipping and culling.
   *
   * Takes ownership of the path and caches it for accelerated rendering.
   * This method should be used for elements that are mostly occluded.
   *
   * @param path The CubicPath to draw.
   * @param fill The Fill properties to use.
   * @param transform The transformation matrix to apply to the path.
   * @param bounding_rect The bounding rectangle of the path.
   */
  void draw_with_clipping(geom::cubic_path&& path, const Fill& fill, const mat2x3& transform, const rect& bounding_rect);

  /**
   * @brief Draws the individual vertices of a path.
   *
   * @param path The Path to draw.
   * @param transform The transformation matrix to apply to the path.
   * @param tolerance The tolerance to use when approximating the path.
   * @param selected_vertices The indices of the selected vertices, if nullptr all vertices are selected.
   * @param stroke The Stroke properties to use.
   */
  void draw_outline_vertices(
    const geom::Path<float, std::enable_if<true>>& path,
    const mat2x3& transform,
    const float tolerance,
    const std::unordered_set<uint32_t>* selected_vertices,
    const Stroke* stroke
  );

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
  template <typename T>
  using IData = InstancedData<T>;
private:
  GPU::Programs m_programs;                               // The shader programs to use.
  GPU::VertexArrays m_vertex_arrays;                      // The vertex arrays to use.

  std::unique_ptr<GPU::Framebuffer> m_framebuffer;        // The framebuffer to render elements to, it is used for caching.

  Viewport m_last_viewport;                               // The last viewport, used for diffing.
  Viewport m_viewport;                                    // The viewport to render to.

  std::vector<rect> m_invalid_rects;                      // The invalid rects to clip against.
  editor::Cache* m_cache;                                 // The cache to use.
  rect m_safe_clip_rect;                                  // The largest clip rect that can be used with cached rendering.
  bool m_cached_rendering;                                // Whether to use cached rendering.

  dmat4 m_vp_matrix;                                      // The view-projection matrix.

  std::vector<vec4> m_transform_vectors;                  // The model matrices of the paths decomposed in two vectors each.
  size_t m_max_transform_vectors;                         // The maximum number of vectors in the transforms array (twice the
                                                          // number of transforms).

  PathInstancedData m_path_instances;                     // The path instances to render.
  BoundarySpanInstancedData m_boundary_span_instances;    // The boundary spans to render.

  IData<LineInstance> m_line_instances;                   // The line instances to render.
  IData<CircleInstance> m_circle_instances;               // The handle instances to render.
  IData<RectInstance> m_rect_instances;                   // The rect instances to render.
  IData<ImageInstance> m_image_instances;                 // The image instances to render.
  IData<FilledSpanInstance> m_filled_span_instances;      // The filled spans to render.

  UIOptions m_ui_options;                                 // The UI options (i.e. handle size, colors, etc.).
private:
  static Renderer* s_instance;                            // The singleton instance of the renderer.
};

}
