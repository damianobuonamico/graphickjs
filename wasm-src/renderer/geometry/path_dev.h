/**
 * @file path.h
 * @brief Path class definition
 */

#pragma once

#include "../../utils/uuid.h"
#include "../../math/vec2.h"

#include <vector>

namespace Graphick::Renderer::Geometry {

  /**
   * @brief The Path class represents the path representation used throughout the graphick editor.
   *
   * The path is represented by a list of points and a list of tightly packed traversing commands.
   *
   * @class Path
   */
  class PathDev {
  public:
    /**
     * @brief The Command enum represents the type of commands used to traverse the path.
     */
    enum Command {
      Move = 0,         /* Move to a point. */
      Line = 1,         /* Linear segment. */
      Quadratic = 2,    /* Quadratic bezier curve. */
      Cubic = 3,        /* Cubic bezier curve. */
    };
  public:
    /**
     * @brief Constructors, copy constructor and move constructor.
     */
    PathDev();
    PathDev(const PathDev& other);
    PathDev(PathDev&& other);

    /**
     * @brief Default destructor.
     */
    ~PathDev() = default;

    /**
     * @brief Copy and move assignment operators.
     */
    PathDev& operator=(const PathDev& other);
    PathDev& operator=(PathDev&& other);

    /**
     * @brief Whether the path is empty or not.
     *
     * A path is considered empty if it has less than two points. An empty path can also be vacant.
     * When a move command is added to a vacant path, it becomes empty.
     *
     * @return true if the path is empty, false otherwise.
     */
    inline bool empty() const { return m_points.size() < 2; }

    /**
     * @brief Whether the path is vacant or not.
     *
     * A path is considered vacant if it has no points. A vacant path is always empty.
     * When a move command is added to a vacant path, it becomes empty.
     *
     * @return true if the path is vacant, false otherwise.
     */
    inline bool vacant() const { return m_points.empty(); }

    /**
     * @brief Returns the number of segments in the path.
     *
     * @return The number of segments in the path.
     */
    inline size_t size() const { return (m_commands_size > 0 ? m_commands_size : 1) - 1; }

    /**
     * @brief Moves the path cursor to the given point.
     *
     * @param point The point to move the cursor to.
     */
    void move_to(const vec2 point);

    /**
     * @brief Adds a line segment to the path.
     *
     * @param point The point to add to the path.
     */
    void line_to(const vec2 point);

    /**
     * @brief Adds a quadratic bezier curve to the path.
     *
     * @param control The control point of the curve.
     * @param point The point to add to the path.
     */
    void quadratic_to(const vec2 control, const vec2 point);

    /**
     * @brief Adds a cubic bezier curve to the path.
     *
     * @param control_1 The first control point of the curve.
     * @param control_2 The second control point of the curve.
     * @param point The point to add to the path.
     */
    void cubic_to(const vec2 control_1, const vec2 control_2, const vec2 point);

    /**
     * @brief Adds a cubic bezier curve to the path.
     *
     * @param control The control point of the curve.
     * @param point The point to add to the path.
     * @param is_control_1 Whether the control point is the first or the second one.
     */
    void cubic_to(const vec2 control, const vec2 point, const bool is_control_1 = true);

    /**
     * @brief Adds an arc to the path.
     *
     * @param center The center of the arc.
     * @param radius The radius of the arc.
     * @param x_axis_rotation The rotation of the arc over the x-axis.
     * @param large_arc_flag Whether the arc is large or not.
     * @param sweep_flag Whether the arc is clockwise or not.
     * @param point The end point of the arc.
     */
    void arc_to(const vec2 center, const vec2 radius, const float x_axis_rotation, const bool large_arc_flag, const bool sweep_flag, const vec2 point);

    /**
     * @brief Adds an ellipse to the path.
     *
     * @param center The center of the ellipse.
     * @param radius The radius of the ellipse.
     */
    void ellipse(const vec2 center, const vec2 radius);

    /**
     * @brief Adds a circle to the path.
     *
     * @param center The center of the circle.
     * @param radius The radius of the circle.
     */
    void circle(const vec2 center, const float radius);

    /**
     * @brief Adds a rectangle to the path.
     *
     * @param point The top-left or center point of the rectangle.
     * @param size The size of the rectangle.
     * @param centered Whether the rectangle is centered or not.
     */
    void rect(const vec2 point, const vec2 size, const bool centered = false);

    /**
     * @brief Adds a rounded rectangle to the path.
     *
     * @param point The top-left or center point of the rectangle.
     * @param size The size of the rectangle.
     * @param radius The corner radius of the rectangle.
     * @param centered Whether the rectangle is centered or not.
     */
    void round_rect(const vec2 point, const vec2 size, const float radius, const bool centered = false);

    /**
     * @brief Closes the path.
     */
    void close();
  private:
    /**
     * @brief Returns the ith command of the path.
     *
     * @param index The index of the command to return.
     * @return The ith command of the path.
     */
    Command get_command(const size_t index) const;

    /**
     * @brief Pushes a command to the path, handling the packing logic.
     *
     * @param command The command to push.
     */
    void push_command(const Command command);
  private:
    std::vector<vec2> m_points;         /* The points of the path. */
    std::vector<uint8_t> m_commands;    /* The commands used to traverse the path. */

    size_t m_commands_size = 0;         /* The effective number of commands in the path. */
  };

}
