/**
 * @file path.h
 * @brief Path class definition
 */

#pragma once

#include "../../math/vec2.h"
#include "../../math/rect.h"

#include <functional>
#include <optional>
#include <vector>
#include <tuple>

namespace Graphick::Math {
  struct mat2x3;
}

namespace Graphick::Renderer {
  struct Fill;
  struct Stroke;
};

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
     * @brief This struct represents a segment of the path.
     *
     * The segment is represented by a type and 4 points, even if not used.
     *
     * @struct Segment
     */
    struct Segment {
      Command type;    /* The type of the segment. */

      vec2 p0;         /* The first point of the segment or move command destination. */
      vec2 p1;         /* The second point of the segment (line, quadratic, cubic). */
      vec2 p2;         /* The third point of the segment (quadratic, cubic). */
      vec2 p3;         /* The fourth point of the segment (cubic). */

      /**
       * @brief Constructors.
       */
      Segment(const vec2 p0) : type(Command::Move), p0(p0), p1(0.0f), p2(0.0f), p3(0.0f) {}
      Segment(const vec2 p0, const vec2 p1) : type(Command::Line), p0(p0), p1(p1), p2(0.0f), p3(0.0f) {}
      Segment(const vec2 p0, const vec2 p1, const vec2 p2) : type(Command::Quadratic), p0(p0), p1(p1), p2(p2), p3(0.0f) {}
      Segment(const vec2 p0, const vec2 p1, const vec2 p2, const vec2 p3) : type(Command::Cubic), p0(p0), p1(p1), p2(p2), p3(p3) {}

      /**
       * @brief Type checking methods.
       */
      inline bool is_move() const { return type == Command::Move; }
      inline bool is_line() const { return type == Command::Line; }
      inline bool is_quadratic() const { return type == Command::Quadratic; }
      inline bool is_cubic() const { return type == Command::Cubic; }
    };

    /**
     * @brief This struct represents a forward iterator over the segments of the path.
     *
     * The iterator's value type is a tuple containing the command and the points of the segment (always 4 points, even if not used).
     *
     * @struct Iterator
     */
    struct Iterator {
    public:
      using iterator_category = std::forward_iterator_tag;
      using difference_type = size_t;
      using value_type = Segment;
      using pointer = value_type*;
      using reference = value_type&;

      /**
       * @brief Constructs an iterator over the given path at the given index.
       *
       * @param path The path to iterate over.
       * @param index The start index of the iterator.
       */
      Iterator(const PathDev& path, const size_t index);

      /**
       * @brief Equality, inequality and comparison operators.
       *
       * @param other The iterator to compare to.
       */
      inline bool operator==(const Iterator& other) const { return m_index == other.m_index; }
      inline bool operator!=(const Iterator& other) const { return m_index != other.m_index; }
      inline bool operator<(const Iterator& other) const { return m_index < other.m_index; }
      inline bool operator<=(const Iterator& other) const { return m_index <= other.m_index; }
      inline bool operator>(const Iterator& other) const { return m_index > other.m_index; }
      inline bool operator>=(const Iterator& other) const { return m_index >= other.m_index; }

      /**
       * @brief Pre and post increment operators.
       */
      Iterator& operator++();
      Iterator operator++(int);

      /**
       * @brief Pre and post decrement operators.
       */
      Iterator& operator--();
      Iterator operator--(int);

      /**
       * @brief Dereference operator.
       *
       * @return The current segment.
       */
      value_type operator*() const;
    private:
      size_t m_index;           /* The current index of the iterator. */
      size_t m_point_index;     /* The index of the last point iterated over. */

      const PathDev& m_path;    /* The path to iterate over. */
    };

    /**
     * @brief This struct represents a reverse iterator over the segments of the path.
     *
     * The iterator's value type is a tuple containing the command and the points of the segment (always 4 points, even if not used).
     *
     * @struct ReverseIterator
     */
    struct ReverseIterator {
    public:
      using iterator_category = std::forward_iterator_tag;
      using difference_type = size_t;
      using value_type = Segment;
      using pointer = value_type*;
      using reference = value_type&;

      /**
       * @brief Constructs an iterator over the given path at the given index.
       *
       * @param path The path to iterate over.
       * @param index The start index of the iterator.
       */
      ReverseIterator(const PathDev& path, const size_t index);

      /**
       * @brief Equality, inequality and comparison operators.
       *
       * @param other The iterator to compare to.
       */
      inline bool operator==(const ReverseIterator& other) const { return m_index == other.m_index; }
      inline bool operator!=(const ReverseIterator& other) const { return m_index != other.m_index; }
      inline bool operator<(const ReverseIterator& other) const { return m_index < other.m_index; }
      inline bool operator<=(const ReverseIterator& other) const { return m_index <= other.m_index; }
      inline bool operator>(const ReverseIterator& other) const { return m_index > other.m_index; }
      inline bool operator>=(const ReverseIterator& other) const { return m_index >= other.m_index; }

      /**
       * @brief Pre and post increment operators.
       */
      ReverseIterator& operator++();
      ReverseIterator operator++(int);

      /**
       * @brief Pre and post decrement operators.
       */
      ReverseIterator& operator--();
      ReverseIterator operator--(int);

      /**
       * @brief Dereference operator.
       *
       * @return The current segment.
       */
      value_type operator*() const;
    private:
      size_t m_index;           /* The current index of the iterator. */
      size_t m_point_index;     /* The index of the last point iterated over. */

      const PathDev& m_path;    /* The path to iterate over. */
    };
  public:
    /**
     * @brief Constructors, copy constructor and move constructor.
     */
    PathDev();
    PathDev(const PathDev& other);
    PathDev(PathDev&& other) noexcept;

    /**
     * @brief Default destructor.
     */
    ~PathDev() = default;

    /**
     * @brief Copy and move assignment operators.
     */
    PathDev& operator=(const PathDev& other);
    PathDev& operator=(PathDev&& other) noexcept;

    /**
     * @brief Returns a segment iterator to the beginning of the path.
     *
     * @return An iterator to the beginning of the path.
     */
    Iterator begin() const { return Iterator(*this, 1); }

    /**
     * @brief Returns a segment iterator to the end of the path.
     *
     * @return An iterator to the end of the path.
     */
    Iterator end() const { return Iterator(*this, m_commands_size); }

    /**
     * @brief Returns a reverse segment iterator to the end of the path.
     *
     * @return A reverse iterator to the end of the path.
     */
    ReverseIterator rbegin() const { return ReverseIterator(*this, m_commands_size - 1); }

    /**
     * @brief Returns a reverse segment iterator to the beginning of the path.
     *
     * @return A reverse iterator to the beginning of the path.
     */
    ReverseIterator rend() const { return ReverseIterator(*this, 0); }

    /**
     * @brief Returns the first segment of the path.
     *
     * @return The first segment of the path.
     */
    inline Segment front() const { return *begin(); }

    /**
     * @brief Returns the last segment of the path.
     *
     * @return The last segment of the path.
     */
    inline Segment back() const { return *rbegin(); }

    /**
     * @brief Iterates over the commands of the path, calling the given callbacks for each command.
     *
     * @param move_callback The callback to call for each move command.
     * @param line_callback The callback to call for each line command.
     * @param quadratic_callback The callback to call for each quadratic command.
     * @param cubic_callback The callback to call for each cubic command.
     */
    void for_each(
      std::function<void(const vec2)> move_callback = nullptr,
      std::function<void(const vec2)> line_callback = nullptr,
      std::function<void(const vec2, const vec2)> quadratic_callback = nullptr,
      std::function<void(const vec2, const vec2, const vec2)> cubic_callback = nullptr
    ) const;

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
     * @brief Checks whether the path is closed or not.
     *
     * @return true if the path is closed, false otherwise.
     */
    inline bool closed() const { return m_points.front() == m_points.back(); }

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

    /**
     * @brief Calculates the bounding rectangle of the path.
     *
     * @return The bounding rectangle of the path.
     */
    Math::rect bounding_rect() const;

    /**
     * @brief Calculates the bounding rectangle of the path in the fixed reference system.
     *
     * @param transform The transformation matrix to apply to the path.
     * @return The bounding rectangle of the path.
     */
    Math::rect bounding_rect(const Math::mat2x3& transform) const;

    /**
     * @brief Calculates the approximate bounding rectangle of the path considering all control points as vertices.
     *
     * @return The approximate bounding rectangle of the path.
     */
    Math::rect approx_bounding_rect() const;

    /**
     * @brief Checks whether the given point is inside the path or not.
     *
     * @param point The point to check.
     * @param fill The fill of the path, can be nullptr.
     * @param stroke The stroke of the path, can be nullptr.
     * @param transform The transformation matrix to apply to the path, can be nullptr.
     * @param threshold The threshold to use for the check.
     * @return true if the point is inside the path, false otherwise.
     */
    bool is_point_inside_path(const vec2 point, const Fill* fill, const Stroke* stroke, const Math::mat2x3* transform, const float threshold) const;
    bool is_point_inside_path(const vec2 point, const Fill* fill, const Stroke* stroke, const Math::mat2x3& transform, const float threshold) const;
  private:
    /**
     * @brief Returns the ith command of the path.
     *
     * @param index The index of the command to return.
     * @return The ith command of the path.
     */
    inline Command get_command(const size_t index) const { return static_cast<Command>((m_commands[index / 4] >> (6 - (index % 4) * 2)) & 0b00000011); }

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
