/**
 * @file renderer_data.h
 * @brief The file contains data structures used by the renderer.
 *
 * @todo maybe replace vectors with preallocated arrays
 * @todo double buffer the tile data
 * @todo batches of tile data overflow handling
 */

#pragma once

#include "properties.h"
#include "renderer_settings.h"

#include "../math/mat4.h"
#include "../math/rect.h"

namespace graphick::renderer {

class RendererCache;

/**
 * @brief Represents the viewport of the renderer.
 *
 * The viewport is the area of the screen where the renderer will draw.
 */
struct Viewport {
  ivec2 size;               // The size of the viewport.
  dvec2 position;           // The position of the viewport.

  double zoom;              // The zoom level of the viewport (it is pre-multiplied by the dpr).
  double dpr;               // The device pixel ratio.

  vec4 background;          // The background color to clear the viewport with.

  dmat4 view_matrix;        // The view matrix of the viewport.
  dmat4 projection_matrix;  // The projection matrix of the viewport.
  dmat4 vp_matrix;          // The view-projection matrix of the viewport.
  dmat4 screen_vp_matrix;   // The screen view-projection matrix of the viewport.

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
  Viewport(const ivec2 size,
           const dvec2 position,
           const double zoom,
           const double dpr,
           const vec4& background)
      : size(size),
        position(position),
        zoom(zoom),
        dpr(dpr),
        background(background),
        m_visible({-position, dvec2(size) / zoom - position})
  {
    const dvec2 dsize = dvec2(size);

    const double factor = 0.5 / zoom;
    const double half_width = -dsize.x * factor;
    const double half_height = dsize.y * factor;

    projection_matrix = dmat4{{-1.0 / half_width, 0.0, 0.0, 0.0},
                              {0.0, -1.0 / half_height, 0.0, 0.0},
                              {0.0, 0.0, -1.0, 0.0},
                              {0.0, 0.0, 0.0, 1.0}};

    view_matrix = dmat4{{1.0, 0.0, 0.0, 0.5 * (-dsize.x / zoom + 2.0 * position.x)},
                        {0.0, 1.0, 0.0, 0.5 * (-dsize.y / zoom + 2.0 * position.y)},
                        {0.0, 0.0, 1.0, 0.0},
                        {0.0, 0.0, 0.0, 1.0}};

    dmat4 screen_projection = dmat4{{2.0 / dsize.x, 0.0, 0.0, 0.0},
                                    {0.0, -2.0 / dsize.y, 0.0, 0.0},
                                    {0.0, 0.0, -1.0, 0.0},
                                    {0.0, 0.0, 0.0, 1.0}};

    dmat4 screen_view = dmat4{{1.0, 0.0, 0.0, -0.5 * dsize.x},
                              {0.0, 1.0, 0.0, -0.5 * dsize.y},
                              {0.0, 0.0, 1.0, 0.0},
                              {0.0, 0.0, 0.0, 1.0}};

    vp_matrix = projection_matrix * view_matrix;
    screen_vp_matrix = screen_projection * screen_view;
  }

  /**
   * @brief Returns the scene-space visible area.
   *
   * @return The scene-space rectangle that is visible in the viewport.
   */
  inline drect visible() const
  {
    return m_visible;
  }

  /**
   * @brief Converts a point from client-space to scene-space.
   *
   * @param p The point in client-space.
   * @return The point in scene-space.
   */
  inline dvec2 project(const dvec2 p) const
  {
    return p / zoom - position;
  }

  /**
   * @brief Converts a point from scene-space to client-space.
   *
   * @param p The point in scene-space.
   * @return The point in client-space.
   */
  inline dvec2 revert(const dvec2 p) const
  {
    return (p + position) * zoom;
  }

 private:
  drect m_visible;  // The visible area of the viewport in scene-space coordinates.
};

/**
 * @brief Represents the options used to render the scene.
 */
struct RenderOptions {
  Viewport viewport;     // The viewport to render to.
  RendererCache* cache;  // The cache to use for rendering, can be nullptr.

  bool ignore_cache;     // Whether to redraw everything from scratch.
};

/**
 * @brief Represents the options to draw a path: fill, stroke and outline.
 */
struct DrawingOptions {
  const Fill* fill = nullptr;              // The fill to use, can be nullptr.
  const Stroke* stroke = nullptr;          // The stroke to use, can be nullptr.
  const Outline* outline = nullptr;        // The outline to use, can be nullptr.
  const Appearance* appearance = nullptr;  // The appearance to use, can be nullptr.
};

/**
 * @brief Collects all the UI related options, transformed based on the viewport.
 */
struct UIOptions {
  vec2 vertex_size;        // The size of a vertex.
  vec2 vertex_inner_size;  // The size of the white part of a vertex.

  float handle_radius;     // The radius of an handle.
  float line_width;        // The default width of the lines.

  vec4 primary_color;      // The primary color of the UI.
  vec4 primary_color_05;   // The primary color 5% darker.

  /**
   * @brief Constructs a new UIOptions object.
   *
   * @param factor The factor to scale the options with (dpr / zoom).
   */
  UIOptions(const double factor)
      : vertex_size(vec2(RendererSettings::ui_handle_size * factor)),
        vertex_inner_size(vec2((RendererSettings::ui_handle_size - 2.0) * factor)),
        handle_radius(static_cast<float>(RendererSettings::ui_handle_size * factor / 2.0)),
        line_width(static_cast<float>(RendererSettings::ui_line_width)),
        primary_color(RendererSettings::ui_primary_color),
        primary_color_05(RendererSettings::ui_primary_color * 0.95f)
  {
  }

  UIOptions() : UIOptions(1.0) {}
};

}  // namespace graphick::renderer
