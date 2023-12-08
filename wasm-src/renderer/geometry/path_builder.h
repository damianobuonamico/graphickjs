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

#include "../../math/rect.h"

namespace Graphick::Renderer::Geometry {
  class PathDev;
}

namespace Graphick::Renderer::Geometry {

  class PathBuilder {
  public:
    /**
     * @brief Constructor and destructor
     */
    PathBuilder(const rect& clip);
    ~PathBuilder() = default;

    Drawable fill(const PathDev& path, const Fill& fill);
  private:
    rect m_clip;        /* The clipping rect. */

    Fill m_fill;        /* The fill properties. */
    Stroke m_stroke;    /* The stroke properties. */
  };

}
