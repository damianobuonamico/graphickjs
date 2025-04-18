/**
 * @file geom/path.cpp
 * @brief Implementation of the Path class.
 *
 * @todo implement compound paths when groups are a thing.
 */

#include "path.h"

#include "curve_ops.h"
#include "intersections.h"
#include "path_builder.h"

#include "../math/matrix.h"

#include "../algorithms/fit.h"

#include "../io/encode/encode.h"

#include "../utils/assert.h"
#include "../utils/debugger.h"

#define FIT_RESOLUTION 10

namespace graphick::geom {

/* -- Segment -- */

template<typename T, typename _>
bool Path<T, _>::Segment::is_point() const
{
  bool point = math::is_almost_equal(p0, p1);

  if (point) {
    if (is_quadratic())
      return math::is_almost_equal(p1, p2);
    if (is_cubic())
      return math::is_almost_equal(p1, p2) && math::is_almost_equal(p2, p3);
  }

  return point;
}

/* -- Iterator -- */

template<typename T, typename _>
Path<T, _>::Iterator::Iterator(const Path<T>& path,
                               const uint32_t index,
                               const IndexType index_type)
    : m_path(path), m_index(index), m_point_index(0)
{
  if (index_type == IndexType::Point) {
    GK_ASSERT(index < path.m_points.size(), "Point index out of range.");

    m_index = 0;

    while (m_point_index <= index) {
      switch (path.get_command(m_index)) {
        case Command::Move:
          if (index == m_point_index) {
            operator++();
            return;
          }

          m_point_index += 1;
          break;
        case Command::Line:
          if (index == m_point_index) {
            return;
          }

          m_point_index += 1;
          break;
        case Command::Quadratic:
          if (index - m_point_index <= 1) {
            return;
          }

          m_point_index += 2;
          break;
        case Command::Cubic:
          if (index - m_point_index <= 2) {
            return;
          }

          m_point_index += 3;
          break;
      }

      m_index += 1;
    }

    return;
  } else if (index_type == IndexType::Segment) {
    GK_ASSERT(index < path.size(), "Segment index out of range.");

    m_index = index + 1;
  }

  if (m_index < path.m_commands_size && path.get_command(m_index) == Command::Move) {
    m_index += 1;
  }

  GK_ASSERT(m_index > 0 && m_index <= path.m_commands_size, "Index out of range.");

  if (m_index <= path.m_commands_size / 2) {
    for (uint32_t i = 0; i < m_index; i++) {
      switch (path.get_command(i)) {
        case Command::Move:
        case Command::Line:
          m_point_index += 1;
          break;
        case Command::Quadratic:
          m_point_index += 2;
          break;
        case Command::Cubic:
          m_point_index += 3;
          break;
      }
    }
  } else {
    m_point_index = static_cast<uint32_t>(path.m_points.size());

    for (uint32_t i = path.m_commands_size - 1; i >= m_index; i--) {
      switch (path.get_command(i)) {
        case Command::Move:
        case Command::Line:
          m_point_index -= 1;
          break;
        case Command::Quadratic:
          m_point_index -= 2;
          break;
        case Command::Cubic:
          m_point_index -= 3;
          break;
      }
    }
  }
}

template<typename T, typename _>
typename Path<T, _>::Iterator& Path<T, _>::Iterator::operator++()
{
  GK_ASSERT(m_index < m_path.m_commands_size, "Cannot increment the end iterator.");

  switch (m_path.get_command(m_index)) {
    case Command::Move:
    case Command::Line:
      m_point_index += 1;
      break;
    case Command::Quadratic:
      m_point_index += 2;
      break;
    case Command::Cubic:
      m_point_index += 3;
      break;
  }

  m_index += 1;

  if (m_index < m_path.m_commands_size && m_path.get_command(m_index) == Command::Move)
    operator++();

  return *this;
}

template<typename T, typename _>
typename Path<T, _>::Iterator Path<T, _>::Iterator::operator++(int)
{
  Iterator tmp = *this;
  ++(*this);

  return tmp;
}

template<typename T, typename _>
typename Path<T, _>::Iterator Path<T, _>::Iterator::operator+(const uint32_t n) const
{
  Iterator tmp = *this;

  for (uint32_t i = 0; i < n; i++) {
    ++tmp;
  }

  return tmp;
}

template<typename T, typename _>
typename Path<T, _>::Iterator& Path<T, _>::Iterator::operator--()
{
  GK_ASSERT(m_index > 0, "Cannot decrement the begin iterator.");

  m_index -= 1;

  switch (m_path.get_command(m_index)) {
    case Command::Move:
      operator--();
    case Command::Line:
      m_point_index -= 1;
      break;
    case Command::Quadratic:
      m_point_index -= 2;
      break;
    case Command::Cubic:
      m_point_index -= 3;
      break;
  }

  return *this;
}

template<typename T, typename _>
typename Path<T, _>::Iterator Path<T, _>::Iterator::operator--(int)
{
  Iterator tmp = *this;
  --(*this);

  return tmp;
}

template<typename T, typename _>
typename Path<T, _>::Iterator Path<T, _>::Iterator::operator-(const uint32_t n) const
{
  Iterator tmp = *this;

  for (uint32_t i = 0; i < n; i++) {
    --tmp;
  }

  return tmp;
}

template<typename T, typename _>
typename Path<T, _>::Iterator::value_type Path<T, _>::Iterator::operator*() const
{
  const Command command = m_path.get_command(m_index);

  switch (command) {
    case Command::Cubic: {
      GK_ASSERT(m_point_index > 0 && m_point_index + 2 < m_path.m_points.size(),
                "Not enough points for a cubic bezier.");
      return Segment{m_path.m_points[m_point_index - 1],
                     m_path.m_points[m_point_index],
                     m_path.m_points[m_point_index + 1],
                     m_path.m_points[m_point_index + 2]};
    }
    case Command::Quadratic: {
      GK_ASSERT(m_point_index > 0 && m_point_index + 1 < m_path.m_points.size(),
                "Not enough points for a quadratic bezier.");
      return Segment{m_path.m_points[m_point_index - 1],
                     m_path.m_points[m_point_index],
                     m_path.m_points[m_point_index + 1]};
    }
    case Command::Line: {
      GK_ASSERT(m_point_index > 0 && m_point_index < m_path.m_points.size(),
                "Points vector subscript out of range.");
      return Segment{m_path.m_points[m_point_index - 1], m_path.m_points[m_point_index]};
    }
    default:
    case Command::Move:
      Iterator temp = operator+(1);
      return *temp;
  }
}

/* -- ReverseIterator -- */

template<typename T, typename _>
Path<T, _>::ReverseIterator::ReverseIterator(const Path<T>& path, const uint32_t index)
    : m_path(path), m_index(index), m_point_index(0)
{
  if (m_index != 0 && path.get_command(m_index) == Command::Move)
    m_index--;

  GK_ASSERT(m_index >= 0 && m_index < path.m_commands_size, "Index out of range.");

  if (m_index < path.m_commands_size / 2) {
    for (uint32_t i = 0; i < m_index; i++) {
      switch (path.get_command(i)) {
        case Command::Move:
        case Command::Line:
          m_point_index += 1;
          break;
        case Command::Quadratic:
          m_point_index += 2;
          break;
        case Command::Cubic:
          m_point_index += 3;
          break;
      }
    }
  } else {
    m_point_index = static_cast<uint32_t>(path.m_points.size());

    for (uint32_t i = path.m_commands_size - 1; i >= m_index; i--) {
      switch (path.get_command(i)) {
        case Command::Move:
        case Command::Line:
          m_point_index -= 1;
          break;
        case Command::Quadratic:
          m_point_index -= 2;
          break;
        case Command::Cubic:
          m_point_index -= 3;
          break;
      }
    }
  }
}

template<typename T, typename _>
typename Path<T, _>::ReverseIterator& Path<T, _>::ReverseIterator::operator++()
{
  GK_ASSERT(m_index > 0, "Cannot increment the rend iterator.");

  m_index -= 1;

  switch (m_path.get_command(m_index)) {
    case Command::Move:
      if (m_index > 0)
        operator++();
    case Command::Line:
      m_point_index -= 1;
      break;
    case Command::Quadratic:
      m_point_index -= 2;
      break;
    case Command::Cubic:
      m_point_index -= 3;
      break;
  }

  return *this;
}

template<typename T, typename _>
typename Path<T, _>::ReverseIterator Path<T, _>::ReverseIterator::operator++(int)
{
  ReverseIterator tmp = *this;
  ++(*this);

  return tmp;
}

template<typename T, typename _>
typename Path<T, _>::ReverseIterator Path<T, _>::ReverseIterator::operator+(const uint32_t n) const
{
  ReverseIterator tmp = *this;

  for (uint32_t i = 0; i < n; i++) {
    ++tmp;
  }

  return tmp;
}

template<typename T, typename _>
typename Path<T, _>::ReverseIterator& Path<T, _>::ReverseIterator::operator--()
{
  GK_ASSERT(m_index < m_path.m_commands_size, "Cannot decrement the rbegin iterator.");

  switch (m_path.get_command(m_index)) {
    case Command::Move:
    case Command::Line:
      m_point_index += 1;
      break;
    case Command::Quadratic:
      m_point_index += 2;
      break;
    case Command::Cubic:
      m_point_index += 3;
      break;
  }

  m_index += 1;

  if (m_index < m_path.m_commands_size && m_path.get_command(m_index) == Command::Move)
    operator++();

  return *this;
}

template<typename T, typename _>
typename Path<T, _>::ReverseIterator Path<T, _>::ReverseIterator::operator--(int)
{
  ReverseIterator tmp = *this;
  --(*this);

  return tmp;
}

template<typename T, typename _>
typename Path<T, _>::ReverseIterator Path<T, _>::ReverseIterator::operator-(const uint32_t n) const
{
  ReverseIterator tmp = *this;

  for (uint32_t i = 0; i < n; i++) {
    --tmp;
  }

  return tmp;
}

template<typename T, typename _>
typename Path<T, _>::ReverseIterator::value_type Path<T, _>::ReverseIterator::operator*() const
{
  const Command command = m_path.get_command(m_index);

  switch (command) {
    case Command::Cubic: {
      GK_ASSERT(m_point_index > 0 && m_point_index + 2 < m_path.m_points.size(),
                "Not enough points for a cubic bezier.");
      return Segment{m_path.m_points[m_point_index - 1],
                     m_path.m_points[m_point_index],
                     m_path.m_points[m_point_index + 1],
                     m_path.m_points[m_point_index + 2]};
    }
    case Command::Quadratic: {
      GK_ASSERT(m_point_index > 0 && m_point_index + 1 < m_path.m_points.size(),
                "Not enough points for a quadratic bezier.");
      return Segment{m_path.m_points[m_point_index - 1],
                     m_path.m_points[m_point_index],
                     m_path.m_points[m_point_index + 1]};
    }
    case Command::Line: {
      GK_ASSERT(m_point_index > 0 && m_point_index < m_path.m_points.size(),
                "Points vector subscript out of range.");
      return Segment{m_path.m_points[m_point_index - 1], m_path.m_points[m_point_index]};
    }
    default:
    case Command::Move:
      ReverseIterator temp = operator-(1);
      return *temp;
  }
}

/* -- Path -- */

template<typename T, typename _>
Path<T, _>::Path()
    : m_points(),
      m_commands(),
      m_commands_size(0),
      m_closed(false),
      m_in_handle(math::Vec2<T>::zero()),
      m_out_handle(math::Vec2<T>::zero())
{
}

template<typename T, typename _>
Path<T, _>::Path(const Path<T>& other)
    : m_points(other.m_points),
      m_commands(other.m_commands),
      m_commands_size(other.m_commands_size),
      m_closed(other.m_closed),
      m_in_handle(other.m_in_handle),
      m_out_handle(other.m_out_handle)
{
}

template<typename T, typename _>
Path<T, _>::Path(Path<T>&& other) noexcept
    : m_points(std::move(other.m_points)),
      m_commands(std::move(other.m_commands)),
      m_commands_size(other.m_commands_size),
      m_closed(other.m_closed),
      m_in_handle(other.m_in_handle),
      m_out_handle(other.m_out_handle)
{
}

template<typename T, typename _>
template<typename U>
Path<T, _>::Path(const Path<U>& other)
    : m_points(other.m_points.begin(), other.m_points.end()),
      m_commands(other.m_commands.begin(), other.m_commands.end()),
      m_commands_size(other.m_commands_size),
      m_closed(other.m_closed),
      m_in_handle(other.m_in_handle),
      m_out_handle(other.m_out_handle)
{
}

template<typename T, typename _>
Path<T, _>::Path(io::DataDecoder& decoder)
{
  /* Commands and points are always present, is_closed is encoded in the properties bitfield. */
  const auto [not_vacant, is_closed, has_in_handle, has_out_handle] = decoder.bitfield<4>();

  if (!not_vacant) {
    m_commands_size = 0;
    m_closed = false;

    return;
  }

  /* Commands are stored in encoded form. */
  m_commands = decoder.vector<uint8_t>();

  if (m_commands.empty()) {
    m_commands_size = 0;
    m_closed = false;

    return;
  }

  /* Points are just stored as a list of coordinates. */
  m_points = decoder.vector<math::Vec2<T>>();
  m_closed = is_closed;

  uint32_t point_index = 0;
  uint32_t last_index = 0;
  uint32_t last_point_index = 0;

  for (uint32_t i = 0; i < m_commands.size() * 4; i++) {
    Command command = get_command(i);

    switch (command) {
      case Command::Move:
      case Command::Line:
        point_index += 1;
        break;
      case Command::Quadratic:
        point_index += 2;
        break;
      case Command::Cubic:
        point_index += 3;
        break;
    }

    if (command != Command::Move) {
      last_index = i;
      last_point_index = point_index;
    }
  }

  if (last_index == 0) {
    /* If there is only one command, it must be a move command. */
    m_commands_size = 1;
    m_commands.resize(1);
    m_points.resize(1);
  } else {
    /* Trim the points in case the last command was a move command. */
    m_commands_size = last_index + 1;
    m_points.resize(last_point_index);
  }

  if (has_in_handle) {
    m_in_handle = math::Vec2<T>(decoder.vec2());
  } else {
    m_in_handle = m_points.front();
  }

  if (has_out_handle) {
    m_out_handle = math::Vec2<T>(decoder.vec2());
  } else {
    m_out_handle = m_points.back();
  }
}

template<typename T, typename _>
Path<T, _>& Path<T, _>::operator=(const Path<T, _>& other)
{
  m_points = other.m_points;
  m_commands = other.m_commands;
  m_commands_size = other.m_commands_size;
  m_closed = other.m_closed;
  m_in_handle = other.m_in_handle;
  m_out_handle = other.m_out_handle;

  return *this;
}

template<typename T, typename _>
Path<T, _>& Path<T, _>::operator=(Path<T, _>&& other) noexcept
{
  m_points = std::move(other.m_points);
  m_commands = std::move(other.m_commands);
  m_commands_size = other.m_commands_size;
  m_closed = other.m_closed;
  m_in_handle = other.m_in_handle;
  m_out_handle = other.m_out_handle;

  return *this;
}

template<typename T, typename _>
void Path<T, _>::for_each(
    std::function<void(const math::Vec2<T>)> move_callback,
    std::function<void(const math::Vec2<T>)> line_callback,
    std::function<void(const math::Vec2<T>, const math::Vec2<T>)> quadratic_callback,
    std::function<void(const math::Vec2<T>, const math::Vec2<T>, const math::Vec2<T>)>
        cubic_callback) const
{
  for (uint32_t i = 0, j = 0; i < m_commands_size; i++) {
    switch (get_command(i)) {
      case Command::Cubic:
        GK_ASSERT(j + 2 < m_points.size(), "Not enough points for a cubic bezier.");

        if (cubic_callback) {
          cubic_callback(m_points[j], m_points[j + 1], m_points[j + 2]);
        }

        j += 3;
        break;
      case Command::Quadratic:
        GK_ASSERT(j + 1 < m_points.size(), "Not enough points for a quadratic bezier.");

        if (quadratic_callback) {
          quadratic_callback(m_points[j], m_points[j + 1]);
        }

        j += 2;
        break;
      case Command::Line:
        GK_ASSERT(j < m_points.size(), "Not enough points for a line.");

        if (line_callback) {
          line_callback(m_points[j]);
        }

        j += 1;
        break;
      case Command::Move:
        GK_ASSERT(j < m_points.size(), "Points vector subscript out of range.");

        if (move_callback) {
          move_callback(m_points[j]);
        }

        j += 1;
        break;
    }
  }
}

template<typename T, typename _>
void Path<T, _>::for_each_reversed(
    std::function<void(const math::Vec2<T>)> move_callback,
    std::function<void(const math::Vec2<T>, const math::Vec2<T>)> line_callback,
    std::function<void(const math::Vec2<T>, const math::Vec2<T>, const math::Vec2<T>)>
        quadratic_callback,
    std::function<
        void(const math::Vec2<T>, const math::Vec2<T>, const math::Vec2<T>, const math::Vec2<T>)>
        cubic_callback) const
{
  for (int64_t i = static_cast<int64_t>(m_commands_size) - 1,
               j = static_cast<int32_t>(m_points.size());
       i >= 0;
       i--)
  {
    switch (get_command(static_cast<uint32_t>(i))) {
      case Command::Cubic:
        GK_ASSERT(j - 4 >= 0, "Not enough points for a cubic bezier.");

        if (cubic_callback) {
          cubic_callback(m_points[j - 4], m_points[j - 3], m_points[j - 2], m_points[j - 1]);
        }

        j -= 3;
        break;
      case Command::Quadratic:
        GK_ASSERT(j - 3 >= 0, "Not enough points for a quadratic bezier.");

        if (quadratic_callback) {
          quadratic_callback(m_points[j - 3], m_points[j - 2], m_points[j - 1]);
        }

        j -= 2;
        break;
      case Command::Line:
        GK_ASSERT(j - 2 >= 0, "Not enough points for a line.");

        if (line_callback) {
          line_callback(m_points[j - 2], m_points[j - 1]);
        }

        j -= 1;
        break;
      case Command::Move:
        GK_ASSERT(j - 1 >= 0, "Points vector subscript out of range.");

        if (move_callback) {
          move_callback(m_points[j - 1]);
        }

        j -= 1;
        break;
    }
  }
}

template<typename T, typename _>
std::vector<uint32_t> Path<T, _>::vertex_indices() const
{
  std::vector<uint32_t> indices;

  indices.reserve(points_count(false));

  for (uint32_t i = 0, point_i = 0; i < m_commands_size; i++) {
    switch (get_command(i)) {
      case Command::Move:
        indices.push_back(point_i);
        point_i += 1;
        break;
      case Command::Line:
        indices.push_back(point_i);
        point_i += 1;
        break;
      case Command::Quadratic:
        indices.push_back(point_i + 1);
        point_i += 2;
        break;
      case Command::Cubic:
        indices.push_back(point_i + 2);
        point_i += 3;
        break;
    }
  }

  if (closed()) {
    indices.pop_back();
  }

  return indices;
}

template<typename T, typename _>
bool Path<T, _>::is_vertex(const uint32_t point_index) const
{
  if (point_index == 0)
    return true;

  for (uint32_t i = 0, point_i = 0; i < m_commands_size; i++) {
    if (point_i > point_index)
      return false;

    switch (get_command(i)) {
      case Command::Move:
      case Command::Line:
        point_i += 1;
        break;
      case Command::Quadratic:
        point_i += 2;
        break;
      case Command::Cubic:
        point_i += 3;
        break;
    }

    if (point_i - 1 == point_index) {
      return true;
    }
  }

  return false;
}

template<typename T, typename _>
typename Path<T, _>::VertexNode Path<T, _>::node_at(const uint32_t point_index) const
{
  GK_ASSERT(point_index < m_points.size() || point_index == in_handle_index ||
                point_index == out_handle_index,
            "Point index out of range.");

  VertexNode node = {0, -1, -1, -1, -1, -1};

  if (empty()) {
    if (!vacant()) {
      node.vertex = 0;
      node.in = in_handle_index;
      node.out = out_handle_index;
    }

    return node;
  }

  if (!closed()) {
    switch (point_index) {
      case in_handle_index:
        node.out = in_handle_index;
        node.vertex = 0;

        if (m_commands_size > 1 && get_command(1) == Command::Cubic) {
          node.in = 1;
          node.in_command = 1;
        } else if (m_commands_size == 1) {
          node.in = out_handle_index;
        }

        return node;
      case out_handle_index:
        node.out = out_handle_index;
        node.vertex = static_cast<uint32_t>(m_points.size()) - 1;

        if (m_commands_size > 1 && get_command(m_commands_size - 1) == Command::Cubic) {
          node.in = static_cast<uint32_t>(m_points.size()) - 2;
          node.in_command = m_commands_size - 1;
        } else if (m_commands_size == 1) {
          node.in = in_handle_index;
        }

        return node;
      default:
        break;
    }
  }

  Iterator it = {*this, point_index, IndexType::Point};
  Segment segment = *it;

  int64_t* in = &node.in;
  int64_t* in_command = &node.in_command;
  int64_t* out = &node.out;
  int64_t* out_command = &node.out_command;

  if (point_index != 0 && !(segment.type == Command::Cubic && it.point_index() >= point_index)) {
    it++;

    if (point_index == static_cast<uint32_t>(m_points.size()) - 1 || it != end()) {
      in = &node.out;
      in_command = &node.out_command;
      out = &node.in;
      out_command = &node.in_command;
    }

    if (it == end()) {
      node.vertex = it.point_index() - 1;
      *out_command = m_commands_size - 1;

      if (segment.type == Command::Cubic) {
        *out = node.vertex - 1;
      }

      if (closed()) {
        *in_command = 1;

        if (get_command(1) == Command::Cubic) {
          *in = 1;
        }

        node.close_vertex = 0;
      } else {
        *in = out_handle_index;
      }

      return node;
    } else {
      segment = *it;
    }
  }

  node.vertex = it.point_index() - 1;
  *out_command = it.command_index();

  if (segment.type == Command::Cubic) {
    *out = node.vertex + 1;
  }

  if (it.segment_index() > 0) {
    Iterator prev_it = it - 1;
    Segment prev_segment = *prev_it;

    *in_command = prev_it.command_index();

    if (prev_segment.type == Command::Cubic) {
      *in = node.vertex - 1;
    }
  } else if (closed()) {
    *in_command = m_commands_size - 1;

    if (get_command(m_commands_size - 1) == Command::Cubic) {
      *in = static_cast<uint32_t>(m_points.size()) - 2;
    }

    node.close_vertex = static_cast<uint32_t>(m_points.size()) - 1;
  } else {
    *in = in_handle_index;
  }

  return node;
}

template<typename T, typename _>
void Path<T, _>::move_to(const math::Vec2<T> point)
{
  GK_ASSERT(empty(), "Cannot add a move to a non-empty path.");

  if (!vacant() && get_command(m_commands_size - 1) == Command::Move) {
    m_points[m_points.size() - 1] = point;
    return;
  }

  m_points.push_back(point);
  m_in_handle = point;
  m_out_handle = point;

  push_command(Command::Move);
}

template<typename T, typename _>
void Path<T, _>::line_to(const math::Vec2<T> point, const bool reverse)
{
  GK_ASSERT(!vacant(), "Cannot add a line to a vacant path.");

  if (reverse) {
    m_points.insert(m_points.begin(), point);
    m_in_handle = point;

    insert_command(Command::Line, 0);
  } else {
    m_points.push_back(point);
    m_out_handle = point;

    push_command(Command::Line);
  }
}

template<typename T, typename _>
void Path<T, _>::quadratic_to(const math::Vec2<T> control,
                              const math::Vec2<T> point,
                              const bool reverse)
{
  GK_ASSERT(!vacant(), "Cannot add a quadratic bezier to a vacant path.");

  if (reverse) {
    m_points.insert(m_points.begin(), {point, control});
    m_in_handle = point;

    insert_command(Command::Quadratic, 0);
  } else {
    m_points.insert(m_points.end(), {control, point});
    m_out_handle = point;

    push_command(Command::Quadratic);
  }
}

template<typename T, typename _>
void Path<T, _>::cubic_to(const math::Vec2<T> control1,
                          const math::Vec2<T> control2,
                          const math::Vec2<T> point,
                          const bool reverse)
{
  GK_ASSERT(!vacant(), "Cannot add a cubic bezier to a vacant path.");

  if (control1 == (reverse ? m_points.front() : m_points.back()) && control2 == point) {
    return line_to(point, reverse);
  }

  if (reverse) {
    m_points.insert(m_points.begin(), {point, control2, control1});
    m_in_handle = point;

    insert_command(Command::Cubic, 0);
  } else {
    m_points.insert(m_points.end(), {control1, control2, point});
    m_out_handle = point;

    push_command(Command::Cubic);
  }
}

template<typename T, typename _>
void Path<T, _>::cubic_to(const math::Vec2<T> control,
                          const math::Vec2<T> point,
                          const bool is_control_1,
                          const bool reverse)
{
  GK_ASSERT(!vacant(), "Cannot add a cubic bezier to a vacant path.");

  if (reverse) {
    if (is_control_1) {
      m_points.insert(m_points.begin(), {point, point, control});
    } else {
      m_points.insert(m_points.begin(), {point, control, m_points.front()});
    }

    m_in_handle = point;

    insert_command(Command::Cubic, 0);
  } else {
    if (is_control_1) {
      m_points.insert(m_points.end(), {control, point, point});
    } else {
      m_points.insert(m_points.end(), {m_points.back(), control, point});
    }

    m_out_handle = point;

    push_command(Command::Cubic);
  }
}

template<typename T, typename _>
void Path<T, _>::arc_to(const math::Vec2<T> center,
                        const math::Vec2<T> radius,
                        const T x_axis_rotation,
                        const bool large_arc_flag,
                        const bool sweep_flag,
                        const math::Vec2<T> point,
                        const bool reverse)
{
  GK_ASSERT(!vacant(), "Cannot add an arc to a vacant path.");

  math::Vec2<T> r = radius;

  const T sin_th = std::sin(math::degrees_to_radians(x_axis_rotation));
  const T cos_th = std::cos(math::degrees_to_radians(x_axis_rotation));

  const math::Vec2<T> d0 = (center - point) / T(2);
  const math::Vec2<T> d1 = {cos_th * d0.x + sin_th * d0.y, -sin_th * d0.x + cos_th * d0.y};

  const math::Vec2<T> sq_r = r * r;
  const math::Vec2<T> sq_p = d1 * d1;

  const T check = sq_p.x / sq_r.x + sq_p.y / sq_r.y;
  if (check > T(1))
    r *= std::sqrt(check);

  math::Mat2<T> a = {cos_th / r.x, sin_th / r.x, -sin_th / r.y, cos_th / r.y};
  math::Vec2<T> p1 = {math::dot(a[0], point), math::dot(a[1], point)};

  const math::Vec2<T> p0 = {math::dot(a[0], center), math::dot(a[1], center)};

  const T d = math::squared_length(p1 - p0);

  T sfactor_sq = T(1) / d - T(0.25);
  if (sfactor_sq < T(0))
    sfactor_sq = T(0);

  T sfactor = std::sqrt(sfactor_sq);
  if (sweep_flag == large_arc_flag)
    sfactor = -sfactor;

  const math::Vec2<T> c1 = {(p0.x + p1.x) / T(2) - sfactor * (p1.y - p0.y),
                            (p0.y + p1.y) / T(2) + sfactor * (p1.x - p0.x)};

  const T th0 = std::atan2(p0.y - c1.y, p0.x - c1.x);
  const T th1 = std::atan2(p1.y - c1.y, p1.x - c1.x);

  T th_arc = th1 - th0;
  if (th_arc < T(0) && sweep_flag)
    th_arc += math::two_pi<T>;
  else if (th_arc > T(0) && !sweep_flag)
    th_arc -= math::two_pi<T>;

  int n_segs = static_cast<int>(
      std::ceil(std::abs(th_arc / (math::pi<T> / T(2) + math::geometric_epsilon<T>))));
  for (int i = 0; i < n_segs; i++) {
    const T th2 = th0 + i * th_arc / n_segs;
    const T th3 = th0 + (i + 1) * th_arc / n_segs;

    a = {cos_th * r.x, -sin_th * r.x, sin_th * r.y, cos_th * r.y};

    const T th_half = (th3 - th2) / T(2);
    const T sin_half_th_half = std::sin(th_half / T(2));
    const T t = (T(8) / T(3)) * sin_half_th_half * sin_half_th_half / std::sin(th_half);

    const T sin_th2 = std::sin(th2);
    const T cos_th2 = std::cos(th2);
    const T sin_th3 = std::sin(th3);
    const T cos_th3 = std::cos(th3);

    p1 = {c1.x + cos_th2 - t * sin_th2, c1.y + sin_th2 + t * cos_th2};

    const math::Vec2<T> p3 = {c1.x + cos_th3, c1.y + sin_th3};
    const math::Vec2<T> p2 = {p3.x + t * sin_th3, p3.y - t * cos_th3};

    const math::Vec2<T> bez1 = {math::dot(a[0], p1), math::dot(a[1], p1)};
    const math::Vec2<T> bez2 = {math::dot(a[0], p2), math::dot(a[1], p2)};
    const math::Vec2<T> bez3 = {math::dot(a[0], p3), math::dot(a[1], p3)};

    cubic_to(bez1, bez2, bez3, reverse);
  }
}

template<typename T, typename _>
void Path<T, _>::ellipse(const math::Vec2<T> center, const math::Vec2<T> radius)
{
  const math::Vec2<T> top_left = center - radius;
  const math::Vec2<T> bottom_right = center + radius;
  const math::Vec2<T> cp = radius * math::bezier_circle_ratio<T>;

  move_to({center.x, top_left.y});
  cubic_to({center.x + cp.x, top_left.y},
           {bottom_right.x, center.y - cp.y},
           {bottom_right.x, center.y});
  cubic_to({bottom_right.x, center.y + cp.y},
           {center.x + cp.x, bottom_right.y},
           {center.x, bottom_right.y});
  cubic_to(
      {center.x - cp.x, bottom_right.y}, {top_left.x, center.y + cp.y}, {top_left.x, center.y});
  cubic_to({top_left.x, center.y - cp.y}, {center.x - cp.x, top_left.y}, {center.x, top_left.y});
  close();
}

template<typename T, typename _>
void Path<T, _>::circle(const math::Vec2<T> center, const T radius)
{
  ellipse(center, {radius, radius});
}

template<typename T, typename _>
void Path<T, _>::rect(const math::Vec2<T> point, const math::Vec2<T> size, const bool centered)
{
  math::Vec2<T> p = point;

  if (centered) {
    p -= size / T(2);
  }

  move_to(p);
  line_to(p + math::Vec2<T>{size.x, T(0)});
  line_to(p + size);
  line_to(p + math::Vec2<T>{T(0), size.y});
  close();
}

template<typename T, typename _>
void Path<T, _>::round_rect(const math::Vec2<T> point,
                            const math::Vec2<T> size,
                            const T radius,
                            const bool centered)
{
  math::Vec2<T> p = point;
  T r = radius;

  if (centered) {
    p -= size / T(2);
  }

  if (r > size.x / T(2))
    r = size.x / T(2);
  if (r > size.y / T(2))
    r = size.y / T(2);

  move_to({p.x + r, p.y});
  line_to({p.x + size.x - r, p.y});
  cubic_to({p.x + size.x - r * math::bezier_circle_ratio<T>, p.y},
           {p.x + size.x, p.y + r * math::bezier_circle_ratio<T>},
           {p.x + size.x, p.y + r});
  line_to({p.x + size.x, p.y + size.y - r});
  cubic_to({p.x + size.x, p.y + size.y - r * math::bezier_circle_ratio<T>},
           {p.x + size.x - r * math::bezier_circle_ratio<T>, p.y + size.y},
           {p.x + size.x - r, p.y + size.y});
  line_to({p.x + r, p.y + size.y});
  cubic_to({p.x + r * math::bezier_circle_ratio<T>, p.y + size.y},
           {p.x, p.y + size.y - r * math::bezier_circle_ratio<T>},
           {p.x, p.y + size.y - r});
  line_to({p.x, p.y + r});
  cubic_to({p.x, p.y + r * math::bezier_circle_ratio<T>},
           {p.x + r * math::bezier_circle_ratio<T>, p.y},
           {p.x + r, p.y});
  close();
}

template<typename T, typename _>
void Path<T, _>::close()
{
  if (empty() || m_commands.empty() || (size() == 1 && get_command(1) == Command::Line))
    return;

  const math::Vec2<T> p = m_points.front();

  if (math::is_almost_equal(m_points.back(), p, math::geometric_epsilon<T>)) {
    m_points[m_points.size() - 1] = p;
  } else {
    const bool has_in = has_in_handle();
    const bool has_out = has_out_handle();

    if (!has_in && !has_out) {
      line_to(p);
    } else {
      cubic_to(m_out_handle, m_in_handle, p);
    }
  }

  m_closed = true;
}

template<typename T, typename _>
uint32_t Path<T, _>::to_line(const uint32_t command_index, uint32_t reference_point)
{
  GK_ASSERT(command_index < m_commands_size, "Command index out of range.");

  const Command command = get_command(command_index);
  if (command == Command::Line || command == Command::Move)
    return reference_point;

  const Iterator it = {*this, command_index, IndexType::Command};
  const Segment segment = *it;
  const uint32_t point_i = it.point_index();

  if (segment.type == Command::Cubic) {
    m_points.erase(m_points.begin() + point_i, m_points.begin() + point_i + 2);

    replace_command(command_index, Command::Line);

    return reference_point > point_i ? reference_point - 2 : reference_point;
  }

  m_points.erase(m_points.begin() + point_i, m_points.begin() + point_i + 1);

  replace_command(command_index, Command::Line);

  return reference_point > point_i ? reference_point - 1 : reference_point;
}

template<typename T, typename _>
uint32_t Path<T, _>::to_quadratic(const uint32_t command_index, uint32_t reference_point)
{
  GK_ASSERT(command_index < m_commands_size, "Command index out of range.");

  const Command command = get_command(command_index);
  if (command != Command::Line)
    return reference_point;

  const Iterator it = {*this, command_index, IndexType::Command};
  const Segment segment = *it;
  const uint32_t point_i = it.point_index();

  m_points.insert(m_points.begin() + point_i, (m_points[point_i - 1] + m_points[point_i]) / T(2));

  replace_command(command_index, Command::Quadratic);

  return reference_point >= point_i ? reference_point + 1 : reference_point;
}

template<typename T, typename _>
uint32_t Path<T, _>::to_cubic(const uint32_t command_index, uint32_t reference_point)
{
  GK_ASSERT(command_index < m_commands_size, "Command index out of range.");

  const Command command = get_command(command_index);
  if (command == Command::Cubic || command == Command::Move)
    return reference_point;

  const Iterator it = {*this, command_index, IndexType::Command};
  const Segment segment = *it;
  const uint32_t point_i = it.point_index();

  if (segment.type == Command::Line) {
    m_points.insert(m_points.begin() + point_i, {m_points[point_i - 1], m_points[point_i]});

    replace_command(command_index, Command::Cubic);

    return reference_point >= point_i ? reference_point + 2 : reference_point;
  }

  const math::Vec2<T> p0 = m_points[point_i - 1];
  const math::Vec2<T> p1 = m_points[point_i];
  const math::Vec2<T> p2 = m_points[point_i + 1];

  const math::Vec2<T> bez1 = p0 + T(2) / T(3) * (p1 - p0);
  const math::Vec2<T> bez2 = p2 + T(2) / T(3) * (p1 - p2);

  m_points[point_i] = bez1;
  m_points.insert(m_points.begin() + point_i + 1, bez2);

  replace_command(command_index, Command::Cubic);

  return reference_point >= point_i + 1 ? reference_point + 1 : reference_point;
}

template<typename T, typename _>
void Path<T, _>::remove(const uint32_t point_index, const bool keep_shape)
{
  GK_ASSERT(point_index < m_points.size(), "Point index out of range.");

  uint32_t to_remove = point_index == m_points.size() - 1 ? 0 : point_index;

  if (empty() || (point_index == 0 && !closed())) {
    return;
  }

  const Iterator it = {*this, to_remove, IndexType::Point};
  const Iterator next_it = it + 1;
  const Segment segment = to_remove == 0 ? back() : *it;
  const Segment next_segment = to_remove == 0 ? front() : *next_it;

  if (size() == 2 && closed()) {
    const math::Vec2<T> p = segment.p0;
    const math::Vec2<T> out = segment.type == Command::Cubic ? segment.p1 : p;
    const math::Vec2<T> in = next_segment.type == Command::Cubic ? next_segment.p2 : p;

    m_points.clear();
    m_commands.clear();
    m_commands_size = 0;

    move_to(p);

    m_in_handle = in;
    m_out_handle = out;

    return;
  }

  CubicBezier<T> cubic;

  if (keep_shape) {
    std::vector<math::Vec2<T>> points(FIT_RESOLUTION * 2 + 2);

    for (int i = 0; i <= FIT_RESOLUTION; i++) {
      T t = static_cast<T>(i) / T(FIT_RESOLUTION);

      points[i] = segment.sample(t);
      points[FIT_RESOLUTION + i + 1] = next_segment.sample(t);
    }

    cubic = algorithms::fit_points_to_cubic(points, math::geometric_epsilon<T>);
  } else {
    const math::Vec2<T> p1 = segment.type == Command::Line ? segment.p0 : segment.p1;

    if (next_segment.type == Command::Line) {
      cubic = {segment.p0, p1, next_segment.p1, next_segment.p1};
    } else if (next_segment.type == Command::Quadratic) {
      cubic = {segment.p0, p1, next_segment.p1, next_segment.p2};
    } else {
      cubic = {segment.p0, p1, next_segment.p2, next_segment.p3};
    }
  }

  uint32_t new_command_index = 0;

  if (to_remove == 0) {
    switch (segment.type) {
      case Command::Line:
        m_points.pop_back();
        break;
      case Command::Quadratic:
        m_points.pop_back();
        m_points.pop_back();
        break;
      case Command::Cubic:
        m_points.pop_back();
        m_points.pop_back();
        m_points.pop_back();
        break;
      case Command::Move:
      default:
        break;
    }

    switch (next_segment.type) {
      case Command::Line:
        m_points.insert(m_points.begin(), {cubic.p0, cubic.p1});
        m_points[2] = cubic.p2;
        break;
      case Command::Quadratic:
        m_points.insert(m_points.begin(), cubic.p0);
        m_points[1] = cubic.p1;
        m_points[2] = cubic.p2;
        break;
      case Command::Cubic:
        m_points[0] = cubic.p0;
        m_points[1] = cubic.p1;
        m_points[2] = cubic.p2;
        break;
      case Command::Move:
      default:
        break;
    }

    new_command_index = 1;

    remove_command(m_commands_size - 1);
    replace_command(new_command_index, Command::Cubic);
  } else {
    switch (segment.type) {
      case Command::Line:
        m_points[to_remove] = cubic.p3;
        m_points.insert(m_points.begin() + to_remove, {cubic.p1, cubic.p2});
        break;
      case Command::Quadratic:
        m_points[to_remove - 1] = cubic.p1;
        m_points[to_remove] = cubic.p3;
        m_points.insert(m_points.begin() + to_remove, cubic.p2);
        break;
      case Command::Cubic:
        m_points[to_remove - 2] = cubic.p1;
        m_points[to_remove - 1] = cubic.p2;
        m_points[to_remove] = cubic.p3;
        break;
      case Command::Move:
      default:
        break;
    }

    switch (next_segment.type) {
      case Command::Line:
        m_points.erase(m_points.begin() + to_remove + 1);
        break;
      case Command::Quadratic:
        m_points.erase(m_points.begin() + to_remove + 1, m_points.begin() + to_remove + 3);
        break;
      case Command::Cubic:
        m_points.erase(m_points.begin() + to_remove + 1, m_points.begin() + to_remove + 4);
        break;
      case Command::Move:
      default:
        break;
    }

    new_command_index = it.command_index();

    replace_command(new_command_index, Command::Cubic);
    remove_command(next_it.command_index());
  }

  if (cubic.p0 == cubic.p1 && cubic.p2 == cubic.p3) {
    to_line(new_command_index);
  }
}

template<typename T, typename _>
uint32_t Path<T, _>::split(const uint32_t segment_index, const T t)
{
  GK_ASSERT(segment_index < m_commands_size - 1, "Segment index out of range.");

  if (empty())
    return 0;

  const Iterator it = {*this, segment_index, IndexType::Segment};
  const Segment segment = *it;
  const uint32_t point_i = it.point_index();

  switch (segment.type) {
    case Command::Line: {
      const math::Vec2<T> p = math::lerp(segment.p0, segment.p1, t);

      m_points.insert(m_points.begin() + point_i, p);
      insert_command(Command::Line, segment_index + 1);

      return point_i;
    }
    case Command::Quadratic: {
      const auto& [left, right] = geom::split(
          QuadraticBezier<T>{m_points[point_i - 1], m_points[point_i], m_points[point_i + 1]}, t);

      m_points[point_i] = left.p1;

      m_points.insert(m_points.begin() + point_i, {left.p2, right.p1});
      insert_command(Command::Quadratic, segment_index + 1);

      return point_i + 1;
    }
    case Command::Cubic: {
      const auto& [left, right] = geom::split(CubicBezier<T>{m_points[point_i - 1],
                                                             m_points[point_i],
                                                             m_points[point_i + 1],
                                                             m_points[point_i + 2]},
                                              t);

      m_points[point_i] = left.p1;
      m_points[point_i + 1] = right.p2;

      m_points.insert(m_points.begin() + point_i + 1, {left.p2, left.p3, right.p1});
      insert_command(Command::Cubic, segment_index + 1);

      return point_i + 2;
    }
    case Command::Move:
    default:
      break;
  };

  return 0;
}

template<typename T, typename _>
math::Rect<T> Path<T, _>::bounding_rect() const
{
  if (empty()) {
    if (vacant())
      return {};
    return {m_points[0], m_points[0]};
  }

  math::Rect<T> rect{};

  for (uint32_t i = 0, j = 0; i < m_commands_size; i++) {
    switch (get_command(i)) {
      case Command::Cubic: {
        GK_ASSERT(j > 0, "Cubic bezier command cannot be the first command of a path.");
        GK_ASSERT(j + 2 < m_points.size(), "Not enough points for a cubic bezier.");

        const CubicBezier<T> cubic = {
            m_points[j - 1], m_points[j], m_points[j + 1], m_points[j + 2]};

        rect = math::Rect<T>::from_rects(rect, geom::bounding_rect(cubic));

        j += 3;

        break;
      }
      case Command::Quadratic: {
        GK_ASSERT(j > 0, "Quadratic bezier command cannot be the first command of a path.");
        GK_ASSERT(j + 1 < m_points.size(), "Not enough points for a quadratic bezier.");

        const QuadraticBezier<T> quadratic = {m_points[j - 1], m_points[j], m_points[j + 1]};

        rect = math::Rect<T>::from_rects(rect, geom::bounding_rect(quadratic));

        j += 2;

        break;
      }
      case Command::Line: {
        GK_ASSERT(j > 0, "Line command cannot be the first command of a path.");
        GK_ASSERT(j < m_points.size(), "Not enough points for a line.");

        rect = math::Rect<T>::from_rects(rect, math::Rect<T>{m_points[j - 1], m_points[j]});

        j += 1;

        break;
      }
      case Command::Move: {
        GK_ASSERT(j < m_points.size(), "Points vector subscript out of range.");

        rect = math::Rect<T>::from_rects(rect, math::Rect<T>{m_points[j], m_points[j]});

        j += 1;

        break;
      }
    }
  }

  return rect;
}

template<typename T, typename _>
math::Rect<T> Path<T, _>::bounding_rect(const math::Mat2x3<T>& transform) const
{
  if (empty()) {
    if (vacant())
      return {};

    const math::Vec2<T> p = transform * m_points[0];
    return {p, p};
  }

  math::Rect<T> rect{};

  for (uint32_t i = 0, j = 0; i < m_commands_size; i++) {
    switch (get_command(i)) {
      case Command::Cubic: {
        GK_ASSERT(j > 0, "Cubic bezier command cannot be the first command of a path.");
        GK_ASSERT(j + 2 < m_points.size(), "Not enough points for a cubic bezier.");

        const CubicBezier<T> cubic = {transform * m_points[j - 1],
                                      transform * m_points[j],
                                      transform * m_points[j + 1],
                                      transform * m_points[j + 2]};

        rect = math::Rect<T>::from_rects(rect, geom::bounding_rect(cubic));

        j += 3;

        break;
      }
      case Command::Quadratic: {
        GK_ASSERT(j > 0, "Quadratic bezier command cannot be the first command of a path.");
        GK_ASSERT(j + 1 < m_points.size(), "Not enough points for a quadratic bezier.");

        const QuadraticBezier<T> quadratic = {
            transform * m_points[j - 1], transform * m_points[j], transform * m_points[j + 1]};

        rect = math::Rect<T>::from_rects(rect, geom::bounding_rect(quadratic));

        j += 2;

        break;
      }
      case Command::Line: {
        GK_ASSERT(j > 0, "Line command cannot be the first command of a path.");
        GK_ASSERT(j < m_points.size(), "Not enough points for a line.");

        const math::Vec2<T> p1 = transform * m_points[j];

        rect = math::Rect<T>::from_rects(rect, math::Rect<T>{p1, p1});

        j += 1;

        break;
      }
      case Command::Move: {
        GK_ASSERT(j < m_points.size(), "Points vector subscript out of range.");

        const math::Vec2<T> p0 = transform * m_points[j];

        rect = math::Rect<T>::from_rects(rect, math::Rect<T>{p0, p0});

        j += 1;

        break;
      }
    }
  }

  return rect;
}

template<typename T, typename _>
math::Rect<T> Path<T, _>::approx_bounding_rect() const
{
  if (empty()) {
    if (vacant())
      return {};
    return {m_points[0], m_points[0]};
  }

  math::Rect<T> rect{};

  for (math::Vec2<T> p : m_points) {
    math::min(rect.min, p, rect.min);
    math::max(rect.max, p, rect.max);
  }

  math::min(rect.min, m_in_handle, rect.min);
  math::max(rect.max, m_in_handle, rect.max);
  math::min(rect.min, m_out_handle, rect.min);
  math::max(rect.max, m_out_handle, rect.max);

  return rect;
}

template<typename T, typename _>
bool Path<T, _>::is_point_inside_path(const math::Vec2<T> point,
                                      const FillingOptions* fill,
                                      const StrokingOptions<T>* stroke,
                                      const math::Mat2x3<T>& transform,
                                      const T threshold,
                                      const bool deep_search) const
{
  if (empty()) {
    if (vacant()) {
      return false;
    }

    return (is_point_in_circle(point, transform * m_points[0], threshold) ||
            (deep_search && (is_point_in_circle(point, transform * m_in_handle, threshold) ||
                             is_point_in_circle(point, transform * m_out_handle, threshold))));
  }

  if (deep_search) {
    /* In case of deep search, it wasn't possible to check whether the point is inside the
     * bounding_box previously, because the cached bounding box doesn't include control points. */

    const math::Rect<T> bounding_rect = math::Rect<T>::expand(
        transform * approx_bounding_rect(),
        stroke ? (stroke->width / T(2) *
                  (stroke->join == LineJoin::Miter ? std::max(T(1), stroke->miter_limit) : T(1))) :
                 T(0));

    if (!is_point_in_rect(point, bounding_rect, threshold)) {
      return false;
    }
  }

  if (fill) {
    CubicPath<T> path = to_cubic_path();

    if (!closed()) {
      path.line_to(path.back());
    }

    const int winding = path.winding_of(math::inverse(transform) * point);

    if ((fill->rule == FillRule::NonZero && winding != 0) ||
        (fill->rule == FillRule::EvenOdd && winding % 2 != 0))
    {
      return true;
    }
  }

  if (stroke == nullptr) {
    return false;
  }

  if (math::is_identity(transform)) {
    return is_point_inside_stroke(point, stroke, threshold, deep_search);
  }

  return transformed<T>(transform).is_point_inside_stroke(point, stroke, threshold, deep_search);
}

template<typename T, typename _>
bool Path<T, _>::is_point_inside_segment(const uint32_t segment_index,
                                         const math::Vec2<T> point,
                                         const StrokingOptions<T>& stroke,
                                         const math::Mat2x3<T>& transform,
                                         const T threshold) const
{
  const Segment segment = segment_at(segment_index);

  Path<T> path;

  path.move_to(segment.p0);

  switch (segment.type) {
    case Command::Line: {
      path.line_to(segment.to_line());
      break;
    }
    case Command::Quadratic: {
      path.quadratic_to(segment.to_quadratic());
      break;
    }
    case Command::Cubic: {
      path.cubic_to(segment.to_cubic());
      break;
    }
    default:
    case Command::Move:
      break;
  }

  return path.is_point_inside_path(point, nullptr, &stroke, transform, threshold, false);
}

template<typename T, typename _>
bool Path<T, _>::is_point_inside_point(const uint32_t point_index,
                                       const math::Vec2<T> point,
                                       const math::Mat2x3<T>& transform,
                                       const T threshold) const
{
  const math::Vec2<T> p = transform * at(point_index);

  if (is_point_in_circle(point, p, threshold)) {
    if (point_index == 0)
      return true;

    for (uint32_t i = 0, point_i = 0; i < m_commands_size; i++) {
      switch (get_command(i)) {
        case Command::Move:
        case Command::Line:
          if (point_i == point_index) {
            return true;
          }

          point_i += 1;
          break;
        case Command::Quadratic:
          if (point_i == point_index) {
            if (m_points[point_i] == m_points[point_i - 1] ||
                m_points[point_i] == m_points[point_i + 1])
            {
              return false;
            }

            return true;
          } else if (point_i + 1 == point_index) {
            return true;
          }

          point_i += 2;
          break;
        case Command::Cubic:
          if (point_i == point_index) {
            if (m_points[point_i] == m_points[point_i - 1] ||
                m_points[point_i] == m_points[point_i + 2])
            {
              return false;
            }

            return true;
          } else if (point_i + 1 == point_index) {
            if (m_points[point_i + 1] == m_points[point_i - 1] ||
                m_points[point_i + 1] == m_points[point_i + 2])
            {
              return false;
            }

            return true;
          } else if (point_i + 2 == point_index) {
            return true;
          }

          point_i += 3;
          break;
      }
    }

    if (point_index == in_handle_index && !has_in_handle())
      return false;
    if (point_index == out_handle_index && !has_out_handle())
      return false;

    return true;
  }

  return false;
}

template<typename T, typename _>
bool Path<T, _>::intersects(const math::Rect<T>& rect, std::vector<uint32_t>* indices) const
{
  if (m_commands_size == 0) {
    return false;
  } else if (m_commands_size == 1) {
    if (is_point_in_rect(m_points[0], rect)) {
      if (indices) {
        indices->push_back(0);
      }

      return true;
    }

    return false;
  }

  if (!does_rect_intersect_rect(rect, approx_bounding_rect())) {
    return false;
  }

  if (indices) {
    indices->reserve(points_count(false));
  }

  bool found = false;

  for (uint32_t i = 0, point_i = 0; i < m_commands_size; i++) {
    switch (get_command(i)) {
      case Command::Move:
        if (is_point_in_rect(m_points[point_i], rect)) {
          if (indices) {
            indices->push_back(point_i);
          }

          found = true;
        }

        point_i += 1;

        break;
      case Command::Line:
        if (is_point_in_rect(m_points[point_i], rect)) {
          if (indices) {
            indices->push_back(point_i);
          }

          found = true;
        } else if (!found &&
                   does_line_intersect_rect({m_points[point_i - 1], m_points[point_i]}, rect))
        {
          found = true;
        }

        point_i += 1;

        break;
      case Command::Quadratic:
        if (is_point_in_rect(m_points[point_i + 1], rect)) {
          if (indices) {
            indices->push_back(point_i + 1);
          }

          found = true;
        } else if (!found &&
                   does_quadratic_intersect_rect(
                       {m_points[point_i - 1], m_points[point_i], m_points[point_i + 1]}, rect))
        {
          found = true;
        }

        point_i += 2;

        break;
      case Command::Cubic:
        if (is_point_in_rect(m_points[point_i + 2], rect)) {
          if (indices) {
            indices->push_back(point_i + 2);
          }

          found = true;
        } else if (!found && does_cubic_intersect_rect({m_points[point_i - 1],
                                                        m_points[point_i],
                                                        m_points[point_i + 1],
                                                        m_points[point_i + 2]},
                                                       rect))
        {
          found = true;
        }

        point_i += 3;

        break;
    }
  }

  if (indices && closed() && !indices->empty() && indices->back() == m_points.size() - 1) {
    indices->pop_back();
  }

  return found;
}

template<typename T, typename _>
bool Path<T, _>::intersects(const math::Rect<T>& rect,
                            const math::Mat2x3<T>& transform,
                            std::vector<uint32_t>* indices) const
{
  if (m_commands_size == 0) {
    return false;
  } else if (m_commands_size == 1) {
    if (is_point_in_rect(transform * m_points[0], rect)) {
      if (indices) {
        indices->push_back(0);
      }

      return true;
    }

    return false;
  }

  math::Vec2<T> last = math::Vec2<T>::zero();

  // if (!does_rect_intersect_rect(rect, transform * approx_bounding_rect())) {
  //   return false;
  // }

  if (indices) {
    indices->reserve(points_count(false));
  }

  bool found = false;

  for (uint32_t i = 0, point_i = 0; i < m_commands_size; i++) {
    switch (get_command(i)) {
      case Command::Move: {
        const math::Vec2<T> p0 = transform * m_points[point_i];

        if (is_point_in_rect(p0, rect)) {
          if (indices) {
            indices->push_back(point_i);
          }

          found = true;
        }

        point_i += 1;
        last = p0;

        break;
      }
      case Command::Line: {
        const math::Vec2<T> p1 = transform * m_points[point_i];

        if (is_point_in_rect(p1, rect)) {
          if (indices) {
            indices->push_back(point_i);
          }

          found = true;
        } else if (!found && does_line_intersect_rect({last, p1}, rect)) {
          found = true;
        }

        point_i += 1;
        last = p1;

        break;
      }
      case Command::Quadratic: {
        const math::Vec2<T> p2 = transform * m_points[point_i + 1];

        if (is_point_in_rect(p2, rect)) {
          if (indices) {
            indices->push_back(point_i + 1);
          }

          found = true;
        } else if (!found &&
                   does_quadratic_intersect_rect({last, transform * m_points[point_i], p2}, rect))
        {
          found = true;
        }

        point_i += 2;
        last = p2;

        break;
      }
      case Command::Cubic: {
        const math::Vec2<T> p1 = transform * m_points[point_i];
        const math::Vec2<T> p2 = transform * m_points[point_i + 1];
        const math::Vec2<T> p3 = transform * m_points[point_i + 2];

        if (is_point_in_rect(p3, rect)) {
          if (indices)
            indices->push_back(point_i + 2);
          found = true;
        } else if (!found && does_cubic_intersect_rect({last, p1, p2, p3}, rect)) {
          found = true;
        }

        point_i += 3;
        last = p3;

        break;
      }
    }
  }

  if (indices && closed() && !indices->empty() && indices->back() == m_points.size() - 1) {
    indices->pop_back();
  }

  return found;
}

template<typename T, typename _>
QuadraticPath<T> Path<T, _>::to_quadratic_path(const T tolerance) const
{
  QuadraticPath<T> path;

  if (empty()) {
    return path;
  }

  for (uint32_t i = 0, j = 0; i < m_commands_size; i++) {
    switch (get_command(i)) {
      case Command::Move:
        path.move_to(m_points[j]);
        j += 1;
        break;
      case Command::Line:
        path.line_to(m_points[j]);
        j += 1;
        break;
      case Command::Quadratic:
        path.quadratic_to(m_points[j], m_points[j + 1]);
        j += 2;
        break;
      case Command::Cubic: {
        cubic_to_quadratics(
            {m_points[j - 1], m_points[j], m_points[j + 1], m_points[j + 2]}, tolerance, path);

        j += 3;
        break;
      }
    }
  }

  return path;
}

template<typename T, typename _>
CubicPath<T> Path<T, _>::to_cubic_path() const
{
  CubicPath<T> path;

  if (empty()) {
    return path;
  }

  // We reserve twice the points to account for monotonic splitting.
  path.points.reserve((m_commands_size - 1) * 3 * 2 + 1);

  for (uint32_t i = 0, j = 0; i < m_commands_size; i++) {
    switch (get_command(i)) {
      case Command::Move:
        path.move_to(m_points[j]);
        j += 1;
        break;
      case Command::Line:
        path.line_to(m_points[j]);
        j += 1;
        break;
      case Command::Quadratic:
        path.quadratic_to(m_points[j], m_points[j + 1]);
        j += 2;
        break;
      case Command::Cubic:
        path.cubic_to(m_points[j], m_points[j + 1], m_points[j + 2]);
        j += 3;
        break;
    }
  }

  return path;
}

template<typename T, typename _>
CubicMultipath<T> Path<T, _>::to_cubic_multipath() const
{
  CubicMultipath<T> path;

  if (empty()) {
    return path;
  }

  // We reserve twice the points to account for monotonic splitting.
  path.points.reserve((m_commands_size - 1) * 3 * 2 + 1);

  for (uint32_t i = 0, j = 0; i < m_commands_size; i++) {
    switch (get_command(i)) {
      case Command::Move:
        path.move_to(m_points[j]);
        j += 1;
        break;
      case Command::Line:
        path.line_to(m_points[j]);
        j += 1;
        break;
      case Command::Quadratic:
        path.quadratic_to(m_points[j], m_points[j + 1]);
        j += 2;
        break;
      case Command::Cubic:
        path.cubic_to(m_points[j], m_points[j + 1], m_points[j + 2]);
        j += 3;
        break;
    }
  }

  return path;
}

template<typename T, typename _>
template<typename U>
Path<U> Path<T, _>::transformed(const math::Mat2x3<T>& transform, bool* r_has_transform) const
{
  math::Mat2x3<U> mat = math::Mat2x3<U>(transform);
  Path<U> path;

  if (empty()) {
    if (!vacant()) {
      path.move_to(mat * math::Vec2<U>(m_points[0]));
    }

    return path;
  }

  const bool has_transform = !math::is_identity(transform);

  if (r_has_transform) {
    *r_has_transform = has_transform;
  }

  if (!has_transform) {
    return Path<U>(*this);
  }

  path.m_commands = m_commands;
  path.m_commands_size = m_commands_size;
  path.m_closed = m_closed;

  path.m_points = std::vector<math::Vec2<U>>(m_points.size());

  for (uint32_t i = 0; i < m_points.size(); i++) {
    math::Vec2<U> p = mat * math::Vec2<U>(m_points[i]);
    path.m_points[i] = p;
  }

  path.m_in_handle = mat * math::Vec2<U>(m_in_handle);
  path.m_out_handle = mat * math::Vec2<U>(m_out_handle);

  return path;
}

template<typename T, typename _>
io::EncodedData& Path<T, _>::encode(io::EncodedData& data) const
{
  if (vacant()) {
    /* The bitset, in particular first bit is set to 0 if the path is vacant. */
    return data.uint8(0);
  }

  bool is_closed = closed();
  bool has_in = has_in_handle();
  bool has_out = has_out_handle();

  data.bitfield({true, is_closed, has_in, has_out});
  data.vector(m_commands);
  data.vector(m_points);

  if (has_in) {
    data.vec2(vec2(m_in_handle));
  }

  if (has_out) {
    data.vec2(vec2(m_out_handle));
  }

  return data;
}

template<typename T, typename _>
void Path<T, _>::push_command(const Command command)
{
  uint32_t rem = m_commands_size % 4;

  if (rem == 0) {
    m_commands.push_back(static_cast<uint8_t>(command) << 6);
  } else {
    m_commands[m_commands_size / 4] |= static_cast<uint8_t>(command) << (6 - rem * 2);
  }

  m_commands_size += 1;
}

template<typename T, typename _>
void Path<T, _>::insert_command(const Command command, const uint32_t index)
{
  if (index >= m_commands_size) {
    return push_command(command);
  } else if (index == 0) {
    std::vector<Command> commands(m_commands_size + 1);

    for (uint32_t i = 0; i < m_commands_size; i++) {
      commands[i + 1] = get_command(i);
    }

    commands[0] = Command::Move;
    commands[1] = command;

    m_commands.clear();
    m_commands_size = 0;

    for (uint32_t i = 0; i < commands.size(); i++) {
      push_command(commands[i]);
    }

    return;
  }

  std::vector<Command> commands_before(index + 1);
  std::vector<Command> commands_after;

  for (uint32_t i = 0; i < index; i++) {
    commands_before[i] = get_command(i);
  }

  commands_before[index] = command;

  for (uint32_t i = index; i < m_commands_size; i++) {
    commands_after.push_back(get_command(i));
  }

  m_commands.clear();
  m_commands_size = 0;

  for (uint32_t i = 0; i < commands_before.size(); i++) {
    push_command(commands_before[i]);
  }

  for (uint32_t i = 0; i < commands_after.size(); i++) {
    push_command(commands_after[i]);
  }
}

template<typename T, typename _>
void Path<T, _>::replace_command(const uint32_t index, const Command command)
{
  GK_ASSERT(index < m_commands_size, "Command index out of range.");

  uint32_t rem = index % 4;

  m_commands[index / 4] &= ~(0b00000011 << (6 - rem * 2));
  m_commands[index / 4] |= static_cast<uint8_t>(command) << (6 - rem * 2);
}

template<typename T, typename _>
void Path<T, _>::remove_command(const uint32_t index)
{
  GK_ASSERT(index < m_commands_size, "Command index out of range.");

  if (index == m_commands_size - 1) {
    uint32_t rem = (m_commands_size - 1) % 4;

    if (rem == 0) {
      m_commands.pop_back();
    } else {
      m_commands[(m_commands_size - 1) / 4] &= ~(0b00000011 << (6 - rem * 2));
    }

    m_commands_size -= 1;

    return;
  } else if (index == 0) {
    std::vector<Command> commands(m_commands_size - 1);

    for (uint32_t i = 0; i < m_commands_size - 1; i++) {
      commands[i] = get_command(i + 1);
    }

    commands[0] = Command::Move;

    m_commands.clear();
    m_commands_size = 0;

    for (uint32_t i = 0; i < commands.size(); i++) {
      push_command(commands[i]);
    }

    return;
  }

  std::vector<Command> commands(m_commands_size - 1);

  for (uint32_t i = 0; i < index; i++) {
    commands[i] = get_command(i);
  }

  for (uint32_t i = index + 1; i < m_commands_size; i++) {
    commands[i - 1] = get_command(i);
  }

  m_commands.clear();
  m_commands_size = 0;

  for (uint32_t i = 0; i < commands.size(); i++) {
    push_command(commands[i]);
  }
}

template<typename T, typename _>
bool Path<T, _>::is_point_inside_stroke(const math::Vec2<T> point,
                                        const StrokingOptions<T>* stroke,
                                        const T threshold,
                                        const bool deep_search) const
{
  for (uint32_t point_index = 0; point_index < m_points.size(); point_index++) {
    if (!is_point_in_circle(point, m_points[point_index], threshold)) {
      continue;
    }

    if (deep_search || point_index == 0) {
      return true;
    }

    for (uint32_t i = 0, point_i = 0; i < m_commands_size; i++) {
      switch (get_command(i)) {
        case Command::Move:
        case Command::Line:
          if (point_i == point_index) {
            return true;
          }

          point_i += 1;
          break;
        case Command::Quadratic:
          if (point_i + 1 == point_index) {
            return true;
          }

          point_i += 2;
          break;
        case Command::Cubic:
          if (point_i + 2 == point_index) {
            return true;
          }

          point_i += 3;
          break;
      }
    }

    return false;
  }

  if (deep_search && (is_point_in_circle(point, m_in_handle, threshold) ||
                      is_point_in_circle(point, m_out_handle, threshold)))
  {
    return true;
  }

  const StrokingOptions<T> stroking_options = {stroke->tolerance,
                                               stroke->width + T(2) * threshold,
                                               stroke->miter_limit,
                                               stroke->cap,
                                               stroke->join};

  const math::Rect<T> point_threshold = {point - threshold, point + threshold};
  const PathBuilder<T> builder = PathBuilder<T>(*this, math::Rect<T>{});
  const StrokeOutline<T> stroke_path = builder.stroke(stroking_options, &point_threshold);

  return stroke_path.path.winding_of(point) != 0;
}

/* -- Template Instantiation -- */

template class Path<float>;
template class Path<double>;

template Path<float>::Path(const Path<double>&);
template Path<double>::Path(const Path<float>&);

template Path<float> Path<float>::transformed(const math::Mat2x3<float>&, bool*) const;
template Path<double> Path<float>::transformed(const math::Mat2x3<float>&, bool*) const;
template Path<float> Path<double>::transformed(const math::Mat2x3<double>&, bool*) const;
template Path<double> Path<double>::transformed(const math::Mat2x3<double>&, bool*) const;

}  // namespace graphick::geom
