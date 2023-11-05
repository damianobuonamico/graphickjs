#pragma once

#include "scalar.h"

#include "../utils/defines.h"

namespace Graphick::Math {

  inline rect rrect_to_rect(const rrect& r) {
    vec2 center = r.center();

    float sin = std::sinf(r.angle);
    float cos = std::cosf(r.angle);

    vec2 r1 = rotate(r.min, center, sin, cos);
    vec2 r2 = rotate({ r.min.x, r.max.y }, center, sin, cos);
    vec2 r3 = rotate(r.max, center, sin, cos);
    vec2 r4 = rotate({ r.max.x, r.min.y }, center, sin, cos);

    return {
      min(min(r1, r2), min(r3, r4)),
      max(max(r1, r2), max(r3, r4))
    };
  }

  inline rect straighten_rect(const rect& r) {
    return {
      min(r.min, r.max),
      max(r.min, r.max)
    };
  }

  inline bool is_point_in_circle(const vec2 point, const vec2 center, const float radius) {
    return squared_distance(point, center) <= radius * radius;
  }

  inline bool is_point_in_ellipse(const vec2 point, const vec2 center, const vec2 radius) {
    return std::pow((point.x - center.x), 2) / std::pow(radius.x, 2) + std::pow((point.y - center.y), 2) / std::pow(radius.y, 2) <= 1.0f;
  }

  inline bool is_point_in_rect(const vec2 point, const rect& rect, const float threshold = 0.0f) {
    return (
      point.x + threshold >= rect.min.x &&
      point.x - threshold <= rect.max.x &&
      point.y + threshold >= rect.min.y &&
      point.y - threshold <= rect.max.y
      );
  }

  inline bool is_point_in_rect(const vec2 point, const rect& rect, const vec2 threshold) {
    return (
      point.x + threshold.x >= rect.min.x &&
      point.x - threshold.x <= rect.max.x &&
      point.y + threshold.y >= rect.min.y &&
      point.y - threshold.y <= rect.max.y
      );
  }

  inline bool does_rect_intersect_rect(const rect& a, const rect& b) {
    return b.max.x >= a.min.x && a.max.x >= b.min.x && b.max.y >= a.min.y && a.max.y >= b.min.y;
  }

  inline bool is_rect_in_rect(const rect& a, const rect& b) {
    return a.min.x >= b.min.x && a.max.x <= b.max.x && a.min.y >= b.min.y && a.max.y <= b.max.y;
  }

  inline float rect_rect_intersection_area(const rect& a, const rect& b) {
    float x_left = std::max(a.min.x, b.min.x);
    float y_top = std::max(a.min.y, b.min.y);
    float x_right = std::min(a.max.x, b.max.x);
    float y_bottom = std::min(a.max.y, b.max.y);

    if (x_right < x_left || y_bottom < y_top) {
      return 0.0f;
    }

    return (x_right - x_left) * (y_bottom - y_top);
  }

  inline std::vector<float> line_line_intersections(const rect& a, const rect& b) {
    float den = b.max.x - b.min.x;

    if (is_almost_zero(den)) {
      float t = (b.min.x - a.min.x) / (a.max.x - a.min.x);
      if (t >= 0.0f && t <= 1.0f) {
        return { t };
      }

      return {};
    }

    float m = (b.max.y - b.min.y) / den;

    float t = (m * b.min.x - b.min.y + a.min.y - m * a.min.x) / (m * (a.max.x - a.min.x) + a.min.y - a.max.y);
    if (t >= 0.0f && t <= 1.0f) {
      return { t };
    }

    return {};
  }

  inline std::vector<vec2> line_line_intersection_points(const rect& a, const rect& b) {
    std::vector<float> values = line_line_intersections(a, b);
    std::vector<vec2> points{};
    rect rect = { min(b.min, b.max), max(b.min, b.max) };

    if (Math::is_almost_equal(b.min.x, b.max.x, GEOMETRY_MAX_INTERSECTION_ERROR)) {
      for (float value : values) {
        vec2 point = lerp(a.min, a.max, value);

        if (point.y >= rect.min.y && point.y <= rect.max.y) {
          points.push_back(point);
        }
      }
    } else if (Math::is_almost_equal(b.min.y, b.max.y, GEOMETRY_MAX_INTERSECTION_ERROR)) {
      for (float value : values) {
        vec2 point = lerp(a.min, a.max, value);

        if (point.x >= rect.min.x && point.x <= rect.max.x) {
          points.push_back(point);
        }
      }
    } else {
      for (float value : values) {
        vec2 point = lerp(a.min, a.max, value);

        if (is_point_in_rect(point, rect, GEOMETRY_MAX_INTERSECTION_ERROR)) {
          points.push_back(point);
        }
      }
    }

    return points;
  }

  inline vec2 line_line_fast_intersection_points(const rect& a, const rect& b) {
    rect rect = { min(b.min, b.max), max(b.min, b.max) };
    float den = b.max.x - b.min.x;

    if (is_almost_zero(den)) {
      float t = (b.min.x - a.min.x) / (a.max.x - a.min.x);
      if (t >= 0.0f && t <= 1.0f) {
        return lerp(a.min, a.max, t);
      }

      return { std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity() };
    }

    float m = (b.max.y - b.min.y) / den;

    float t = (m * b.min.x - b.min.y + a.min.y - m * a.min.x) / (m * (a.max.x - a.min.x) + a.min.y - a.max.y);
    if (t >= 0.0f && t <= 1.0f) {
      return lerp(a.min, a.max, t);
    }

    return { std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity() };
  }

  inline std::vector<vec2> line_circle_intersection_points(const rect& line, const vec2 center, const float radius) {
    vec2 ldir = line.max - line.min;
    vec2 tvec = line.min - center;

    const float a = squared_length(ldir);
    const float b = 2.0f * dot(ldir, tvec);
    const float c = squared_length(center) + squared_length(line.min) - (2.0f * dot(center, line.min)) - radius * radius;

    const float i = b * b - 4.0f * a * c;

    if ((i < 0.0f) || (a == 0.0f)) {
      return {};
    } else if (i == 0.0f) {
      const float mu = -b / (2.0f * a);
      return { ldir * mu + line.min };
    } else if (i > 0.0f) {
      const float i_sqrt = sqrt(i);

      float mu1 = (-b + i_sqrt) / (2.0f * a);
      float mu2 = (-b - i_sqrt) / (2.0f * a);

      return { ldir * mu1 + line.min, ldir * mu2 + line.min };
    } else {
      return {};
    }
  }

  inline std::vector<rect> lines_from_rect(const rect& rect) {
    return {
      { rect.min, { rect.max.x, rect.min.y }},
      { { rect.max.x, rect.min.y }, rect.max },
      { rect.max, { rect.min.x, rect.max.y } },
      { { rect.min.x, rect.max.y }, rect.min }
    };
  }

  inline vec2 circle_center(const vec2 a, const vec2 b, const vec2 c) {
    float offset = squared_length(b);
    float bc = 0.5f * (squared_length(a) - offset);
    float cd = 0.5f * (offset - (squared_length(c)));
    float det = (a.x - b.x) * (b.y - c.y) - (b.x - c.x) * (a.y - b.y);

    if (std::fabsf(det) < GK_EPSILON) {
      return { 0.0f, 0.0f };
    }

    float inverse_det = 1.0f / det;

    return {
      (bc * (b.y - c.y) - cd * (a.y - b.y)) * inverse_det,
      (cd * (a.x - b.x) - bc * (b.x - c.x)) * inverse_det
    };
  }

  inline bool clockwise(const std::vector<vec2>& points) {
    float sum = (points[0].x - points[points.size() - 1].x) * (points[0].y + points[points.size() - 1].y);

    for (size_t i = 0; i < points.size() - 1; i++) {
      sum += (points[i + 1].x - points[i].x) * (points[i + 1].y + points[i].y);
    }

    return sum >= 0.0f;
  }

  inline std::tuple<vec2, vec2, vec2, vec2, vec2> split_bezier(const vec2 p0, const vec2 p1, const vec2 p2, const vec2 p3, const float t) {
    vec2 p = bezier(p0, p1, p2, p3, t);

    vec2 q0 = lerp(p0, p1, t);
    vec2 q1 = lerp(p1, p2, t);
    vec2 q2 = lerp(p2, p3, t);

    vec2 r0 = lerp(q0, q1, t);
    vec2 r1 = lerp(q1, q2, t);

    return { p, q0, r0, r1, q2 };
  }

  // TODO: try inlining lerps
  inline std::tuple<vec2, vec2, vec2, vec2> split_bezier(const vec2 p0, const vec2 p1, const vec2 p2, const vec2 p3, const float t1, const float t2) {
    float a = t1;
    float b = t2;

    vec2 P000 = p0;
    vec2 P001 = p1;
    vec2 P011 = p2;
    vec2 P111 = p3;

    vec2 P00a = lerp(P000, P001, a);
    vec2 P00b = lerp(P000, P001, b);
    vec2 P01a = lerp(P001, P011, a);
    vec2 P01b = lerp(P001, P011, b);
    vec2 Pa11 = lerp(P011, P111, a);
    vec2 Pb11 = lerp(P011, P111, b);

    vec2 P0aa = lerp(P00a, P01a, a);
    vec2 P0bb = lerp(P00b, P01b, b);
    // vec2 P0ab = lerp(P00a, P01a, b);
    vec2 P1aa = lerp(P01a, Pa11, a);
    vec2 P1bb = lerp(P01b, Pb11, b);
    // vec2 P1ab = lerp(P01a, Pa11, b);

    vec2 Paaa = lerp(P0aa, P1aa, a);
    vec2 Paab = lerp(P0aa, P1aa, b);
    vec2 Pabb = lerp(P0bb, P1bb, a);
    vec2 Pbbb = lerp(P0bb, P1bb, b);

    return { Paaa, Paab, Pabb, Pbbb };
  }

  inline int hash(const std::initializer_list<float> floats) {
    int h = 1;

    for (float f : floats) {
      int i = *(int*)(&f);
      h = 31 * h + i;
    }

    h ^= (h >> 20) ^ (h >> 12);
    return h ^ (h >> 7) ^ (h >> 4);
  }

}
