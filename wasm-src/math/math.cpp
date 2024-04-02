/**
 * @file math.cpp
 * @brief Contains the implementation of math functions used by the Graphick editor.
 *
 * @todo try inlining lerps in split_bezier()
 * @todo ask if roots with multiplicity > 1 should be considered as separate roots
 */

#include "math.h"

#include <algorithm>

namespace Graphick::Math {

  /**
   * @brief Calculates the x-coordinate of the intersection point between two lines defined by their endpoints.
   *
   * If one of the lines is horizontal, use the x_intersect_horizontal() function.
   *
   * @param x1, y1, x2, y2 The coordinates of the first line.
   * @param x3, y3, x4, y4 The coordinates of the second line.
   * @return The x-coordinate of the intersection point.
   */
  static inline float x_intersect(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4) {
    float num = (x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4);
    float den = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);

    return num / den;
  }

  /**
   * @brief Calculates the y-coordinate of the intersection point between two lines defined by their endpoints.
   *
   * If one of the lines is vertical, use the y_intersect_vertical() function.
   *
   * @param x1, y1, x2, y2 The coordinates of the first line.
   * @param x3, y3, x4, y4 The coordinates of the second line.
   * @return The y-coordinate of the intersection point.
   */
  static inline float y_intersect(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4) {
    float num = (x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4);
    float den = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);

    return num / den;
  }

  /**
   * @brief Calculates the x-coordinate of the intersection point between two lines.
   *
   * If the first line is not horizontal, use the x_intersect() function.
   *
   * @param y The y-coordinate of the horizontal line
   * @param x1, y1, x2, y2 The coordinates of the second line.
   * @return The x-coordinate of the intersection point.
   */
  static inline float x_intersect_horizontal(float y, float x1, float y1, float x2, float y2) {
    float num = x1 * y2 - y1 * x2 - y * (x1 - x2);
    float den = y2 - y1;

    return num / den;
  }
  static inline f24x8 x_intersect_horizontal(f24x8 y, f24x8 x1, f24x8 y1, f24x8 x2, f24x8 y2) {
    int64_t num = static_cast<int64_t>(x1) * y2 - static_cast<int64_t>(y1) * x2 - static_cast<int64_t>(y) * (x1 - x2);
    int64_t den = y2 - y1;

    return static_cast<f24x8>(num / den);
  }

  /**
   * @brief Calculates the y-coordinate of the intersection point between two lines.
   *
   * If the first line is not vertical, use the y_intersect() function.
   *
   * @param x The x-coordinate of the vertical line
   * @param x1, y1, x2, y2 The coordinates of the second line.
   * @return The y-coordinate of the intersection point.
   */
  static inline float y_intersect_vertical(float x, float x1, float y1, float x2, float y2) {
    float num = x1 * y2 - y1 * x2 + x * (y1 - y2);
    float den = x1 - x2;

    return num / den;
  }
  static inline f24x8 y_intersect_vertical(f24x8 x, f24x8 x1, f24x8 y1, f24x8 x2, f24x8 y2) {
    int64_t num = static_cast<int64_t>(x1) * y2 - static_cast<int64_t>(y1) * x2 + static_cast<int64_t>(x) * (y1 - y2);
    int64_t den = x1 - x2;

    return static_cast<f24x8>(num / den);
  }

  QuadraticSolutions solve_quadratic(double a, double b, double c) {
    if (Math::is_almost_zero(a)) {
      /* It is a linear equation */

      return { solve_linear(b, c) };
    }

    double discriminant = b * b - 4.0 * a * c;

    if (Math::is_almost_zero(discriminant)) {
      /* One real root. */

      double root = -b / (2.0 * a);

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

  CubicSolutions solve_cubic(double a, double b, double c, double d) {
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

    /* Calculate coefficients of the depressed cubic equation: y^3 + py + q = 0 */
    double p = (3 * a * c - b * b) / (3 * a * a);
    double q = (2 * b * b * b - 9 * a * b * c + 27 * a * a * d) / (27 * a * a * a);

    /* Calculate discriminant */
    double discriminant = (q * q) / 4 + (p * p * p) / 27;

    if (Math::is_almost_zero(discriminant)) {
      double u = std::cbrt(-q / 2);
      /* Three real roots, two of them are equal */
      double realRoot1 = 2 * u - b / (3 * a);
      double realRoot2 = -u - b / (3 * a);

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
      double root2 = xi * std::cos((phi + 2 * MATH_F_PI) / 3) + b1;
      double root3 = xi * std::cos((phi + 4 * MATH_F_PI) / 3) + b1;

      return { root1, root2, root3 };
    }
  }

  rect rrect_to_rect(const rrect& r) {
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

  std::optional<float> line_line_intersection(const rect& a, const rect& b) {
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
      return t;
    }

    return std::nullopt;
  }

  std::optional<vec2> line_line_intersection_point(const rect& a, const rect& b) {
    rect rect = { min(b.min, b.max), max(b.min, b.max) };
    float den = b.max.x - b.min.x;

    if (is_almost_zero(den)) {
      float t = (b.min.x - a.min.x) / (a.max.x - a.min.x);

      if (t >= 0.0f && t <= 1.0f) {
        return lerp(a.min, a.max, t);
      }

      return std::nullopt;
    }

    float m = (b.max.y - b.min.y) / den;
    float t = (m * b.min.x - b.min.y + a.min.y - m * a.min.x) / (m * (a.max.x - a.min.x) + a.min.y - a.max.y);

    if (t >= 0.0f && t <= 1.0f) {
      return lerp(a.min, a.max, t);
    }

    return std::nullopt;
  }

  std::vector<vec2> line_circle_intersection_points(const rect& line, const vec2 center, const float radius) {
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

  std::vector<vec2> line_rect_intersection_points(const vec2 p0, const vec2 p3, const rect& rect) {
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

      if (is_point_in_rect({ (float)p.x, (float)p.y }, rect, GK_POINT_EPSILON)) {
        intersection_points.push_back({ (float)p.x, (float)p.y });
      }
    }

    return intersection_points;
  }

  std::vector<float> quadratic_rect_intersections(const vec2 p0, const vec2 p1, const vec2 p2, const rect& rect) {
    std::vector<float> intersections_t;
    std::vector<double> intersections;

    dvec2 dp0 = { p0.x, p0.y };
    dvec2 dp1 = { p1.x, p1.y };
    dvec2 dp2 = { p2.x, p2.y };

    dvec2 a = dp0 - 2.0 * dp1 + dp2;
    dvec2 b = 2.0 * (dp1 - dp0);

    for (int j = 0; j < 2; j++) {
      for (int k = 0; k < 2; k++) {
        QuadraticSolutions roots = solve_quadratic(a[k], b[k], dp0[k] - rect[j][k]);

        for (uint8_t i = 0; i < roots.count; i++) {
          double t = roots.solutions[i];

          if (t >= 0.0 && t <= 1.0) intersections.push_back(t);
        }
      }
    }

    if (intersections.empty()) return intersections_t;

    std::sort(intersections.begin(), intersections.end());

    for (double t : intersections) {
      double t_sq = t * t;
      dvec2 p = a * t_sq + b * t + dp0;

      if (is_point_in_rect({ (float)p.x, (float)p.y }, rect, GK_POINT_EPSILON)) {
        intersections_t.push_back((float)t);
      }
    }

    return intersections_t;
  }

  std::vector<float> cubic_rect_intersections(const vec2 p0, const vec2 p1, const vec2 p2, const vec2 p3, const rect& rect) {
    std::vector<float> intersections_t;
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

    if (intersections.empty()) return intersections_t;

    std::sort(intersections.begin(), intersections.end());

    for (double t : intersections) {
      double t_sq = t * t;
      dvec2 p = a * t_sq * t + b * t_sq + c * t + dp0;

      if (is_point_in_rect({ (float)p.x, (float)p.y }, rect, GK_POINT_EPSILON)) {
        intersections_t.push_back((float)t);
      }
    }

    return intersections_t;
  }

  bool does_line_intersect_rect(const vec2 p0, const vec2 p1, const rect& rect) {
    return line_rect_intersection_points(p0, p1, rect).size() > 0;
  }

  bool does_quadratic_intersect_rect(const vec2 p0, const vec2 p1, const vec2 p2, const rect& rect) {
    return quadratic_rect_intersections(p0, p1, p2, rect).size() > 0;
  }

  bool does_cubic_intersect_rect(const vec2 p0, const vec2 p1, const vec2 p3, const vec2 p4, const rect& rect) {
    return cubic_rect_intersections(p0, p1, p3, p4, rect).size() > 0;
  }

  float linear_closest_to(const vec2 p0, const vec2 p1, const vec2 p) {
    const vec2 v = p1 - p0;
    const vec2 w = p - p0;

    const float len_sq = squared_length(v);
    const float t = len_sq == 0 ? -1.0f : dot(v, w) / len_sq;

    return std::clamp(t, 0.0f, 1.0f);
  }

  float quadratic_closest_to(const vec2 p0, const vec2 p1, const vec2 p2, const vec2 p) {
    const vec2 bez1 = p0 + 2.0f / 3.0f * (p1 - p0);
    const vec2 bez2 = p2 + 2.0f / 3.0f * (p1 - p2);

    return cubic_closest_to(p0, bez1, bez2, p2, p);
  }

  float cubic_closest_to(const vec2 p0, const vec2 p1, const vec2 p2, const vec2 p3, const vec2 p) {
    const vec2 A_sq = p0 * p0;
    const vec2 B_sq = p1 * p1;
    const vec2 C_sq = p2 * p2;
    const vec2 D_sq = p3 * p3;

    const vec2 AB = p0 * p1;
    const vec2 AC = p0 * p2;
    const vec2 AD = p0 * p3;
    const vec2 BC = p1 * p2;
    const vec2 BD = p1 * p3;
    const vec2 CD = p2 * p3;

    const vec2 Apos = p0 * p;
    const vec2 Bpos = p1 * p;
    const vec2 Cpos = p2 * p;
    const vec2 Dpos = p3 * p;

    float a = 0.0f;
    float b = 0.0f;
    float c = 0.0f;
    float d = 0.0f;
    float e = 0.0f;
    float f = 0.0f;

    for (int i = 0; i < 2; ++i) {
      a +=
        6.0f * A_sq[i] -
        36.0f * AB[i] +
        36.0f * AC[i] -
        12.0f * AD[i] +
        54.0f * B_sq[i] -
        108.0f * BC[i] +
        36.0f * BD[i] +
        54.0f * C_sq[i] -
        36.0f * CD[i] +
        6.0f * D_sq[i];

      b +=
        -30.0f * A_sq[i] +
        150.0f * AB[i] -
        120.0f * AC[i] +
        30.0f * AD[i] -
        180.0f * B_sq[i] +
        270.0f * BC[i] -
        60.0f * BD[i] -
        90.0f * C_sq[i] +
        30.0f * CD[i];

      c +=
        60.0f * A_sq[i] -
        240.0f * AB[i] +
        144.0f * AC[i] -
        24.0f * AD[i] +
        216.0f * B_sq[i] -
        216.0f * BC[i] +
        24.0f * BD[i] +
        36.0f * C_sq[i];

      d +=
        -60.0f * A_sq[i] +
        180.0f * AB[i] -
        72.0f * AC[i] +
        6.0f * AD[i] +
        6.0f * Apos[i] -
        108.0f * B_sq[i] +
        54.0f * BC[i] -
        18.0f * Bpos[i] +
        18.0f * Cpos[i] -
        6.0f * Dpos[i];

      e +=
        30.0f * A_sq[i] -
        60.0f * AB[i] +
        12.0f * AC[i] -
        12.0f * Apos[i] +
        18.0f * B_sq[i] +
        24.0f * Bpos[i] -
        12.0f * Cpos[i];

      f +=
        -6.0f * A_sq[i] +
        6.0f * AB[i] +
        6.0f * Apos[i] -
        6.0f * Bpos[i];
    }

    float min_sq_distance = squared_distance(p0, p);
    float min_t = 0.0f;

    for (int i = 0; i <= GK_NEWTON_ITERATIONS; ++i) {
      float t = (float)i / (float)GK_NEWTON_ITERATIONS;

      for (int j = 0; j < 5; ++j) {
        const float t_sq = t * t;
        const float t_cu = t_sq * t;
        const float t_qu = t_cu * t;
        const float t_qui = t_qu * t;

        t -= (a * t_qui + b * t_qu + c * t_cu + d * t_sq + e * t + f) /
          (5.0f * a * t_qu + 4.0f * b * t_cu + 3.0f * c * t_sq + 2.0f * d * t + e);
      }

      if (t < 0 || t > 1) continue;

      const vec2 point = p0 * (1.0f - t) * (1.0f - t) * (1.0f - t) +
        p1 * 3.0f * t * (1.0f - t) * (1.0f - t) +
        p2 * 3.0f * t * t * (1.0f - t) +
        p3 * t * t * t;

      const float sq_dist = squared_distance(point, p);

      if (sq_dist < min_sq_distance) {
        min_t = t;
        min_sq_distance = sq_dist;
      }
    }

    return min_t;
  }

  void clip_to_left(std::vector<vec2>& points, float x) {
    if (points.empty()) return;

    std::vector<vec2> new_points;

    for (int i = 0; i < points.size() - 1; i++) {
      vec2 point = points[i];

      if (point.x < x) {
        if (points[i + 1].x > x) {
          new_points.push_back({ x, y_intersect_vertical(x, point.x, point.y, points[i + 1].x, points[i + 1].y) });
        }
      } else {
        new_points.push_back(point);

        if (points[i + 1].x < x) {
          new_points.push_back({ x, y_intersect_vertical(x, point.x, point.y, points[i + 1].x, points[i + 1].y) });
        }
      }
    }

    if (new_points.size() > 2 && new_points.front() != new_points.back()) {
      new_points.push_back(new_points.front());
    }

    points = new_points;
  }

  void clip_to_left(std::vector<f24x8x2>& points, f24x8 x) {
    if (points.empty()) return;

    std::vector<f24x8x2> new_points;

    for (int i = 0; i < points.size() - 1; i++) {
      f24x8x2 point = points[i];

      if (point.x < x) {
        if (points[i + 1].x > x) {
          new_points.push_back({ x, y_intersect_vertical(x, point.x, point.y, points[i + 1].x, points[i + 1].y) });
        }
      } else {
        new_points.push_back(point);

        if (points[i + 1].x < x) {
          new_points.push_back({ x, y_intersect_vertical(x, point.x, point.y, points[i + 1].x, points[i + 1].y) });
        }
      }
    }

    if (new_points.size() > 2 && new_points.front() != new_points.back()) {
      new_points.push_back(new_points.front());
    }

    points = new_points;
  }

  void clip_to_right(std::vector<vec2>& points, float x) {
    if (points.empty()) return;

    std::vector<vec2> new_points;

    for (int i = 0; i < points.size() - 1; i++) {
      vec2 point = points[i];

      if (point.x > x) {
        if (points[i + 1].x < x) {
          new_points.push_back({ x, y_intersect_vertical(x, point.x, point.y, points[i + 1].x, points[i + 1].y) });
        }
      } else {
        new_points.push_back(point);

        if (points[i + 1].x > x) {
          new_points.push_back({ x, y_intersect_vertical(x, point.x, point.y, points[i + 1].x, points[i + 1].y) });
        }
      }
    }

    if (new_points.size() > 2 && new_points.front() != new_points.back()) {
      new_points.push_back(new_points.front());
    }

    points = new_points;
  }

  void clip_to_right(std::vector<f24x8x2>& points, f24x8 x) {
    if (points.empty()) return;

    std::vector<f24x8x2> new_points;

    for (int i = 0; i < points.size() - 1; i++) {
      f24x8x2 point = points[i];

      if (point.x > x) {
        if (points[i + 1].x < x) {
          new_points.push_back({ x, y_intersect_vertical(x, point.x, point.y, points[i + 1].x, points[i + 1].y) });
        }
      } else {
        new_points.push_back(point);

        if (points[i + 1].x > x) {
          new_points.push_back({ x, y_intersect_vertical(x, point.x, point.y, points[i + 1].x, points[i + 1].y) });
        }
      }
    }

    if (new_points.size() > 2 && new_points.front() != new_points.back()) {
      new_points.push_back(new_points.front());
    }

    points = new_points;
  }

  void clip_to_top(std::vector<vec2>& points, float y) {
    if (points.empty()) return;

    std::vector<vec2> new_points;

    for (int i = 0; i < points.size() - 1; i++) {
      vec2 point = points[i];

      if (point.y < y) {
        if (points[i + 1].y > y) {
          new_points.push_back({ x_intersect_horizontal(y, point.x, point.y, points[i + 1].x, points[i + 1].y), y });
        }
      } else {
        new_points.push_back(point);

        if (points[i + 1].y < y) {
          new_points.push_back({ x_intersect_horizontal(y, point.x, point.y, points[i + 1].x, points[i + 1].y), y });
        }
      }
    }

    if (new_points.size() > 2 && new_points.front() != new_points.back()) {
      new_points.push_back(new_points.front());
    }

    points = new_points;
  }

  void clip_to_top(std::vector<f24x8x2>& points, f24x8 y) {
    if (points.empty()) return;

    std::vector<f24x8x2> new_points;

    for (int i = 0; i < points.size() - 1; i++) {
      f24x8x2 point = points[i];

      if (point.y < y) {
        if (points[i + 1].y > y) {
          new_points.push_back({ x_intersect_horizontal(y, point.x, point.y, points[i + 1].x, points[i + 1].y), y });
        }
      } else {
        new_points.push_back(point);

        if (points[i + 1].y < y) {
          new_points.push_back({ x_intersect_horizontal(y, point.x, point.y, points[i + 1].x, points[i + 1].y), y });
        }
      }
    }

    if (new_points.size() > 2 && new_points.front() != new_points.back()) {
      new_points.push_back(new_points.front());
    }

    points = new_points;
  }

  void clip_to_bottom(std::vector<vec2>& points, float y) {
    if (points.empty()) return;

    std::vector<vec2> new_points;

    for (int i = 0; i < points.size() - 1; i++) {
      vec2 point = points[i];

      if (point.y > y) {
        if (points[i + 1].y < y) {
          new_points.push_back({ x_intersect_horizontal(y, point.x, point.y, points[i + 1].x, points[i + 1].y), y });
        }
      } else {
        new_points.push_back(point);

        if (points[i + 1].y > y) {
          new_points.push_back({ x_intersect_horizontal(y, point.x, point.y, points[i + 1].x, points[i + 1].y), y });
        }
      }
    }

    if (new_points.size() > 2 && new_points.front() != new_points.back()) {
      new_points.push_back(new_points.front());
    }

    points = new_points;
  }

  void clip_to_bottom(std::vector<f24x8x2>& points, f24x8 y) {
    if (points.empty()) return;

    std::vector<f24x8x2> new_points;

    for (int i = 0; i < points.size() - 1; i++) {
      f24x8x2 point = points[i];

      if (point.y > y) {
        if (points[i + 1].y < y) {
          new_points.push_back({ x_intersect_horizontal(y, point.x, point.y, points[i + 1].x, points[i + 1].y), y });
        }
      } else {
        new_points.push_back(point);

        if (points[i + 1].y > y) {
          new_points.push_back({ x_intersect_horizontal(y, point.x, point.y, points[i + 1].x, points[i + 1].y), y });
        }
      }
    }

    if (new_points.size() > 2 && new_points.front() != new_points.back()) {
      new_points.push_back(new_points.front());
    }

    points = new_points;
  }

  vec2 circle_center(const vec2 a, const vec2 b, const vec2 c) {
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

  bool clockwise(const std::vector<vec2>& points) {
    float sum = (points[0].x - points[points.size() - 1].x) * (points[0].y + points[points.size() - 1].y);

    for (size_t i = 0; i < points.size() - 1; i++) {
      sum += (points[i + 1].x - points[i].x) * (points[i + 1].y + points[i].y);
    }

    return sum >= 0.0f;
  }

  rect linear_bounding_rect(const vec2 p0, const vec2 p1) {
    return { min(p0, p1), max(p0, p1) };
  }

  rect quadratic_bounding_rect(const vec2 p0, const vec2 p1, const vec2 p2) {
    rect bounds = { min(p0, p2), max(p0, p2) };

    const vec2 a = p0 - 2.0f * p1 + p2;
    const vec2 b = 2.0f * (p1 - p0);

    for (int i = 0; i < 2; i++) {
      if (a[i] == 0.0f) continue;

      const float t = static_cast<float>(solve_linear(2 * a[i], b[i]));

      if (is_normalized(t, false)) {
        const vec2 p = a * t * t + b * t + p0;

        min(bounds.min, p, bounds.min);
        max(bounds.max, p, bounds.max);
      }
    }

    return bounds;
  }

  rect cubic_bounding_rect(const vec2 p0, const vec2 p1, const vec2 p2, const vec2 p3) {
    rect bounds = { min(p0, p3), max(p0, p3) };

    const vec2 a = -p0 + 3.0f * p1 - 3.0f * p2 + p3;
    const vec2 b = 3.0f * p0 - 6.0f * p1 + 3.0f * p2;
    const vec2 c = -3.0f * p0 + 3.0f * p1;

    for (int i = 0; i < 2; i++) {
      if (a[i] == 0.0f) {
        if (b[i] == 0.0f) continue;

        const float t = static_cast<float>(Math::solve_linear(2 * b[i], c[i]));

        if (Math::is_normalized(t, false)) {
          const float t_sq = t * t;
          const vec2 p = a * t_sq * t + b * t_sq + c * t + p0;

          Math::min(bounds.min, p, bounds.min);
          Math::max(bounds.max, p, bounds.max);
        }

        continue;
      }

      const Math::QuadraticSolutions ts = Math::solve_quadratic(3.0f * a[i], 2.0f * b[i], c[i]);

      for (int j = 0; j < ts.count; j++) {
        const float t = static_cast<float>(ts.solutions[j]);

        if (Math::is_normalized(t, false)) {
          const float t_sq = t * t;
          const vec2 p = a * t_sq * t + b * t_sq + c * t + p0;

          Math::min(bounds.min, p, bounds.min);
          Math::max(bounds.max, p, bounds.max);
        }
      }
    }

    return bounds;
  }

  std::tuple<vec2, vec2, vec2, vec2, vec2> split_bezier(const vec2 p0, const vec2 p1, const vec2 p2, const vec2 p3, const float t) {
    vec2 p = bezier(p0, p1, p2, p3, t);

    vec2 q0 = lerp(p0, p1, t);
    vec2 q1 = lerp(p1, p2, t);
    vec2 q2 = lerp(p2, p3, t);

    vec2 r0 = lerp(q0, q1, t);
    vec2 r1 = lerp(q1, q2, t);

    return { p, q0, r0, r1, q2 };
  }

  std::tuple<vec2, vec2, vec2, vec2> split_bezier(const vec2 p0, const vec2 p1, const vec2 p2, const vec2 p3, const float t1, const float t2) {
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
    vec2 P1aa = lerp(P01a, Pa11, a);
    vec2 P1bb = lerp(P01b, Pb11, b);

    vec2 Paaa = lerp(P0aa, P1aa, a);
    vec2 Paab = lerp(P0aa, P1aa, b);
    vec2 Pabb = lerp(P0bb, P1bb, a);
    vec2 Pbbb = lerp(P0bb, P1bb, b);

    return { Paaa, Paab, Pabb, Pbbb };
  }
  
}
