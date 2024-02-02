/**
 * @file path.cpp
 * @brief Implementation of the Path class.
 *
 * @todo Implement history.
 * @todo Implement in and out handles.
 * @todo Better closing algorithm.
 * @todo is_point_in_path() should have a stroke of std::max(stroke_width, threshold).
 */

#include "path_dev.h"

#include "path_builder.h"

#include "../properties.h"
 // TEMP
#include "../renderer.h"
#include "internal.h"

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

  bool PathDev::Segment::is_point() const {
    bool point = p0 == p1;

    if (point) {
      if (is_quadratic()) return p1 == p2;
      if (is_cubic()) return p1 == p2 && p2 == p3;
    }

    return point;
  }

  /* -- Iterator -- */

  PathDev::Iterator::Iterator(const PathDev& path, const size_t index) : m_path(path), m_index(index), m_point_index(0) {
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

  PathDev::Iterator& PathDev::Iterator::operator++() {
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

  PathDev::Iterator PathDev::Iterator::operator++(int) {
    Iterator tmp = *this;
    ++(*this);

    return tmp;
  }

  PathDev::Iterator& PathDev::Iterator::operator--() {
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

  PathDev::Iterator PathDev::Iterator::operator--(int) {
    Iterator tmp = *this;
    --(*this);

    return tmp;
  }

  PathDev::Iterator::value_type PathDev::Iterator::operator*() const {
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

  PathDev::ReverseIterator::ReverseIterator(const PathDev& path, const size_t index) : m_path(path), m_index(index), m_point_index(0) {
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

  PathDev::ReverseIterator& PathDev::ReverseIterator::operator++() {
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

  PathDev::ReverseIterator PathDev::ReverseIterator::operator++(int) {
    ReverseIterator tmp = *this;
    ++(*this);

    return tmp;
  }

  PathDev::ReverseIterator& PathDev::ReverseIterator::operator--() {
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

  PathDev::ReverseIterator PathDev::ReverseIterator::operator--(int) {
    ReverseIterator tmp = *this;
    --(*this);

    return tmp;
  }

  PathDev::ReverseIterator::value_type PathDev::ReverseIterator::operator*() const {
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

  PathDev::PathDev() : m_points(), m_commands() {}

  PathDev::PathDev(const PathDev& other) : m_points(other.m_points), m_commands(other.m_commands), m_commands_size(other.m_commands_size) {}

  PathDev::PathDev(PathDev&& other) noexcept : m_points(std::move(other.m_points)), m_commands(std::move(other.m_commands)), m_commands_size(other.m_commands_size) {}

  PathDev& PathDev::operator=(const PathDev& other) {
    m_points = other.m_points;
    m_commands = other.m_commands;

    return *this;
  }

  PathDev& PathDev::operator=(PathDev&& other) noexcept {
    m_points = std::move(other.m_points);
    m_commands = std::move(other.m_commands);

    return *this;
  }

  PathDev::Segment PathDev::front(const size_t move_index) const {
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

  PathDev::Segment PathDev::back(const size_t move_index) const {
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

  void PathDev::for_each(
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

  void PathDev::for_each_reversed(
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

  bool PathDev::closed(const size_t move_index) const {
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

  void PathDev::move_to(const vec2 point) {
    if (!vacant() && get_command(m_commands_size - 1) == Command::Move) {
      m_points[m_points.size() - 1] = point;
      return;
    }

    m_points.push_back(point);
    push_command(Command::Move);
  }

  void PathDev::line_to(const vec2 point) {
    GK_ASSERT(!vacant(), "Cannot add a line to a vacant path.");

    m_points.push_back(point);
    push_command(Command::Line);
  }

  void PathDev::quadratic_to(const vec2 control, const vec2 point) {
    GK_ASSERT(!vacant(), "Cannot add a quadratic bezier to a vacant path.");

    m_points.insert(m_points.end(), { control, point });
    push_command(Command::Quadratic);
  }

  void PathDev::cubic_to(const vec2 control1, const vec2 control2, const vec2 point) {
    GK_ASSERT(!vacant(), "Cannot add a cubic bezier to a vacant path.");

    m_points.insert(m_points.end(), { control1, control2, point });
    push_command(Command::Cubic);
  }

  void PathDev::cubic_to(const vec2 control, const vec2 point, const bool is_control_1) {
    GK_ASSERT(!vacant(), "Cannot add a cubic bezier to a vacant path.");

    if (is_control_1) {
      m_points.insert(m_points.end(), { control, point, point });
    } else {
      m_points.insert(m_points.end(), { m_points.back(), control, point });
    }

    push_command(Command::Cubic);
  }

  void PathDev::arc_to(const vec2 center, const vec2 radius, const float x_axis_rotation, const bool large_arc_flag, const bool sweep_flag, const vec2 point) {
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

  void PathDev::ellipse(const vec2 center, const vec2 radius) {
    const vec2 top_left = center - radius;
    const vec2 bottom_right = center + radius;
    const vec2 cp = radius * GEOMETRY_CIRCLE_RATIO;

    move_to({ cp.x, top_left.y });
    cubic_to({ center.x + cp.x, top_left.y }, { bottom_right.x, center.y - cp.y }, { bottom_right.x, center.y });
    cubic_to({ bottom_right.x, center.y + cp.y }, { center.x + cp.x, bottom_right.y }, { center.x, bottom_right.y });
    cubic_to({ center.x - cp.x, bottom_right.y }, { top_left.x, center.y + cp.y }, { top_left.x, center.y });
    cubic_to({ top_left.x, center.y - cp.y }, { center.x - cp.x, top_left.y }, { center.x, top_left.y });
    close();
  }

  void PathDev::circle(const vec2 center, const float radius) {
    ellipse(center, { radius, radius });
  }

  void PathDev::rect(const vec2 point, const vec2 size, const bool centered) {
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

  void PathDev::round_rect(const vec2 point, const vec2 size, const float radius, const bool centered) {
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

  void PathDev::close() {
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

    m_points.push_back(p);
    push_command(Command::Line);
  }

  Math::rect PathDev::bounding_rect() const {
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

  Math::rect PathDev::bounding_rect(const mat2x3& transform) const {
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

  Math::rect PathDev::approx_bounding_rect() const {
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

  bool PathDev::is_point_inside_path(const vec2 point, const Fill* fill, const Stroke* stroke, const mat2x3& transform, const float threshold, const double zoom) const {
    GK_TOTAL("PathDev::is_point_inside_path");

    const Math::rect bounds = approx_bounding_rect();
    const bool consider_miters = stroke ? (stroke->join == LineJoin::Miter) && (stroke->width > threshold) : false;

    if (!Math::is_point_in_rect(inverse(transform) * point, bounds, stroke ? 0.5f * stroke->width * (consider_miters ? stroke->miter_limit : 1.0f) + threshold : threshold)) return false;

    const Math::rect threshold_box = { point - threshold - GK_POINT_EPSILON / zoom, point + threshold + GK_POINT_EPSILON / zoom };
    const f24x8x2 p = { Math::float_to_f24x8(point.x), Math::float_to_f24x8(point.y) };

    PathBuilder builder{ threshold_box, dmat2x3(transform), GK_PATH_TOLERANCE / zoom };

    if (fill) {
      Drawable drawable = builder.fill(*this, *fill);

      GK_DEBUGGER_DRAW(drawable);

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

    GK_DEBUGGER_DRAW(drawable);

    for (Contour& contour : drawable.contours) {
      const int winding = contour.winding_of(p);

      if (winding != 0) {
        return true;
      }
    }

    return false;
  }

  void PathDev::push_command(const Command command) {
    size_t rem = m_commands_size % 4;

    if (rem == 0) {
      m_commands.push_back(command << 6);
    } else {
      m_commands[m_commands_size / 4] |= command << (6 - rem * 2);
    }

    m_commands_size += 1;
  }

}
