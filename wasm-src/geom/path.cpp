/**
 * @file geom/path.cpp
 * @brief Implementation of the Path class.
 *
 * @todo implement compound paths when groups are a thing.
 */

#include "path.h"

#include "../math/vector.h"
#include "../math/mat2x3.h"

#include "../io/encode/encode.h"

#include "../utils/assert.h"

namespace graphick::geom {

  /* -- Segment -- */

  template <typename T, typename _>
  bool Path<T, _>::Segment::is_point() const {
    bool point = math::is_almost_equal(p0, p1);

    if (point) {
      if (is_quadratic()) return math::is_almost_equal(p1, p2);
      if (is_cubic()) return math::is_almost_equal(p1, p2) && math::is_almost_equal(p2, p3);
    }

    return point;
  }

  /* -- Iterator -- */

  template <typename T, typename _>
  Path<T, _>::Iterator::Iterator(const Path<T>& path, const uint32_t index, const IndexType index_type) :
    m_path(path), m_index(index), m_point_index(0)
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


  template <typename T, typename _>
  Path<T, _>::Iterator& Path<T, _>::Iterator::operator++() {
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

    if (m_index < m_path.m_commands_size && m_path.get_command(m_index) == Command::Move) operator++();

    return *this;
  }

  template <typename T, typename _>
  Path<T, _>::Iterator Path<T, _>::Iterator::operator++(int) {
    Iterator tmp = *this;
    ++(*this);

    return tmp;
  }

  template <typename T, typename _>
  Path<T, _>::Iterator Path<T, _>::Iterator::operator+(const uint32_t n) const {
    Iterator tmp = *this;

    for (uint32_t i = 0; i < n; i++) {
      ++tmp;
    }

    return tmp;
  }

  template <typename T, typename _>
  Path<T, _>::Iterator& Path<T, _>::Iterator::operator--() {
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

  template <typename T, typename _>
  Path<T, _>::Iterator Path<T, _>::Iterator::operator--(int) {
    Iterator tmp = *this;
    --(*this);

    return tmp;
  }

  template <typename T, typename _>
  Path<T, _>::Iterator Path<T, _>::Iterator::operator-(const uint32_t n) const {
    Iterator tmp = *this;

    for (uint32_t i = 0; i < n; i++) {
      --tmp;
    }

    return tmp;
  }

  template <typename T, typename _>
  Path<T, _>::Iterator::value_type Path<T, _>::Iterator::operator*() const {
    const Command command = m_path.get_command(m_index);

    switch (command) {
    case Command::Cubic: {
      GK_ASSERT(m_point_index > 0 && m_point_index + 2 < m_path.m_points.size(), "Not enough points for a cubic bezier.");
      return { m_path.m_points[m_point_index - 1], m_path.m_points[m_point_index], m_path.m_points[m_point_index + 1], m_path.m_points[m_point_index + 2] };
    }
    case Command::Quadratic: {
      GK_ASSERT(m_point_index > 0 && m_point_index + 1 < m_path.m_points.size(), "Not enough points for a quadratic bezier.");
      return { m_path.m_points[m_point_index - 1], m_path.m_points[m_point_index], m_path.m_points[m_point_index + 1] };
    }
    case Command::Line: {
      GK_ASSERT(m_point_index > 0 && m_point_index < m_path.m_points.size(), "Points vector subscript out of range.");
      return { m_path.m_points[m_point_index - 1], m_path.m_points[m_point_index] };
    }
    default:
    case Command::Move:
      GK_ASSERT(m_point_index < m_path.m_points.size(), "Points vector subscript out of range.");
      return { m_path.m_points[m_point_index] };
    }
  }

  /* -- ReverseIterator -- */

  template <typename T, typename _>
  Path<T, _>::ReverseIterator::ReverseIterator(const Path<T>& path, const uint32_t index) :
    m_path(path), m_index(index), m_point_index(0)
  {
    if (m_index != 0 && path.get_command(m_index) == Command::Move) m_index--;

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

  template <typename T, typename _>
  Path<T, _>::ReverseIterator& Path<T, _>::ReverseIterator::operator++() {
    GK_ASSERT(m_index > 0, "Cannot increment the rend iterator.");

    m_index -= 1;

    switch (m_path.get_command(m_index)) {
    case Command::Move:
      if (m_index > 0) operator++();
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

  template <typename T, typename _>
  Path<T, _>::ReverseIterator Path<T, _>::ReverseIterator::operator++(int) {
    ReverseIterator tmp = *this;
    ++(*this);

    return tmp;
  }

  template <typename T, typename _>
  Path<T, _>::ReverseIterator Path<T, _>::ReverseIterator::operator+(const uint32_t n) const {
    ReverseIterator tmp = *this;

    for (uint32_t i = 0; i < n; i++) {
      ++tmp;
    }

    return tmp;
  }

  template <typename T, typename _>
  Path<T, _>::ReverseIterator& Path<T, _>::ReverseIterator::operator--() {
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

    if (m_index < m_path.m_commands_size && m_path.get_command(m_index) == Command::Move) operator++();

    return *this;
  }

  template <typename T, typename _>
  Path<T, _>::ReverseIterator Path<T, _>::ReverseIterator::operator--(int) {
    ReverseIterator tmp = *this;
    --(*this);

    return tmp;
  }

  template <typename T, typename _>
  Path<T, _>::ReverseIterator Path<T, _>::ReverseIterator::operator-(const uint32_t n) const {
    ReverseIterator tmp = *this;

    for (uint32_t i = 0; i < n; i++) {
      --tmp;
    }

    return tmp;
  }

  template <typename T, typename _>
  Path<T, _>::ReverseIterator::value_type Path<T, _>::ReverseIterator::operator*() const {
    const Command command = m_path.get_command(m_index);

    switch (command) {
    case Command::Cubic: {
      GK_ASSERT(m_point_index > 0 && m_point_index + 2 < m_path.m_points.size(), "Not enough points for a cubic bezier.");
      return { m_path.m_points[m_point_index - 1], m_path.m_points[m_point_index], m_path.m_points[m_point_index + 1], m_path.m_points[m_point_index + 2] };
    }
    case Command::Quadratic: {
      GK_ASSERT(m_point_index > 0 && m_point_index + 1 < m_path.m_points.size(), "Not enough points for a quadratic bezier.");
      return { m_path.m_points[m_point_index - 1], m_path.m_points[m_point_index], m_path.m_points[m_point_index + 1] };
    }
    case Command::Line: {
      GK_ASSERT(m_point_index > 0 && m_point_index < m_path.m_points.size(), "Points vector subscript out of range.");
      return { m_path.m_points[m_point_index - 1], m_path.m_points[m_point_index] };
    }
    default:
    case Command::Move:
      GK_ASSERT(m_point_index < m_path.m_points.size(), "Points vector subscript out of range.");
      return { m_path.m_points[m_point_index] };
    }
  }

  /* -- Path -- */

  template <typename T, typename _>
  Path<T, _>::Path() :
    m_commands_size(0), m_points_size(0) {}

  template <typename T, typename _>
  Path<T, _>::Path(const Path<T>& other) :
    m_points(other.m_points), m_commands(other.m_commands), m_commands_size(other.m_commands_size),
    m_closed(other.m_closed), m_in_handle(other.m_in_handle), m_out_handle(other.m_out_handle) {}

  template <typename T, typename _>
  Path<T, _>::Path(Path<T>&& other) noexcept :
    m_points(std::move(other.m_points)), m_commands(std::move(other.m_commands)), m_commands_size(other.m_commands_size),
    m_closed(other.m_closed), m_in_handle(other.m_in_handle), m_out_handle(other.m_out_handle) {}

  template <typename T, typename _>
  template <typename U>
  Path<T, _>::Path(const Path<U>& other) :
    m_points(other.m_points.begin(), other.m_points.end()), m_commands(other.m_commands.begin(), other.m_commands.end()),
    m_commands_size(other.m_commands_size), m_closed(other.m_closed), m_in_handle(other.m_in_handle), m_out_handle(other.m_out_handle) {}

  template <typename T, typename _>
  Path<T, _>::Path(io::DataDecoder& decoder) {
    // Commands and points are always present, is_closed is encoded in the properties bitfield.
    const auto [is_closed, has_in_handle, has_out_handle] = decoder.bitfield<3>();

    // Commands are stored in encoded form.
    m_commands = decoder.vector<uint8_t>();

    if (m_commands.empty()) {
      m_commands_size = 0;
      m_closed = false;

      return;
    }

    // Points are just stored as a list of coordinates.
    m_points = decoder.vector<math::Vec2<T>>();
    m_closed = is_closed;

    if (has_in_handle) m_in_handle = decoder.math::Vec2<T>();
    if (has_out_handle) m_out_handle = decoder.math::Vec2<T>();

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
      // If there is only one command, it must be a move command.
      m_commands_size = 1;
      m_commands.resize(1);
      m_points.resize(1);
    } else {
      // Trim the points in case the last command was a move command.
      m_commands_size = last_index + 1;
      m_points.resize(last_point_index);
    }
  }

  template <typename T, typename _>
  Path<T, _>& Path<T, _>::operator=(const Path<T, _>& other) {
    m_points = other.m_points;
    m_commands = other.m_commands;
    m_commands_size = other.m_commands_size;
    m_closed = other.m_closed;
    m_in_handle = other.m_in_handle;
    m_out_handle = other.m_out_handle;

    return *this;
  }

  template <typename T, typename _>
  Path<T, _>& Path<T, _>::operator=(Path<T, _>&& other) noexcept {
    m_points = std::move(other.m_points);
    m_commands = std::move(other.m_commands);
    m_commands_size = other.m_commands_size;
    m_closed = other.m_closed;
    m_in_handle = other.m_in_handle;
    m_out_handle = other.m_out_handle;

    return *this;
  }

  template <typename T, typename _>
  void Path<T, _>::for_each(
    std::function<void(const math::Vec2<T>)> move_callback = nullptr,
    std::function<void(const math::Vec2<T>)> line_callback = nullptr,
    std::function<void(const math::Vec2<T>, const math::Vec2<T>)> quadratic_callback = nullptr,
    std::function<void(const math::Vec2<T>, const math::Vec2<T>, const math::Vec2<T>)> cubic_callback = nullptr
  ) const {
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

  template <typename T, typename _>
  void Path<T, _>::for_each_reversed(
    std::function<void(const math::Vec2<T>)> move_callback = nullptr,
    std::function<void(const math::Vec2<T>, const math::Vec2<T>)> line_callback = nullptr,
    std::function<void(const math::Vec2<T>, const math::Vec2<T>, const math::Vec2<T>)> quadratic_callback = nullptr,
    std::function<void(const math::Vec2<T>, const math::Vec2<T>, const math::Vec2<T>, const math::Vec2<T>)> cubic_callback = nullptr
  ) const {
    for (int64_t i = static_cast<int64_t>(m_commands_size) - 1, j = static_cast<int32_t>(m_points.size()); i >= 0; i--) {
      switch (get_command(i)) {
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

  template <typename T, typename _>
  std::vector<uint32_t> Path<T, _>::vertex_indices() const {
    std::vector<uint32_t> indices;

    indices.reserve(points_size(false));

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

  template <typename T, typename _>
  bool Path<T, _>::is_vertex(const uint32_t point_index) const {
    if (point_index == 0) return true;

    for (uint32_t i = 0, point_i = 0; i < m_commands_size; i++) {
      if (point_i > point_index) return false;

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

  template <typename T, typename _>
  Path<T, _>::VertexNode Path<T, _>::node_at(const uint32_t point_index) const {
    GK_ASSERT(point_index < m_points.size() || point_index == in_handle_index || point_index == out_handle_index, "Point index out of range.");

    VertexNode node = { 0, -1, -1, -1, -1, -1 };

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

    Iterator it = { *this, point_index, IndexType::Point };
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

  template <typename T, typename _>
  void Path<T, _>::move_to(const math::Vec2<T> point) {
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

  template <typename T, typename _>
  void Path<T, _>::line_to(const math::Vec2<T> point, const bool reverse) {
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

  template <typename T, typename _>
  void Path<T, _>::quadratic_to(const math::Vec2<T> control, const math::Vec2<T> point, const bool reverse) {
    GK_ASSERT(!vacant(), "Cannot add a quadratic bezier to a vacant path.");

    if (reverse) {
      m_points.insert(m_points.begin(), { point, control });
      m_in_handle = point;

      insert_command(Command::Quadratic, 0);
    } else {
      m_points.insert(m_points.end(), { control, point });
      m_out_handle = point;

      push_command(Command::Quadratic);
    }
  }

  template <typename T, typename _>
  void Path<T, _>::cubic_to(const math::Vec2<T> control1, const math::Vec2<T> control2, const math::Vec2<T> point, const bool reverse) {
    GK_ASSERT(!vacant(), "Cannot add a cubic bezier to a vacant path.");

    if (control1 == (reverse ? m_points.front() : m_points.back()) && control2 == point) {
      return line_to(point, reverse);
    }

    if (reverse) {
      m_points.insert(m_points.begin(), { point, control2, control1 });
      m_in_handle = point;

      insert_command(Command::Cubic, 0);
    } else {
      m_points.insert(m_points.end(), { control1, control2, point });
      m_out_handle = point;

      push_command(Command::Cubic);
    }
  }

  template <typename T, typename _>
  void Path<T, _>::cubic_to(const math::Vec2<T> control, const math::Vec2<T> point, const bool is_control_1, const bool reverse) {
    GK_ASSERT(!vacant(), "Cannot add a cubic bezier to a vacant path.");

    if (reverse) {
      if (is_control_1) {
        m_points.insert(m_points.begin(), { point, point, control });
      } else {
        m_points.insert(m_points.begin(), { point, control, m_points.front() });
      }

      m_in_handle = point;

      insert_command(Command::Cubic, 0);
    } else {
      if (is_control_1) {
        m_points.insert(m_points.end(), { control, point, point });
      } else {
        m_points.insert(m_points.end(), { m_points.back(), control, point });
      }

      m_out_handle = point;

      push_command(Command::Cubic);
    }

  }

  template <typename T, typename _>
  void Path<T, _>::arc_to(const math::Vec2<T> center, const math::Vec2<T> radius, const T x_axis_rotation, const bool large_arc_flag, const bool sweep_flag, const math::Vec2<T> point, const bool reverse) {
    GK_ASSERT(!vacant(), "Cannot add an arc to a vacant path.");

    math::Vec2<T> r = radius;

    const T sin_th = std::sin(math::degrees_to_radians(x_axis_rotation));
    const T cos_th = std::cos(math::degrees_to_radians(x_axis_rotation));

    const math::Vec2<T> d0 = (center - point) / T(2);
    const math::Vec2<T> d1 = {
      cos_th * d0.x + sin_th * d0.y,
      -sin_th * d0.x + cos_th * d0.y
    };

    const math::Vec2<T> sq_r = r * r;
    const math::Vec2<T> sq_p = d1 * d1;

    const T check = sq_p.x / sq_r.x + sq_p.y / sq_r.y;
    if (check > T(1)) r *= std::sqrt(check);

    mat2 a = {
      cos_th / r.x, sin_th / r.x,
      -sin_th / r.y, cos_th / r.y
    };
    math::Vec2<T> p1 = {
      math::dot(a[0], point),
      math::dot(a[1], point)
    };

    const math::Vec2<T> p0 = {
      math::dot(a[0], center),
      math::dot(a[1], center)
    };

    const T d = math::squared_length(p1 - p0);

    T sfactor_sq = T(1) / d - T(0.25);
    if (sfactor_sq < T(0)) sfactor_sq = T(0);

    T sfactor = std::sqrt(sfactor_sq);
    if (sweep_flag == large_arc_flag) sfactor = -sfactor;

    const math::Vec2<T> c1 = {
      (p0.x + p1.x) / T(2) - sfactor * (p1.y - p0.y),
      (p0.y + p1.y) / T(2) + sfactor * (p1.x - p0.x)
    };

    const T th0 = std::atan2(p0.y - c1.y, p0.x - c1.x);
    const T th1 = std::atan2(p1.y - c1.y, p1.x - c1.x);

    T th_arc = th1 - th0;
    if (th_arc < T(0) && sweep_flag) th_arc += math::two_pi<T>;
    else if (th_arc > T(0) && !sweep_flag) th_arc -= math::two_pi<T>;

    int n_segs = static_cast<int>(std::ceil(std::abs(th_arc / (math::pi<T> / T(2) + math::geometric_epsilon<T>))));
    for (int i = 0; i < n_segs; i++) {
      const T th2 = th0 + i * th_arc / n_segs;
      const T th3 = th0 + (i + 1) * th_arc / n_segs;

      a = {
        cos_th * r.x, -sin_th * r.x,
        sin_th * r.y, cos_th * r.y
      };

      const T th_half = (th3 - th2) / T(2);
      const T sin_half_th_half = std::sin(th_half / T(2));
      const T t = (T(8) / T(3)) * sin_half_th_half * sin_half_th_half / std::sin(th_half);

      const T sin_th2 = std::sin(th2);
      const T cos_th2 = std::cos(th2);
      const T sin_th3 = std::sin(th3);
      const T cos_th3 = std::cos(th3);

      p1 = {
        c1.x + cos_th2 - t * sin_th2,
        c1.y + sin_th2 + t * cos_th2
      };

      const math::Vec2<T> p3 = {
        c1.x + cos_th3,
        c1.y + sin_th3
      };
      const math::Vec2<T> p2 = {
        p3.x + t * sin_th3,
        p3.y - t * cos_th3
      };

      const math::Vec2<T> bez1 = {
        math::dot(a[0], p1),
        math::dot(a[1], p1)
      };
      const math::Vec2<T> bez2 = {
        math::dot(a[0], p2),
        math::dot(a[1], p2)
      };
      const math::Vec2<T> bez3 = {
        math::dot(a[0], p3),
        math::dot(a[1], p3)
      };

      cubic_to(bez1, bez2, bez3, reverse);
    }
  }

  template <typename T, typename _>
  void Path<T, _>::ellipse(const math::Vec2<T> center, const math::Vec2<T> radius) {
    const math::Vec2<T> top_left = center - radius;
    const math::Vec2<T> bottom_right = center + radius;
    const math::Vec2<T> cp = radius * math::circle_ratio<T>;

    move_to({ center.x, top_left.y });
    cubic_to({ center.x + cp.x, top_left.y }, { bottom_right.x, center.y - cp.y }, { bottom_right.x, center.y });
    cubic_to({ bottom_right.x, center.y + cp.y }, { center.x + cp.x, bottom_right.y }, { center.x, bottom_right.y });
    cubic_to({ center.x - cp.x, bottom_right.y }, { top_left.x, center.y + cp.y }, { top_left.x, center.y });
    cubic_to({ top_left.x, center.y - cp.y }, { center.x - cp.x, top_left.y }, { center.x, top_left.y });
    close();
  }

  template <typename T, typename _>
  void Path<T, _>::circle(const math::Vec2<T> center, const T radius) {
    ellipse(center, { radius, radius });
  }

  template <typename T, typename _>
  void Path<T, _>::rect(const math::Vec2<T> point, const math::Vec2<T> size, const bool centered) {
    math::Vec2<T> p = point;

    if (centered) {
      p -= size / T(2);
    }

    move_to(p);
    line_to(p + math::Vec2<T>{ size.x, T(0) });
    line_to(p + size);
    line_to(p + math::Vec2<T>{ T(0), size.y });
    close();
  }

  template <typename T, typename _>
  void Path<T, _>::round_rect(const math::Vec2<T> point, const math::Vec2<T> size, const T radius, const bool centered) {
    math::Vec2<T> p = point;
    T r = radius;

    if (centered) {
      p -= size / T(2);
    }

    if (r > size.x / T(2)) r = size.x / T(2);
    if (r > size.y / T(2)) r = size.y / T(2);

    move_to({ p.x + r, p.y });
    line_to({ p.x + size.x - r, p.y });
    cubic_to({ p.x + size.x - r * math::circle_ratio<T>, p.y }, { p.x + size.x, p.y + r * math::circle_ratio<T> }, { p.x + size.x, p.y + r });
    line_to({ p.x + size.x, p.y + size.y - r });
    cubic_to({ p.x + size.x, p.y + size.y - r * math::circle_ratio<T> }, { p.x + size.x - r * math::circle_ratio<T>, p.y + size.y }, { p.x + size.x - r, p.y + size.y });
    line_to({ p.x + r, p.y + size.y });
    cubic_to({ p.x + r * math::circle_ratio<T>, p.y + size.y }, { p.x, p.y + size.y - r * math::circle_ratio<T> }, { p.x, p.y + size.y - r });
    line_to({ p.x, p.y + r });
    cubic_to({ p.x, p.y + r * math::circle_ratio<T> }, { p.x + r * math::circle_ratio<T>, p.y }, { p.x + r, p.y });
    close();
  }

  template <typename T, typename _>
  uint32_t Path<T, _>::to_line(const uint32_t command_index, uint32_t reference_point = 0) {
    GK_ASSERT(command_index < m_commands_size, "Command index out of range.");

    const Command command = get_command(command_index);
    if (command == Command::Line || command == Command::Move) return reference_point;

    const Iterator it = { *this, command_index, IndexType::Command };
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

  template <typename T, typename _>
  uint32_t Path<T, _>::to_quadratic(const uint32_t command_index, uint32_t reference_point = 0) {
    GK_ASSERT(command_index < m_commands_size, "Command index out of range.");

    const Command command = get_command(command_index);
    if (command != Command::Line) return reference_point;

    const Iterator it = { *this, command_index, IndexType::Command };
    const Segment segment = *it;
    const uint32_t point_i = it.point_index();

    m_points.insert(m_points.begin() + point_i, (m_points[point_i - 1] + m_points[point_i]) / T(2));

    replace_command(command_index, Command::Quadratic);

    return reference_point >= point_i ? reference_point + 1 : reference_point;
  }

  template <typename T, typename _>
  uint32_t Path<T, _>::to_cubic(const uint32_t command_index, uint32_t reference_point = 0) {
    GK_ASSERT(command_index < m_commands_size, "Command index out of range.");

    const Command command = get_command(command_index);
    if (command == Command::Cubic || command == Command::Move) return reference_point;

    const Iterator it = { *this, command_index, IndexType::Command };
    const Segment segment = *it;
    const uint32_t point_i = it.point_index();

    if (segment.type == Command::Line) {
      m_points.insert(m_points.begin() + point_i, { m_points[point_i - 1], m_points[point_i] });

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

  template <typename T, typename _>
  void Path<T, _>::remove(const uint32_t point_index, const bool keep_shape = false) {
    GK_ASSERT(point_index < m_points.size(), "Point index out of range.");

    uint32_t to_remove = point_index == m_points.size() - 1 ? 0 : point_index;

    if (empty() || (point_index == 0 && !closed())) {
      return;
    }

    const Iterator it = { *this, to_remove, IndexType::Point };
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

    geom::CubicBezier<T> cubic;

    if (keep_shape) {
      // TODO: implement settings and unify algorithms::CubicBezier and geom::cubic_bezier
      std::vector<math::Vec2<T>> points(GK_FIT_RESOLUTION * 2 + 2);

      for (size_t i = 0; i <= GK_FIT_RESOLUTION; i++) {
        float t = static_cast<float>(i) / static_cast<float>(GK_FIT_RESOLUTION);

        points[i] = segment.sample(t);
        points[GK_FIT_RESOLUTION + i + 1] = next_segment.sample(t);
      }

      cubic = math::Algorithms::fit_points_to_cubic(points, GK_PATH_TOLERANCE);
    } else {
      const math::Vec2<T> p1 = segment.type == Command::Line ? segment.p0 : segment.p1;

      if (next_segment.type == Command::Line) {
        cubic = { segment.p0, p1, next_segment.p1, next_segment.p1 };
      } else if (next_segment.type == Command::Quadratic) {
        cubic = { segment.p0, p1, next_segment.p1, next_segment.p2 };
      } else {
        cubic = { segment.p0, p1, next_segment.p2, next_segment.p3 };
      }
    }

    size_t new_command_index = 0;

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
        m_points.insert(m_points.begin(), { cubic.p0, cubic.p1 });
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
        m_points.insert(m_points.begin() + to_remove, { cubic.p1, cubic.p2 });
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

  template <typename T, typename _>
  uint32_t Path<T, _>::split(const uint32_t segment_index, const T t) {

  }

  template <typename T, typename _>
  math::Rect<T> Path<T, _>::bounding_rect() const {

  }

  template <typename T, typename _>
  math::Rect<T> Path<T, _>::bounding_rect(const math::Mat2x3<T>& transform) const {

  }

  template <typename T, typename _>
  math::Rect<T> Path<T, _>::approx_bounding_rect() const {

  }

  /* -- Template Instantiation -- */

  template Path<float>;
  template Path<double>;

}