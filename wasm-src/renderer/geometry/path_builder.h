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
  class PathDev;
}

namespace Graphick::Renderer::Geometry {

  class PathBuilder {
  public:
    /**
     * @brief Constructor and destructor.
     */
    PathBuilder(const rect& clip, const dmat2x3& transform, const double tolerance);
    ~PathBuilder() = default;

    Drawable fill(const PathDev& path, const Fill& fill);
    Drawable stroke(const PathDev& path, const Stroke& stroke);
  private:
    double m_tolerance;            /* The tessellation tolerance. */
    rect m_clip;                   /* The clipping rect. */
    const dmat2x3 m_transform;     /* The transformation matrix. */
  };

}
