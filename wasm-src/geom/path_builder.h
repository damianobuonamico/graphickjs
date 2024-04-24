/**
 * @file geom/path_builder.h
 * @brief Containes the PathBuilder class definition and its options.
 */

#pragma once

namespace graphick::geom {

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
   * @brief The options used to stroke a path.
   */
  template <typename T>
  struct StrokingOptions {
    T width;          /* The distance between the two offset curves of a stroke. */
    T miter_limit;    /* The miter limit used to determine whether the line join is mitered or beveled. */

    LineCap cap;      /* The line cap used to determine how the ends of a stroke are drawn. */
    LineJoin join;    /* The line join used to determine how the corners of a stroke are drawn. */
  };

  /**
   * @brief The options used to fill a path.
   */
  struct FillingOptions {
    FillRule rule;    /* The fill rule used to determine how self-intersecting paths are filled. */
  };
}
