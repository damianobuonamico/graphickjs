/**
 * @file path.h
 * @brief Path class definition
 */

#pragma once

#include "../../math/vec2.h"
#include "../../math/rect.h"
#include "../../math/mat2x3.h"

#include "../../io/encode/encode.h"

#include <unordered_set>
#include <functional>
#include <optional>
#include <vector>
#include <tuple>

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
  class Path {
  public:
    /**
     * @brief The Command enum represents the type of commands used to traverse the path.
     */
    enum Command {
      Move = 0,         /* Move to a point. */
      Line = 1,         /* Linear segment. */
      Quadratic = 2,    /* Quadratic bezier curve. */
      Cubic = 3         /* Cubic bezier curve. */
    };

    /**
     * @brief The IndexType enum represents the type of index used to traverse the path.
     */
    enum class IndexType {
      Point = 0,        /* The index represents the ith control point. */
      Command = 1,      /* The index represents the ith command. */
      Segment = 2       /* The index represents the ith segment. */
    };

    static const size_t in_handle_index = std::numeric_limits<uint32_t>::max() - 1;     /* The index of the incoming handle. */
    static const size_t out_handle_index = std::numeric_limits<uint32_t>::max() - 2;    /* The index of the outgoing handle. */
  public:
    /**
     * @brief This struct represents a vertex node of the path.
     *
     * The vertex node is represented by the index of the incoming and outgoing handles and the index of the vertex itself.
     *
     * @struct VertexNode
     */
    struct VertexNode {
      size_t vertex;           /* The index of the vertex. */

      int64_t in;              /* The index of the incoming handle, -1 if no incoming handle is present. */
      int64_t out;             /* The index of the outgoing handle. -1 if no outgoing handle is present. */

      int64_t close_vertex;    /* The index of the vertex that closes the sub-path, -1 if the sub-path is not closed or the vertex is not a sub-path end. */

      int64_t in_command;      /* The index of the incoming command, -1 if no incoming command is present. */
      int64_t out_command;     /* The index of the outgoing command, -1 if no outgoing command is present. */
    };

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

      /**
       * @brief Whether the segment is a point or not.
       *
       * A segment is considered a point if all control points are equal.
       *
       * @return true if the segment is a point, false otherwise.
       */
      bool is_point() const;

      /**
       * @brief Samples the segment at the given t value.
       *
       * @param t The t value to sample the segment at.
       * @return The point on the segment at the given t value.
       */
      vec2 sample(const float t) const;

      /**
       * @brief Returns the bounding rectangle of the segment.
       *
       * @return The bounding rectangle of the segment.
       */
      Math::rect bounding_rect() const;

      /**
       * @brief Calculates the bounding rectangle of the segment in the fixed reference system.
       *
       * @param transform The transformation matrix to apply to the segment.
       * @return The bounding rectangle of the segment.
       */
      Math::rect bounding_rect(const mat2x3& transform) const;

      /**
       * @brief Calculates the approximate bounding rectangle of the segment considering all control points as vertices.
       *
       * @return The approximate bounding rectangle of the segment.
       */
      Math::rect approx_bounding_rect() const;
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
       * If index_type is IndexType::Point, and the point corresponds to a vertex, the iterator will point to the segment that ends at the vertex.
       *
       * @param path The path to iterate over.
       * @param index The start index of the iterator.
       * @param index_type The type of index to use, default is IndexType::Command.
       */
      Iterator(const Path& path, const size_t index, const IndexType index_type = IndexType::Command);

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
      Iterator operator+(const size_t n) const;

      /**
       * @brief Pre and post decrement operators.
       */
      Iterator& operator--();
      Iterator operator--(int);
      Iterator operator-(const size_t n) const;

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
      inline size_t command_index() const { return m_index; }

      /**
       * @brief Returns the index of the segment the iterator is currently pointing to.
       *
       * @return The index of the segment the iterator is currently pointing to.
       */
      inline size_t segment_index() const { return std::max(size_t(1), m_index) - 1; }

      /**
       * @brief Returns the index of the point the iterator is currently pointing to.
       *
       * @return The index of the point the iterator is currently pointing to.
       */
      inline size_t point_index() const { return m_point_index; }
    private:
      size_t m_index;           /* The current command index of the iterator. */
      size_t m_point_index;     /* The index of the last point iterated over. */

      const Path& m_path;       /* The path to iterate over. */
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
       * @brief Constructs an iterator over the given path at the given command index.
       *
       * @param path The path to iterate over.
       * @param index The start index of the iterator.
       */
      ReverseIterator(const Path& path, const size_t index);

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
      size_t m_index;           /* The current segment index of the iterator. */
      size_t m_point_index;     /* The index of the last point iterated over. */

      const Path& m_path;       /* The path to iterate over. */
    };
  public:
    /**
     * @brief Constructors, copy constructor and move constructor.
     */
    Path();
    Path(const Path& other);
    Path(Path&& other) noexcept;

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
     * @brief Returns the ith segment of the path.
     *
     * @param index The index of the segment to return;
     * @param index_type The type of index to use, default is IndexType::Segment.
     * @return The ith segment of the path.
     */
    inline Segment at(const size_t index, const IndexType index_type = IndexType::Segment) const { return *Iterator(*this, index, index_type); }

    /**
     * @brief Returns the ith control point of the path.
     *
     * To retrieve the position of the incoming handle, use the Path::in_handle_index constant.
     * To retrieve the position of the outgoing handle, use the Path::out_handle_index constant.
     *
     * @param point_index The index of the point to return.
     * @return The ith control point of the path.
     */
    vec2 point_at(const size_t point_index) const;

    /**
     * @brief Checks whether the path has an incoming handle or not.
     *
     * @return true if the path has an incoming handle, false otherwise.
     */
    inline bool has_in_handle() const { return !vacant() && !closed() && m_in_handle != m_points.front(); }

    /**
     * @brief Checks whether the path has an outgoing handle or not.
     *
     * @return true if the path has an outgoing handle, false otherwise.
     */
    inline bool has_out_handle() const { return !vacant() && !closed() && m_out_handle != m_points.back(); }

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
     * @brief Reverse iterates over the commands of the path, calling the given callbacks for each command.
     *
     * @param move_callback The callback to call for each move command.
     * @param line_callback The callback to call for each line command.
     * @param quadratic_callback The callback to call for each quadratic command.
     * @param cubic_callback The callback to call for each cubic command.
     */
    void for_each_reversed(
      std::function<void(const vec2)> move_callback = nullptr,
      std::function<void(const vec2, const vec2)> line_callback = nullptr,
      std::function<void(const vec2, const vec2, const vec2)> quadratic_callback = nullptr,
      std::function<void(const vec2, const vec2, const vec2, const vec2)> cubic_callback = nullptr
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
    size_t size() const;

    /**
     * @brief Returns the number of control points in the path.
     *
     * @param include_handles Whether or not to count the handles when counting the points, default is true.
     * @return The number of points in the path.
     */
    size_t points_size(const bool include_handles = true) const;

    /**
     * @brief Returns a vector containing the incdices of all of the vertex control points.
     *
     * Closing vertices are not included in the vector.
     *
     * @return The vector of indices.
    */
    std::vector<size_t> vertex_indices() const;

    /**
     * @brief Checks whether the given point is a vertex or not.
     *
     * A point is considered a vertex if it is the destination of a command.
     *
     * @param point_index The index of the point to check.
     * @return true if the point is a vertex, false otherwise.
     */
    bool is_vertex(const size_t point_index) const;

    /**
     * @brief Checks whether the given point is a handle or not.
     *
     * A point is considered a handle if it is not a vertex, i.e. it is a control point of a curve.
     *
     * @param point_index The index of the point to check.
     * @return true if the point is a handle, false otherwise.
     */
    inline bool is_handle(const size_t point_index) const { return !is_vertex(point_index); }

    /**
     * @brief Checks whether the given point is the first or last point of an open path.
     *
     * @param point_index The index of the point to check.
     * @return true if the point is an open end, false otherwise.
     */
    bool is_open_end(const size_t point_index) const;

    /**
     * @brief Returns the vertex node the given control point is part of.
     *
     * @param point_index The index of the point to check.
     * @return The vertex node the given control point is part of.
     */
    VertexNode node_at(const size_t point_index) const;

    /**
     * @brief Checks whether the path is closed or not.
     *
     * @return true if the path is closed, false otherwise.
     */
    inline bool closed() const { return !empty() && m_closed; }

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
     * @param reverse Whether to add the point to the beginning of the path, instead of the end. Default is false.
     */
    void line_to(const vec2 point, const bool reverse = false);

    /**
     * @brief Adds a quadratic bezier curve to the path.
     *
     * @param control The control point of the curve.
     * @param point The point to add to the path.
     * @param reverse Whether to add the point to the beginning of the path, instead of the end. Default is false.
     */
    void quadratic_to(const vec2 control, const vec2 point, const bool reverse = false);

    /**
     * @brief Adds a cubic bezier curve to the path.
     *
     * @param control_1 The first control point of the curve.
     * @param control_2 The second control point of the curve.
     * @param point The point to add to the path.
     * @param reverse Whether to add the point to the beginning of the path, instead of the end. Default is false.
     */
    void cubic_to(const vec2 control_1, const vec2 control_2, const vec2 point, const bool reverse = false);

    /**
     * @brief Adds a cubic bezier curve to the path.
     *
     * @param control The control point of the curve.
     * @param point The point to add to the path.
     * @param is_control_1 Whether the control point is the first or the second one.
     * @param reverse Whether to add the point to the beginning of the path, instead of the end. Default is false.
     */
    void cubic_to(const vec2 control, const vec2 point, const bool is_control_1 = true, const bool reverse = false);

    /**
     * @brief Adds an arc to the path.
     *
     * @param center The center of the arc.
     * @param radius The radius of the arc.
     * @param x_axis_rotation The rotation of the arc over the x-axis.
     * @param large_arc_flag Whether the arc is large or not.
     * @param sweep_flag Whether the arc is clockwise or not.
     * @param point The end point of the arc.
     * @param reverse Whether to add the point to the beginning of the path, instead of the end. Default is false.
     */
    void arc_to(const vec2 center, const vec2 radius, const float x_axis_rotation, const bool large_arc_flag, const bool sweep_flag, const vec2 point, const bool reverse = false);

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
     * @brief Translates the ith point of the path by the given delta.
     *
     * This method takes care of updating the control points of the curves if the point is a vertex.
     *
     * @param point_index The index of the point to translate.
     * @param delta The delta to translate the point by.
    */
    void translate(const size_t point_index, const vec2 delta);

    /**
     * @brief Converts the given command to a line command.
     *
     * @param command_index The index of the command to convert.
    */
    void to_line(const size_t command_index);

    /**
     * @brief Converts the given command to a cubic command.
     *
     * @param command_index The index of the command to convert.
     * @param reference_point The control point to return the updated index of
     * @return The updated index of the reference point.
     */
    size_t to_cubic(const size_t command_index, size_t reference_point = 0);

      /**
     * @brief Removes the ith control point from the path.
     *
     * @param point_index The index of the control point to remove.
     * @param keep_shape Whether to keep the shape of the path after removing the control point. Default is false.
     */
    void remove(const size_t point_index, const bool keep_shape = false);

    /**
     * @brief Splits the segment at the given index at the given t value.
     *
     * @param segment_index The index of the segment to split.
     * @param t The t value to split the segment at.
     * @return The index of the newly added vertex.
     */
    size_t split(const size_t segment_index, const float t);

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
    Math::rect bounding_rect(const mat2x3& transform) const;

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
     * @param transform The transformation matrix to apply to the path.
     * @param threshold The threshold to use for the check.
     * @param zoom The zoom level to use for the check.
     * @param deep_search Whether to include handles in the search or not.
     * @return true if the point is inside the path, false otherwise.
     */
    bool is_point_inside_path(const vec2 point, const Fill* fill, const Stroke* stroke, const mat2x3& transform, const float threshold = 0.0f, const double zoom = 1.0, const bool depp_search = false) const;

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
    bool is_point_inside_segment(const size_t segment_index, const vec2 point, const Stroke* stroke, const mat2x3& transform, const float threshold = 0.0f, const double zoom = 1.0) const;

    /**
     * @brief Checks whether the given point is inside the specified path's point or not.
     *
     * A point can be either a vertex or a handle, to check use the is_vertex() or is_handle() methods.
     *
     * @param point_index The index of the point to check.
     * @param point The point to check.
     * @param transform The transformation matrix to apply to the path.
     * @param threshold The threshold to use for the check.
     * @return true if the two points are near each other, false otherwise.
    */
    bool is_point_inside_point(const size_t point_index, const vec2 point, const mat2x3& transform, const float threshold = 0.0f) const;

    /**
     * @brief Checks whether the path intersects the given rect or not.
     *
     * @param rect The rect to check.
     * @param indices An optional set to fill with the indices of the vertices that intersect the rect.
     * @return true if the path intersects the rect, false otherwise.
     */
    bool intersects(const Math::rect& rect, std::unordered_set<size_t>* indices = nullptr) const;

    /**
     * @brief Checks whether the path intersects the given rect or not.
     *
     * @param rect The rect to check.
     * @param transform The transformation matrix to apply to the path.
     * @param indices An optional set to fill with the indices of the vertices that intersect the rect.
     * @return true if the path intersects the rect, false otherwise.
     */
    bool intersects(const Math::rect& rect, const mat2x3& transform, std::unordered_set<size_t>* indices = nullptr) const;

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
    inline Command get_command(const size_t index) const { return static_cast<Command>((m_commands[index / 4] >> (6 - (index % 4) * 2)) & 0b00000011); }

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
    void insert_command(const Command command, const size_t index);

    /**
     * @brief Replaces the ith command of the path with the given one.
     *
     * @param index The index of the command to replace.
     * @param command The command to replace the ith command with.
     */
    void replace_command(const size_t index, const Command command);

    /**
     * @brief Removes the ith command of the path.
     *
     * @param index The index of the command to remove.
     */
    void remove_command(const size_t index);
  private:
    std::vector<uint8_t> m_commands;    /* The commands used to traverse the path. */
    std::vector<vec2> m_points;         /* The points of the path. */

    size_t m_commands_size = 0;         /* The effective number of commands in the path. */
    bool m_closed = false;              /* Whether the path is closed or not. */

    vec2 m_in_handle;                   /* The incoming handle of the path, its index is Path::in_handle_index. */
    vec2 m_out_handle;                  /* The outgoing handle of the path, its_index is Path::out_handle_index. */
  };

}
