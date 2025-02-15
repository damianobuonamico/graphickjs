/**
 * @file renderer.h
 * @brief The file contains the definition of the main Graphick renderer.
 */

#pragma once

#include "../math/mat2x3.h"

#include "../utils/defines.h"

#include "gpu/shaders.h"

#include "instances.h"
#include "renderer_data.h"
#include "tiles.h"

namespace graphick::geom {
template<typename T, typename>

class Path;

using path = geom::Path<float, std::enable_if<true>>;
using dpath = geom::Path<double, std::enable_if<true>>;

}  // namespace graphick::geom

#ifdef GK_DEBUG
#  define __debug_rect(...) graphick::renderer::Renderer::__debug_rect_impl(__VA_ARGS__)
#  define __debug_square(...) graphick::renderer::Renderer::__debug_square_impl(__VA_ARGS__)
#  define __debug_circle(...) graphick::renderer::Renderer::__debug_circle_impl(__VA_ARGS__)
#  define __debug_line(...) graphick::renderer::Renderer::__debug_line_impl(__VA_ARGS__)
#  define __debug_lines(...) graphick::renderer::Renderer::__debug_lines_impl(__VA_ARGS__)
#  define __debug_text(...) graphick::renderer::Renderer::__debug_text_impl(__VA_ARGS__)
#else
#  define __debug_rect(...) ((void)0)
#  define __debug_square(...) ((void)0)
#  define __debug_circle(...) ((void)0)
#  define __debug_line(...) ((void)0)
#  define __debug_lines(...) ((void)0)
#  define __debug_text(...) ((void)0)
#endif

namespace graphick::renderer {

/**
 * @brief The main Graphick renderer.
 *
 * The renderer takes Paths, Texts and Images as input and draws them on the screen based on the
 * provided Stroke, Fill and Outline properties.
 *
 * The renderer currently has 4 rendering layers:
 *  1. The background layer (solid color or pattern in the future).
 *  2. The scene layer (paths, texts and images that compose the scene), scene-space coordinates.
 *  3. The UI layer (outlines, vertices and handles, drawn in this order), scene-space coordinates.
 *  4. The debug layer (bounding rects, timings, etc.), screen-space coordinates.
 *
 * The __debug_[...] methods are stripped away if GK_DEBUG is not defined.
 * The debug layer has no flush method, it is drawn in immediate mode through the __debug_[...]
 * methods.
 *
 * Takes floats as input, but uses doubles internally for better precision.
 */
class Renderer {
 public:
  Renderer(const Renderer&) = delete;
  Renderer(Renderer&&) = delete;

  /**
   * @brief Returns the viewport size of the renderer.
   *
   * @return The viewport size of the renderer.
   */
  inline static ivec2 viewport_size()
  {
    return get()->m_viewport.size;
  }

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
   * @brief Draws a Path with the provided Fill, Stroke and Outline properties in the scene layer.
   *
   * @param path The Path to draw.
   * @param transform The transformation matrix to apply to the path.
   * @param options The DrawingOptions to use.
   * @param id The id used for caching, default is uuid::null.
   * @return true if the path was visible and drawn, false otherwise.
   */
  static bool draw(const geom::path& path,
                   const mat2x3& transform,
                   const DrawingOptions& options,
                   const uuid id = uuid::null);

  /**
   * @brief Draws a Text with the provided Fill, Stroke and Outline properties in the scene layer.
   *
   * @param text The Text to draw.
   * @param transform The transformation matrix to apply to the text.
   * @param options The DrawingOptions to use.
   * @param id The id used for caching, default is uuid::null.
   * @return true if the text was visible and drawn, false otherwise.
   */
  static bool draw(const renderer::Text& text,
                   const mat2x3& transform,
                   const DrawingOptions& options,
                   const uuid id = uuid::null);

  /**
   * @brief Draws an Image with the provided Stroke and Outline properties (Fill is ignored) in the
   * scene layer.
   *
   * @param image The Image to draw.
   * @param transform The transformation matrix to apply to the image.
   * @param options The DrawingOptions to use.
   * @param id The id used for caching, default is uuid::null.
   * @return true if the image was visible and drawn, false otherwise.
   */
  static bool draw(const renderer::Image& image,
                   const mat2x3& transform,
                   const DrawingOptions& options,
                   const uuid id = uuid::null);

  /**
   * @brief Draws a line with the provided color in the UI layer.
   *
   * @param start The start of the line.
   * @param end The end of the line.
   * @param color The color to use, default is the primary color.
   * @param width The width of the line, default is the line width.
   */
  inline static void ui_line(const vec2 start,
                             const vec2 end,
                             const vec4& color = vec4::identity(),
                             const float width = 1.0f)
  {
    get()->m_instances.push_line(start, end, color, width);
  }

  /**
   * @brief Draws a rectangle with the provided color in the UI layer.
   *
   * @param rect The rectangle to draw.
   * @param color The color to use, default is white.
   */
  inline static void ui_rect(const rect& rect, const vec4& color = vec4::identity())
  {
    get()->m_instances.push_rect(rect.center(), rect.size(), color);
  }

  /**
   * @brief Draws a square with the provided color in the UI layer.
   *
   * @param center The center of the square.
   * @param radius The radius of the square (half the length of one side).
   * @param color The color to use, default is white.
   */
  static void ui_square(const vec2 center,
                        const float radius,
                        const vec4& color = vec4::identity())
  {
    get()->m_instances.push_rect(center, vec2(radius * 2.0f), color);
  }

  /**
   * @brief Draws a circle with the provided color in the UI layer.
   *
   * @param center The center of the circle.
   * @param radius The radius of the circle.
   * @param color The color to use, default is white.
   */
  static void ui_circle(const vec2 center,
                        const float radius,
                        const vec4& color = vec4::identity())
  {
    get()->m_instances.push_circle(center, radius, color);
  }

#ifdef GK_DEBUG

  /**
   * @brief Draws a rectangle with the provided color in the debug layer.
   *
   * This method is stripped away if GK_DEBUG is not defined, use __debug_rect macro instead.
   *
   * @param rect The rectangle to draw.
   * @param color The color to use, default is white.
   */
  static void __debug_rect_impl(const rect& rect, const vec4& color = vec4::identity());

  /**
   * @brief Draws a square with the provided color in the debug layer.
   *
   * This method is stripped away if GK_DEBUG is not defined, use __debug_square macro instead.
   *
   * @param center The center of the square.
   * @param radius The radius of the square (half the length of one side).
   * @param color The color to use, default is white.
   */
  static void __debug_square_impl(const vec2 center,
                                  const float radius,
                                  const vec4& color = vec4::identity());

  /**
   * @brief Draws a circle with the provided color in the debug layer.
   *
   * This method is stripped away if GK_DEBUG is not defined, use __debug_circle macro instead.
   *
   * @param center The center of the circle.
   * @param radius The radius of the circle.
   * @param color The color to use.
   */
  static void __debug_circle_impl(const vec2 center,
                                  const float radius,
                                  const vec4& color = vec4::identity());

  /**
   * @brief Draws a line with the provided color in the debug layer.
   *
   * This method is stripped away if GK_DEBUG is not defined, use __debug_line macro instead.
   *
   * @param start The start of the line.
   * @param end The end of the line.
   * @param color The color to use.
   */
  static void __debug_line_impl(const vec2 start,
                                const vec2 end,
                                const vec4& color = vec4::identity());

  /**
   * @brief Draws lines with the provided color in the debug layer.
   *
   * This method is stripped away if GK_DEBUG is not defined, use __debug_lines macro instead.
   *
   * @param points The points to draw.
   * @param color The color to use.
   */
  static void __debug_lines_impl(const std::vector<vec2>& points,
                                 const vec4& color = vec4::identity());

  /**
   * @brief Draws text with the provided color in the debug layer.
   *
   * This method is stripped away if GK_DEBUG is not defined, use __debug_text macro instead.
   *
   * @param text The text to draw.
   * @param position The position of the text.
   * @param color The color to use, default is white.
   * @return The width of the text.
   */
  static float __debug_text_impl(const std::string& text,
                                 const vec2 position,
                                 const vec4& color = vec4::identity());

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
   * @param texture_coords The texture coordinates to use for the fill.
   * @return true if the path was visible and drawn, false otherwise.
   */
  bool draw_transformed(const geom::dpath& path,
                        const drect& bounding_rect,
                        const DrawingOptions& options,
                        const std::array<vec2, 4>& texture_coords,
                        const uuid id);

  /**
   * @brief Draws a cubic multipath with the provided Fill properties.
   *
   * @param path The cubic multipath to draw.
   * @param bounding_rect The bounding rectangle of the path.
   * @param fill The Fill properties to use.
   * @param texture_coords The texture coordinates to use for the fill.
   * @param drawable The Drawable to use.
   * @return true if the path was visible and drawn, false otherwise.
   */
  bool draw_multipath(const geom::dcubic_multipath& path,
                      const drect& bounding_rect,
                      const Fill& fill,
                      const std::array<vec2, 4>& texture_coords,
                      Drawable& drawable);

  /**
   * @brief Draws the outline of a path.
   *
   * @param path The Path to draw.
   * @param bounding_rect The bounding rectangle of the path.
   * @param outline The Outline properties to use.
   */
  void draw_outline(const geom::dpath& path, const drect& bounding_rect, const Outline& outline);

  /**
   * @brief Draws the individual vertices of a path.
   *
   * @param path The Path to draw.
   * @param outline The Outline properties to use.
   */
  void draw_outline_vertices(const geom::dpath& path, const Outline& outline);

  /**
   * @brief If needed, uploads the texture to the GPU.
   *
   * @param texture_id The id of the texture stored in the ResourceManager cache.
   */
  void request_texture(const uuid texture_id);

  /**
   * @brief Flushes the background layer.
   *
   * This method takes care of the viewport setup (i.e. setting the viewport size)
   */
  void flush_background_layer();

  /**
   * @brief Flushes the scene layer.
   */
  void flush_scene_layer();

  /**
   * @brief Flushes the UI layer.
   */
  void flush_ui_layer();

#ifdef GK_DEBUG

  /**
   * @brief The DebugOptions struct to use when flushing the debug layer.
   */
  struct DebugOptions {
    bool show_LODs;  // Whether to show the scene grid up to the smallest LOD level in use.
  };

  /**
   * @brief Flushes the debug layer.
   *
   * This method is stripped away if GK_DEBUG is not defined.
   *
   * @param options The DebugOptions to use.
   */
  void __debug_flush_layer(const DebugOptions& options) const;

#endif

 private:
  GPU::Programs m_programs;                           // The shader programs to use.
  GPU::VertexArrays m_vertex_arrays;                  // The vertex arrays to use.

  Viewport m_viewport;                                // The viewport of the renderer.
  Tiler m_tiler;                                      // Takes care of tiling and LOD.

  std::unordered_map<uuid, GPU::Texture> m_textures;  // The textures loaded in the GPU.

  InstancedRenderer m_instances;                      // The line instances.
  TiledRenderer m_tiles;                              // The tiles renderer.

  UIOptions m_ui_options;       // The UI options (i.e. handle size, colors, etc.).

  RendererCache* m_cache;       // The cache to use for the renderer.
 private:
  static Renderer* s_instance;  // The singleton instance of the renderer.
};

}  // namespace graphick::renderer
