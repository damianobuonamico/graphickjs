/**
 * @file contour.h
 * @brief Contains the Contour and OutlineContour classes definition.
 *
 * @todo test if it is faster to copy or to use const reference for dvec2s
 */

#pragma once

#include "../../math/f8x8.h"
#include "../../math/f24x8.h"
#include "../../math/vec2.h"

#include "../../utils/defines.h"

#include "../properties.h"

#include <vector>
#include <memory>

namespace Graphick::Renderer::Geometry {

  /**
   * @brief A Contour is a sequence of points used for rendering filled line strips.
   *
   * The points are stored in fixed point format to speed up some operations and avoid floating point errors.
   *
   * @class Contour
   */
  class Contour {
  public:
    using Parameterization = std::vector<std::pair<dvec2, dvec2>>;
  public:
    std::vector<f24x8x2> points;    /* The points of the contour. */
  public:
    /**
     * @brief Constructs a Contour.
     *
     * @param tolerance The tolerance used for the path approximation.
     */
    Contour(const double tolerance = GK_PATH_TOLERANCE) : m_tolerance(tolerance) {}

    /**
     * @brief Adds a new point to the contour.
     *
     * @param p0 The point to add.
     */
    void move_to(const f24x8x2 p0);
    void move_to(const dvec2 p0);

    /**
     * @brief Adds a line to the contour.
     *
     * @param p1 The end point of the line.
     */
    void line_to(const f24x8x2 p1);
    void line_to(const dvec2 p1);

    /**
     * @brief Adds a cubic bezier to the contour.
     *
     * @param p1 The first control point.
     * @param p2 The second control point.
     * @param p3 The end point.
     */
    void cubic_to(const f24x8x2 p1, const f24x8x2 p2, const f24x8x2 p3);
    void cubic_to(const dvec2 p1, const dvec2 p2, const dvec2 p3);

    /**
     * @brief Offsets a cubic bezier adding the outer points to the contour and returing the inner points parameterization.
     *
     * @param p0 The first control point.
     * @param p1 The second control point.
     * @param p2 The third control point.
     * @param p3 The end point.
     * @param end_normal The normal of the end point.
     * @param radius The offset radius.
     * @return The inner points parameterization.
     */
    std::unique_ptr<Parameterization> offset_cubic(const dvec2 p0, const dvec2 p1, const dvec2 p2, const dvec2 p3, const dvec2 end_normal, const double radius);

    /**
     * @brief Adds the inner points of a parameterized cubic bezier offset.
     *
     * @param parameterization The parameterization of the inner points.
     * @param end_point The end point of the cubic bezier.
     * @param radius The offset radius.
     */
    void offset_cubic(const Parameterization& parameterization, const dvec2 end_point, const double radius);

    /**
     * @brief Adds the given cap to the contour.
     *
     * @param from The start point of the cap.
     * @param to The end point of the cap.
     * @param n The normal of the cap.
     * @param radius The radius of the cap.
     * @param cap The cap type.
     */
    void add_cap(const dvec2 from, const dvec2 to, const dvec2 n, const double radius, const LineCap cap);

    /**
     * @brief Adds the given join to the contour.
     *
     * @param from The start point of the join.
     * @param to The end point of the join.
     * @param pivot The pivot point of the join.
     * @param from_normal The normal of the start point.
     * @param to_normal The normal of the end point.
     * @param radius The radius of the join.
     * @param inv_miter_limit The inverse miter limit.
     * @param join The join type.
     */
    void add_join(const dvec2 from, const dvec2 to, const dvec2 pivot, const dvec2 from_normal, const dvec2 to_normal, const double radius, const double inv_miter_limit, LineJoin join);

    /**
     * @brief Closes the contour.
     */
    void close();

    /**
     * @brief Reverses the contour.
     */
    void reverse();

    /**
     * @brief Returns the winding of a point.
     *
     * The winding is the number of times the contour winds around the point.
     * It is used to determine if the point is inside or outside the contour.
     *
     * @param point The point to check.
     * @return The winding of the point.
     */
    int winding_of(const f24x8x2 point);
  private:
    /**
     * @brief Adds an arc to the contour.
     *
     * @param center The center of the arc.
     * @param from The start point of the arc.
     * @param radius The radius of the arc.
     * @param to The end point of the arc.
     */
    void arc(const dvec2 center, const dvec2 from, const double radius, const dvec2 to);

    /**
     * @brief The recursive cubic offset algorithm.
     *
     * @param p0 The first control point.
     * @param p1 The second control point.
     * @param p2 The third control point.
     * @param p3 The end point.
     * @param level The current level of recursion.
     * @param angular_tolerance The angular tolerance.
     * @param parameterization The parameterization of the inner points where the offset will be added.
     */
    void recursive_cubic_offset(const dvec2 p0, const dvec2 p1, const dvec2 p2, const dvec2 p3, const unsigned int level, const double angular_tolerance, Parameterization& parameterization);
  private:
    f24x8x2 m_p0 = { 0, 0 };    /* The last point added in fixed point notation. */
    dvec2 m_d_p0 = { 0, 0 };    /* The last point added in double precision. */

    double m_tolerance;         /* The tessellation tolerance. */
  };

  /**
   * @brief An OutlineContour is a sequence of points used for rendering line strips.
   *
   * The points are stored in double precision.
   *
   * @class OutlineContour
   */
  class OutlineContour {
  public:
    std::vector<dvec2> points;    /* The points of the contour. */
  public:
    /**
     * @brief Constructs an OutlineContour.
     *
     * @param tolerance The tolerance used for the path approximation.
     */
    OutlineContour(const double tolerance = GK_PATH_TOLERANCE) : m_tolerance(tolerance) {}

    /**
     * @brief Adds a new point to the contour.
     *
     * @param p0 The point to add.
     */
    void move_to(const dvec2 p0);

    /**
     * @brief Adds a line to the contour.
     *
     * @param p1 The end point of the line.
     */
    void line_to(const dvec2 p1);

    /**
     * @brief Adds a cubic bezier to the contour.
     *
     * @param p1 The first control point.
     * @param p2 The second control point.
     * @param p3 The end point.
     */
    void cubic_to(const dvec2 p1, const dvec2 p2, const dvec2 p3);

    /**
     * @brief Closes the contour.
     */
    void close();
  private:
    dvec2 m_p0 = { 0, 0 };    /* The last point added. */

    double m_tolerance;       /* The tessellation tolerance. */
  };

}
