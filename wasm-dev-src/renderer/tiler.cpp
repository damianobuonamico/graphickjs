#include "tiler.h"

#include "geometry/path.h"
#include "geometry/contour.h"

#include "../math/mat2x3.h"
#include "../math/matrix.h"
#include "../math/math.h"

#include "../utils/console.h"

#include <algorithm>

// TODO: zoom and transform operations should use doubles
// TODO: fix right border of tiger (near min_y)

namespace Graphick::Renderer {

#define OPAQUE_AND_MASKED 0
#define SAMPLE_COUNT 16

  static constexpr float tolerance = 0.25f;

  static rect offset_line(const rect& line, const float distance) {
    // TODO: maybe check if line is actually a point
    return line + Math::normal(line.min, line.max) * distance;
  }

  // http://www.cs.swan.ac.uk/~cssimon/line_intersection.html
  static std::optional<float> intersection_t(const rect& line, const rect& other) {
    vec2 p0p1 = line.max - line.min;
    mat2 matrix = { other.max - other.min, -p0p1 };

    if (std::fabsf(Math::determinant(matrix) < GEOMETRY_MAX_INTERSECTION_ERROR)) {
      return std::nullopt;
    }

    return (Math::inverse(matrix) * (line.min - other.min)).y;
  }

  struct ContourSegment {
    const vec2 p0;
    const vec2 p1;
    const vec2 p2;
    const vec2 p3;

    bool is_linear;

    ContourSegment(const vec2 p0, const vec2 p3) :
      p0(p0),
      p1(p0),
      p2(p0),
      p3(p3),
      is_linear(true) {}

    ContourSegment(const vec2 p0, const vec2 p1, const vec2 p2, const vec2 p3) :
      p0(p0),
      p1(p1),
      p2(p2),
      p3(p3),
      is_linear(false) {}

    void offset(const float distance, const LineJoin join, Contour& contour) const {
      vec2 join_point = p0;

      if (Math::squared_distance(p0, p3) < GEOMETRY_BUTT_CAP_LENGTH * GEOMETRY_BUTT_CAP_LENGTH) {
        add_to_contour(distance, join, join_point, contour);
        return;
      }

      ContourSegment candidate = offset_once(distance);
      if (error_is_within_tolerance(candidate, distance)) {
        candidate.add_to_contour(distance, join, join_point, contour);
        return;
      }

      auto [before, after] = split(0.5f);

      before.offset(distance, join, contour);
      after.offset(distance, join, contour);
    }

    void add_to_contour(const float distance, const LineJoin join, const vec2 join_point, Contour& contour) const {
      // Add join if necessary.
      if (contour.might_need_join(join)) {
        vec2 p3 = this->p0;
        vec2 p4 = is_linear ? this->p3 : this->p1;

        contour.add_join(distance, join, join_point, { p4, p3 });
      }

      // Push segment.
      contour.push_segment(*this, UPDATE_BOUNDS | INCLUDE_FROM_POINT);
    }

    ContourSegment offset_once(const float distance) const {
      if (is_linear) {
        rect line = offset_line(rect{ p0, p3 }, distance);
        return ContourSegment{ line.min, line.max };
      }

      // TODO: quadratic

      if (p0 == p1) {
        rect segment_0 = { p0, p2 };
        rect segment_1 = { p2, p3 };

        segment_0 = offset_line(segment_0, distance);
        segment_1 = offset_line(segment_1, distance);

        auto intersection = intersection_t(segment_0, segment_1);

        vec2 ctrl = intersection.has_value() ? (Math::lerp(segment_0.min, segment_0.max, intersection.value())) : (Math::lerp(segment_0.max, segment_1.min, 0.5f));

        return ContourSegment{ segment_0.min, segment_0.min, ctrl, segment_1.max };
      }

      if (p2 == p3) {
        rect segment_0 = { p0, p1 };
        rect segment_1 = { p1, p3 };

        segment_0 = offset_line(segment_0, distance);
        segment_1 = offset_line(segment_1, distance);

        auto intersection = intersection_t(segment_0, segment_1);

        vec2 ctrl = intersection.has_value() ? (Math::lerp(segment_0.min, segment_0.max, intersection.value())) : (Math::lerp(segment_0.max, segment_1.min, 0.5f));

        return ContourSegment{ segment_0.min, ctrl, segment_1.max, segment_1.max };
      }

      rect segment_0 = { p0, p1 };
      rect segment_1 = { p1, p2 };
      rect segment_2 = { p2, p3 };

      segment_0 = offset_line(segment_0, distance);
      segment_1 = offset_line(segment_1, distance);
      segment_2 = offset_line(segment_2, distance);

      auto intersection_0 = intersection_t(segment_0, segment_1);
      auto intersection_1 = intersection_t(segment_1, segment_2);

      vec2 ctrl_0 = intersection_0.has_value() ? (Math::lerp(segment_0.min, segment_0.max, intersection_0.value())) : (Math::lerp(segment_0.max, segment_1.min, 0.5f));
      vec2 ctrl_1 = intersection_1.has_value() ? (Math::lerp(segment_1.min, segment_1.max, intersection_1.value())) : (Math::lerp(segment_1.max, segment_2.min, 0.5f));

      return ContourSegment{ segment_0.min, ctrl_0, ctrl_1, segment_2.max };
    }

    bool error_is_within_tolerance(const ContourSegment& other, const float distance) const {
      float min = std::fabsf(distance) - GEOMETRY_CURVE_ERROR;
      float max = std::fabsf(distance) + GEOMETRY_CURVE_ERROR;

      min = min <= 0.0f ? 0.0f : min * min;
      max = max <= 0.0f ? 0.0f : max * max;

      for (int t_num = 0; t_num < SAMPLE_COUNT + 1; t_num++) {
        float t = (float)t_num / SAMPLE_COUNT;

        // TODO: Use signed distance
        vec2 this_p = Math::lerp(p0, p3, t);
        vec2 other_p = Math::lerp(other.p0, other.p3, t);

        vec2 vector = this_p - other_p;
        float square_distance = Math::squared_length(vector);

        if (square_distance < min || square_distance > max) {
          return false;
        }
      }

      return true;
    }

    std::pair<ContourSegment, ContourSegment> split(const float t) const {
      if (is_linear) {
        vec2 p = Math::lerp(p0, p3, t);

        return {
          ContourSegment{ p0, p },
          ContourSegment{ p, p3 }
        };
      }

      rect baseline0, baseline1, ctrl0, ctrl1;

      if (t <= 0.0f) {
        vec2 from = p0;

        baseline0 = { from, from };
        ctrl0 = { from, from };
        baseline1 = { p0, p3 };
        ctrl1 = { p1, p2 };
      } else if (t >= 1.0f) {
        vec2 to = p3;

        baseline0 = { p0, p3 };
        ctrl0 = { p1, p2 };
        baseline1 = { to, to };
        ctrl1 = { to, to };
      } else {
        vec2 p01 = Math::lerp(p0, p1, t);
        vec2 p12 = Math::lerp(p1, p2, t);
        vec2 p23 = Math::lerp(p2, p3, t);

        vec2 p012 = Math::lerp(p01, p12, t);
        vec2 p123 = Math::lerp(p12, p23, t);

        vec2 p0123 = Math::lerp(p012, p123, t);

        baseline0 = { p0, p0123 };
        ctrl0 = { p01, p012 };
        baseline1 = { p0123, p3 };
        ctrl1 = { p123, p23 };
      }

      return {
        ContourSegment{ baseline0.min, ctrl0.min, ctrl0.max, baseline0.max },
        ContourSegment{ baseline1.min, ctrl1.min, ctrl1.max, baseline1.max }
      };
    }
  };

  size_t Contour::len() const {
    return points.size();
  }

  bool Contour::might_need_join(const LineJoin join) const {
    if (len() < 2 || join == LineJoin::Bevel) return false;
    return true;
  }

  vec2 Contour::position_of_last(const size_t index) const {
    return points[len() - index];
  }

  void Contour::push_point(const vec2 point, const PointFlags flag, const bool update_bounds) {
    if (update_bounds) {
      Math::min(bounds.min, point);
      Math::max(bounds.max, point);
    }

    points.push_back(point);
    flags.push_back(flag);
  }

  void Contour::push_endpoint(const vec2 to) {
    push_point(to, PointFlags::NONE, true);
  }

  void Contour::push_segment(const ContourSegment& segment, const int flags) {
    bool update_bounds = flags & PushSegmentFlags::UPDATE_BOUNDS;

    push_point(segment.p0, PointFlags::NONE, update_bounds);

    if (!segment.is_linear) {
      push_point(segment.p1, PointFlags::CONTROL_POINT_0, update_bounds);
      // TODO: quadratic segments
      push_point(segment.p2, PointFlags::CONTROL_POINT_1, update_bounds);
    }

    push_point(segment.p3, PointFlags::NONE, update_bounds);
  }

  void Contour::add_join(const float distance, const LineJoin join, const vec2 join_point, const rect next_tangent) {
    Math::rect prev_tangent = { position_of_last(2), position_of_last(1) };

    if (Math::squared_distance(prev_tangent.min, prev_tangent.max) < GEOMETRY_MAX_INTERSECTION_ERROR || Math::squared_distance(next_tangent.min, next_tangent.max) < GEOMETRY_MAX_INTERSECTION_ERROR) {
      return;
    }

    const float miter_limit = 10.0f;

    switch (join) {
    case LineJoin::Bevel:
      break;
    case LineJoin::Miter: {
      auto intersection = intersection_t(prev_tangent, next_tangent);
      if (intersection) {
        float prev_tangent_t = intersection.value();
        if (prev_tangent_t < -GEOMETRY_MAX_INTERSECTION_ERROR) {
          return;
        }

        vec2 miter_endpoint = Math::lerp(prev_tangent.min, prev_tangent.max, prev_tangent_t);
        float threshold = miter_limit * distance;
        if (Math::squared_distance(miter_endpoint, join_point) > threshold * threshold) {
          return;
        }

        push_endpoint(miter_endpoint);
      }

      break;
    }
    case LineJoin::Round:
      // TODO: implement
      break;
    }
  }

  static inline ivec2 tile_coords(const vec2 p) {
    return { (int)std::floor(p.x / TILE_SIZE), (int)std::floor(p.y / TILE_SIZE) };
  }

  static inline ivec2 tile_coords_clamp(const vec2 p, const ivec2 tiles_count) {
    return { std::clamp((int)std::floor(p.x / TILE_SIZE), 0, tiles_count.x - 1), std::clamp((int)std::floor(p.y / TILE_SIZE), 0, tiles_count.y - 1) };
  }

  static inline int tile_index(const ivec2 coords, const ivec2 tiles_count) {
    return coords.x + coords.y * tiles_count.x;
  }

  static inline int tile_index(const int16_t tile_x, const int16_t tile_y, const int16_t tiles_count_x) {
    return tile_x + tile_y * tiles_count_x;
  }

  static inline float x_intersect(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4) {
    float num = (x1 * y2 - y1 * x2) * (x3 - x4) -
      (x1 - x2) * (x3 * y4 - y3 * x4);
    float den = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    return num / den;
  }

  static inline float x_intersect(float one_over_m, float q, float y) {
    return (y - q) * one_over_m;
  }

  static inline float y_intersect(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4) {
    float num = (x1 * y2 - y1 * x2) * (y3 - y4) -
      (y1 - y2) * (x3 * y4 - y3 * x4);
    float den = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    return num / den;
  }

  static inline float y_intersect(float m, float q, float x) {
    return m * x + q;
  }

  static void clip_to_left(std::vector<vec2>& points, float x) {
    if (points.empty()) return;
    std::vector<vec2> new_points;

    for (int i = 0; i < points.size() - 1; i++) {
      vec2 point = points[i];

      if (point.x < x) {
        if (points[i + 1].x > x) {
          new_points.push_back({ x, y_intersect(x, -1000.0f, x, 1000.0f, point.x, point.y, points[i + 1].x, points[i + 1].y) });
        }
      } else {
        new_points.push_back(point);

        if (points[i + 1].x < x) {
          new_points.push_back({ x, y_intersect(x, -1000.0f, x, 1000.0f, point.x, point.y, points[i + 1].x, points[i + 1].y) });
        }
      }
    }

    if (new_points.size() > 2 && new_points.front() != new_points.back()) {
      new_points.push_back(new_points.front());
    }

    points = new_points;
  }

  static void clip_to_right(std::vector<vec2>& points, float x) {
    if (points.empty()) return;
    std::vector<vec2> new_points;

    for (int i = 0; i < points.size() - 1; i++) {
      vec2 point = points[i];

      if (point.x > x) {
        if (points[i + 1].x < x) {
          new_points.push_back({ x, y_intersect(x, -1000.0f, x, 1000.0f, point.x, point.y, points[i + 1].x, points[i + 1].y) });
        }
      } else {
        new_points.push_back(point);

        if (points[i + 1].x > x) {
          new_points.push_back({ x, y_intersect(x, -1000.0f, x, 1000.0f, point.x, point.y, points[i + 1].x, points[i + 1].y) });
        }
      }
    }

    if (new_points.size() > 2 && new_points.front() != new_points.back()) {
      new_points.push_back(new_points.front());
    }

    points = new_points;
  }

  static void clip_to_top(std::vector<vec2>& points, float y) {
    if (points.empty()) return;
    std::vector<vec2> new_points;

    for (int i = 0; i < points.size() - 1; i++) {
      vec2 point = points[i];

      if (point.y < y) {
        if (points[i + 1].y > y) {
          new_points.push_back({ x_intersect(-1000.0f, y, 1000.0f, y, point.x, point.y, points[i + 1].x, points[i + 1].y), y });
        }
      } else {
        new_points.push_back(point);

        if (points[i + 1].y < y) {
          new_points.push_back({ x_intersect(-1000.0f, y, 1000.0f, y, point.x, point.y, points[i + 1].x, points[i + 1].y), y });
        }
      }
    }

    if (new_points.size() > 2 && new_points.front() != new_points.back()) {
      new_points.push_back(new_points.front());
    }

    points = new_points;
  }

  static vec2 clip_to_bottom(std::vector<vec2>& points, float y) {
    vec2 min = std::numeric_limits<vec2>::max();

    if (points.empty()) return min;
    std::vector<vec2> new_points;

    for (int i = 0; i < points.size() - 1; i++) {
      vec2 point = points[i];

      if (point.y > y) {
        if (points[i + 1].y < y) {
          new_points.push_back({ x_intersect(-1000.0f, y, 1000.0f, y, point.x, point.y, points[i + 1].x, points[i + 1].y), y });
          min = Math::min(min, new_points.back());
        }
      } else {
        new_points.push_back(point);
        min = Math::min(min, new_points.back());

        if (points[i + 1].y > y) {
          new_points.push_back({ x_intersect(-1000.0f, y, 1000.0f, y, point.x, point.y, points[i + 1].x, points[i + 1].y), y });
          min = Math::min(min, new_points.back());
        }
      }
    }

    if (new_points.size() > 2 && new_points.front() != new_points.back()) {
      new_points.push_back(new_points.front());
      min = Math::min(min, new_points.back());
    }

    points = new_points;

    return min;
  }

  static vec2 clip(std::vector<vec2>& points, rect visible) {
    // OPTICK_EVENT();

    clip_to_left(points, visible.min.x);
    clip_to_right(points, visible.max.x);
    clip_to_top(points, visible.min.y);
    return clip_to_bottom(points, visible.max.y);
  }

  struct LinearOffsetSegment {
    vec2 a;
    vec2 d;
    vec2 a_normal;
    vec2 d_normal;
    vec2 d_pivot;

    LinearOffsetSegment(vec2 p0, vec2 p3, const float radius) {
      vec2 n = Math::normal(p0, p3);
      vec2 nr = n * radius;

      a = p0 + nr;
      d = p3 + nr;
      a_normal = n;
      d_normal = n;
      d_pivot = p3;
    }
  };

  struct CubicOffsetSegment {
    vec2 a;
    vec2 b;
    vec2 c;
    vec2 d;
    vec2 a_normal;
    vec2 d_normal;
    vec2 d_pivot;

    CubicOffsetSegment(const vec2 p0, const vec2 p1, const vec2 p2, const vec2 p3, const float radius) {
      const float EPS = 0.5f;

      vec2 normal_ab = Math::is_almost_equal(p0, p1, EPS) ? (
        Math::is_almost_equal(p0, p2) ? Math::normal(p0, p3) : Math::normal(p0, p2)
        ) : Math::normal(p0, p1);
      vec2 normal_bc = Math::is_almost_equal(p1, p2, EPS) ? (
        Math::is_almost_equal(p1, p3) ? Math::normal(p0, p3) : Math::normal(p1, p3)
        ) : Math::normal(p1, p2);
      vec2 normal_cd = Math::is_almost_equal(p2, p3, EPS) ? (
        Math::is_almost_equal(p1, p3) ? Math::normal(p0, p3) : Math::normal(p1, p3)
        ) : Math::normal(p2, p3);

      vec2 normal_b = normal_ab + normal_bc;
      vec2 normal_c = normal_cd + normal_bc;

      float dot = Math::dot(normal_ab, normal_bc);
      normal_b = Math::normalize_length(normal_b, std::sqrtf(radius / ((1.0f + dot) * 0.5f)));

      dot = Math::dot(normal_cd, normal_bc);
      normal_c = Math::normalize_length(normal_c, std::sqrtf(radius / ((1. + dot) * 0.5)));

      a = p0 + normal_ab * radius;
      d = p3 + normal_cd * radius;

      b = p1 + normal_b;
      c = p2 + normal_c;

      a_normal = normal_ab;
      d_normal = normal_cd;
      d_pivot = p3;
    }
  };

  PathTiler::PathTiler(const Geometry::Path& path, const mat2x3& transform, const vec4& color, const rect& visible, float zoom, ivec2 position, const std::vector<bool>& culled, const ivec2 tiles_count) :
    m_zoom(zoom),
    m_position(position),
    m_tile_y_prev(0)
  {
    // OPTICK_EVENT();
    GK_TOTAL("PathTiler::PathTiler");

    rect rect = transform * path.bounding_rect();

    float intersection_overlap = Math::rect_rect_intersection_area(rect, visible) / rect.area();
    if (intersection_overlap <= 0.0f) return;

    mat2x3 transform_zoom = transform * zoom;
    float zoom_factor = m_zoom / TILE_SIZE;

    rect.min -= 10.0f;
    rect.max += 10.0f;

    rect.min = floor(rect.min * zoom_factor) * TILE_SIZE;
    rect.max = ceil(rect.max * zoom_factor) * TILE_SIZE;

    ivec2 min_coords = tile_coords(rect.min) + m_position;
    ivec2 max_coords = tile_coords(rect.max) + m_position;

    m_offset = min_coords;
    m_bounds_size = max_coords - min_coords;

    const auto& segments = path.segments();

    m_prev = transform_zoom * segments.front().p0() - rect.min;

    if (intersection_overlap < 0.7f) {
      std::vector<vec2> points;
      points.reserve(segments.size() + 1);

      vec2 first_point = transform_zoom * segments.front().p0();
      points.push_back(first_point);

      Math::rect vis = visible * zoom;

      vis.min = floor(vis.min / TILE_SIZE) * TILE_SIZE - 1;
      vis.max = ceil(vis.max / TILE_SIZE) * TILE_SIZE + TILE_SIZE + 1;

      for (const auto& segment : segments) {
        vec2 p0 = transform_zoom * segment->p0();
        vec2 p3 = transform_zoom * segment->p3();

        if (segment->is_cubic()) {
          vec2 p1 = transform_zoom * segment->p1();
          vec2 p2 = transform_zoom * segment->p2();

          Math::rect segment_rect = transform * segment->bounding_rect();

          if (Math::does_rect_intersect_rect(segment_rect, visible)) {
            vec2 a = -1.0f * p0 + 3.0f * p1 - 3.0f * p2 + p3;
            vec2 b = 3.0f * (p0 - 2.0f * p1 + p2);

            float conc = std::max(Math::length(b), Math::length(a + b));
            float dt = std::sqrtf((std::sqrtf(8.0f) * tolerance) / conc);
            float t = 0.0f;

            while (t < 1.0f) {
              t = std::min(t + dt, 1.0f);

              vec2 p01 = Math::lerp(p0, p1, t);
              vec2 p12 = Math::lerp(p1, p2, t);
              vec2 p23 = Math::lerp(p2, p3, t);
              vec2 p012 = Math::lerp(p01, p12, t);
              vec2 p123 = Math::lerp(p12, p23, t);

              points.push_back(Math::lerp(p012, p123, t));
            }
          } else {
            points.push_back(p3);
          }
        } else if (segment->is_quadratic()) {
          vec2 p1 = transform_zoom * segment->p1();

          float dt = std::sqrtf((4.0f * tolerance) / Math::length(p0 - 2.0f * p1 + p3));
          float t = 0.0f;

          while (t < 1.0f) {
            t = std::min(t + dt, 1.0f);

            vec2 p01 = Math::lerp(p0, p1, t);
            vec2 p12 = Math::lerp(p1, p3, t);

            points.push_back(Math::lerp(p01, p12, t));
          }
        } else {
          points.push_back(p3);
        }
      }

      if (points.size() > 1 && points.front() != points.back()) {
        points.push_back(points.front());
      }

      vec2 min = clip(points, vis);
      if (points.empty()) return;

      min = floor(min / TILE_SIZE) * TILE_SIZE;

      for (int i = 0; i < points.size() - 1; i++) {
        process_linear_segment(points[i] - min, points[i + 1] - min);
      }

      m_offset = tile_coords(min) + m_position;
    } else {
      // TEMP: move outside of PathTiler, and change parameters from Path to Shape(array of points)
      // TODO: contours
#if 1
      Geometry::Contour contour;
      const float radius = 10.0f * zoom;

      contour.begin(transform_zoom * segments.front().p0() - rect.min);

      for (size_t i = 0; i < segments.size(); i++) {
        // LineJoin join = i == 0 ? LineJoin::Bevel : line_join;
        auto& raw_segment = segments.at(i);

        if (raw_segment.is_linear()) {
          contour.offset_segment(
            transform_zoom * raw_segment.p3() - rect.min,
            radius
          );
        } else {
          contour.offset_segment(
            transform_zoom * raw_segment.p1() - rect.min,
            transform_zoom * raw_segment.p2() - rect.min,
            transform_zoom * raw_segment.p3() - rect.min,
            radius
          );
        }
      }

      if (!path.closed()) {
        for (int i = segments.size() - 1; i >= 0; i--) {
          // LineJoin join = i == 0 ? LineJoin::Bevel : line_join;
          auto& raw_segment = segments.at(i);

          if (raw_segment.is_linear()) {
            contour.offset_segment(
              transform_zoom * raw_segment.p0() - rect.min,
              radius
            );
          } else {
            contour.offset_segment(
              transform_zoom * raw_segment.p2() - rect.min,
              transform_zoom * raw_segment.p1() - rect.min,
              transform_zoom * raw_segment.p0() - rect.min,
              radius
            );
          }
        }

        m_prev = contour.points[0];

        for (size_t i = 1; i < contour.points.size(); i++) {
          process_linear_segment(contour.points[i - 1], contour.points[i]);
        }

        process_linear_segment(contour.points.back(), contour.points.front());
      } else {
        Geometry::Contour inside_contour;

        inside_contour.begin(transform_zoom * segments.back().p3() - rect.min);

        for (size_t i = segments.size() - 1; i >= 0; i--) {
          // LineJoin join = i == 0 ? LineJoin::Bevel : line_join;
          auto& raw_segment = segments.at(i);

          if (raw_segment.is_linear()) {
            inside_contour.offset_segment(
              transform_zoom * raw_segment.p0() - rect.min,
              radius
            );
          } else {
            inside_contour.offset_segment(
              transform_zoom * raw_segment.p2() - rect.min,
              transform_zoom * raw_segment.p1() - rect.min,
              transform_zoom * raw_segment.p0() - rect.min,
              radius
            );
          }
        }

        m_prev = contour.points[0];

        for (size_t i = 1; i < contour.points.size(); i++) {
          process_linear_segment(contour.points[i - 1], contour.points[i]);
        }

        process_linear_segment(contour.points.back(), contour.points.front());

        m_prev = inside_contour.points[0];

        for (size_t i = 1; i < inside_contour.points.size(); i++) {
          process_linear_segment(inside_contour.points[i - 1], inside_contour.points[i]);
        }

        process_linear_segment(inside_contour.points.back(), inside_contour.points.front());
      }
#else

      return;

      Contour contour;

      const LineJoin line_join = LineJoin::Bevel;
      const float radius = 10.0f * zoom;

      for (size_t i = 0; i < segments.size(); i++) {
        LineJoin join = i == 0 ? LineJoin::Bevel : line_join;
        auto& raw_segment = segments.at(i);

        ContourSegment segment = raw_segment.is_linear() ? ContourSegment{
          transform_zoom * raw_segment.p0() - rect.min,
            transform_zoom * raw_segment.p3() - rect.min
        } : ContourSegment{
          transform_zoom * raw_segment.p0() - rect.min,
            transform_zoom * raw_segment.p1() - rect.min,
            transform_zoom * raw_segment.p2() - rect.min,
            transform_zoom * raw_segment.p3() - rect.min,
        };

        // ContourSegment segment = ContourSegment{
        //   transform_zoom * raw_segment.p0() - rect.min,
        //     transform_zoom * raw_segment.p3() - rect.min
        // };

        segment.offset(-radius, join, contour);
      }

      for (int i = segments.size() - 1; i >= 0; i--) {
        LineJoin join = i == 0 ? LineJoin::Bevel : line_join;
        auto& raw_segment = segments.at(i);

        ContourSegment segment = raw_segment.is_linear() ? ContourSegment{
          transform_zoom * raw_segment.p3() - rect.min,
            transform_zoom * raw_segment.p0() - rect.min
        } : ContourSegment{
          transform_zoom * raw_segment.p3() - rect.min,
            transform_zoom * raw_segment.p2() - rect.min,
            transform_zoom * raw_segment.p1() - rect.min,
            transform_zoom * raw_segment.p0() - rect.min,
        };

        // ContourSegment segment = ContourSegment{
        //      transform_zoom * raw_segment.p3() - rect.min,
        //        transform_zoom * raw_segment.p0() - rect.min
        // };

        segment.offset(-radius, join, contour);
      }

      m_prev = contour.points[0];

      int none_delta = 0;

      for (int i = 1; i < contour.len(); i++) {
        // if (contour.flags[i] & NONE) {
        //   if ()
        // }

        if (contour.flags[i] & PointFlags::CONTROL_POINT_0 || contour.flags[i] & PointFlags::CONTROL_POINT_1) {
          none_delta++;
          continue;
        }

        if (none_delta > 0) {
          process_cubic_segment(contour.points[i - 3], contour.points[i - 2], contour.points[i - 1], contour.points[i]);
        } else {
          process_linear_segment(contour.points[i - 1 - none_delta], contour.points[i]);
        }

        none_delta = 0;
      }
#endif

      finish(tiles_count);
      return;
    }

    finish(tiles_count);
  }

  void PathTiler::push_segment(const uvec4 segment, int16_t tile_x, int16_t tile_y) {
    if (tile_x >= m_bounds_size.x || tile_y >= m_bounds_size.y) return;

    int index = tile_index(tile_x, tile_y, m_bounds_size.x);

    auto& mask = m_masks[index];

    if (segment.y0 != segment.y1) {
      mask.segments.push_back(segment);
    }
  }

  inline static int16_t sign(float x) {
    return (0 < x) - (x < 0);
  }

  void PathTiler::process_linear_segment(const vec2 p0, const vec2 p3) {
    if (Math::is_almost_equal(p0, p3)) return;

    float x_vec = p3.x - p0.x;
    float y_vec = p3.y - p0.y;

    int16_t x_dir = sign(x_vec);
    int16_t y_dir = sign(y_vec);

    int16_t x_tile_dir = x_dir * TILE_SIZE;
    int16_t y_tile_dir = y_dir * TILE_SIZE;

    float dtdx = (float)TILE_SIZE / (x_vec);
    float dtdy = (float)TILE_SIZE / (y_vec);

    int16_t x = (int16_t)std::floor(p0.x);
    int16_t y = (int16_t)std::floor(p0.y);

    m_prev = p3;
    m_tile_y_prev = y / TILE_SIZE;

    float row_t1 = std::numeric_limits<float>::infinity();
    float col_t1 = std::numeric_limits<float>::infinity();

    int16_t tile_x = x / TILE_SIZE;
    int16_t tile_y = y / TILE_SIZE;

    if (p0.y != p3.y) {
      float next_y = (float)(tile_y + (p3.y > p0.y ? 1 : 0)) * TILE_SIZE;
      row_t1 = std::min(1.0f, (next_y - p0.y) / (y_vec));
    }
    if (p0.x != p3.x) {
      float next_x = (float)(tile_x + (p3.x > p0.x ? 1 : 0)) * TILE_SIZE;
      col_t1 = std::min(1.0f, (next_x - p0.x) / (x_vec));
    }

    float x_step = std::abs(dtdx);
    float y_step = std::abs(dtdy);

    vec2 from = p0;
    float max_over_tile_size = 255.0f / TILE_SIZE;

    while (true) {
      float t1 = std::min(row_t1, col_t1);

      vec2 to = lerp(p0, p3, t1);

      if (tile_x != m_bin.tile_x || tile_y != m_bin.tile_y) {
        m_bins.push_back(m_bin);
        m_bin = { tile_x, tile_y };
      }

      vec2 tile_pos = TILE_SIZE * vec2{ (float)tile_x, (float)tile_y };
      vec2 from_delta = from - tile_pos;
      vec2 to_delta = to - tile_pos;

      push_segment({
        (uint8_t)std::round(from_delta.x * max_over_tile_size),
        (uint8_t)std::round(from_delta.y * max_over_tile_size),
        (uint8_t)std::round(to_delta.x * max_over_tile_size),
        (uint8_t)std::round(to_delta.y * max_over_tile_size),
        }, tile_x, tile_y);

      bool fuzzy_equal;

      if (row_t1 < col_t1) {
        fuzzy_equal = row_t1 >= 1.0f - 0.0001f;
        row_t1 = std::min(1.0f, row_t1 + y_step);

        y += y_tile_dir;
        tile_y += y_dir;
      } else {
        fuzzy_equal = col_t1 >= 1.0f - 0.0001f;
        col_t1 = std::min(1.0f, col_t1 + x_step);

        x += x_tile_dir;
        tile_x += x_dir;
      }

      if (fuzzy_equal) {
        x = (int16_t)std::floor(p3.x);
        y = (int16_t)std::floor(p3.y);

        tile_x = x / TILE_SIZE;
        tile_y = y / TILE_SIZE;
      }

      from = to;

      if (tile_y != m_tile_y_prev) {
        m_tile_increments.push_back(Increment{ tile_x, std::min(tile_y, m_tile_y_prev), (int8_t)(tile_y - m_tile_y_prev) });
        m_tile_y_prev = tile_y;
      }

      if (fuzzy_equal) break;
    }
  }

  void PathTiler::process_quadratic_segment(const vec2 p0, const vec2 p1, const vec2 p3) {
    float dt = std::sqrtf((4.0f * tolerance) / Math::length(p0 - 2.0f * p1 + p3));
    float t = 0.0f;

    while (t < 1.0f) {
      t = std::min(t + dt, 1.0f);

      vec2 p01 = Math::lerp(p0, p1, t);
      vec2 p12 = Math::lerp(p1, p3, t);

      process_linear_segment(m_prev, Math::lerp(p01, p12, t));
    }
  }

  void PathTiler::process_cubic_segment(const vec2 p0, const vec2 p1, const vec2 p2, const vec2 p3) {
    // OPTICK_EVENT();

    vec2 a = -1.0f * p0 + 3.0f * p1 - 3.0f * p2 + p3;
    vec2 b = 3.0f * (p0 - 2.0f * p1 + p2);

    float conc = std::max(Math::length(b), Math::length(a + b));
    float dt = std::sqrtf((std::sqrtf(8.0f) * tolerance) / conc);
    float t = 0.0f;

    while (t < 1.0f) {
      t = std::min(t + dt, 1.0f);

      vec2 p01 = Math::lerp(p0, p1, t);
      vec2 p12 = Math::lerp(p1, p2, t);
      vec2 p23 = Math::lerp(p2, p3, t);
      vec2 p012 = Math::lerp(p01, p12, t);
      vec2 p123 = Math::lerp(p12, p23, t);

      process_linear_segment(m_prev, Math::lerp(p012, p123, t));
    }
  }

  void PathTiler::finish(const ivec2 tiles_count) {
    m_bins.push_back(m_bin);

    {
      std::sort(m_bins.begin(), m_bins.end(), [](const Bin& a, const Bin& b) {
        return a.tile_y < b.tile_y || (a.tile_y == b.tile_y && a.tile_x < b.tile_x);
        });
    }

    {
      std::sort(m_tile_increments.begin(), m_tile_increments.end(), [](const Increment& a, const Increment& b) {
        return a.tile_y < b.tile_y || (a.tile_y == b.tile_y && a.tile_x < b.tile_x);
        });
    }

    int tile_increments_i = 0;
    int winding = 0;
    size_t bins_len = m_bins.size();
    ivec2 prev_coords = { -1, -1 };
    float cover_table[TILE_SIZE] = { 0.0f };

    for (size_t i = 0; i < bins_len; i++) {
      const Bin& bin = m_bins[i];
      ivec2 coords = { bin.tile_x, bin.tile_y };

      if (coords != prev_coords) {
        if (coords.y != prev_coords.y) {
          memset(cover_table, 0, TILE_SIZE * sizeof(float));
        }

        int index = tile_index({ bin.tile_x, bin.tile_y }, m_bounds_size);
        if (index < 0 || index >= m_bounds_size.x * m_bounds_size.y) continue;
        // const auto& it = m_masks.find(index);

        // if (it != m_masks.end()) {
        Mask& mask = m_masks[index];

        memcpy(mask.cover_table, cover_table, TILE_SIZE * sizeof(float));

        {
          // GK_TOTAL("Calculate Cover");
          // TODO: Optimize further if possible, maybe use SIMD

          float tile_size_over_255 = (float)TILE_SIZE / 255.0f;

          for (auto& segment : mask.segments) {
            float p0_y = tile_size_over_255 * (float)segment.y0;
            float p1_y = tile_size_over_255 * (float)segment.y1;

            /* Segment is always on the left of the tile so we don't need to check x */
            for (int j = 0; j < TILE_SIZE; j++) {
              float y0 = (float)j;
              float y1 = y0 + 1.0f;

              cover_table[j] += std::clamp(p1_y, y0, y1) - std::clamp(p0_y, y0, y1);
            }
          }
        }

        prev_coords = coords;
        // }
      }

      if (i + 1 == bins_len || m_bins[i + 1].tile_x != bin.tile_x || m_bins[i + 1].tile_y != bin.tile_y) {
        uint8_t tile[TILE_SIZE * TILE_SIZE] = { 0 };

        ivec2 coords = { bin.tile_x + m_offset.x + 1, bin.tile_y + m_offset.y + 1 };
        int index = tile_index(coords, tiles_count);

        if (i + 1 < m_bins.size() && m_bins[i + 1].tile_y == bin.tile_y && m_bins[i + 1].tile_x > bin.tile_x + 1) {
          while (tile_increments_i < m_tile_increments.size()) {
            Increment& tile_increment = m_tile_increments[tile_increments_i];
            if (std::tie(tile_increment.tile_y, tile_increment.tile_x) > std::tie(bin.tile_y, bin.tile_x)) break;

            winding += tile_increment.sign;
            tile_increments_i++;
          }

          // Non-zero winding rule
          if (winding != 0) {
            // Even-odd winding rule
          // if (winding % 2 != 0) {
            int16_t width = m_bins[i + 1].tile_x - bin.tile_x - 1;
            m_spans.push_back(Span{ (int16_t)(bin.tile_x + 1), bin.tile_y, width });
          }
        }
      }
    }
  }

  /* -- DrawableTiler -- */

  DrawableTiler::DrawableTiler(const Drawable& drawable, const rect& visible, const float zoom, const ivec2 position, const vec2 subpixel, const ivec2 tiles_count) {
    rect bounds = {
      Math::floor((drawable.bounds.min - subpixel - 1.0f) / TILE_SIZE) * TILE_SIZE,
      Math::ceil((drawable.bounds.max - subpixel + 1.0f) / TILE_SIZE) * TILE_SIZE
    };

    ivec2 min_coords = tile_coords(bounds.min) + position;
    ivec2 max_coords = tile_coords(bounds.max) + position;

    bounds += subpixel;

    m_offset = min_coords;
    m_size = max_coords - min_coords;

    for (const Geometry::Contour& contour : drawable.contours) {
      if (contour.points.size() < 2) continue;

      move_to(contour.points.front() - bounds.min);

      for (size_t i = 1; i < contour.points.size(); i++) {
        line_to(contour.points[i] - bounds.min);
      }
    }

    pack(drawable.paint.rule, tiles_count);
  }

  void DrawableTiler::move_to(const vec2 p0) {
    m_p0 = p0;
  }

  void DrawableTiler::line_to(const vec2 p3) {
    if (Math::is_almost_equal(m_p0, p3)) return;

    vec2 p0 = m_p0;

    float x_vec = p3.x - p0.x;
    float y_vec = p3.y - p0.y;

    int16_t x_dir = sign(x_vec);
    int16_t y_dir = sign(y_vec);

    int16_t x_tile_dir = x_dir * TILE_SIZE;
    int16_t y_tile_dir = y_dir * TILE_SIZE;

    float dtdx = (float)TILE_SIZE / (x_vec);
    float dtdy = (float)TILE_SIZE / (y_vec);

    int16_t x = (int16_t)std::floor(p0.x);
    int16_t y = (int16_t)std::floor(p0.y);

    m_p0 = p3;
    m_tile_y_prev = y / TILE_SIZE;

    float row_t1 = std::numeric_limits<float>::infinity();
    float col_t1 = std::numeric_limits<float>::infinity();

    int16_t tile_x = x / TILE_SIZE;
    int16_t tile_y = y / TILE_SIZE;

    if (p0.y != p3.y) {
      float next_y = (float)(tile_y + (p3.y > p0.y ? 1 : 0)) * TILE_SIZE;
      row_t1 = std::min(1.0f, (next_y - p0.y) / (y_vec));
    }
    if (p0.x != p3.x) {
      float next_x = (float)(tile_x + (p3.x > p0.x ? 1 : 0)) * TILE_SIZE;
      col_t1 = std::min(1.0f, (next_x - p0.x) / (x_vec));
    }

    float x_step = std::abs(dtdx);
    float y_step = std::abs(dtdy);

    vec2 from = p0;
    float max_over_tile_size = 255.0f / TILE_SIZE;

    while (true) {
      float t1 = std::min(row_t1, col_t1);

      vec2 to = lerp(p0, p3, t1);

      if (tile_x != m_bin.tile_x || tile_y != m_bin.tile_y) {
        m_bins.push_back(m_bin);
        m_bin = { tile_x, tile_y };
      }

      vec2 tile_pos = TILE_SIZE * vec2{ (float)tile_x, (float)tile_y };
      vec2 from_delta = from - tile_pos;
      vec2 to_delta = to - tile_pos;

      push_segment({
        (uint8_t)std::round(from_delta.x * max_over_tile_size),
        (uint8_t)std::round(from_delta.y * max_over_tile_size),
        (uint8_t)std::round(to_delta.x * max_over_tile_size),
        (uint8_t)std::round(to_delta.y * max_over_tile_size),
        }, tile_x, tile_y);

      bool fuzzy_equal;

      if (row_t1 < col_t1) {
        fuzzy_equal = row_t1 >= 1.0f - 0.0001f;
        row_t1 = std::min(1.0f, row_t1 + y_step);

        y += y_tile_dir;
        tile_y += y_dir;
      } else {
        fuzzy_equal = col_t1 >= 1.0f - 0.0001f;
        col_t1 = std::min(1.0f, col_t1 + x_step);

        x += x_tile_dir;
        tile_x += x_dir;
      }

      if (fuzzy_equal) {
        x = (int16_t)std::floor(p3.x);
        y = (int16_t)std::floor(p3.y);

        tile_x = x / TILE_SIZE;
        tile_y = y / TILE_SIZE;
      }

      from = to;

      if (tile_y != m_tile_y_prev) {
        m_tile_increments.push_back(Increment{ tile_x, std::min(tile_y, m_tile_y_prev), (int8_t)(tile_y - m_tile_y_prev) });
        m_tile_y_prev = tile_y;
      }

      if (fuzzy_equal) break;
    }
  }

  void DrawableTiler::push_segment(const uvec4 segment, int16_t tile_x, int16_t tile_y) {
    if (tile_x >= m_size.x || tile_y >= m_size.y) return;

    int index = tile_index(tile_x, tile_y, m_size.x);

    auto& mask = m_masks[index];

    if (segment.y0 != segment.y1) {
      mask.segments.push_back(segment);
    }
  }

  void DrawableTiler::pack(const FillRule rule, const ivec2 tiles_count) {
    m_bins.push_back(m_bin);

    std::sort(m_bins.begin(), m_bins.end(), [](const Bin& a, const Bin& b) {
      return a.tile_y < b.tile_y || (a.tile_y == b.tile_y && a.tile_x < b.tile_x);
      });

    std::sort(m_tile_increments.begin(), m_tile_increments.end(), [](const Increment& a, const Increment& b) {
      return a.tile_y < b.tile_y || (a.tile_y == b.tile_y && a.tile_x < b.tile_x);
      });


    int tile_increments_i = 0;
    int winding = 0;
    size_t bins_len = m_bins.size();
    ivec2 prev_coords = { -1, -1 };
    float cover_table[TILE_SIZE] = { 0.0f };

    for (size_t i = 0; i < bins_len; i++) {
      const Bin& bin = m_bins[i];
      ivec2 coords = { bin.tile_x, bin.tile_y };

      if (coords != prev_coords) {
        if (coords.y != prev_coords.y) {
          memset(cover_table, 0, TILE_SIZE * sizeof(float));
        }

        int index = tile_index({ bin.tile_x, bin.tile_y }, m_size);
        if (index < 0 || index >= m_size.x * m_size.y) continue;

        Mask& mask = m_masks[index];

        memcpy(mask.cover_table, cover_table, TILE_SIZE * sizeof(float));

        // TODO: Optimize further if possible, maybe use SIMD

        float tile_size_over_255 = (float)TILE_SIZE / 255.0f;

        for (auto& segment : mask.segments) {
          float p0_y = tile_size_over_255 * (float)segment.y0;
          float p1_y = tile_size_over_255 * (float)segment.y1;

          /* Segment is always on the left of the tile so we don't need to check x */
          for (int j = 0; j < TILE_SIZE; j++) {
            float y0 = (float)j;
            float y1 = y0 + 1.0f;

            cover_table[j] += std::clamp(p1_y, y0, y1) - std::clamp(p0_y, y0, y1);
          }
        }

        prev_coords = coords;

        if (bin.tile_x < 0 || bin.tile_y < 0 || bin.tile_x > m_size.x || bin.tile_y > m_size.y) {
          //m_masks.erase(index);
        }
      }

      if (i + 1 == bins_len || m_bins[i + 1].tile_x != bin.tile_x || m_bins[i + 1].tile_y != bin.tile_y) {
        uint8_t tile[TILE_SIZE * TILE_SIZE] = { 0 };

        ivec2 coords = { bin.tile_x + m_offset.x + 1, bin.tile_y + m_offset.y + 1 };
        int index = tile_index(coords, tiles_count);

        if (i + 1 < m_bins.size() && m_bins[i + 1].tile_y == bin.tile_y && m_bins[i + 1].tile_x > bin.tile_x + 1) {
          while (tile_increments_i < m_tile_increments.size()) {
            Increment& tile_increment = m_tile_increments[tile_increments_i];
            if (std::tie(tile_increment.tile_y, tile_increment.tile_x) > std::tie(bin.tile_y, bin.tile_x)) break;

            winding += tile_increment.sign;
            tile_increments_i++;
          }

          if (
            (rule == FillRule::NonZero && winding != 0) ||
            (rule == FillRule::EvenOdd && winding % 2 != 0)
            ) {
            int16_t width = m_bins[i + 1].tile_x - bin.tile_x - 1;
            m_spans.push_back(Span{ (int16_t)(bin.tile_x + 1), bin.tile_y, width });
          }
        }
      }
    }
  }

  /* -- Tiler -- */

  Tiler::Tiler() {
    m_segments = new uint8_t[SEGMENTS_TEXTURE_SIZE * SEGMENTS_TEXTURE_SIZE * 4];
    m_cover_table = new float[SEGMENTS_TEXTURE_SIZE * SEGMENTS_TEXTURE_SIZE];
  }

  Tiler::~Tiler() {
    delete[] m_segments;
    delete[] m_cover_table;
  }

  void Tiler::reset(const Viewport& viewport) {
    m_tiles_count = { (int)(std::ceil((float)viewport.size.x / TILE_SIZE)) + 2, (int)(std::ceil((float)viewport.size.y / TILE_SIZE)) + 2 };
    m_position = {
      (int)(viewport.position.x > 0 ? std::floor(viewport.position.x * viewport.zoom / TILE_SIZE) : std::ceil(viewport.position.x * viewport.zoom / TILE_SIZE)),
      (int)(viewport.position.y > 0 ? std::floor(viewport.position.y * viewport.zoom / TILE_SIZE) : std::ceil(viewport.position.y * viewport.zoom / TILE_SIZE))
    };
    vec2 offset = {
      std::fmodf(viewport.position.x * viewport.zoom, TILE_SIZE),
      std::fmodf(viewport.position.y * viewport.zoom, TILE_SIZE)
    };
    m_subpixel = (viewport.position * viewport.zoom) % TILE_SIZE - offset;
    m_zoom = viewport.zoom;
    m_visible = { -viewport.position, vec2{(float)viewport.size.x / viewport.zoom, (float)viewport.size.y / viewport.zoom} - viewport.position };

    m_opaque_tiles.clear();
    m_masked_tiles.clear();

    m_segments_ptr = m_segments;
    m_cover_table_ptr = m_cover_table;

    m_culled_tiles = std::vector<bool>(m_tiles_count.x * m_tiles_count.y, false);
  }

  void Tiler::process_path(const Geometry::Path& path, const mat2x3& transform, const vec4& color, const float z_index) {
    GK_TOTAL("Tiler::process_path");

    PathTiler tiler(path, transform, color, m_visible, m_zoom, m_position, m_culled_tiles, m_tiles_count);

    const std::vector<PathTiler::Span>& spans = tiler.spans();
    const std::unordered_map<int, PathTiler::Mask>& masks = tiler.masks();
    const ivec2 offset = tiler.offset();
    const ivec2 size = tiler.size();

    for (const auto& [index, mask] : masks) {
      ivec2 coords = {
        index % size.x + offset.x + 1,
        index / size.x + offset.y + 1
      };

      if (coords.x < 0 || coords.y < 0 || coords.x >= m_tiles_count.x || coords.y >= m_tiles_count.y) continue;

      int absolute_index = tile_index(coords, m_tiles_count);
      if (m_culled_tiles[absolute_index]) continue;

      int offset = (int)(m_segments_ptr - m_segments) / 4;
      int cover_offset = (int)(m_cover_table_ptr - m_cover_table);

      uint32_t segments_size = (uint32_t)mask.segments.size();

      m_segments_ptr[0] = (uint8_t)segments_size;
      m_segments_ptr[1] = (uint8_t)(segments_size >> 8);
      m_segments_ptr[2] = (uint8_t)(segments_size >> 16);
      m_segments_ptr[3] = (uint8_t)(segments_size >> 24);
      m_segments_ptr += 4;

      for (auto segment : mask.segments) {
        m_segments_ptr[0] = segment.x0;
        m_segments_ptr[1] = segment.y0;
        m_segments_ptr[2] = segment.x1;
        m_segments_ptr[3] = segment.y1;
        m_segments_ptr += 4;
      }

      // TODO: bounds check
      memcpy(m_cover_table_ptr, &mask.cover_table, TILE_SIZE * sizeof(float));
      m_cover_table_ptr += TILE_SIZE;

      m_masked_tiles.push_back(MaskedTile{
        color,
        absolute_index,
        { (uint16_t)(offset % SEGMENTS_TEXTURE_SIZE), (uint16_t)(offset / SEGMENTS_TEXTURE_SIZE) },
        { (uint16_t)(cover_offset % SEGMENTS_TEXTURE_SIZE), (uint16_t)(cover_offset / SEGMENTS_TEXTURE_SIZE) },
        z_index
        });
    }

    for (auto& span : spans) {
      ivec2 coords = { span.tile_x + offset.x + 1, span.tile_y + offset.y + 1 };
      if (coords.x + span.width < 0 || coords.y < 0 || coords.x >= m_tiles_count.x || coords.y >= m_tiles_count.y) continue;

      int16_t width = coords.x < 0 ? span.width + coords.x : span.width;
      coords.x = std::max(coords.x, 0);

      for (int i = 0; i < width; i++) {
        if (coords.x + i >= m_tiles_count.x) break;

        int index = tile_index({ coords.x + i, coords.y }, m_tiles_count);
        if (!m_culled_tiles[index]/*&& color.a == 1.0f*/) {
          m_opaque_tiles.push_back({ color, index, z_index });
          m_culled_tiles[index] = true;
        }
      }
    }
  }

  struct QuadraticSolutions {
    uint8_t count;
    double solutions[2];

    QuadraticSolutions() : count(0), solutions{ 0.0f, 0.0f } {}
    QuadraticSolutions(const double x) : count(1), solutions{ x, 0.0f } {}
    QuadraticSolutions(const double x1, const double x2) : count(2), solutions{ x1, x2 } {}
  };

  struct CubicSolutions {
    uint8_t count;
    double solutions[3];

    CubicSolutions() : count(0), solutions{ 0.0f, 0.0f, 0.0f } {}
    CubicSolutions(const double x) : count(1), solutions{ x, 0.0f, 0.0f } {}
    CubicSolutions(const double x1, const double x2) : count(2), solutions{ x1, x2, 0.0f } {}
    CubicSolutions(const double x1, const double x2, const double x3) : count(3), solutions{ x1, x2, x3 } {}
    CubicSolutions(const QuadraticSolutions& quadratic) : count(quadratic.count), solutions{ quadratic.solutions[0], quadratic.solutions[1], 0.0f } {}
  };


  static inline double solve_linear(const double a, const double b) {
    /* Assuming a != 0 */
    return -b / a;
  }

  static QuadraticSolutions solve_quadratic(double a, double b, double c) {
    if (Math::is_almost_zero(a)) {
      /* It is a linear equation */

      return { solve_linear(b, c) };
    }

    double discriminant = b * b - 4.0 * a * c;

    if (Math::is_almost_zero(discriminant)) {
      /* One real root. */

      double root = -b / (2.0 * a);

      // TODO: ask if roots with multiplicity > 1 should be considered as separate roots
      return { root, root };
    } else if (discriminant < 0.0) {
      /* No real roots. */

      return {};
    }

    /* Two real roots. */

    double q = std::sqrt(discriminant);
    double a2 = 2.0 * a;

    return { (q - b) / a2, (-b - q) / a2 };
  }

  static CubicSolutions solve_cubic(double a, double b, double c, double d) {
    if (Math::is_almost_zero(a)) {
      /* It is a quadratic equation */

      return solve_quadratic(b, c, d);
    }

    if (Math::is_almost_zero(d)) {
      /* One root is 0. */

      CubicSolutions solutions = solve_quadratic(a, b, c);
      solutions.count++;
      return solutions;
    }

    double p = (3 * a * c - b * b) / (3 * a * a);
    /* Calculate coefficients of the depressed cubic equation: y^3 + py + q = 0 */
    // double q = (2 * b * b * b - 9 * a * b * c + 27 * a * a * d) / (27 * a * a * a);
    double q = (2 * b * b / a - 9 * c + 27 * a * d / b) / (27 * a * a / b);

    double discriminant = (q * q) / 4 + (p * p * p) / 27;
    /* Calculate discriminant */

    if (Math::is_almost_zero(discriminant)) {
      double u = std::cbrt(-q / 2);
      /* Three real roots, two of them are equal */
      double realRoot1 = 2 * u - b / (3 * a);
      double realRoot2 = -u - b / (3 * a);

      // TODO: ask if roots with multiplicity > 1 should be considered as different roots
      return { realRoot1, realRoot2, realRoot2 };
    } else if (discriminant > 0) {
      double u = std::cbrt(-q / 2 + std::sqrt(discriminant));
      /* One real root and two complex roots */
      double v = std::cbrt(-q / 2 - std::sqrt(discriminant));
      double realRoot = u + v - b / (3 * a);

      return { realRoot };
    } else {
      double phi = std::acos(-q / 2 * std::sqrt(-27 / (p * p * p)));
      double b1 = -b / (3.0 * a);
      double xi = 2.0 * std::sqrt(-p / 3);
      /* Three distinct real roots */
      double root1 = xi * std::cos(phi / 3) + b1;
      double root2 = xi * std::cos((phi + 2 * MATH_PI) / 3) + b1;
      double root3 = xi * std::cos((phi + 4 * MATH_PI) / 3) + b1;

      return { root1, root2, root3 };
    }
  }

  struct dvec2 {
    double x;
    double y;

    double operator[] (const int i) const {
      return i == 0 ? x : y;
    }

    dvec2 operator* (const double s) const {
      return { x * s, y * s };
    }

    dvec2 operator+ (const dvec2& v) const {
      return { x + v.x, y + v.y };
    }

    dvec2 operator+(const double s) const {
      return { x + s, y + s };
    }

    dvec2 operator- (const dvec2& v) const {
      return { x - v.x, y - v.y };
    }

    dvec2 operator- (const double s) const {
      return { x - s, y - s };
    }

    dvec2 operator- () const {
      return { -x, -y };
    }
  };

  dvec2 operator* (const double s, const dvec2& v) {
    return { v.x * s, v.y * s };
  }

  static std::vector<vec2> line_rect_intersection_points(const vec2 p0, const vec2 p3, const rect& rect) {
    std::vector<vec2> intersection_points;
    std::vector<double> intersections;

    dvec2 dp0 = { p0.x, p0.y };
    dvec2 dp3 = { p3.x, p3.y };

    dvec2 a = dp3 - dp0;

    double t1 = solve_linear(a.x, dp0.x - (double)rect.min.x);
    double t2 = solve_linear(a.x, dp0.x - (double)rect.max.x);
    double t3 = solve_linear(a.y, dp0.y - (double)rect.min.y);
    double t4 = solve_linear(a.y, dp0.y - (double)rect.max.y);

    if (t1 >= 0.0 && t1 <= 1.0) intersections.push_back(t1);
    if (t2 >= 0.0 && t2 <= 1.0) intersections.push_back(t2);
    if (t3 >= 0.0 && t3 <= 1.0) intersections.push_back(t3);
    if (t4 >= 0.0 && t4 <= 1.0) intersections.push_back(t4);

    if (intersections.empty()) return intersection_points;

    std::sort(intersections.begin(), intersections.end());

    for (double t : intersections) {
      dvec2 p = dp0 + (dp3 - dp0) * t;

      if (Math::is_point_in_rect({ (float)p.x, (float)p.y }, rect, GK_POINT_EPSILON)) {
        intersection_points.push_back({ (float)p.x, (float)p.y });
      }
    }

    return intersection_points;
  }

  static std::vector<vec3> bezier_rect_intersection_points(const vec2 p0, const vec2 p1, const vec2 p2, const vec2 p3, const rect& rect) {
    std::vector<vec3> intersection_points;
    std::vector<double> intersections;

    dvec2 dp0 = { p0.x, p0.y };
    dvec2 dp1 = { p1.x, p1.y };
    dvec2 dp2 = { p2.x, p2.y };
    dvec2 dp3 = { p3.x, p3.y };

    dvec2 a = -dp0 + 3.0 * dp1 - 3.0 * dp2 + dp3;
    dvec2 b = 3.0 * dp0 - 6.0 * dp1 + 3.0 * dp2;
    dvec2 c = -3.0 * dp0 + 3.0 * dp1;

    for (int j = 0; j < 2; j++) {
      for (int k = 0; k < 2; k++) {
        CubicSolutions roots = solve_cubic(a[k], b[k], c[k], dp0[k] - rect[j][k]);

        for (uint8_t i = 0; i < roots.count; i++) {
          double t = roots.solutions[i];

          if (t >= 0.0 && t <= 1.0) intersections.push_back(t);
        }
      }
    }

    if (intersections.empty()) return intersection_points;

    std::sort(intersections.begin(), intersections.end());


    for (double t : intersections) {
      double t_sq = t * t;
      dvec2 p = a * t_sq * t + b * t_sq + c * t + dp0;

      if (Math::is_point_in_rect({ (float)p.x, (float)p.y }, rect, GK_POINT_EPSILON)) {
        intersection_points.push_back({ (float)t, (float)p.x, (float)p.y });
      }
    }

    // intersection_points.erase(std::unique(intersection_points.begin(), intersection_points.end(), [](const vec3& l, const vec3& r) { return std::abs(l.y - r.y) < GEOMETRY_CURVE_ERROR && std::abs(l.z - r.z) < GEOMETRY_CURVE_ERROR; }), intersection_points.end());

    return intersection_points;
  }

  struct Segment {
    vec2 p0;
    vec2 p1;
    vec2 p2;
    vec2 p3;

    bool is_linear;

    Segment(vec2 p0, vec2 p3) : p0(p0), p3(p3), is_linear(true) {}
    Segment(vec2 p0, vec2 p1, vec2 p2, vec2 p3) : p0(p0), p1(p1), p2(p2), p3(p3), is_linear(false) {}
  };

  enum Bound {
    BoundTop,
    BoundRight,
    BoundBottom,
    BoundLeft,
    BoundNone
  };

  static std::vector<std::pair<vec2, Bound>> line_rect_intersection_points_bound(const vec2 p0, const vec2 p3, const rect& rect) {
    std::vector<std::pair<vec2, Bound>> intersection_points;
    std::vector<std::pair<double, Bound>> intersections;

    dvec2 dp0 = { p0.x, p0.y };
    dvec2 dp3 = { p3.x, p3.y };

    dvec2 a = dp3 - dp0;

    double t1 = solve_linear(a.x, dp0.x - (double)rect.min.x);
    double t2 = solve_linear(a.x, dp0.x - (double)rect.max.x);
    double t3 = solve_linear(a.y, dp0.y - (double)rect.min.y);
    double t4 = solve_linear(a.y, dp0.y - (double)rect.max.y);

    if (t1 >= 0.0 && t1 <= 1.0) intersections.emplace_back(t1, Bound::BoundLeft);
    if (t2 >= 0.0 && t2 <= 1.0) intersections.emplace_back(t2, Bound::BoundRight);
    if (t3 >= 0.0 && t3 <= 1.0) intersections.emplace_back(t3, Bound::BoundTop);
    if (t4 >= 0.0 && t4 <= 1.0) intersections.emplace_back(t4, Bound::BoundBottom);

    if (intersections.empty()) return intersection_points;

    std::sort(intersections.begin(), intersections.end());

    for (auto& [t, bound] : intersections) {
      dvec2 p = dp0 + (dp3 - dp0) * t;

      if (Math::is_point_in_rect({ (float)p.x, (float)p.y }, rect, GK_POINT_EPSILON)) {
        intersection_points.emplace_back(vec2{ (float)p.x, (float)p.y }, bound);
      }
    }

    return intersection_points;
  }


  static Drawable clip_drawable(const Drawable& drawable, const rect& clip) {
    // Drawable clipped(0, drawable.paint, { Math::max(drawable.bounds.min, clip.min), Math::min(drawable.bounds.max, clip.max) });
    Drawable clipped(0, drawable.paint, clip);

    for (auto& contour : drawable.contours) {
      Geometry::Contour& clipped_contour = clipped.contours.emplace_back();

#if 1
      std::vector<vec2> new_points = contour.points;
      /*vec2 p0 = contour.points.front();

      for (size_t i = 1; i < contour.points.size(); i++) {
        vec2 p3 = contour.points[i];

        new_points.push_back(p0);
        new_points.push_back(p3);
      }*/

      clip_to_left(new_points, clip.min.x);
      clip_to_top(new_points, clip.min.y);
      clip_to_bottom(new_points, clip.max.y);
      clip_to_right(new_points, clip.max.x);

      clipped_contour.points = new_points;
#else
      size_t len = contour.points.size();
      size_t start_i = 0;
      vec2 p0;
      bool p0_in;

      for (; start_i < len; start_i++) {
        p0 = contour.points[start_i];
        p0_in = Math::is_point_in_rect(p0, clip);

        if (p0_in) {
          if (start_i == 0) {
            start_i += 1;
          } else {
            p0 = contour.points[start_i - 1];
            p0_in = false;
          }

          break;
        }
      }

      // TODO: check const reference
      // TODO: in drawable check if adjacent points are equal

      if (p0_in) {
        clipped_contour.begin(p0);
      }

      Bound last_bound = Bound::BoundNone;

      for (size_t i = start_i; i < len; i++) {
        vec2 p3 = contour.points[i];
        bool p3_in = Math::is_point_in_rect(p3, clip);

        if (p0_in && p3_in) {
          /* Entire segment is visible. */

          clipped_contour.push_segment(p3);
          p0 = p3;
          p0_in = p3_in;
          continue;
        }

        std::vector<std::pair<vec2, Bound>> intersections = line_rect_intersection_points_bound(p0, p3, clip);

        if (intersections.empty()) {
          p0 = p3;
          p0_in = p3_in;
          continue;
        }

        // TODO: optimize away for loop, intersections are at most 2
        for (int k = 0; k < (int)intersections.size(); k++) {
          if (k % 2 == 0) {
            if (p0_in) {
              clipped_contour.push_segment(intersections[k].first);
              last_bound = intersections[k].second;
            } else {
              if (last_bound != Bound::BoundNone) {
                int bound_delta = intersections[k].second - last_bound;

                if (bound_delta < 0) {
                  for (int j = 0; j > bound_delta; j--) {
                    Bound bound = (Bound)((last_bound + j) % BoundNone);

                    switch (bound) {
                    case Bound::BoundTop:
                      clipped_contour.push_segment({ clip.min.x, clip.min.y });
                      break;
                    case Bound::BoundLeft:
                      clipped_contour.push_segment({ clip.min.x, clip.max.y });
                      break;
                    case Bound::BoundBottom:
                      clipped_contour.push_segment({ clip.max.x, clip.max.y });
                      break;
                    case Bound::BoundRight:
                      clipped_contour.push_segment({ clip.max.x, clip.min.y });
                      break;
                    }
                  }
                } else if (bound_delta > 0) {
                  for (int j = 0; j < bound_delta; j++) {
                    Bound bound = (Bound)((last_bound + j) % BoundNone);

                    switch (bound) {
                    case Bound::BoundTop:
                      clipped_contour.push_segment({ clip.max.x, clip.min.y });
                      break;
                    case Bound::BoundRight:
                      clipped_contour.push_segment({ clip.max.x, clip.max.y });
                      break;
                    case Bound::BoundBottom:
                      clipped_contour.push_segment({ clip.min.x, clip.max.y });
                      break;
                    case Bound::BoundLeft:
                      clipped_contour.push_segment({ clip.min.x, clip.min.y });
                      break;
                    }
                  }
                }
              }

              clipped_contour.push_segment(intersections[k].first);
              clipped_contour.push_segment((k >= intersections.size() - 1 ? p3 : intersections[k + 1].first));
            }
          }
        }

        p0 = p3;
        p0_in = p3_in;
      }

      if (!p0_in && start_i > 0 && start_i < len) {
        p0 = contour.points[start_i - 1];
        vec2 p3 = contour.points[start_i];

        std::vector<std::pair<vec2, Bound>> intersections = line_rect_intersection_points_bound(p0, p3, clip);

        for (int k = 0; k < (int)intersections.size(); k++) {
          if (k % 2 == 0) {

            if (last_bound != Bound::BoundNone) {
              int bound_delta = intersections[k].second - last_bound;

              if (bound_delta < 0) {
                for (int j = 0; j > bound_delta; j--) {
                  Bound bound = (Bound)((last_bound + j) % BoundNone);

                  switch (bound) {
                  case Bound::BoundTop:
                    clipped_contour.push_segment({ clip.min.x, clip.min.y });
                    break;
                  case Bound::BoundLeft:
                    clipped_contour.push_segment({ clip.min.x, clip.max.y });
                    break;
                  case Bound::BoundBottom:
                    clipped_contour.push_segment({ clip.max.x, clip.max.y });
                    break;
                  case Bound::BoundRight:
                    clipped_contour.push_segment({ clip.max.x, clip.min.y });
                    break;
                  }
                }
              } else if (bound_delta > 0) {
                for (int j = 0; j < bound_delta; j++) {
                  Bound bound = (Bound)((last_bound + j) % BoundNone);

                  switch (bound) {
                  case Bound::BoundTop:
                    clipped_contour.push_segment({ clip.max.x, clip.min.y });
                    break;
                  case Bound::BoundRight:
                    clipped_contour.push_segment({ clip.max.x, clip.max.y });
                    break;
                  case Bound::BoundBottom:
                    clipped_contour.push_segment({ clip.min.x, clip.max.y });
                    break;
                  case Bound::BoundLeft:
                    clipped_contour.push_segment({ clip.min.x, clip.min.y });
                    break;
                  }
                }
              }
            }
          }
        }
      }

#endif

      clipped_contour.close();
    }

    return clipped;
  }

  void Tiler::process_drawable(const Drawable& drawable, const rect& visible, const vec2 offset, const bool clip) {
    ivec2 tile_offset = tile_coords(offset);
    vec2 pixel_offset = offset - TILE_SIZE * vec2{ (float)tile_offset.x, (float)tile_offset.y };

    DrawableTiler tiler(clip ? clip_drawable(drawable, rect{ { -32.0f, -32.0f }, (visible.max - visible.min) * m_zoom + 32.0f }) : drawable, visible, m_zoom, m_position + tile_offset, m_subpixel - pixel_offset, m_tiles_count);

    const std::unordered_map<int, DrawableTiler::Mask>& masks = tiler.masks();
    const std::vector<DrawableTiler::Span>& spans = tiler.spans();
    const ivec2 tiler_offset = tiler.offset();
    const ivec2 size = tiler.size();

    for (const auto& [index, mask] : masks) {
      ivec2 coords = {
        index % size.x + tiler_offset.x + 1,
        index / size.x + tiler_offset.y + 1
      };

      if (coords.x < 0 || coords.y < 0 || coords.x >= m_tiles_count.x || coords.y >= m_tiles_count.y) continue;

      int absolute_index = tile_index(coords, m_tiles_count);
      if (m_culled_tiles[absolute_index]) continue;

      int offset = (int)(m_segments_ptr - m_segments) / 4;
      int cover_offset = (int)(m_cover_table_ptr - m_cover_table);

      uint32_t segments_size = (uint32_t)mask.segments.size();

      m_segments_ptr[0] = (uint8_t)segments_size;
      m_segments_ptr[1] = (uint8_t)(segments_size >> 8);
      m_segments_ptr[2] = (uint8_t)(segments_size >> 16);
      m_segments_ptr[3] = (uint8_t)(segments_size >> 24);
      m_segments_ptr += 4;

      for (auto segment : mask.segments) {
        m_segments_ptr[0] = segment.x0;
        m_segments_ptr[1] = segment.y0;
        m_segments_ptr[2] = segment.x1;
        m_segments_ptr[3] = segment.y1;
        m_segments_ptr += 4;
      }

      // TODO: bounds check
      memcpy(m_cover_table_ptr, &mask.cover_table, TILE_SIZE * sizeof(float));
      m_cover_table_ptr += TILE_SIZE;

      m_masked_tiles.push_back(MaskedTile{
        drawable.paint.color,
        absolute_index,
        { (uint16_t)(offset % SEGMENTS_TEXTURE_SIZE), (uint16_t)(offset / SEGMENTS_TEXTURE_SIZE) },
        { (uint16_t)(cover_offset % SEGMENTS_TEXTURE_SIZE), (uint16_t)(cover_offset / SEGMENTS_TEXTURE_SIZE) },
        drawable.paint.z_index
        });
    }

    for (auto& span : spans) {
      ivec2 coords = { span.tile_x + tiler_offset.x + 1, span.tile_y + tiler_offset.y + 1 };
      if (coords.x + span.width < 0 || coords.y < 0 || coords.x >= m_tiles_count.x || coords.y >= m_tiles_count.y) continue;

      int16_t width = coords.x < 0 ? span.width + coords.x : span.width;
      coords.x = std::max(coords.x, 0);

      for (int i = 0; i < width; i++) {
        if (coords.x + i >= m_tiles_count.x) break;

        int index = tile_index({ coords.x + i, coords.y }, m_tiles_count);
        if (!m_culled_tiles[index]/*&& color.a == 1.0f*/) {
          m_opaque_tiles.push_back({ drawable.paint.color, index, drawable.paint.z_index });
          m_culled_tiles[index] = true;
        }
      }
    }
  }

  void Tiler::process_stroke(const Geometry::Path& path, const mat2x3& transform, const Stroke& stroke) {
    GK_TOTAL("Tiler::process_stroke");
    const float radius = 0.5f * stroke.width * m_zoom;

    rect path_rect = transform * path.bounding_rect();

    path_rect.min -= 1.1f * 0.5f * stroke.width * (stroke.join == LineJoin::Miter ? stroke.miter_limit : 1.0f);
    path_rect.max += 1.1f * 0.5f * stroke.width * (stroke.join == LineJoin::Miter ? stroke.miter_limit : 1.0f);

    const float overlap = Math::rect_rect_intersection_area(path_rect, m_visible) / path_rect.area();
    if (overlap <= 0.0f) return;

    const mat2x3 transform_zoom = transform * m_zoom;
    const auto& segments = path.segments();

    const bool clip_drawable = stroke.width > std::min(m_visible.size().x, m_visible.size().y);

    // if (true) {
    // TODO: test and tweak overlap threshold
    if (overlap > 0.7f) {
      Drawable drawable(path.closed() ? 2 : 1, Paint{ stroke.color, FillRule::NonZero, stroke.z_index }, (path_rect - m_visible.min) * m_zoom);
      Geometry::Contour* contour = &drawable.contours.front();

      contour->begin((transform * segments.front().p0() - m_visible.min) * m_zoom, false);

      for (size_t i = 0; i < segments.size(); i++) {
        auto& raw_segment = segments.at(i);

        if (raw_segment.is_linear()) {
          // TODO: test const reference vs copy
          contour->offset_segment((transform * raw_segment.p3() - m_visible.min) * m_zoom, radius);
        } else {
          contour->offset_segment(
            (transform * raw_segment.p1() - m_visible.min) * m_zoom,
            (transform * raw_segment.p2() - m_visible.min) * m_zoom,
            (transform * raw_segment.p3() - m_visible.min) * m_zoom,
            radius
          );
        }
      }

      if (path.closed()) {
        contour->close();
        contour = &drawable.contours.back();
      }

      contour->begin((transform * segments.back().p3() - m_visible.min) * m_zoom, false);

      for (int i = static_cast<int>(segments.size()) - 1; i >= 0; i--) {
        auto& raw_segment = segments.at(i);

        if (raw_segment.is_linear()) {
          contour->offset_segment(
            (transform * raw_segment.p0() - m_visible.min) * m_zoom,
            radius
          );
        } else {
          contour->offset_segment(
            (transform * raw_segment.p2() - m_visible.min) * m_zoom,
            (transform * raw_segment.p1() - m_visible.min) * m_zoom,
            (transform * raw_segment.p0() - m_visible.min) * m_zoom,
            radius
          );
        }
      }

      contour->close();

      return process_drawable(drawable, m_visible, m_visible.min * m_zoom, clip_drawable);
    }

    // contour->begin(transform_zoom * segments.front().p0());

    size_t len = segments.size();
    size_t i = 0;

    // while (i < len) {
    //   if (Math::is_point_in_rect(segments.at(i).p0(), m_visible)) break;
    //   i++;
    // }

    // if (i >= len) return;

    std::vector<std::vector<Segment>> clipped_contours(1);

    // rect temp_visible = m_visible * m_zoom;

    // temp_visible.min -= 1.1f * radius * (stroke.join == LineJoin::Miter ? stroke.miter_limit : 1.0f);
    // temp_visible.max += 1.1f * radius * (stroke.join == LineJoin::Miter ? stroke.miter_limit : 1.0f);
    // temp_visible.min -= -32.0f;
    // temp_visible.max += -32.0f;

    // TODO: if only one clipped contour, close it

    rect visible = m_visible;

    // TODO: fix strokes disappearing
    visible.min -= 1.1f * 0.5f * stroke.width * (stroke.join == LineJoin::Miter ? stroke.miter_limit : 1.0f);
    visible.max += 1.1f * 0.5f * stroke.width * (stroke.join == LineJoin::Miter ? stroke.miter_limit : 1.0f);

    visible.min -= 32.0f / m_zoom;
    visible.max += 32.0f / m_zoom;

    for (size_t i = 0; i < len; i++) {
      auto& raw_segment = segments.at(i);

      vec2 p0 = transform * raw_segment.p0() - visible.min;
      vec2 p3 = transform * raw_segment.p3() - visible.min;

      bool p0_in = Math::is_point_in_rect(p0, visible - visible.min);
      bool p3_in = Math::is_point_in_rect(p3, visible - visible.min);

      if (raw_segment.is_linear()) {

        if (p0_in && p3_in) {
          /* Entire segment is visible. */

          clipped_contours.back().emplace_back(p0, p3);
          continue;
        }

        std::vector<vec2> intersections = line_rect_intersection_points(p0, p3, visible - visible.min);

        if (intersections.empty()) continue;

        for (int k = 0; k < (int)intersections.size(); k++) {
          if (k % 2 == 0) {
            if (p0_in) {
              clipped_contours.back().emplace_back((k < 1 ? p0 : intersections[k - 1]), intersections[k]);
            } else {
              if (!clipped_contours.back().empty()) clipped_contours.emplace_back();
              clipped_contours.back().emplace_back(intersections[k], (k >= intersections.size() - 1 ? p3 : intersections[k + 1]));
            }
          }
        }
      } else {
        vec2 p1 = transform * raw_segment.p1() - visible.min;
        vec2 p2 = transform * raw_segment.p2() - visible.min;

        // TODO: check if all inside or all outside with control points
        std::vector<vec3> intersections = bezier_rect_intersection_points(p0, p1, p2, p3, visible - visible.min);

        if (intersections.empty()) {
          if (p0_in) {
            /* Entire segment is visible. */

            clipped_contours.back().emplace_back(p0, p1, p2, p3);
          }

          /* Segment is completely outside. */

          continue;
        }

        for (int k = 0; k < (int)intersections.size(); k++) {
          if (k % 2 == 0) {
            if (p0_in) {
              auto segment = Math::split_bezier(
                p0,
                p1,
                p2,
                p3,
                (k < 1 ? 0.0f : intersections[k - 1].x),
                intersections[k].x
              );

              clipped_contours.back().emplace_back(std::get<0>(segment), std::get<1>(segment), std::get<2>(segment), std::get<3>(segment));
            } else {
              if (!clipped_contours.back().empty()) clipped_contours.emplace_back();

              auto segment = Math::split_bezier(
                p0,
                p1,
                p2,
                p3,
                intersections[k].x,
                (k >= intersections.size() - 1 ? 1.0f : intersections[k + 1].x)
              );

              clipped_contours.back().emplace_back(std::get<0>(segment), std::get<1>(segment), std::get<2>(segment), std::get<3>(segment));
            }
          } else if (p3_in && k == (int)intersections.size() - 1) {
            if (!clipped_contours.back().empty()) clipped_contours.emplace_back();

            auto segment = Math::split_bezier(
              p0,
              p1,
              p2,
              p3,
              intersections[k].x,
              (k >= intersections.size() - 1 ? 1.0f : intersections[k + 1].x)
            );

            clipped_contours.back().emplace_back(std::get<0>(segment), std::get<1>(segment), std::get<2>(segment), std::get<3>(segment));
          } else {
            if (!clipped_contours.back().empty()) clipped_contours.emplace_back();
          }
        }
      }
    }

    Drawable drawable(0, Paint{ stroke.color, FillRule::NonZero, stroke.z_index }, rect{});

    for (auto& clipped_contour : clipped_contours) {
      if (clipped_contour.empty()) continue;

      Geometry::Contour* contour = &drawable.contours.emplace_back();

      vec2 p0 = clipped_contour.front().p0 * m_zoom;

      contour->begin(p0, false);

      Math::min(drawable.bounds.min, p0, drawable.bounds.min);
      Math::max(drawable.bounds.max, p0, drawable.bounds.max);

      for (size_t i = 0; i < clipped_contour.size(); i++) {
        auto& segment = clipped_contour.at(i);

        vec2 p3 = segment.p3 * m_zoom;

        if (segment.is_linear) {
          // TODO: test const reference vs copy
          contour->offset_segment(p3, radius);
        } else {
          vec2 p1 = segment.p1 * m_zoom;
          vec2 p2 = segment.p2 * m_zoom;

          contour->offset_segment(
            p1,
            p2,
            p3,
            radius
          );

          // TODO: optimize away this operations (perform intersection of bounds with visible rect)
          Math::min(drawable.bounds.min, p1, drawable.bounds.min);
          Math::max(drawable.bounds.max, p1, drawable.bounds.max);

          Math::min(drawable.bounds.min, p2, drawable.bounds.min);
          Math::max(drawable.bounds.max, p2, drawable.bounds.max);
        }

        Math::min(drawable.bounds.min, p3, drawable.bounds.min);
        Math::max(drawable.bounds.max, p3, drawable.bounds.max);
      }

      contour->begin(clipped_contour.back().p3 * m_zoom, false);

      for (int i = static_cast<int>(clipped_contour.size()) - 1; i >= 0; i--) {
        auto& segment = clipped_contour.at(i);

        vec2 p0 = segment.p0 * m_zoom;

        if (segment.is_linear) {
          contour->offset_segment(p0, radius);
        } else {
          vec2 p1 = segment.p1 * m_zoom;
          vec2 p2 = segment.p2 * m_zoom;

          contour->offset_segment(
            p2,
            p1,
            p0,
            radius
          );
        }
      }

      contour->close();
    }

    // drawable.bounds.min -= 1.1f * 0.5f * stroke.width * (stroke.join == LineJoin::Miter ? stroke.miter_limit : 1.0f);
    // drawable.bounds.max += 1.1f * 0.5f * stroke.width * (stroke.join == LineJoin::Miter ? stroke.miter_limit : 1.0f);
    drawable.bounds.min -= 1.1f * radius * (stroke.join == LineJoin::Miter ? stroke.miter_limit : 1.0f);
    drawable.bounds.max += 1.1f * radius * (stroke.join == LineJoin::Miter ? stroke.miter_limit : 1.0f);

    // drawable.bounds *= m_zoom;

    process_drawable(drawable, visible, visible.min * m_zoom, clip_drawable);
#if 0

    Drawable drawable(1, Paint{ stroke.color, FillRule::NonZero, stroke.z_index }, path_rect);
    Geometry::Contour* contour = &drawable.contours.front();

    size_t last_start_i = i;
    vec2 last_start_p = transform_zoom * segments.at(i).p0();

    contour->begin(last_start_p);

    for (; i < len; i++) {
      auto& raw_segment = segments.at(i);

      vec2 p0 = transform_zoom * raw_segment.p0();
      vec2 p3 = transform_zoom * raw_segment.p3();

      bool p0_in = Math::is_point_in_rect(p0, m_visible);
      bool p3_in = Math::is_point_in_rect(p3, m_visible);

      if (!p0_in && !p3_in) continue;

      if (p0_in && p3_in) {
        // add entire segment
        contour->offset_segment(p3, radius);
        continue;
      }

      std::vector<float> intersections = line_rect_intersections(p0, p3, m_visible);
      vec2 p = p0;

      for (int k = 0; k < (int)intersections.size(); k++) {
        p = Math::lerp(p0, p3, intersections[k]);

        if (k % 2 == 0) {
          if (p0_in) {
            contour->offset_segment(p, radius);

            for (int j = i; j > last_start_i; j--) {
              // TODO: inline first segment
              auto& back_raw_segment = segments.at(i);

              // if (back_raw_segment.is_linear()) {
              contour->offset_segment(
                transform_zoom * back_raw_segment.p0(),
                radius
              );
              // } else {
              //   contour->offset_segment(
              //     transform_zoom * back_raw_segment.p2(),
              //     transform_zoom * back_raw_segment.p1(),
              //     transform_zoom * back_raw_segment.p0(),
              //     radius
              //   );
              // }
            }

            contour->offset_segment(last_start_p, radius);
          } else {
            contour->close();
            contour = &drawable.contours.emplace_back();
            contour->begin(p);

            last_start_i = i;
            last_start_p = p;
          }
        } else {
          // TODO: simplify
          if (p0_in) {
            contour->close();
            contour = &drawable.contours.emplace_back();
            contour->begin(p);

            last_start_i = i;
            last_start_p = p;
          } else {
            contour->offset_segment(p, radius);

            for (int j = i; j > last_start_i; j--) {
              // TODO: inline first segment
              auto& back_raw_segment = segments.at(i);

              // if (back_raw_segment.is_linear()) {
              contour->offset_segment(
                transform_zoom * back_raw_segment.p0(),
                radius
              );
              // } else {
              //   contour->offset_segment(
              //     transform_zoom * back_raw_segment.p2(),
              //     transform_zoom * back_raw_segment.p1(),
              //     transform_zoom * back_raw_segment.p0(),
              //     radius
              //   );
              // }
            }

            contour->offset_segment(last_start_p, radius);
          }
        }
      }

    }
#endif
  }

  void Tiler::process_fill(const Geometry::Path& path, const mat2x3& transform, const Fill& fill) {
    GK_TOTAL("Tiler::process_fill");

    const rect path_rect = transform * path.bounding_rect();

    const float overlap = Math::rect_rect_intersection_area(path_rect, m_visible) / path_rect.area();
    if (overlap <= 0.0f) return;

    Drawable drawable(1, fill, (path_rect - m_visible.min) * m_zoom);
    Geometry::Contour& contour = drawable.contours.front();

    // const mat2x3 transform_zoom = transform * m_zoom;
    const auto& segments = path.segments();
    const vec2 first = (transform * segments.front().p0() - m_visible.min) * m_zoom;

    contour.begin(first);

    for (size_t i = 0; i < segments.size(); i++) {
      auto& raw_segment = segments.at(i);

      if (raw_segment.is_linear()) {
        // TODO: test const reference vs copy
        contour.push_segment((transform * raw_segment.p3() - m_visible.min) * m_zoom);
      } else {
        contour.push_segment(
          (transform * raw_segment.p1() - m_visible.min) * m_zoom,
          (transform * raw_segment.p2() - m_visible.min) * m_zoom,
          (transform * raw_segment.p3() - m_visible.min) * m_zoom
        );
      }
    }

    contour.close();

    process_drawable(drawable, m_visible, m_visible.min * m_zoom, overlap < 0.7f);
  }

}
