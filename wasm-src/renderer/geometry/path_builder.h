/**
 * @file path_builder.h
 * @brief PathBuilder class definition
 *
 * A PathBuilder is used to generate drawables from a path. It computes strokes and fills.
 *
 * @todo stroke() method
 * @todo fill_and_stroke() method
*/

#pragma once

#include "../drawable.h"

#include "../properties.h"

#include "../../math/mat2x3.h"
#include "../../math/rect.h"

namespace Graphick::Renderer::Geometry {
  class Path;
}

namespace Graphick::Renderer::Geometry {

  class PathBuilder {
  public:
    /**
     * @brief Constructor and destructor.
     */
    PathBuilder(const rect& clip, const dmat2x3& transform, const double tolerance);
    ~PathBuilder() = default;

    /**
     * @brief Generate the outline drawable of a path.
     *
     * @param path The path to outline.
     * @return The drawable.
     */
    OutlineDrawable outline(const Path& path) const;

    /**
     * @brief Generate the fill drawable of a path.
     *
     * @param path The path to fill.
     * @param fill The fill properties.
     * @return The drawable.
     */
    Drawable fill(const Path& path, const Fill& fill) const;

    /**
     * @brief Generate the stroke drawable of a path.
     *
     * @param path The path to stroke.
     * @param stroke The stroke properties.
     * @return The drawable.
     */
    Drawable stroke(const Path& path, const Stroke& stroke) const;
  private:
    double m_tolerance;            /* The tessellation tolerance. */
    rect m_clip;                   /* The clipping rect. */
    const dmat2x3 m_transform;     /* The transformation matrix. */
  };

}
