/**
 * @file path.cpp
 * @brief Implementation of the Path class.
 *
 * @todo Implement history.
 * @todo Implement in and out handles.
 * @todo Better closing algorithm.
 * @todo is_point_in_path() should have a stroke of std::max(stroke_width, threshold).
 */

#include "path.h"

#include "path_builder.h"

#include "../properties.h"

#include "../../math/matrix.h"
#include "../../math/vector.h"
#include "../../math/scalar.h"
#include "../../math/mat2x3.h"
#include "../../math/mat2.h"
#include "../../math/math.h"

#include "../../utils/console.h"
#include "../../utils/assert.h"

namespace Graphick::Renderer::Geometry {

  /* -- Segment -- */

  bool Path::Segment::is_point() const {
    bool point = p0 == p1;

    if (point) {
      if (is_quadratic()) return p1 == p2;
      if (is_cubic()) return p1 == p2 && p2 == p3;
    }

    return point;
  }

  Math::rect Path::Segment::bounding_rect() const {
    switch (type) {
    case Command::Cubic: {
      return Math::cubic_bounding_rect(p0, p1, p2, p3);
    }
    case Command::Quadratic: {
      return Math::quadratic_bounding_rect(p0, p1, p2);
    }
    case Command::Line: {
      return Math::linear_bounding_rect(p0, p1);
    }
    default:
    case Command::Move: {
      return { p0, p0 };
    }
    }
  }

  Math::rect Path::Segment::bounding_rect(const mat2x3& transform) const {
    const vec2 a = transform * p0;
    const vec2 b = transform * p1;

    switch (type) {
    case Command::Cubic: {
      const vec2 c = transform * p2;
      const vec2 d = transform * p3;

      return Math::cubic_bounding_rect(a, b, c, d);
    }
    case Command::Quadratic: {
      const vec2 c = transform * p2;

      return Math::quadratic_bounding_rect(a, b, c);
    }
    case Command::Line: {
      return Math::linear_bounding_rect(a, b);
    }
    default:
      return { a, a };
    }
  }

  Math::rect Path::Segment::approx_bounding_rect() const {
    Math::rect rect = { p0, p0 };

    switch (type) {
    case Command::Cubic: {
      Math::min(rect.min, p1, rect.min);
      Math::min(rect.min, p2, rect.min);
      Math::min(rect.min, p3, rect.min);
      Math::max(rect.max, p1, rect.max);
      Math::max(rect.max, p2, rect.max);
      Math::max(rect.max, p3, rect.max);
    }
    case Command::Quadratic: {
      Math::min(rect.min, p1, rect.min);
      Math::min(rect.min, p2, rect.min);
      Math::max(rect.max, p1, rect.max);
      Math::max(rect.max, p2, rect.max);
    }
    case Command::Line: {
      Math::min(rect.min, p1, rect.min);
      Math::max(rect.max, p1, rect.max);
    }
    default:
      break;
    }

    return rect;
  }

  /* -- Iterator -- */

  Path::Iterator::Iterator(const Path& path, const size_t index, const bool is_segment_index) : m_path(path), m_index(index), m_point_index(0) {
    if (is_segment_index) {
      GK_ASSERT(index < path.size(), "Segment index out of range.");

      m_index = 0;
      size_t i = 0;

      while (i <= index) {
        Command command = path.get_command(m_index);

        if (command == Command::Move) {
          m_point_index++;
          m_index++;
          continue;
        } else if (i == index) {
          break;
        }

        switch (command) {
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

        i++;
        m_index += 1;
      }

      return;
    }

    if (m_index < path.m_commands_size && path.get_command(m_index) == Command::Move) m_index++;

    GK_ASSERT(m_index > 0 && m_index <= path.m_commands_size, "Index out of range.");

    if (m_index < path.m_commands_size / 2) {
      for (size_t i = 0; i < m_index; i++) {
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
      m_point_index = path.m_points.size();

      for (size_t i = path.m_commands_size - 1; i >= m_index; i--) {
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

  Path::Iterator& Path::Iterator::operator++() {
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

  Path::Iterator Path::Iterator::operator++(int) {
    Iterator tmp = *this;
    ++(*this);

    return tmp;
  }

  Path::Iterator& Path::Iterator::operator--() {
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

  Path::Iterator Path::Iterator::operator--(int) {
    Iterator tmp = *this;
    --(*this);

    return tmp;
  }

  Path::Iterator::value_type Path::Iterator::operator*() const {
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

  Path::ReverseIterator::ReverseIterator(const Path& path, const size_t index) : m_path(path), m_index(index), m_point_index(0) {
    if (m_index != 0 && path.get_command(m_index) == Command::Move) m_index--;

    GK_ASSERT(m_index >= 0 && m_index < path.m_commands_size, "Index out of range.");

    if (m_index < path.m_commands_size / 2) {
      for (size_t i = 0; i < m_index; i++) {
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
      m_point_index = path.m_points.size();

      for (size_t i = path.m_commands_size - 1; i >= m_index; i--) {
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

  Path::ReverseIterator& Path::ReverseIterator::operator++() {
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

  Path::ReverseIterator Path::ReverseIterator::operator++(int) {
    ReverseIterator tmp = *this;
    ++(*this);

    return tmp;
  }

  Path::ReverseIterator& Path::ReverseIterator::operator--() {
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

  Path::ReverseIterator Path::ReverseIterator::operator--(int) {
    ReverseIterator tmp = *this;
    --(*this);

    return tmp;
  }

  Path::ReverseIterator::value_type Path::ReverseIterator::operator*() const {
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

  Path::Path() : m_points(), m_commands() {}

  Path::Path(const Path& other) : m_points(other.m_points), m_commands(other.m_commands), m_commands_size(other.m_commands_size) {}

  Path::Path(Path&& other) noexcept : m_points(std::move(other.m_points)), m_commands(std::move(other.m_commands)), m_commands_size(other.m_commands_size) {}

  Path::Path(io::DataDecoder& decoder) {
    m_commands = decoder.vector<uint8_t>();

    if (m_commands.empty()) {
      m_commands_size = 0;
      return;
    }

    m_points = decoder.vector<vec2>();

    size_t point_index = 0;
    size_t last_non_move_index = 0;
    size_t last_non_move_point_index = 0;

    for (size_t i = 0; i < m_commands.size() * 4; i++) {
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
        last_non_move_index = i;
        last_non_move_point_index = point_index;
      }
    }

    if (last_non_move_index == 0) {
      m_commands_size = 0;

      m_commands.resize(1);
      m_points.resize(1);

      return;
    }

    m_commands_size = last_non_move_index + 1;

    m_commands.resize(m_commands_size / 4 + 1);
    m_points.resize(last_non_move_point_index);
  }

  Path& Path::operator=(const Path& other) {
    m_points = other.m_points;
    m_commands = other.m_commands;

    return *this;
  }

  Path& Path::operator=(Path&& other) noexcept {
    m_points = std::move(other.m_points);
    m_commands = std::move(other.m_commands);

    return *this;
  }

  Path::Segment Path::front(const size_t move_index) const {
    size_t move_i = 0;

    for (size_t i = 0; i < m_commands_size; i++) {
      if (get_command(i) == Command::Move) {
        if (move_i == move_index) {
          return *Iterator(*this, i + 0);
        }

        move_i += 1;
      }
    }

    return front();
  }

  Path::Segment Path::back(const size_t move_index) const {
    size_t move_i = 0;

    for (size_t i = 0; i < m_commands_size; i++) {
      if (get_command(i) == Command::Move) {
        if (move_i == move_index + 1) {
          return *Iterator(*this, i - 1);
        }

        move_i += 1;
      }
    }

    return back();
  }

  void Path::for_each(
    std::function<void(const vec2)> move_callback,
    std::function<void(const vec2)> line_callback,
    std::function<void(const vec2, const vec2)> quadratic_callback,
    std::function<void(const vec2, const vec2, const vec2)> cubic_callback
  ) const {
    for (size_t i = 0, j = 0; i < m_commands_size; i++) {
      switch (get_command(i)) {
      case Command::Cubic: {
        GK_ASSERT(j + 2 < m_points.size(), "Not enough points for a cubic bezier.");

        if (cubic_callback) {
          cubic_callback(m_points[j], m_points[j + 1], m_points[j + 2]);
        }

        j += 3;

        break;
      }
      case Command::Quadratic: {
        GK_ASSERT(j + 1 < m_points.size(), "Not enough points for a quadratic bezier.");

        if (quadratic_callback) {
          quadratic_callback(m_points[j], m_points[j + 1]);
        }

        j += 2;

        break;
      }
      case Command::Line: {
        GK_ASSERT(j < m_points.size(), "Not enough points for a line.");

        if (line_callback) {
          line_callback(m_points[j]);
        }

        j += 1;

        break;
      }
      case Command::Move: {
        GK_ASSERT(j < m_points.size(), "Points vector subscript out of range.");

        if (move_callback) {
          move_callback(m_points[j]);
        }

        j += 1;

        break;
      }
      }
    }
  }

  void Path::for_each_reversed(
    std::function<void(const vec2)> move_callback,
    std::function<void(const vec2, const vec2)> line_callback,
    std::function<void(const vec2, const vec2, const vec2)> quadratic_callback,
    std::function<void(const vec2, const vec2, const vec2, const vec2)> cubic_callback
  ) const {
    for (int i = static_cast<int>(m_commands_size) - 1, j = static_cast<int>(m_points.size()); i >= 0; i--) {
      switch (get_command(i)) {
      case Command::Cubic: {
        GK_ASSERT(j - 4 >= 0, "Not enough points for a cubic bezier.");

        if (cubic_callback) {
          cubic_callback(m_points[j - 4], m_points[j - 3], m_points[j - 2], m_points[j - 1]);
        }

        j -= 3;

        break;
      }
      case Command::Quadratic: {
        GK_ASSERT(j - 3 >= 0, "Not enough points for a quadratic bezier.");

        if (quadratic_callback) {
          quadratic_callback(m_points[j - 3], m_points[j - 2], m_points[j - 1]);
        }

        j -= 2;

        break;
      }
      case Command::Line: {
        GK_ASSERT(j - 2 >= 0, "Not enough points for a line.");

        if (line_callback) {
          line_callback(m_points[j - 2], m_points[j - 1]);
        }

        j -= 1;

        break;
      }
      case Command::Move: {
        GK_ASSERT(j - 1 >= 0, "Points vector subscript out of range.");

        if (move_callback) {
          move_callback(m_points[j - 1]);
        }

        j -= 1;

        break;
      }
      }
    }
  }

  size_t Path::size() const {
    size_t size = 0;

    for (size_t i = 0; i < m_commands_size; i++) {
      switch (get_command(i)) {
      case Command::Move:
        break;
      default:
        size += 1;
      }
    }

    return size;
  }

  size_t Path::points_size(const bool include_handles) const {
    if (include_handles) {
      return m_points.size();
    }

    return m_commands_size;
  }

  std::vector<size_t> Path::vertex_indices() const {
    std::vector<size_t> indices;
    vec2 last_move_point = { 0.0f, 0.0f };

    indices.reserve(points_size(false));

    for (size_t i = 0, point_i = 0; i < m_commands_size; i++) {
      switch (get_command(i)) {
      case Command::Move:
        indices.push_back(point_i);

        last_move_point = m_points[point_i];
        point_i += 1;
        break;
      case Command::Line:
        if (m_points[point_i] != last_move_point) {
          indices.push_back(point_i);
        }

        point_i += 1;
        break;
      case Command::Quadratic:
        if (m_points[point_i + 1] != last_move_point) {
          indices.push_back(point_i + 1);
        }

        point_i += 2;
        break;
      case Command::Cubic:
        if (m_points[point_i + 2] != last_move_point) {
          indices.push_back(point_i + 2);
        }

        point_i += 3;
        break;
      }
    }

    return indices;
  }

  bool Path::is_vertex(const size_t point_index) const {
    if (point_index == 0) return true;

    for (size_t i = 0, point_i = 0; i < m_commands_size; i++) {
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

  Path::VertexNode Path::node_at(const size_t point_index) const {
    VertexNode node = { 0, -1, -1, -1 };

    size_t last_move_point = 0;
    size_t last_move_i = 0;

    for (size_t i = 0, point_i = 0; i < m_commands_size; i++) {
      if (point_i > point_index) return node;

      switch (get_command(i)) {
      case Command::Move:
        last_move_point = point_i;
        last_move_i = i;

        if (point_i == point_index) {
          size_t k = i + 1;
          size_t h = point_i + 1;

          while (k < m_commands_size) {
            switch (get_command(k)) {
            case Command::Move:
              if (m_points[h - 1] == m_points[point_i]) {
                if (get_command(k - 1) == Command::Cubic) {
                  node.in = h - 2;
                }

                node.close_vertex = h - 1;
              }

              goto exit_closing_while;
            case Command::Line:
              h += 1;
              break;
            case Command::Quadratic:
              h += 2;
              break;
            case Command::Cubic:
              h += 3;
              break;
            }

            k++;
          }

        exit_closing_while:;

          h = m_points.size();

          if (k == m_commands_size && m_points.back() == m_points[point_i]) {
            if (get_command(k - 1) == Command::Cubic) {
              node.in = h - 2;
            }

            node.close_vertex = h - 1;
          }

          node.vertex = point_i;

          if (i < m_commands_size - 1 && get_command(i + 1) == Command::Cubic) {
            node.out = point_i + 1;
          }

          return node;
        }

        point_i += 1;
        break;
      case Command::Line:
        if (point_i == point_index) {
          node.vertex = point_i;

          if (i < m_commands_size - 1 && get_command(i + 1) == Command::Cubic) {
            node.out = point_i + 1;
          }

          return node;
        }

        point_i += 1;
        break;
      case Command::Quadratic:
        if (point_i == point_index) {
          node.vertex = point_i;

          return node;
        } else if (point_i + 1 == point_index) {
          node.vertex = point_i + 1;

          if (i < m_commands_size - 1 && get_command(i + 1) == Command::Cubic) {
            node.out = point_i + 2;
          }

          return node;
        }

        point_i += 2;
        break;
      case Command::Cubic:
        if (point_i == point_index) {
          node.out = point_i;

          if (i > 0) {
            Command prev_command = get_command(i - 1);

            if (prev_command == Command::Cubic) {
              node.in = point_i - 2;
            } else if (prev_command == Command::Move) {
              size_t k = i + 1;
              size_t h = point_i + 3;

              while (k < m_commands_size) {
                switch (get_command(k)) {
                case Command::Move:
                  if (m_points[h - 1] == m_points[point_i - 1]) {
                    if (get_command(k - 1) == Command::Cubic) {
                      node.in = h - 2;
                    }

                    node.close_vertex = h - 1;
                  }

                  goto exit_cubic_closing_while;
                case Command::Line:
                  h += 1;
                  break;
                case Command::Quadratic:
                  h += 2;
                  break;
                case Command::Cubic:
                  h += 3;
                  break;
                }

                k++;
              }

            exit_cubic_closing_while:;

              h = m_points.size();

              if (k == m_commands_size && m_points.back() == m_points[point_i - 1]) {
                if (get_command(k - 1) == Command::Cubic) {
                  node.in = h - 2;
                }

                node.close_vertex = h - 1;
              }
            }

            node.vertex = point_i - 1;
          }

          return node;
        } else if (point_i + 1 == point_index || point_i + 2 == point_index) {
          node.out = point_i + 1;
          node.vertex = point_i + 2;

          if (i < m_commands_size - 1 && get_command(i + 1) == Command::Cubic) {
            node.in = point_i + 3;
          } else if (i >= m_commands_size - 1 || get_command(i + 1) == Command::Move) {
            if (m_points[point_i + 2] == m_points[last_move_point]) {
              node.close_vertex = last_move_point;

              if (get_command(last_move_i + 1) == Command::Cubic) {
                node.in = last_move_point + 1;
              }
            }
          }

          return node;
        }

        point_i += 3;
        break;
      }
    }

    node.vertex = point_index;

    return node;
  }

  bool Path::closed(const size_t move_index) const {
    size_t last_point = 0;
    size_t move_i = 0;

    for (size_t i = 0, point_i = 0; i < m_commands_size; i++) {
      switch (get_command(i)) {
      case Command::Move:
        if (move_i == move_index) {
          last_point = point_i;
        } else if (move_i == move_index + 1) {
          return m_points[point_i - 1] == m_points[last_point];
        }

        move_i += 1;
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
    }

    return m_points.back() == m_points[last_point];
  }

  void Path::move_to(const vec2 point) {
    if (!vacant() && get_command(m_commands_size - 1) == Command::Move) {
      m_points[m_points.size() - 1] = point;
      return;
    }

    m_points.push_back(point);
    push_command(Command::Move);
  }

  void Path::line_to(const vec2 point) {
    GK_ASSERT(!vacant(), "Cannot add a line to a vacant path.");

    m_points.push_back(point);
    push_command(Command::Line);
  }

  void Path::quadratic_to(const vec2 control, const vec2 point) {
    GK_ASSERT(!vacant(), "Cannot add a quadratic bezier to a vacant path.");

    m_points.insert(m_points.end(), { control, point });
    push_command(Command::Quadratic);
  }

  void Path::cubic_to(const vec2 control1, const vec2 control2, const vec2 point) {
    GK_ASSERT(!vacant(), "Cannot add a cubic bezier to a vacant path.");

    m_points.insert(m_points.end(), { control1, control2, point });
    push_command(Command::Cubic);
  }

  void Path::cubic_to(const vec2 control, const vec2 point, const bool is_control_1) {
    GK_ASSERT(!vacant(), "Cannot add a cubic bezier to a vacant path.");

    if (is_control_1) {
      m_points.insert(m_points.end(), { control, point, point });
    } else {
      m_points.insert(m_points.end(), { m_points.back(), control, point });
    }

    push_command(Command::Cubic);
  }

  void Path::arc_to(const vec2 center, const vec2 radius, const float x_axis_rotation, const bool large_arc_flag, const bool sweep_flag, const vec2 point) {
    GK_ASSERT(!vacant(), "Cannot add an arc to a vacant path.");

    vec2 r = radius;

    const float sin_th = std::sinf(Math::degrees_to_radians(x_axis_rotation));
    const float cos_th = std::cosf(Math::degrees_to_radians(x_axis_rotation));

    const vec2 d0 = (center - point) / 2.0f;
    const vec2 d1 = {
      cos_th * d0.x + sin_th * d0.y,
      -sin_th * d0.x + cos_th * d0.y
    };

    const vec2 sq_r = r * r;
    const vec2 sq_p = d1 * d1;

    const float check = sq_p.x / sq_r.x + sq_p.y / sq_r.y;
    if (check > 1.0f) r *= std::sqrtf(check);

    mat2 a = {
      cos_th / r.x, sin_th / r.x,
      -sin_th / r.y, cos_th / r.y
    };
    vec2 p1 = {
      Math::dot(a[0], point),
      Math::dot(a[1], point)
    };

    const vec2 p0 = {
      Math::dot(a[0], center),
      Math::dot(a[1], center)
    };

    const float d = Math::squared_length(p1 - p0);

    float sfactor_sq = 1.0f / d - 0.25f;
    if (sfactor_sq < 0.0f) sfactor_sq = 0.0f;

    float sfactor = std::sqrtf(sfactor_sq);
    if (sweep_flag == large_arc_flag) sfactor = -sfactor;

    const vec2 c1 = {
      0.5f * (p0.x + p1.x) - sfactor * (p1.y - p0.y),
      0.5f * (p0.y + p1.y) + sfactor * (p1.x - p0.x)
    };

    const float th0 = std::atan2f(p0.y - c1.y, p0.x - c1.x);
    const float th1 = std::atan2f(p1.y - c1.y, p1.x - c1.x);

    float th_arc = th1 - th0;
    if (th_arc < 0.0f && sweep_flag) th_arc += MATH_F_TWO_PI;
    else if (th_arc > 0.0f && !sweep_flag) th_arc -= MATH_F_TWO_PI;

    int n_segs = static_cast<int>(std::ceilf(std::fabsf(th_arc / (MATH_F_PI * 0.5f + 0.001f))));
    for (int i = 0; i < n_segs; i++) {
      const float th2 = th0 + i * th_arc / n_segs;
      const float th3 = th0 + (i + 1) * th_arc / n_segs;

      a = {
        cos_th * r.x, -sin_th * r.x,
        sin_th * r.y, cos_th * r.y
      };

      const float th_half = 0.5f * (th3 - th2);
      const float sin_half_th_half = std::sinf(th_half * 0.5f);
      const float t = (8.0f / 3.0f) * sin_half_th_half * sin_half_th_half / std::sin(th_half);

      const float sin_th2 = std::sinf(th2);
      const float cos_th2 = std::cosf(th2);
      const float sin_th3 = std::sinf(th3);
      const float cos_th3 = std::cosf(th3);

      p1 = {
        c1.x + cos_th2 - t * sin_th2,
        c1.y + sin_th2 + t * cos_th2
      };

      const vec2 p3 = {
        c1.x + cos_th3,
        c1.y + sin_th3
      };
      const vec2 p2 = {
        p3.x + t * sin_th3,
        p3.y - t * cos_th3
      };

      const vec2 bez1 = {
        Math::dot(a[0], p1),
        Math::dot(a[1], p1)
      };
      const vec2 bez2 = {
        Math::dot(a[0], p2),
        Math::dot(a[1], p2)
      };
      const vec2 bez3 = {
        Math::dot(a[0], p3),
        Math::dot(a[1], p3)
      };

      cubic_to(bez1, bez2, bez3);
    }
  }

  void Path::ellipse(const vec2 center, const vec2 radius) {
    const vec2 top_left = center - radius;
    const vec2 bottom_right = center + radius;
    const vec2 cp = radius * GEOMETRY_CIRCLE_RATIO;

    move_to({ center.x, top_left.y });
    cubic_to({ center.x + cp.x, top_left.y }, { bottom_right.x, center.y - cp.y }, { bottom_right.x, center.y });
    cubic_to({ bottom_right.x, center.y + cp.y }, { center.x + cp.x, bottom_right.y }, { center.x, bottom_right.y });
    cubic_to({ center.x - cp.x, bottom_right.y }, { top_left.x, center.y + cp.y }, { top_left.x, center.y });
    cubic_to({ top_left.x, center.y - cp.y }, { center.x - cp.x, top_left.y }, { center.x, top_left.y });
    close();
  }

  void Path::circle(const vec2 center, const float radius) {
    ellipse(center, { radius, radius });
  }

  void Path::rect(const vec2 point, const vec2 size, const bool centered) {
    vec2 p = point;

    if (centered) {
      p -= size * 0.5f;
    }

    move_to(p);
    line_to(p + vec2{ size.x, 0.0f });
    line_to(p + size);
    line_to(p + vec2{ 0.0f, size.y });
    close();
  }

  void Path::round_rect(const vec2 point, const vec2 size, const float radius, const bool centered) {
    float r = radius;
    vec2 p = point;

    if (centered) {
      p -= size * 0.5f;
    }

    if (r > size.x * 0.5f) r = size.x * 0.5f;
    if (r > size.y * 0.5f) r = size.y * 0.5f;

    move_to({ p.x + r, p.y });
    line_to({ p.x + size.x - r, p.y });
    cubic_to({ p.x + size.x - r * GEOMETRY_CIRCLE_RATIO, p.y }, { p.x + size.x, p.y + r * GEOMETRY_CIRCLE_RATIO }, { p.x + size.x, p.y + r });
    line_to({ p.x + size.x, p.y + size.y - r });
    cubic_to({ p.x + size.x, p.y + size.y - r * GEOMETRY_CIRCLE_RATIO }, { p.x + size.x - r * GEOMETRY_CIRCLE_RATIO, p.y + size.y }, { p.x + size.x - r, p.y + size.y });
    line_to({ p.x + r, p.y + size.y });
    cubic_to({ p.x + r * GEOMETRY_CIRCLE_RATIO, p.y + size.y }, { p.x, p.y + size.y - r * GEOMETRY_CIRCLE_RATIO }, { p.x, p.y + size.y - r });
    line_to({ p.x, p.y + r });
    cubic_to({ p.x, p.y + r * GEOMETRY_CIRCLE_RATIO }, { p.x + r * GEOMETRY_CIRCLE_RATIO, p.y }, { p.x + r, p.y });
    close();
  }

  void Path::close() {
    if (empty() || m_commands.empty() || (size() == 1 && get_command(1) == Command::Line)) return;

    vec2 p = m_points.front();

    for (size_t i = m_commands_size - 1, point_index = m_points.size(); i > 0; i--) {
      switch (get_command(i)) {
      case Command::Move:
        p = m_points[point_index - 1];
        goto exit_loop;
      case Command::Line:
        point_index -= 1;
        break;
      case Command::Quadratic:
        point_index -= 2;
        break;
      case Command::Cubic:
        point_index -= 3;
        break;
      }
    }

  exit_loop:;

    if (Math::is_almost_equal(m_points.back(), p, GK_POINT_EPSILON)) {
      m_points[m_points.size() - 1] = p;
    } else {
      line_to(p);
    }
  }

  Math::rect Path::bounding_rect() const {
    if (empty()) {
      if (vacant()) return {};
      return { m_points[0], m_points[0] };
    }

    Math::rect rect{};

    for (size_t i = 0, j = 0; i < m_commands_size; i++) {
      switch (get_command(i)) {
      case Command::Cubic: {
        GK_ASSERT(j > 0, "Cubic bezier command cannot be the first command of a path.");
        GK_ASSERT(j + 2 < m_points.size(), "Not enough points for a cubic bezier.");

        const vec2 p0 = m_points[j - 1];
        const vec2 p1 = m_points[j];
        const vec2 p2 = m_points[j + 1];
        const vec2 p3 = m_points[j + 2];

        Math::rect r = Math::cubic_bounding_rect(p0, p1, p2, p3);

        Math::min(rect.min, r.min, rect.min);
        Math::max(rect.max, r.max, rect.max);

        j += 3;

        break;
      }
      case Command::Quadratic: {
        GK_ASSERT(j > 0, "Quadratic bezier command cannot be the first command of a path.");
        GK_ASSERT(j + 1 < m_points.size(), "Not enough points for a quadratic bezier.");

        const vec2 p0 = m_points[j - 1];
        const vec2 p1 = m_points[j];
        const vec2 p2 = m_points[j + 1];

        Math::rect r = Math::quadratic_bounding_rect(p0, p1, p2);

        Math::min(rect.min, r.min, rect.min);
        Math::max(rect.max, r.max, rect.max);

        j += 2;

        break;
      }
      case Command::Line: {
        GK_ASSERT(j > 0, "Line command cannot be the first command of a path.");
        GK_ASSERT(j < m_points.size(), "Not enough points for a line.");

        Math::min(rect.min, m_points[j], rect.min);
        Math::max(rect.max, m_points[j], rect.max);

        j += 1;

        break;
      }
      case Command::Move: {
        GK_ASSERT(j < m_points.size(), "Points vector subscript out of range.");

        Math::min(rect.min, m_points[j], rect.min);
        Math::max(rect.max, m_points[j], rect.max);

        j += 1;

        break;
      }
      }
    }

    return rect;
  }

  Math::rect Path::bounding_rect(const mat2x3& transform) const {
    if (empty()) {
      if (vacant()) return {};

      const vec2 p = transform * m_points[0];
      return { p, p };
    }

    Math::rect rect{};

    for (size_t i = 0, j = 0; i < m_commands_size; i++) {
      switch (get_command(i)) {
      case Command::Cubic: {
        GK_ASSERT(j > 0, "Cubic bezier command cannot be the first command of a path.");
        GK_ASSERT(j + 2 < m_points.size(), "Not enough points for a cubic bezier.");

        const vec2 p0 = transform * m_points[j - 1];
        const vec2 p1 = transform * m_points[j];
        const vec2 p2 = transform * m_points[j + 1];
        const vec2 p3 = transform * m_points[j + 2];

        Math::rect r = Math::cubic_bounding_rect(p0, p1, p2, p3);

        Math::min(rect.min, r.min, rect.min);
        Math::max(rect.max, r.max, rect.max);

        j += 3;

        break;
      }
      case Command::Quadratic: {
        GK_ASSERT(j > 0, "Quadratic bezier command cannot be the first command of a path.");
        GK_ASSERT(j + 1 < m_points.size(), "Not enough points for a quadratic bezier.");

        const vec2 p0 = transform * m_points[j - 1];
        const vec2 p1 = transform * m_points[j];
        const vec2 p2 = transform * m_points[j + 1];

        Math::rect r = Math::quadratic_bounding_rect(p0, p1, p2);

        Math::min(rect.min, r.min, rect.min);
        Math::max(rect.max, r.max, rect.max);

        j += 2;

        break;
      }
      case Command::Line: {
        GK_ASSERT(j > 0, "Line command cannot be the first command of a path.");
        GK_ASSERT(j < m_points.size(), "Not enough points for a line.");

        const vec2 p1 = transform * m_points[j];

        Math::min(rect.min, p1, rect.min);
        Math::max(rect.max, p1, rect.max);

        j += 1;

        break;
      }
      case Command::Move: {
        GK_ASSERT(j < m_points.size(), "Points vector subscript out of range.");

        const vec2 p0 = transform * m_points[j];

        Math::min(rect.min, p0, rect.min);
        Math::max(rect.max, p0, rect.max);

        j += 1;

        break;
      }
      }
    }

    return rect;
  }

  Math::rect Path::approx_bounding_rect() const {
    if (empty()) {
      if (vacant()) return {};
      return { m_points[0], m_points[0] };
    }

    Math::rect rect{};

    for (vec2 p : m_points) {
      Math::min(rect.min, p, rect.min);
      Math::max(rect.max, p, rect.max);
    }

    return rect;
  }

  bool Path::is_point_inside_path(const vec2 point, const Fill* fill, const Stroke* stroke, const mat2x3& transform, const float threshold, const double zoom, const bool deep_search) const {
    GK_TOTAL("Path::is_point_inside_path");

    const Math::rect bounds = approx_bounding_rect();
    const bool consider_miters = stroke ? (stroke->join == LineJoin::Miter) && (stroke->width > threshold) : false;

    if (!Math::is_point_in_rect(Math::inverse(transform) * point, bounds, stroke ? 0.5f * stroke->width * (consider_miters ? stroke->miter_limit : 1.0f) + threshold : threshold)) return false;

    const Math::rect threshold_box = { point - threshold - GK_POINT_EPSILON / zoom, point + threshold + GK_POINT_EPSILON / zoom };
    const f24x8x2 p = { Math::float_to_f24x8(point.x), Math::float_to_f24x8(point.y) };

    PathBuilder builder{ threshold_box, dmat2x3(transform), GK_PATH_TOLERANCE / zoom };

    if (fill) {
      Drawable drawable = builder.fill(*this, *fill);

      for (Contour& contour : drawable.contours) {
        const int winding = contour.winding_of(p);

        if (
          (fill->rule == FillRule::NonZero && winding != 0) ||
          (fill->rule == FillRule::EvenOdd && winding % 2 != 0)
          ) {
          return true;
        }
      }
    }

    Stroke s = stroke ? *stroke : Stroke{ vec4{}, LineCap::Round, LineJoin::Round, 0.0f, 0.0f, 0.0f };
    s.width += threshold;

    if (!consider_miters) {
      s.miter_limit = 0.0f;
    }

    Drawable drawable = builder.stroke(*this, s);

    for (Contour& contour : drawable.contours) {
      const int winding = contour.winding_of(p);

      if (winding != 0) {
        return true;
      }
    }

    for (size_t point_index = 0; point_index < m_points.size(); point_index++) {
      if (Math::is_point_in_circle(point, transform * m_points[point_index], threshold)) {
        if (deep_search || point_index == 0) return true;

        for (size_t i = 0, point_i = 0; i < m_commands_size; i++) {
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

        return true;
      }
    }

    return false;
  }

  bool Path::is_point_inside_segment(const size_t segment_index, const vec2 point, const Stroke* stroke, const mat2x3& transform, const float threshold, const double zoom) const {
    const Segment segment = at(segment_index);
    const Math::rect bounds = segment.approx_bounding_rect();

    if (!Math::is_point_in_rect(Math::inverse(transform) * point, bounds, stroke ? 0.5f * stroke->width + threshold : threshold)) return false;

    const Math::rect threshold_box = { point - threshold - GK_POINT_EPSILON / zoom, point + threshold + GK_POINT_EPSILON / zoom };
    const f24x8x2 p = { Math::float_to_f24x8(point.x), Math::float_to_f24x8(point.y) };

    PathBuilder builder{ threshold_box, dmat2x3(transform), GK_PATH_TOLERANCE / zoom };

    Stroke s = stroke ? *stroke : Stroke{ vec4{}, LineCap::Butt, LineJoin::Bevel, 0.0f, 0.0f, 0.0f };
    s.width += threshold;

    Path segment_path;

    segment_path.move_to(segment.p0);

    switch (segment.type) {
    case Command::Cubic:
      segment_path.cubic_to(segment.p1, segment.p2, segment.p3);
      break;
    case Command::Quadratic:
      segment_path.quadratic_to(segment.p1, segment.p2);
      break;
    case Command::Line:
      segment_path.line_to(segment.p1);
      break;
    default:
      break;
    }

    Drawable drawable = builder.stroke(segment_path, s);

    for (Contour& contour : drawable.contours) {
      const int winding = contour.winding_of(p);

      if (winding != 0) {
        return true;
      }
    }

    return false;
  }

  bool Path::is_point_inside_point(const size_t point_index, const vec2 point, const mat2x3& transform, const float threshold) const {
    const vec2 p = transform * m_points[point_index];

    if (Math::is_point_in_circle(point, p, threshold)) {
      if (point_index == 0) return true;

      for (size_t i = 0, point_i = 0; i < m_commands_size; i++) {
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
            if (m_points[point_i] == m_points[point_i - 1] || m_points[point_i] == m_points[point_i + 1]) {
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
            if (m_points[point_i] == m_points[point_i - 1] || m_points[point_i] == m_points[point_i + 2]) {
              return false;
            }

            return true;
          } else if (point_i + 1 == point_index) {
            if (m_points[point_i + 1] == m_points[point_i - 1] || m_points[point_i + 1] == m_points[point_i + 2]) {
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

      return true;
    }
  }

  std::vector<uint8_t> Path::encode() const {
    if (vacant()) return {};

    std::vector<uint8_t> buffer(4 + m_commands.size() + 4 + m_points.size() * sizeof(vec2));

    buffer[0] = static_cast<uint8_t>(m_commands_size >> 24);
    buffer[1] = static_cast<uint8_t>(m_commands_size >> 16);
    buffer[2] = static_cast<uint8_t>(m_commands_size >> 8);
    buffer[3] = static_cast<uint8_t>(m_commands_size);

    for (size_t i = 0; i < m_commands.size(); i++) {
      buffer[i + 4] = m_commands[i];
    }

    buffer[m_commands.size() + 4] = static_cast<uint8_t>(m_points.size() >> 24);
    buffer[m_commands.size() + 5] = static_cast<uint8_t>(m_points.size() >> 16);
    buffer[m_commands.size() + 6] = static_cast<uint8_t>(m_points.size() >> 8);
    buffer[m_commands.size() + 7] = static_cast<uint8_t>(m_points.size());

    for (size_t i = 0; i < m_points.size(); i++) {
      const uint8_t* p = reinterpret_cast<const uint8_t*>(&m_points[i]);

      for (size_t j = 0; j < sizeof(vec2); j++) {
        buffer[m_commands.size() + 8 + i * sizeof(vec2) + j] = p[j];
      }
    }

    return buffer;
  }

  io::EncodedData& Path::encode(io::EncodedData& data) const {
    if (vacant()) return data.uint32(0);

    data.vector(m_commands);
    data.vector(m_points);
  }

  void Path::push_command(const Command command) {
    size_t rem = m_commands_size % 4;

    if (rem == 0) {
      m_commands.push_back(command << 6);
    } else {
      m_commands[m_commands_size / 4] |= command << (6 - rem * 2);
    }

    m_commands_size += 1;
  }

}
