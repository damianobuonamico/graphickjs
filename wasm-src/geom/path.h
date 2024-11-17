/**
 * @file geom/path.h
 * @brief Contains the definition of the Path class, the main building block of the Graphick
 * editor.
 */

#pragma once

#include "cubic_bezier.h"
#include "cubic_path.h"
#include "line.h"
#include "quadratic_bezier.h"
#include "quadratic_path.h"

#include "../utils/assert.h"

#include <functional>

/* -- Forward declarations -- */

namespace graphick::io {
struct EncodedData;
struct DataDecoder;
}  // namespace graphick::io

namespace graphick::math {
template<typename T>
struct Mat2x3;
}

namespace graphick::geom {

template<typename T>
struct StrokingOptions;
struct FillingOptions;

/**
 * @brief The Path class represents the path representation used throughout the graphick editor.
 *
 * The path is represented by a list of points and a list of tightly packed traversing commands.
 */
template<typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
class Path {
 public:
  /**
   * @brief The Command enum represents the type of commands used to traverse the path.
   */
  enum class Command : uint8_t {
    Move = 0,       // Move to a point.
    Line = 1,       // Linear segment.
    Quadratic = 2,  // Quadratic bezier curve.
    Cubic = 3       // Cubic bezier curve.
  };

  /**
   * @brief The IndexType enum represents the type of index used to traverse the path.
   */
  enum class IndexType {
    Point = 0,    // The index represents the ith control point.
    Command = 1,  // The index represents the ith command.
    Segment = 2   // The index represents the ith segment.
  };

  static const uint32_t in_handle_index = std::numeric_limits<uint32_t>::max() -
                                          1;   // The index of the incoming handle.
  static const uint32_t out_handle_index = std::numeric_limits<uint32_t>::max() -
                                           2;  // The index of the outgoing handle.
 public:
  /**
   * @brief This struct represents a vertex node of the path.
   *
   * The vertex node is represented by the index of the incoming and outgoing handles and the index
   * of the vertex itself.
   */
  struct VertexNode {
    uint32_t vertex;  // The index of the vertex.

    int64_t in;       // The index of the incoming handle, -1 if no incoming handle is present.
    int64_t out;      // The index of the outgoing handle. -1 if no outgoing handle is present.

    int64_t close_vertex;  // The index of the vertex that closes the sub-path, -1 if the sub-path
                           // is not closed or the vertex is not a sub-path end.

    int64_t
        in_command;   // The index of the incoming command, -1 if no incoming command is present.
    int64_t
        out_command;  // The index of the outgoing command, -1 if no outgoing command is present.
  };

  /**
   * @brief This struct represents a segment of the path.
   *
   * The segment is represented by a type and 4 points, even if not used.
   */
  struct Segment {
    Command type = Command::Move;  // The type of the segment.

    math::Vec2<T> p0 =
        math::Vec2<T>::zero();     // The first point of the segment or move command destination.
    math::Vec2<T> p1 =
        math::Vec2<T>::zero();     // The second point of the segment (line, quadratic, cubic).
    math::Vec2<T> p2 =
        math::Vec2<T>::zero();     // The third point of the segment (quadratic, cubic).
    math::Vec2<T> p3 = math::Vec2<T>::zero();  // The fourth point of the segment (cubic).

    /**
     * @brief Constructors.
     */
    Segment(const math::Vec2<T> p0, const math::Vec2<T> p1) : type(Command::Line), p0(p0), p1(p1)
    {
    }
    Segment(const math::Vec2<T> p0, const math::Vec2<T> p1, const math::Vec2<T> p2)
        : type(Command::Quadratic), p0(p0), p1(p1), p2(p2)
    {
    }
    Segment(const math::Vec2<T> p0,
            const math::Vec2<T> p1,
            const math::Vec2<T> p2,
            const math::Vec2<T> p3)
        : type(Command::Cubic), p0(p0), p1(p1), p2(p2), p3(p3)
    {
    }

    /**
     * @brief Type checking methods.
     */
    inline bool is_line() const
    {
      return type == Command::Line;
    }
    inline bool is_quadratic() const
    {
      return type == Command::Quadratic;
    }
    inline bool is_cubic() const
    {
      return type == Command::Cubic;
    }

    /**
     * @brief Whether the segment is a point or not.
     *
     * A segment is considered a point if all control points are equal.
     *
     * @return true if the segment is a point, false otherwise.
     */
    bool is_point() const;

    /**
     * @brief Type casts.
     */
    inline geom::Line<T> to_line() const
    {
      return geom::Line<T>(p0, p1);
    }
    inline geom::QuadraticBezier<T> to_quadratic() const
    {
      return geom::QuadraticBezier<T>(p0, p1, p2);
    }
    inline geom::CubicBezier<T> to_cubic() const
    {
      return geom::CubicBezier<T>(p0, p1, p2, p3);
    }

    /**
     * @brief Type casts operators.
     */
    inline operator geom::Line<T>()
    {
      return to_line();
    }
    inline operator geom::QuadraticBezier<T>()
    {
      return to_quadratic();
    }
    inline operator geom::CubicBezier<T>()
    {
      return to_cubic();
    }

    /**
     * @brief Samples the segment at the given t value.
     *
     * @param t The t value to sample the segment at.
     * @return The sampled point.
     */
    inline math::Vec2<T> sample(const T t) const
    {
      switch (type) {
        case Command::Line:
          return to_line().sample(t);
        case Command::Quadratic:
          return to_quadratic().sample(t);
        case Command::Cubic:
          return to_cubic().sample(t);
        default:
          return p0;
      }
    }
  };

  /**
   * @brief This struct represents a forward iterator over the segments of the path.
   *
   * The iterator's value type is a tuple containing the command and the points of the segment
   * (always 4 points, even if not used).
   */
  struct Iterator {
   public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = uint32_t;
    using value_type = Segment;
    using pointer = value_type*;
    using reference = value_type&;

    /**
     * @brief Constructs an iterator over the given path at the given index.
     *
     * If index_type is IndexType::Point, and the point corresponds to a vertex, the iterator will
     * point to the segment that ends at the vertex.
     *
     * @param path The path to iterate over.
     * @param index The start index of the iterator.
     * @param index_type The type of index to use, default is IndexType::Command.
     */
    Iterator(const Path<T>& path,
             const uint32_t index,
             const IndexType index_type = IndexType::Command);

    /**
     * @brief Equality, inequality and comparison operators.
     *
     * @param other The iterator to compare to.
     */
    inline bool operator==(const Iterator& other) const
    {
      return m_index == other.m_index;
    }
    inline bool operator!=(const Iterator& other) const
    {
      return m_index != other.m_index;
    }
    inline bool operator<(const Iterator& other) const
    {
      return m_index < other.m_index;
    }
    inline bool operator<=(const Iterator& other) const
    {
      return m_index <= other.m_index;
    }
    inline bool operator>(const Iterator& other) const
    {
      return m_index > other.m_index;
    }
    inline bool operator>=(const Iterator& other) const
    {
      return m_index >= other.m_index;
    }

    /**
     * @brief Pre and post increment operators.
     */
    Iterator& operator++();
    Iterator operator++(int);
    Iterator operator+(const uint32_t n) const;

    /**
     * @brief Pre and post decrement operators.
     */
    Iterator& operator--();
    Iterator operator--(int);
    Iterator operator-(const uint32_t n) const;

    /**
     * @brief Dereference operator.
     *
     * @return The current segment.
     */
    value_type operator*() const;

    /**
     * @brief Returns the index of the command the iterator is currently pointing to.
     *
     * @return The index of the command the iterator is currently pointing to.
     */
    inline uint32_t command_index() const
    {
      return m_index;
    }

    /**
     * @brief Returns the index of the segment the iterator is currently pointing to.
     *
     * @return The index of the segment the iterator is currently pointing to.
     */
    inline uint32_t segment_index() const
    {
      return std::max(uint32_t(1), m_index) - 1;
    }

    /**
     * @brief Returns the index of the point the iterator is currently pointing to.
     *
     * @return The index of the point the iterator is currently pointing to.
     */
    inline uint32_t point_index() const
    {
      return m_point_index;
    }

   private:
    uint32_t m_index;        // The current command index of the iterator.
    uint32_t m_point_index;  // The index of the last point iterated over.

    const Path<T>& m_path;   // The path to iterate over.
  };

  /**
   * @brief This struct represents a reverse iterator over the segments of the path.
   *
   * The iterator's value type is a tuple containing the command and the points of the segment
   * (always 4 points, even if not used).
   */
  struct ReverseIterator {
   public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = uint32_t;
    using value_type = Segment;
    using pointer = value_type*;
    using reference = value_type&;

    /**
     * @brief Constructs an iterator over the given path at the given command index.
     *
     * @param path The path to iterate over.
     * @param index The start index of the iterator.
     */
    ReverseIterator(const Path<T>& path, const uint32_t index);

    /**
     * @brief Equality, inequality and comparison operators.
     *
     * @param other The iterator to compare to.
     */
    inline bool operator==(const ReverseIterator& other) const
    {
      return m_index == other.m_index;
    }
    inline bool operator!=(const ReverseIterator& other) const
    {
      return m_index != other.m_index;
    }
    inline bool operator<(const ReverseIterator& other) const
    {
      return m_index < other.m_index;
    }
    inline bool operator<=(const ReverseIterator& other) const
    {
      return m_index <= other.m_index;
    }
    inline bool operator>(const ReverseIterator& other) const
    {
      return m_index > other.m_index;
    }
    inline bool operator>=(const ReverseIterator& other) const
    {
      return m_index >= other.m_index;
    }

    /**
     * @brief Pre and post increment operators.
     */
    ReverseIterator& operator++();
    ReverseIterator operator++(int);
    ReverseIterator operator+(const uint32_t n) const;

    /**
     * @brief Pre and post decrement operators.
     */
    ReverseIterator& operator--();
    ReverseIterator operator--(int);
    ReverseIterator operator-(const uint32_t n) const;

    /**
     * @brief Dereference operator.
     *
     * @return The current segment.
     */
    value_type operator*() const;

   private:
    uint32_t m_index;        // The current segment index of the iterator.
    uint32_t m_point_index;  // The index of the last point iterated over.

    const Path<T>& m_path;   // The path to iterate over.
  };

 public:
  /**
   * @brief Constructors, copy constructor and move constructor.
   */
  Path();
  Path(const Path<T>& other);
  Path(Path<T>&& other) noexcept;

  /**
   * @brief Type conversion constructor.
   *
   * @param other The path to convert from.
   */
  template<typename U>
  explicit Path(const Path<U>& other);

  /**
   * @brief Constructs a path from encoded data.
   *
   * @param data The encoded data to construct the path from.
   */
  Path(io::DataDecoder& decoder);

  /**
   * @brief Default destructor.
   */
  ~Path() = default;

  /**
   * @brief Copy and move assignment operators.
   */
  Path& operator=(const Path& other);
  Path& operator=(Path&& other) noexcept;

  /**
   * @brief Returns a segment iterator to the beginning of the path.
   *
   * @return An iterator to the beginning of the path.
   */
  Iterator begin() const
  {
    return Iterator(*this, 1);
  }

  /**
   * @brief Returns a segment iterator to the end of the path.
   *
   * @return An iterator to the end of the path.
   */
  Iterator end() const
  {
    return Iterator(*this, m_commands_size);
  }

  /**
   * @brief Returns a reverse segment iterator to the end of the path.
   *
   * @return A reverse iterator to the end of the path.
   */
  ReverseIterator rbegin() const
  {
    return ReverseIterator(*this, m_commands_size - 1);
  }

  /**
   * @brief Returns a reverse segment iterator to the beginning of the path.
   *
   * @return A reverse iterator to the beginning of the path.
   */
  ReverseIterator rend() const
  {
    return ReverseIterator(*this, 0);
  }

  /**
   * @brief Returns the first segment of the path.
   *
   * @return The first segment of the path.
   */
  inline Segment front() const
  {
    return *begin();
  }

  /**
   * @brief Whether the path is empty or not.
   *
   * A path is considered empty if it has less than two points. An empty path can also be vacant.
   * When a move command is added to a vacant path, it becomes empty.
   *
   * @return true if the path is empty, false otherwise.
   */
  inline bool empty() const
  {
    return m_points.size() < 2;
  }

  /**
   * @brief Whether the path is vacant or not.
   *
   * A path is considered vacant if it has no points. A vacant path is always empty.
   * When a move command is added to a vacant path, it becomes empty.
   *
   * @return true if the path is vacant, false otherwise.
   */
  inline bool vacant() const
  {
    return m_points.empty();
  }

  /**
   * @brief Checks whether the path is closed or not.
   *
   * @return true if the path is closed, false otherwise.
   */
  inline bool closed() const
  {
    return !empty() && m_closed;
  }

  /**
   * @brief Returns the number of segments in the path.
   *
   * @return The number of segments in the path.
   */
  inline uint32_t size() const
  {
    return m_commands_size > 1 ? m_commands_size - 1 : 0;
  }

  /**
   * @brief Returns the number of control points in the path.
   *
   * @param include_handles Whether or not to count the handles when counting the points, default
   * is true.
   * @return The number of points in the path.
   */
  inline uint32_t points_count(const bool include_handles = true) const
  {
    return include_handles ? static_cast<uint32_t>(m_points.size()) : m_commands_size;
  }

  /**
   * @brief Returns the last segment of the path.
   *
   * @return The last segment of the path.
   */
  inline Segment back() const
  {
    return *rbegin();
  }

  /**
   * @brief Checks whether the path has an incoming handle or not.
   *
   * @return true if the path has an incoming handle, false otherwise.
   */
  inline bool has_in_handle() const
  {
    return !vacant() && !closed() && m_in_handle != m_points.front();
  }

  /**
   * @brief Checks whether the path has an outgoing handle or not.
   *
   * @return true if the path has an outgoing handle, false otherwise.
   */
  inline bool has_out_handle() const
  {
    return !vacant() && !closed() && m_out_handle != m_points.back();
  }

  /**
   * @brief Returns the incoming handle of the path.
   *
   * @return The incoming handle of the path.
   */
  inline math::Vec2<T> in_handle() const
  {
    return m_in_handle;
  }

  /**
   * @brief Returns the outgoing handle of the path.
   *
   * @return The outgoing handle of the path.
   */
  inline math::Vec2<T> out_handle() const
  {
    return m_out_handle;
  }

  /**
   * @brief Returns the ith control point of the path.
   *
   * To retrieve the position of the incoming handle, use the Path::in_handle_index constant or the
   * in_handle() method. To retrieve the position of the outgoing handle, use the
   * Path::out_handle_index constant or the out_handle() method.
   *
   * @param point_index The index of the point to return.
   * @return The ith control point of the path.
   */
  inline math::Vec2<T> at(const uint32_t point_index) const
  {
    switch (point_index) {
      case in_handle_index:
        return m_in_handle;
      case out_handle_index:
        return m_out_handle;
      default: {
        GK_ASSERT(point_index < m_points.size(), "Point index out of range.");
        return m_points[point_index];
      }
    }
  }

  /**
   * @brief Returns the ith control point of the path.
   *
   * To retrieve the position of the incoming handle, use the Path::in_handle_index constant or the
   * in_handle() method. To retrieve the position of the outgoing handle, use the
   * Path::out_handle_index constant or the out_handle() method.
   *
   * @param point_index The index of the point to return.
   * @return The ith control point of the path.
   */
  inline math::Vec2<T> operator[](const uint32_t point_index) const
  {
    return at(point_index);
  }

  /**
   * @brief Returns the ith segment of the path.
   *
   * @param index The index of the segment to return;
   * @param index_type The type of index to use, default is IndexType::Segment.
   * @return The ith segment of the path.
   */
  inline Segment segment_at(const uint32_t index,
                            const IndexType index_type = IndexType::Segment) const
  {
    return *Iterator(*this, index, index_type);
  }

  /**
   * @brief Returns the ith command of the path.
   *
   * @param command_index The index of the command to return.
   * @return The ith command of the path.
   */
  inline Command command_at(const uint32_t command_index) const
  {
    return get_command(command_index);
  }

  /**
   * @brief Iterates over the commands of the path, calling the given callbacks for each command.
   *
   * @param move_callback The callback to call for each move command.
   * @param line_callback The callback to call for each line command.
   * @param quadratic_callback The callback to call for each quadratic command.
   * @param cubic_callback The callback to call for each cubic command.
   */
  void for_each(
      std::function<void(const math::Vec2<T>)> move_callback = nullptr,
      std::function<void(const math::Vec2<T>)> line_callback = nullptr,
      std::function<void(const math::Vec2<T>, const math::Vec2<T>)> quadratic_callback = nullptr,
      std::function<void(const math::Vec2<T>, const math::Vec2<T>, const math::Vec2<T>)>
          cubic_callback = nullptr) const;

  /**
   * @brief Reverse iterates over the commands of the path, calling the given callbacks for each
   * command.
   *
   * @param move_callback The callback to call for each move command.
   * @param line_callback The callback to call for each line command.
   * @param quadratic_callback The callback to call for each quadratic command.
   * @param cubic_callback The callback to call for each cubic command.
   */
  void for_each_reversed(
      std::function<void(const math::Vec2<T>)> move_callback = nullptr,
      std::function<void(const math::Vec2<T>, const math::Vec2<T>)> line_callback = nullptr,
      std::function<void(const math::Vec2<T>, const math::Vec2<T>, const math::Vec2<T>)>
          quadratic_callback = nullptr,
      std::function<
          void(const math::Vec2<T>, const math::Vec2<T>, const math::Vec2<T>, const math::Vec2<T>)>
          cubic_callback = nullptr) const;

  /**
   * @brief Returns a vector containing the incdices of all of the vertex control points.
   *
   * Closing vertices are not included in the vector.
   *
   * @return The vector of indices.
   */
  std::vector<uint32_t> vertex_indices() const;

  /**
   * @brief Checks whether the given point is a vertex or not.
   *
   * A point is considered a vertex if it is the destination of a command.
   *
   * @param point_index The index of the point to check.
   * @return true if the point is a vertex, false otherwise.
   */
  bool is_vertex(const uint32_t point_index) const;

  /**
   * @brief Checks whether the given point is a handle or not.
   *
   * A point is considered a handle if it is not a vertex, i.e. it is a control point of a curve.
   *
   * @param point_index The index of the point to check.
   * @return true if the point is a handle, false otherwise.
   */
  inline bool is_handle(const uint32_t point_index) const
  {
    return !is_vertex(point_index);
  }

  /**
   * @brief Checks whether the given point is the first or last point of an open path.
   *
   * @param point_index The index of the point to check.
   * @return true if the point is an open end, false otherwise.
   */
  bool is_open_end(const uint32_t point_index) const
  {
    return (point_index == 0 || point_index == m_points.size() - 1) && !closed();
  }

  /**
   * @brief Returns the vertex node the given control point is part of.
   *
   * @param point_index The index of the point to check.
   * @return The vertex node the given control point is part of.
   */
  VertexNode node_at(const uint32_t point_index) const;

  /**
   * @brief Moves the path cursor to the given point.
   *
   * @param point The point to move the cursor to.
   */
  void move_to(const math::Vec2<T> point);

  /**
   * @brief Adds a line segment to the path.
   *
   * @param p1 The point to add to the path.
   * @param reverse Whether to add the point at the beginning of the path, instead of the end.
   * Default is false.
   */
  void line_to(const math::Vec2<T> p1, const bool reverse = false);

  /**
   * @brief Adds a line segment to the path.
   *
   * @param line The line to add to the path, only p2 is added.
   * @param reverse Whether to add the point at the beginning of the path, instead of the end.
   * Default is false.
   */
  inline void line_to(const geom::Line<T> line, const bool reverse = false)
  {
    line_to(line.p1, reverse);
  }

  /**
   * @brief Adds a quadratic bezier curve to the path.
   *
   * @param p1 The control point of the curve.
   * @param p2 The point to add to the path.
   * @param reverse Whether to add the point at the beginning of the path, instead of the end.
   * Default is false.
   */
  void quadratic_to(const math::Vec2<T> p1, const math::Vec2<T> p2, const bool reverse = false);

  /**
   * @brief Adds a quadratic bezier curve to the path.
   *
   * @param quadratic The quadratic bezier curve to add to the path, only p1 and p2 are added.
   * @param reverse Whether to add the point at the beginning of the path, instead of the end.
   * Default is false.
   */
  inline void quadratic_to(const geom::QuadraticBezier<T> quadratic, const bool reverse = false)
  {
    quadratic_to(quadratic.p1, quadratic.p2, reverse);
  }

  /**
   * @brief Adds a cubic bezier curve to the path.
   *
   * @param p1 The first control point of the curve.
   * @param p2 The second control point of the curve.
   * @param p3 The point to add to the path.
   * @param reverse Whether to add the point at the beginning of the path, instead of the end.
   * Default is false.
   */
  void cubic_to(const math::Vec2<T> p1,
                const math::Vec2<T> p2,
                const math::Vec2<T> p3,
                const bool reverse = false);

  /**
   * @brief Adds a cubic bezier curve to the path.
   *
   * @param p The control point of the curve.
   * @param p3 The point to add to the path.
   * @param is_p1 Whether the control point is the first or the second one.
   * @param reverse Whether to add the point at the beginning of the path, instead of the end.
   * Default is false.
   */
  void cubic_to(const math::Vec2<T> p,
                const math::Vec2<T> p3,
                const bool is_p1 = true,
                const bool reverse = false);

  /**
   * @brief Adds a cubic bezier curve to the path.
   *
   * @param cubic The cubic bezier curve to add to the path, only p1, p2 and p3 are added.
   * @param reverse Whether to add the point at the beginning of the path, instead of the end.
   * Default is false.
   */
  inline void cubic_to(const geom::CubicBezier<T> cubic, const bool reverse = false)
  {
    cubic_to(cubic.p1, cubic.p2, cubic.p3, reverse);
  }

  /**
   * @brief Adds an arc to the path.
   *
   * @param center The center of the arc.
   * @param radius The radius of the arc.
   * @param x_axis_rotation The rotation of the arc over the x-axis.
   * @param large_arc_flag Whether the arc is large or not.
   * @param sweep_flag Whether the arc is clockwise or not.
   * @param point The end point of the arc.
   * @param reverse Whether to add the point at the beginning of the path, instead of the end.
   * Default is false.
   */
  void arc_to(const math::Vec2<T> center,
              const math::Vec2<T> radius,
              const T x_axis_rotation,
              const bool large_arc_flag,
              const bool sweep_flag,
              const math::Vec2<T> point,
              const bool reverse = false);

  /**
   * @brief Adds an ellipse to the path.
   *
   * @param center The center of the ellipse.
   * @param radius The radius of the ellipse.
   */
  void ellipse(const math::Vec2<T> center, const math::Vec2<T> radius);

  /**
   * @brief Adds a circle to the path.
   *
   * @param center The center of the circle.
   * @param radius The radius of the circle.
   */
  void circle(const math::Vec2<T> center, const T radius);

  /**
   * @brief Adds a rectangle to the path.
   *
   * @param point The top-left or center point of the rectangle.
   * @param size The size of the rectangle.
   * @param centered Whether the rectangle is centered or not.
   */
  void rect(const math::Vec2<T> point, const math::Vec2<T> size, const bool centered = false);

  /**
   * @brief Adds a rounded rectangle to the path.
   *
   * @param point The top-left or center point of the rectangle.
   * @param size The size of the rectangle.
   * @param radius The corner radius of the rectangle.
   * @param centered Whether the rectangle is centered or not.
   */
  void round_rect(const math::Vec2<T> point,
                  const math::Vec2<T> size,
                  const T radius,
                  const bool centered = false);

  /**
   * @brief Closes the path.
   */
  void close();

  /**
   * @brief Translates the ith point of the path by the given delta.
   *
   * This method takes care of updating the control points of the curves if the point is a vertex.
   *
   * @param point_index The index of the point to translate.
   * @param delta The delta to translate the point by.
   */
  void translate(const uint32_t point_index, const math::Vec2<T> delta)
  {
    switch (point_index) {
      case in_handle_index:
        m_in_handle += delta;
        break;
      case out_handle_index:
        m_out_handle += delta;
        break;
      default: {
        GK_ASSERT(point_index < m_points.size(), "Point index out of range.");
        m_points[point_index] += delta;
        break;
      }
    }
  }

  /**
   * @brief Converts the given command to a line command.
   *
   * @param command_index The index of the command to convert.
   * @param reference_point The control point to return the updated index of.
   * @return The updated index of the reference point.
   */
  uint32_t to_line(const uint32_t command_index, uint32_t reference_point = 0);

  /**
   * @brief Converts the given command to a quadratic command.
   *
   * Only line commands can be converted to quadratic commands.
   *
   * @param command_index The index of the command to convert.
   * @param reference_point The control point to return the updated index of.
   * @return The updated index of the reference point.
   */
  uint32_t to_quadratic(const uint32_t command_index, uint32_t reference_point = 0);

  /**
   * @brief Converts the given command to a cubic command.
   *
   * @param command_index The index of the command to convert.
   * @param reference_point The control point to return the updated index of.
   * @return The updated index of the reference point.
   */
  uint32_t to_cubic(const uint32_t command_index, uint32_t reference_point = 0);

  /**
   * @brief Removes the ith control point from the path.
   *
   * @param point_index The index of the control point to remove.
   * @param keep_shape Whether to keep the shape of the path after removing the control point.
   * Default is false.
   */
  void remove(const uint32_t point_index, const bool keep_shape = false);

  /**
   * @brief Splits the segment at the given index at the given t value.
   *
   * @param segment_index The index of the segment to split.
   * @param t The t value to split the segment at.
   * @return The index of the newly added vertex.
   */
  uint32_t split(const uint32_t segment_index, const T t);

  /**
   * @brief Calculates the bounding rectangle of the path.
   *
   * @return The bounding rectangle of the path.
   */
  math::Rect<T> bounding_rect() const;

  /**
   * @brief Calculates the bounding rectangle of the path in the fixed reference system.
   *
   * @param transform The transformation matrix to apply to the path.
   * @return The bounding rectangle of the path.
   */
  math::Rect<T> bounding_rect(const math::Mat2x3<T>& transform) const;

  /**
   * @brief Calculates the approximate bounding rectangle of the path considering all control
   * points as vertices.
   *
   * @return The approximate bounding rectangle of the path.
   */
  math::Rect<T> approx_bounding_rect() const;

  /**
   * @brief Checks whether the given point is inside the path or not.
   *
   * @param point The point to check.
   * @param fill The fill of the path, can be nullptr.
   * @param stroke The stroke of the path, can be nullptr.
   * @param transform The transformation matrix to apply to the path.
   * @param threshold The threshold to use for the check.
   * @param zoom The zoom level to use for the check.
   * @param deep_search Whether to include handles in the search or not.
   * @return true if the point is inside the path, false otherwise.
   */
  bool is_point_inside_path(const math::Vec2<T> point,
                            const FillingOptions* fill,
                            const StrokingOptions<T>* stroke,
                            const math::Mat2x3<T>& transform,
                            const T threshold = T(0),
                            const bool deep_search = false) const;

  /**
   * @brief Checks whether the given point is inside the specified segment of the path or not.
   *
   * @param segment_index The index of the segment to check.
   * @param point The point to check.
   * @param stroke The stroke of the path, can be nullptr.
   * @param transform The transformation matrix to apply to the path.
   * @param threshold The threshold to use for the check.
   * @param zoom The zoom level to use for the check.
   * @return true if the point is near the segment, false otherwise.
   */
  bool is_point_inside_segment(const uint32_t segment_index,
                               const math::Vec2<T> point,
                               const StrokingOptions<T>* stroke,
                               const math::Mat2x3<T>& transform,
                               const T threshold = T(0)) const;

  /**
   * @brief Checks whether the given point is inside the specified path's point or not.
   *
   * A point can be either a vertex or a handle, to check use the is_vertex() or is_handle()
   * methods.
   *
   * @param point_index The index of the point to check.
   * @param point The point to check.
   * @param transform The transformation matrix to apply to the path.
   * @param threshold The threshold to use for the check.
   * @return true if the two points are near each other, false otherwise.
   */
  bool is_point_inside_point(const uint32_t point_index,
                             const math::Vec2<T> point,
                             const math::Mat2x3<T>& transform,
                             const T threshold = T(0)) const;

  /**
   * @brief Checks whether the path intersects the given rect or not.
   *
   * @param rect The rect to check.
   * @param indices An optional vector to fill with the indices of the vertices that intersect the
   * rect.
   * @return true if the path intersects the rect, false otherwise.
   */
  bool intersects(const math::Rect<T>& rect, std::vector<uint32_t>* indices = nullptr) const;

  /**
   * @brief Checks whether the path intersects the given rect or not.
   *
   * @param rect The rect to check.
   * @param transform The transformation matrix to apply to the path.
   * @param indices An optional vector to fill with the indices of the vertices that intersect the
   * rect.
   * @return true if the path intersects the rect, false otherwise.
   */
  bool intersects(const math::Rect<T>& rect,
                  const math::Mat2x3<T>& transform,
                  std::vector<uint32_t>* indices = nullptr) const;

  /**
   * @brief Converts the path to a list of quadratic bezier curves.
   *
   * @param tolerance The tolerance to use for the conversion, default is 0.25.
   * @return The quadratic representation of the path.
   */
  QuadraticPath<T> to_quadratic_path(const T tolerance = T(2e-2)) const;

  /**
   * @brief Converts the path to a list of cubic bezier curves.
   *
   * @return The cubic representation of the path.
   */
  CubicPath<T> to_cubic_path() const;

  /**
   * @brief Returns a new path that is the result of transforming the current path by the given
   * matrix.
   *
   * Does not check if the transform is an identity matrix, it should be done before calling this
   * method. Incoming and outgoing handles are transformed as well.
   *
   * @param transform The transformation matrix to apply to the path.
   * @return The transformed path with the new underlying type.
   */
  template<typename U>
  Path<U> transformed(const math::Mat2x3<T>& transform) const;

  /**
   * @brief Returns a new path that is the result of transforming the current path by the given
   * matrix.
   *
   * Does not check if the transform is an identity matrix, it should be done before calling this
   * method. Incoming and outgoing handles are transformed as well.
   *
   * @param transform The transformation matrix to apply to the path.
   * @param r_bounding_rect The bounding rectangle of the transformed path to fill.
   * @return The transformed path with the new underlying type.
   */
  template<typename U>
  Path<U> transformed(const math::Mat2x3<T>& transform, math::Rect<U>& r_bounding_rect) const;

  /**
   * @brief Encodes the path to a list of bytes.
   *
   * @return The encoded path.
   */
  io::EncodedData& encode(io::EncodedData& data) const;

 private:
  /**
   * @brief Returns the ith command of the path.
   *
   * @param index The index of the command to return.
   * @return The ith command of the path.
   */
  inline Command get_command(const uint32_t index) const
  {
    return static_cast<Command>((m_commands[index / 4] >> (6 - (index % 4) * 2)) & 0b00000011);
  }

  /**
   * @brief Pushes a command to the path, handling the packing logic.
   *
   * @param command The command to push.
   */
  void push_command(const Command command);

  /**
   * @brief Inserts a command to the path, handling the packing logic.
   *
   * @param command The command to insert.
   * @param index The index to insert the command at.
   */
  void insert_command(const Command command, const uint32_t index);

  /**
   * @brief Replaces the ith command of the path with the given one.
   *
   * @param index The index of the command to replace.
   * @param command The command to replace the ith command with.
   */
  void replace_command(const uint32_t index, const Command command);

  /**
   * @brief Removes the ith command of the path.
   *
   * @param index The index of the command to remove.
   */
  void remove_command(const uint32_t index);

  /**
   * @brief Checks whether the given point is inside the stroked path or not.
   *
   * The path should be pre-transformed before calling this method.
   *
   * @param point The point to check.
   * @param stroke The stroke of the path, can be nullptr.
   * @param threshold The threshold to use for the check.
   * @param deep_search Whether to include handles in the search or not.
   * @return true if the point is inside the stroked path, false otherwise.
   */
  bool is_point_inside_stroke(const math::Vec2<T> point,
                              const StrokingOptions<T>* stroke,
                              const T threshold = T(0),
                              const bool deep_search = false) const;

 private:
  std::vector<uint8_t> m_commands;      // The commands used to traverse the path.
  std::vector<math::Vec2<T>> m_points;  // The points of the path.

  uint32_t m_commands_size = 0;         // The effective number of commands in the path.
  bool m_closed = false;                // Whether the path is closed or not.

  math::Vec2<T>
      m_in_handle;   // The incoming handle of the path, its index is Path::in_handle_index.
  math::Vec2<T>
      m_out_handle;  // The outgoing handle of the path, its index is Path::out_handle_index.
 private:
  template<typename U, typename _>
  friend class Path;

  template<typename U, typename _>
  friend class PathBuilder;
};

}  // namespace graphick::geom

/* -- Aliases -- */

namespace graphick::geom {

using path = Path<float>;
using dpath = Path<double>;

}  // namespace graphick::geom
