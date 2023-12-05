/**
 * @file path.cpp
 * @brief Implementation of the Path class.
 *
 * @todo Implement history.
 * @todo Implement in and out handles.
 */

#include "path_dev.h"

#include "../../math/vector.h"
#include "../../math/scalar.h"
#include "../../math/mat2.h"

#include "../../utils/assert.h"

namespace Graphick::Renderer::Geometry {

  PathDev::PathDev() : m_points(), m_commands() {}

  PathDev::PathDev(const PathDev& other) : m_points(other.m_points), m_commands(other.m_commands) {}

  PathDev::PathDev(PathDev&& other) : m_points(std::move(other.m_points)), m_commands(std::move(other.m_commands)) {}

  PathDev& PathDev::operator=(const PathDev& other) {
    m_points = other.m_points;
    m_commands = other.m_commands;

    return *this;
  }

  PathDev& PathDev::operator=(PathDev&& other) {
    m_points = std::move(other.m_points);
    m_commands = std::move(other.m_commands);

    return *this;
  }

  void PathDev::move_to(const vec2 point) {
    GK_ASSERT(vacant(), "Cannot move to a point when the path is not vacant.");

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
    if (empty() || m_commands.empty() || (size() == 1 && get_command(1) == Command::Line)) {}

    m_points.push_back(m_points.front());
    push_command(Command::Line);
  }

  PathDev::Command PathDev::get_command(const size_t index) const {
    GK_ASSERT(index < m_commands_size, "Commands vector subscript out of range.");

    size_t rem = index % 4;

    return static_cast<Command>((m_commands[index / 4] >> (6 - rem * 2)) & 0b00000011);
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
