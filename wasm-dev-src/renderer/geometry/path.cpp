#include "path.h"

#include "../../math/math.h"
#include "../../math/mat2.h"
#include "../../math/vector.h"

#include "../../utils/defines.h"
#include "../../utils/console.h"

namespace Graphick::Renderer::Geometry {

  void Path::move_to(vec2 p) {
    m_last_point = std::make_shared<ControlPoint>(p);
  }

  void Path::line_to(vec2 p) {
    Segment::ControlPointVertex point = std::make_shared<ControlPoint>(p);
    m_segments.emplace_back(m_last_point, point);
    m_last_point = point;
  }

  void Path::quadratic_to(vec2 p1, vec2 p2) {
    Segment::ControlPointVertex point = std::make_shared<ControlPoint>(p2);
    m_segments.emplace_back(m_last_point, p1, point, true);
    m_last_point = point;
  }

  void Path::cubic_to(vec2 p1, vec2 p2, vec2 p3) {
    Segment::ControlPointVertex point = std::make_shared<ControlPoint>(p3);
    m_segments.emplace_back(m_last_point, p1, p2, point);
    m_last_point = point;
  }

  void Path::arc_to(vec2 c, vec2 radius, float x_axis_rotation, bool large_arc_flag, bool sweep_flag, vec2 p) {
    float sin_th = std::sinf(Math::degrees_to_radians(x_axis_rotation));
    float cos_th = std::cosf(Math::degrees_to_radians(x_axis_rotation));

    vec2 d0 = (c - p) / 2.0f;
    vec2 d1 = {
      cos_th * d0.x + sin_th * d0.y,
      -sin_th * d0.x + cos_th * d0.y
    };

    vec2 sq_r = radius * radius;
    vec2 sq_p = d1 * d1;

    float check = sq_p.x / sq_r.x + sq_p.y / sq_r.y;
    if (check > 1.0f) {
      radius *= std::sqrtf(check);
    }

    mat2 a = {
      cos_th / radius.x, sin_th / radius.x,
      -sin_th / radius.y, cos_th / radius.y
    };
    vec2 p0 = {
      Math::dot(a[0], c),
      Math::dot(a[1], c)
    };
    vec2 p1 = {
      Math::dot(a[0], p),
      Math::dot(a[1], p)
    };

    float d = Math::squared_length(p1 - p0);
    float sfactor_sq = 1.0f / d - 0.25f;
    if (sfactor_sq < 0.0f) sfactor_sq = 0.0f;

    float sfactor = std::sqrt(sfactor_sq);
    if (sweep_flag == large_arc_flag) sfactor = -sfactor;

    vec2 c1 = {
      0.5f * (p0.x + p1.x) - sfactor * (p1.y - p0.y),
      0.5f * (p0.y + p1.y) + sfactor * (p1.x - p0.x)
    };

    float th0 = std::atan2f(p0.y - c1.y, p0.x - c1.x);
    float th1 = std::atan2f(p1.y - c1.y, p1.x - c1.x);
    float th_arc = th1 - th0;

    if (th_arc < 0.0f && sweep_flag) {
      th_arc += MATH_TWO_PI;
    } else if (th_arc > 0.0f && !sweep_flag) {
      th_arc -= MATH_TWO_PI;
    }

    int n_segs = static_cast<int>(std::ceil(std::fabs(th_arc / (MATH_PI * 0.5f + 0.001f))));
    for (int i = 0; i < n_segs; i++) {
      float th2 = th0 + i * th_arc / n_segs;
      float th3 = th0 + (i + 1) * th_arc / n_segs;

      a = {
        cos_th * radius.x, -sin_th * radius.x,
        sin_th * radius.y, cos_th * radius.y
      };

      float th_half = 0.5f * (th3 - th2);
      float sin_half_th_half = std::sinf(th_half * 0.5f);
      float t = (8.0f / 3.0f) * sin_half_th_half * sin_half_th_half / std::sin(th_half);

      float sin_th2 = std::sinf(th2);
      float cos_th2 = std::cosf(th2);
      float sin_th3 = std::sinf(th3);
      float cos_th3 = std::cosf(th3);

      p1 = {
        c1.x + cos_th2 - t * sin_th2,
        c1.y + sin_th2 + t * cos_th2
      };
      vec2 p3 = {
        c1.x + cos_th3,
        c1.y + sin_th3
      };
      vec2 p2 = {
        p3.x + t * sin_th3,
        p3.y - t * cos_th3
      };

      vec2 bez1 = {
        Math::dot(a[0], p1),
        Math::dot(a[1], p1)
      };
      vec2 bez2 = {
        Math::dot(a[0], p2),
        Math::dot(a[1], p2)
      };
      vec2 bez3 = {
        Math::dot(a[0], p3),
        Math::dot(a[1], p3)
      };

      cubic_to(bez1, bez2, bez3);
    }
  }

  void Path::ellipse(vec2 c, vec2 radius) {
    vec2 top_left = c - radius;
    vec2 bottom_right = c + radius;

    vec2 cp = radius * GEOMETRY_CIRCLE_RATIO;

    move_to({ cp.x, top_left.y });
    cubic_to({ c.x + cp.x, top_left.y }, { bottom_right.x, c.y - cp.y }, { bottom_right.x, c.y });
    cubic_to({ bottom_right.x, c.y + cp.y }, { c.x + cp.x, bottom_right.y }, { c.x, bottom_right.y });
    cubic_to({ c.x - cp.x, bottom_right.y }, { top_left.x, c.y + cp.y }, { top_left.x, c.y });
    cubic_to({ top_left.x, c.y - cp.y }, { c.x - cp.x, top_left.y }, { c.x, top_left.y });
    close();
  }

  void Path::circle(vec2 c, float radius) {
    ellipse(c, { radius, radius });
  }

  void Path::rect(vec2 p, vec2 size, bool centered) {
    if (centered) {
      p -= size * 0.5f;
    }

    move_to(p);
    line_to(p + vec2{ size.x, 0.0f });
    line_to(p + size);
    line_to(p + vec2{ 0.0f, size.y });
    close();
  }

  void Path::round_rect(vec2 p, vec2 size, float radius, bool centered) {
    if (centered) {
      p -= size * 0.5f;
    }

    if (radius > size.x * 0.5f) radius = size.x * 0.5f;
    if (radius > size.y * 0.5f) radius = size.y * 0.5f;

    move_to({ p.x + radius, p.y });
    line_to({ p.x + size.x - radius, p.y });
    cubic_to({ p.x + size.x - radius * GEOMETRY_CIRCLE_RATIO, p.y }, { p.x + size.x, p.y + radius * GEOMETRY_CIRCLE_RATIO }, { p.x + size.x, p.y + radius });
    line_to({ p.x + size.x, p.y + size.y - radius });
    cubic_to({ p.x + size.x, p.y + size.y - radius * GEOMETRY_CIRCLE_RATIO }, { p.x + size.x - radius * GEOMETRY_CIRCLE_RATIO, p.y + size.y }, { p.x + size.x - radius, p.y + size.y });
    line_to({ p.x + radius, p.y + size.y });
    cubic_to({ p.x + radius * GEOMETRY_CIRCLE_RATIO, p.y + size.y }, { p.x, p.y + size.y - radius * GEOMETRY_CIRCLE_RATIO }, { p.x, p.y + size.y - radius });
    line_to({ p.x, p.y + radius });
    cubic_to({ p.x, p.y + radius * GEOMETRY_CIRCLE_RATIO }, { p.x + radius * GEOMETRY_CIRCLE_RATIO, p.y }, { p.x + radius, p.y });
    close();
  }

  void Path::close() {
    if (m_segments.size() < 2) return;

    Segment& first_segment = m_segments.front();
    Segment& last_segment = m_segments.back();

    if (is_almost_equal(last_segment.p3(), first_segment.p0())) {
      if (first_segment.has_p1()) {
        last_segment.m_p3->set_relative_handle(first_segment.m_p1);
      }

      first_segment.m_p0 = last_segment.m_p3;
    } else {
      m_segments.emplace_back(m_last_point, first_segment.m_p0);
      m_last_point = m_segments.front().m_p0;
    }

    m_closed = true;
  }

  Math::rect Path::bounding_rect() const {
    Math::rect rect{};

    for (const auto& segment : m_segments) {
      Math::rect segment_rect = segment.bounding_rect();

      min(rect.min, segment_rect.min, rect.min);
      max(rect.max, segment_rect.max, rect.max);
    }

    return rect;
  }

  Math::rect Path::large_bounding_rect() const {
    Math::rect rect{};

    for (const auto& segment : m_segments) {
      Math::rect segment_rect = segment.large_bounding_rect();

      min(rect.min, segment_rect.min, rect.min);
      max(rect.max, segment_rect.max, rect.max);
    }

    return rect;
  }

  bool Path::is_inside(const vec2 position, bool lower_level, float threshold) const {
    if (!Math::is_point_in_rect(position, lower_level ? large_bounding_rect() : bounding_rect(), threshold)) {
      return false;
    }

    for (const Segment& segment : m_segments) {
      if (segment.is_inside(position, lower_level, threshold)) {
        return true;
      }
    }

    return false;
  }

  bool Path::intersects(const Math::rect& rect) const {
    Math::rect bounding_rect = this->bounding_rect();

    if (!Math::does_rect_intersect_rect(rect, bounding_rect)) return false;

    for (const Segment& segment : m_segments) {
      if (segment.intersects(rect)) return true;
    }

    return false;
  }

  bool Path::intersects(const Math::rect& rect, std::vector<uuid>& vertices) const {
    Math::rect bounding_rect = this->bounding_rect();

    if (!Math::does_rect_intersect_rect(rect, bounding_rect)) return false;

    bool found = false;
    for (const Segment& segment : m_segments) {
      if (segment.intersects(rect, found, vertices)) found = true;
    }

    return found;
  }

}
