/**
 * @file geom/path.h
 * @brief Contains the definition of the Path class, the main building block of the Graphick editor.
 */

#pragma once

#include "quadratic_bezier.h"
#include "quadratic_path.h"
#include "cubic_bezier.h"
#include "line.h"

namespace graphick::geom {

  /**
   * @brief The Path class represents the path representation used throughout the graphick editor.
   *
   * The path is represented by a list of points and a list of tightly packed traversing commands.
   *
   * @class Path
   */
  template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
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

    static const uint32_t in_handle_index = std::numeric_limits<uint32_t>::max() - 1;     /* The index of the incoming handle. */
    static const uint32_t out_handle_index = std::numeric_limits<uint32_t>::max() - 2;    /* The index of the outgoing handle. */
  public:
    /**
     * @brief This struct represents a vertex node of the path.
     *
     * The vertex node is represented by the index of the incoming and outgoing handles and the index of the vertex itself.
     *
     * @struct VertexNode
     */
    struct VertexNode {
      uint32_t vertex;         /* The index of the vertex. */

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
      Command type = Command::Move;                /* The type of the segment. */

      math::Vec2<T> p0 = math::Vec2<T>::zero();    /* The first point of the segment or move command destination. */
      math::Vec2<T> p1 = math::Vec2<T>::zero();    /* The second point of the segment (line, quadratic, cubic). */
      math::Vec2<T> p2 = math::Vec2<T>::zero();    /* The third point of the segment (quadratic, cubic). */
      math::Vec2<T> p3 = math::Vec2<T>::zero();    /* The fourth point of the segment (cubic). */

      /**
       * @brief Constructors.
       */
      Segment(const math::Vec2<T> p0, const math::Vec2<T> p1) :
        type(Command::Line), p0(p0), p1(p1) {}
      Segment(const math::Vec2<T> p0, const math::Vec2<T> p1, const math::Vec2<T> p2) :
        type(Command::Quadratic), p0(p0), p1(p1), p2(p2) {}
      Segment(const math::Vec2<T> p0, const math::Vec2<T> p1, const math::Vec2<T> p2, const math::Vec2<T> p3) :
        type(Command::Cubic), p0(p0), p1(p1), p2(p2), p3(p3) {}

      /**
       * @brief Type checking methods.
       */
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
       * @brief Type casts.
      */
      inline geom::Line<T> to_line() const { return geom::Line<T>(p0, p1); }
      inline geom::QuadraticBezier<T> to_quadratic() const { return geom::QuadraticBezier<T>(p0, p1, p2); }
      inline geom::CubicBezier<T> to_cubic() const { return geom::CubicBezier<T>(p0, p1, p2, p3); }

      /**
       * @brief Type casts operators.
       */
      inline operator geom::Line<T>() { return to_line(); }
      inline operator geom::QuadraticBezier<T>() { return to_quadratic(); }
      inline operator geom::CubicBezier<T>() { return to_cubic(); }
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
      using difference_type = uint32_t;
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
      Iterator(const Path<T>& path, const uint32_t index, const IndexType index_type = IndexType::Command);

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
      inline uint32_t command_index() const { return m_index; }

      /**
       * @brief Returns the index of the segment the iterator is currently pointing to.
       *
       * @return The index of the segment the iterator is currently pointing to.
       */
      inline uint32_t segment_index() const { return std::max(uint32_t(1), m_index) - 1; }

      /**
       * @brief Returns the index of the point the iterator is currently pointing to.
       *
       * @return The index of the point the iterator is currently pointing to.
       */
      inline uint32_t point_index() const { return m_point_index; }
    private:
      uint32_t m_index;          /* The current command index of the iterator. */
      uint32_t m_point_index;    /* The index of the last point iterated over. */

      const Path<T>& m_path;     /* The path to iterate over. */
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
      uint32_t m_index;          /* The current segment index of the iterator. */
      uint32_t m_point_index;    /* The index of the last point iterated over. */

      const Path<T>& m_path;     /* The path to iterate over. */
    };
  public:
  
  };

}

/* -- Aliases -- */

namespace graphick::geom {

  using path = Path<float>;
  using dpath = Path<double>;

}

namespace graphick {

  using geom::path;
  using geom::dpath;

}
