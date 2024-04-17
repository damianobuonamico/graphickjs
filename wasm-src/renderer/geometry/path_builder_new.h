/**
 * @file path_builder.h
 * @brief Contains the PathBuilder class definition.
 *
 * @todo move stroke and fill methods from old PathBuilder
 */

#pragma once

#include "../../math/vec4.h"
#include "../../math/rect.h"
#include "../../math/mat2x3.h"

#include "quadratic_path.h"

#include <vector>

//TEMP
namespace graphick::geom {
  class Path;
}

namespace graphick::renderer {
  struct Stroke;
}

namespace graphick::renderer::geometry {

  /**
   * @brief A class to generate drawables from a path.
   *
   * A PathBuilder is used to stroke, fill, outline and flatten a path, clipping it to a rectangle.
   *
   * @class PathBuilder
   */
  class PathBuilder {
  public:
    /**
     * @brief A structure to hold the inner and outer outlines of a stroke.
     *
     * If the stroke is not closed, the inner outline will be empty.
     *
     * @struct StrokeOutline
     */
    struct StrokeOutline {
      geom::QuadraticPath outer;    /* The outer outline of the stroke. */
      geom::QuadraticPath inner;    /* The inner outline of the stroke, in reverse order. */
    };
  public:
    /**
     * @brief Constructs a PathBuilder.
     *
     * @param path The path to process.
     * @param transform The transformation matrix to apply to the path.
     * @param bounding_rect The bounding rectangle of the path if known, default is nullptr.
     */
    PathBuilder(const geom::QuadraticPath& path, const mat2x3& transform, const rect* bounding_rect = nullptr);

    /**
     * @brief Default destructor.
     */
    ~PathBuilder() = default;

    /**
     * @brief Strokes a path and outputs a fill composed of quadratic bezier curves.
     *
     * @param stroke The stroke properties to use.
     * @param tolerance The offset error tolerance.
     * @return The output fill.
     */
    StrokeOutline stroke(const graphick::renderer::Stroke& stroke, const float tolerance) const;

    /**
     * @brief Flattens a path and outputs the line segments to a sink vector.
     *
     * If the portion of the path visible is less than 50%, it is clipped.
     *
     * @param clip The rectangle to clip the path to.
     * @param tolerance The tolerance to use when flattening the path.
     * @param sink The vector to output the lines to as vec4s.
     */
    void flatten(const rect& clip, const float tolerance, std::vector<vec4>& sink) const;
  private:
    /**
     * @brief Clips and flattens a path and outputs the line segments to a sink vector.
     *
     * @param clip The rectangle to clip the path to.
     * @param tolerance The tolerance to use when flattening the path.
     * @param sink The vector to output the lines to as vec4s.
     */
    void flatten_clipped(const rect& clip, const float tolerance, std::vector<vec4>& sink) const;

    /**
     * @brief Flattens a path and outputs the line segments to a sink vector.
     *
     * @param tolerance The tolerance to use when flattening the path.
     * @param sink The vector to output the lines to as vec4s.
     */
    void flatten_unclipped(const float tolerance, std::vector<vec4>& sink) const;
  private:
    const geom::QuadraticPath& m_path;    /* The path to process. */
    const mat2x3& m_transform;      /* The transformation matrix to apply to the path. */
    const rect m_bounding_rect;     /* The bounding rectangle of the path. */
  };

};
