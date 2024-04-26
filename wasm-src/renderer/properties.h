/**
 * @file properties.h
 * @brief Contains the properties used to render a path.
 */

#pragma once

#include "../math/vec4.h"

#include "../geom/options.h"

namespace graphick::renderer {

  using LineCap = geom::LineCap;
  using LineJoin = geom::LineJoin;
  using FillRule = geom::FillRule;

  /**
   * @brief Fill properties used to render a path.
   *
   * @struct Fill
   */
  struct Fill {
    vec4 color;       /* The color used to fill the path. */

    FillRule rule;    /* The fill rule used to determine how self-intersecting paths are filled. */

    float z_index;    /* The z-index of the fill. */

    /**
     * @brief Default constructor
     */
    Fill() :
      color(0.0f, 0.0f, 0.0f, 1.0f),
      rule(FillRule::NonZero),
      z_index(0.0f) {}

    /**
     * @brief Complete constructor
     */
    Fill(const vec4& color, FillRule rule, float z_index) :
      color(color),
      rule(rule),
      z_index(z_index) {}
  };

  /**
   * @brief Stroke properties used to render a path.
   *
   * @struct Stroke
   */
  struct Stroke {
    vec4 color;           /* The color used to stroke the path. */

    LineCap cap;          /* The line cap used to determine how the ends of a stroke are drawn. */
    LineJoin join;        /* The line join used to determine how the corners of a stroke are drawn. */

    float width;          /* The width of the stroke. */
    float miter_limit;    /* The miter limit used to determine whether the join is mitered or beveled. */
    float z_index;        /* The z-index of the stroke. */

    /**
     * @brief Default constructor
     */
    Stroke() :
      color(0.0f, 0.0f, 0.0f, 1.0f),
      cap(LineCap::Butt),
      join(LineJoin::Miter),
      width(1.0f),
      miter_limit(10.0f),
      z_index(0.0f) {}

    /**
     * @brief Complete constructor
     */
    Stroke(const vec4& color, LineCap cap, LineJoin join, float width, float miter_limit, float z_index) :
      color(color),
      cap(cap),
      join(join),
      width(width),
      miter_limit(miter_limit),
      z_index(z_index) {}
  };

}
