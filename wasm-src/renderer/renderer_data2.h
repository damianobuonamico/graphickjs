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

#include "../math/rect.h"

namespace graphick::renderer {

class RendererCache;

/**
 * @brief Represents the viewport of the renderer.
 *
 * The viewport is the area of the screen where the renderer will draw.
 */
struct Viewport {
  ivec2 size;       // The size of the viewport.
  dvec2 position;   // The position of the viewport.

  double zoom;      // The zoom level of the viewport (it is pre-multiplied by the dpr).
  double dpr;       // The device pixel ratio.

  vec4 background;  // The background color to clear the viewport with.

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
  const Fill* fill;        // The fill to use, can be nullptr.
  const Stroke* stroke;    // The stroke to use, can be nullptr.
  const Outline* outline;  // The outline to use, can be nullptr.
};

}  // namespace graphick::renderer
