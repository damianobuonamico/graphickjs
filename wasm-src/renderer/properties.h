/**
 * @file properties.h
 * @brief Contains the properties used to render a path.
 */

#pragma once

#include "../math/vec4.h"

namespace Graphick::Renderer {

  /**
   * @brief The line cap used to determine how the ends of a stroke are drawn.
   */
  enum class LineCap {
    Butt,     /* The ends of lines are squared off at the endpoints. */
    Round,    /* The ends of lines are rounded. */
    Square    /* The ends of lines are squared off by adding a box with an equal width and half the height of the line's thickness. */
  };

  /**
   * @brief The line join used to determine how the corners of a stroke are drawn.
   */
  enum class LineJoin {
    Miter,    /* Connected segments are joined by extending their outside edges to connect at a single point. */
    Round,    /* Rounds off the corners of a shape by filling an additional sector of disc centered at the common endpoint of connected segments. */
    Bevel     /* Fills an additional triangular area between the common endpoint of connected segments, and the separate outside rectangular corners of each segment. */
  };

  /**
   * @brief The fill rule used to determine how self-intersecting paths are filled.
   *
   * Paths that don't intersect themselves are unaffected by the fill rule.
   */
  enum class FillRule {
    NonZero,    /* The non-zero rule: <https://en.wikipedia.org/wiki/Nonzero-rule> */
    EvenOdd     /* The even-odd rule: <https://en.wikipedia.org/wiki/Even%E2%80%93odd_rule> */
  };

  /**
   * @brief Fill properties used to render a path.
   */
  struct Fill {
    vec4 color;       /* The color used to fill the path. */

    FillRule rule;    /* The fill rule used to determine how self-intersecting paths are filled. */

    float z_index;    /* The z-index of the fill. */
  };

  /**
   * @brief Stroke properties used to render a path.
   */
  struct Stroke {
    vec4 color;           /* The color used to stroke the path. */

    LineCap cap;          /* The line cap used to determine how the ends of a stroke are drawn. */
    LineJoin join;        /* The line join used to determine how the corners of a stroke are drawn. */

    float width;          /* The width of the stroke. */
    float miter_limit;    /* The miter limit used to determine whether the join is mitered or beveled. */
    float z_index;        /* The z-index of the stroke. */
  };

}
