/**
 * @file geom/path_builder.h
 * @brief Containes the PathBuilder class definition and its options.
 */

#pragma once

#include "options.h"

#include "quadratic_path.h"

#include "../math/mat2x3.h"
#include "../math/rect.h"

#include <functional>

namespace graphick::geom {

  /* -- Forward Declarations -- */

  template <typename T, typename>
  class Path;

  /**
   * @brief A structure to hold the inner and outer outlines of a stroke.
   *
   * If the path is not closed, the inner outline will be empty.
   *
   * @struct StrokeOutline
   */
  template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
  struct StrokeOutline {
    geom::QuadraticPath<T> outer;    /* The outer outline of the stroke. */
    geom::QuadraticPath<T> inner;    /* The inner outline of the stroke, in reverse order. */
  };

  /**
   * @brief The PathBuilder class is used to build strokes and flatten paths.
   *
   * @tparam T The input and output type of the path builder, intermediate operations are carried out in double precision.
   * @class PathBuilder
   */
  template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
  class PathBuilder {
  public:
    /**
     * @brief Constructs a PathBuilder.
     *
     * @param path The path to process.
     * @param transform The transformation matrix to apply to the path.
     * @param bounding_rect The bounding rectangle of the path if known, default is nullptr.
     */
    PathBuilder(
      const QuadraticPath<T>& path,
      const math::Mat2x3<T>& transform,
      const math::Rect<T>* bounding_rect = nullptr
    );

    /**
     * @brief Flattens a path and outputs the line segments to a sink vector.
     *
     * If the portion of the path visible is less than 50%, it is clipped.
     *
     * @param clip The rectangle to clip the path to.
     * @param tolerance The tolerance to use when flattening the path.
     * @param sink_callback The callback to output the lines to.
     */
    void flatten(
      const math::Rect<T>& clip,
      const T tolerance,
      std::function<void(const math::Vec2<T>, const math::Vec2<T>)> sink_callback
    ) const;

    /**
     * @brief Strokes a path and outputs the resulting quadratic curves grouped in contours.
     *
     * @param options The options to use when stroking the path.
     * @param tolerance The offset error tolerance.
     * @return The resulting quadratic curves grouped in contours.
     */
    StrokeOutline<T> stroke(const StrokingOptions<T>& options, const T tolerance) const;
    StrokeOutline<T> stroke(const Path<T, std::enable_if<true>>& path, const StrokingOptions<T>& options, const T tolerance) const;
  private:
    /**
     * @brief Clips and flattens a path and outputs the line segments to a sink vector.
     *
     * @param clip The rectangle to clip the path to.
     * @param tolerance_sq The squared tolerance to use when flattening the path.
     * @param sink_callback The callback to output the lines to.
     */
    void flatten_clipped(
      const drect& clip,
      const double tolerance_sq,
      std::function<void(const math::Vec2<T>, const math::Vec2<T>)> sink_callback
    ) const;

    /**
     * @brief Flattens a path and outputs the line segments to a sink vector.
     *
     * @param tolerance The tolerance to use when flattening the path.
     * @param sink_callback The callback to output the lines to.
     */
    void flatten_unclipped(
      const double tolerance,
      std::function<void(const math::Vec2<T>, const math::Vec2<T>)> sink_callback
    ) const;
  private:
    const QuadraticPath<T>& m_path;    /* The path to process. */
    const dmat2x3 m_transform;         /* The transformation matrix to apply to the path. */

    drect m_bounding_rect;             /* The bounding rectangle of the path. */
  };

}

/* -- Aliases -- */

namespace graphick::geom {

  using path_builder = PathBuilder<float>;
  using dpath_builder = PathBuilder<double>;

}