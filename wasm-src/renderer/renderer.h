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

#include "renderer_cache.h"
#include "renderer_data.h"

namespace graphick::geom {
template<typename T, typename>

class Path;

using path = geom::Path<float, std::enable_if<true>>;
using dpath = geom::Path<double, std::enable_if<true>>;

}  // namespace graphick::geom

// TEMP
namespace graphick::editor {
class TextComponent;
}

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
   * @brief Draws a Path with the provided Fill and Stroke properties.
   *
   * @param path The Path to draw.
   * @param transform The transformation matrix to apply to the path.
   * @param options The DrawingOptions to use.
   * @param id The id used for caching.
   * @return true if the path was visible and drawn, false otherwise.
   */
  static bool draw(const geom::path& path,
                   const mat2x3& transform,
                   const DrawingOptions& options,
                   const uuid id);

  /**
   * @brief Draws a Text with the provided Fill and Stroke properties.
   *
   * @param text The Text to draw.
   * @param transform The transformation matrix to apply to the text.
   * @param options The DrawingOptions to use.
   * @param id The id used for caching.
   * @return true if the text was visible and drawn, false otherwise.
   */
  static bool draw(const renderer::Text& text,
                   const mat2x3& transform,
                   const DrawingOptions& options,
                   const uuid id);
  /**
   * @brief Draws an image with the provided image_id.
   *
   * @param image_id The id of the image in the ResourceManager to draw.
   * @param transform The transformation matrix to apply to the image.
   */
  static bool draw_image(const uuid image_id, const mat2x3& transform);

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
  // TODO: delete this

  /**
   * @brief Groups the data required to be populated in order to draw a path.
   */
  struct PathDrawable {
    const drect& bounding_rect;       // The bounding rectangle of the path.
    const dvec2 bounds_size;          // The size of the bounding rectangle.

    const Fill& fill;                 // The Fill properties to use.

    const size_t curves_start_index;  // The index of the first curve in the curves buffer.
    const size_t bands_start_index;   // The index of the first band in the bands buffer.
    const size_t num;                 // The number of curves in the path.

    const bool culling;               // Whether to perform culling.

    std::vector<dvec2> min;           // The cached minimum points of the curves.
    std::vector<dvec2> max;           // The cached maximum points of the curves.

    uint8_t horizontal_bands;         // The number of horizontal bands.
    double band_delta;                // The height of a band.
  };

  /**
   * @brief Groups the data required to perform culling on a path.
   */
  struct PathCullingData {
    std::vector<Band> bands;   // The culling bands.
    std::vector<std::vector<Intersection>>
        bottom_intersections;  // The intersections with the bottom edge of the bands.

    size_t accumulator = 0;    // The number of contours processed.

    inline PathCullingData(const size_t num) : bands(num), bottom_intersections(num) {}
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
   * @param bounding_rect The bounding rectangle of the path.
   * @param options The DrawingOptions to use.
   * @param cache_bounding_rect Wether the bounding rectangle should be stored in the cache.
   * @return true if the path was visible and drawn, false otherwise.
   */
  bool draw_transformed(const geom::dpath& path,
                        const drect& bounding_rect,
                        const DrawingOptions& options,
                        const std::array<vec2, 4>& fill_texture_coords,
                        const uuid id = uuid::null);

  /**
   * @brief Populates the PathData.
   *
   * @param data The PathData to populate.
   * @return true if the path was culled, false otherwise.
   */
  bool draw_cubic_path_impl(PathData& data);

  /**
   * @brief Populates the PathCullingData.
   *
   * @param path The transformed CubicPath to draw.
   * @param data The PathData to use.
   * @param culling_data The PathCullingData to populate.
   */
  void draw_cubic_path_cull(const geom::dcubic_path& path,
                            PathData& data,
                            PathCullingData& culling_data);

  /**
   * @brief Populates the PathData with the PathCullingData.
   *
   * @param data The PathData to populate.
   * @param culling_data The PathCullingData to use.
   */
  void draw_cubic_path_cull_commit(PathData& data, PathCullingData& culling_data);

  /**
   * @brief Draws a cubic_path with the provided Fill properties.
   *
   * @param path The transformed CubicPath to draw.
   * @param bounding_rect The bounding rectangle of the path.
   * @param fill The Fill properties to use.
   * @param culling Whether to perform culling (calculate smaller tiles and fills).
   * @param drawable The final drawable object to draw.
   */
  void draw_cubic_path(const geom::dcubic_path& path,
                       const drect& bounding_rect,
                       const Fill& fill,
                       const bool culling,
                       const std::array<vec2, 4>& texture_coords,
                       Drawable& drawable);

  /**
   * @brief Draws multiple cubic_paths with the provided Fill properties.
   *
   * @param paths The transformed CubicPaths to draw.
   * @param bounding_rect The bounding rectangle of the path.
   * @param fill The Fill properties to use.
   * @param culling Whether to perform culling (calculate smaller tiles and fills).
   * @param drawable The final drawable object to draw.
   */
  void draw_cubic_paths(const std::vector<const geom::dcubic_path*>& paths,
                        const drect& bounding_rect,
                        const Fill& fill,
                        const bool culling,
                        const std::array<vec2, 4>& texture_coords,
                        Drawable& drawable);

  /**
   * @brief Draws the individual vertices of a path.
   *
   * @param path The Path to draw.
   * @param transform The transformation matrix to apply to the path.
   * @param selected_vertices The indices of the selected vertices, if nullptr all vertices are
   * selected.
   * @param stroke The Stroke properties to use.
   */
  void draw_outline_vertices(const geom::dpath& path, const Outline& outline);

  /**
   * @brief Upload a Drawable object to the GPU.
   *
   * @param drawable The Drawable object to upload.
   */
  void draw(const Drawable& drawable);

  /**
   * @brief If needed, uploads the texture to the GPU.
   *
   * @param texture_id The id of the texture stored in the ResourceManager cache.
   */
  void request_texture(const uuid texture_id);

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
  RendererCache* m_cache;             // The cache to use for rendering.
  rect m_safe_clip_rect;    // The largest clip rect that can be used with cached rendering.
  bool m_cached_rendering;  // Whether to use cached rendering.

  dmat4 m_vp_matrix;        // The view-projection matrix.

  InstancedData<LineInstance> m_line_instances;       // The line instances to render.
  InstancedData<CircleInstance> m_circle_instances;   // The handle instances to render.
  InstancedData<RectInstance> m_rect_instances;       // The rect instances to render.
  InstancedData<ImageInstance> m_image_instances;     // The image instances to render.

  Batch m_batch;                                      // The tile/fill batch to render.

  std::unordered_map<uuid, GPU::Texture> m_textures;  // The textures loaded in the GPU.

  // TODO: should move this into the batch I think
  std::unordered_map<uuid, uint32_t> m_binded_textures;  // The textures bound to the GPU.
  uint32_t m_z_index;                                    // The current z-index.

  UIOptions m_ui_options;       // The UI options (i.e. handle size, colors, etc.).
 private:
  static Renderer* s_instance;  // The singleton instance of the renderer.
};

}  // namespace graphick::renderer
