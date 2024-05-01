/**
 * @file geom/intersections.cpp
 * @brief This file contains the implementations of hit testing and intersection methods for geometric shapes.
 */

#include "intersections.h"

#include "../math/math.h"

#include <algorithm>

namespace graphick::geom {

  /* -- Line -- */

  std::optional<double> line_line_intersection(const dline& a, const dline& b) {
    const double den = b.p1.x - b.p0.x;

    if (math::is_almost_zero(den)) {
      const double t = (b.p0.x - a.p0.x) / (a.p1.x - a.p0.x);

      if (math::is_normalized(t)) {
        return { t };
      }

      return std::nullopt;
    }

    const double m = (b.p1.y - b.p0.y) / den;
    const double t = (m * b.p0.x - b.p0.y + a.p0.y - m * a.p0.x) / (m * (a.p1.x - a.p0.x) + a.p0.y - a.p1.y);

    if (math::is_normalized(t)) {
      return t;
    }

    return std::nullopt;
  }

  std::optional<dvec2> line_line_intersection_point(const dline& a, const dline& b) {
    const drect rect = drect::from_vectors(b.p0, b.p1);
    const double den = b.p1.x - b.p0.x;

    if (math::is_almost_zero(den)) {
      const double t = (b.p0.x - a.p0.x) / (a.p1.x - a.p0.x);

      if (math::is_normalized(t)) {
        return math::lerp(a.p0, a.p1, t);
      }

      return std::nullopt;
    }

    const double m = (b.p1.y - b.p0.y) / den;
    const double t = (m * b.p0.x - b.p0.y + a.p0.y - m * a.p0.x) / (m * (a.p1.x - a.p0.x) + a.p0.y - a.p1.y);

    if (math::is_normalized(t)) {
      return math::lerp(a.p0, a.p1, t);
    }

    return std::nullopt;
  }

  math::QuadraticSolutions<dvec2> line_circle_intersection_points(const dline& line, const dvec2 center, const double radius) {
    const dvec2 ldir = line.p1 - line.p0;
    const dvec2 tvec = line.p0 - center;

    const double a = math::squared_length(ldir);
    const double b = 2.0 * dot(ldir, tvec);
    const double c = math::squared_length(center) + math::squared_length(line.p0) - (2.0 * math::dot(center, line.p0)) - radius * radius;

    const double i = b * b - 4.0 * a * c;

    if ((i < 0.0) || math::is_almost_zero(a)) {
      return {};
    } else if (math::is_almost_zero(i)) {
      const double mu = -b / (2.0 * a);

      return { ldir * mu + line.p0 };
    } else if (i > 0.0) {
      const double i_sqrt = std::sqrt(i);

      const double mu1 = (-b + i_sqrt) / (2.0 * a);
      const double mu2 = (-b - i_sqrt) / (2.0 * a);

      return { ldir * mu1 + line.p0, ldir * mu2 + line.p0 };
    }

    return {};
  }

  std::vector<dvec2> line_rect_intersection_points(const dline& line, const drect& rect) {
    std::vector<dvec2> intersection_points;
    std::vector<double> intersections;

    const dvec2 a = line.p1 - line.p0;

    const double t1 = math::solve_linear(a.x, line.p0.x - rect.min.x);
    const double t2 = math::solve_linear(a.x, line.p0.x - rect.max.x);
    const double t3 = math::solve_linear(a.y, line.p0.y - rect.min.y);
    const double t4 = math::solve_linear(a.y, line.p0.y - rect.max.y);

    if (math::is_normalized(t1)) intersections.push_back(t1);
    if (math::is_normalized(t2)) intersections.push_back(t2);
    if (math::is_normalized(t3)) intersections.push_back(t3);
    if (math::is_normalized(t4)) intersections.push_back(t4);

    if (intersections.empty()) return intersection_points;

    std::sort(intersections.begin(), intersections.end());

    for (const double t : intersections) {
      const dvec2 p = math::lerp(line.p0, line.p1, t);

      if (is_point_in_rect(p, rect, math::geometric_epsilon<double>)) {
        intersection_points.push_back(p);
      }
    }

    return intersection_points;
  }

  std::vector<double> quadratic_rect_intersections(const dquadratic_bezier& quad, const drect& rect) {
    std::vector<double> intersections_t;
    std::vector<double> intersections;

    const auto [a, b, c] = quad.coefficients();

    for (int j = 0; j < 2; j++) {
      for (int k = 0; k < 2; k++) {
        const math::QuadraticSolutions roots = math::solve_quadratic(a[k], b[k], c[k] - rect[j][k]);

        for (uint8_t i = 0; i < roots.count; i++) {
          const double t = roots.solutions[i];

          if (math::is_normalized(t)) {
            intersections.push_back(t);
          }
        }
      }
    }

    if (intersections.empty()) return intersections_t;

    std::sort(intersections.begin(), intersections.end());

    for (const double t : intersections) {
      const double t_sq = t * t;
      const dvec2 p = a * t_sq + b * t + c;

      if (is_point_in_rect(p, rect, math::geometric_epsilon<double>)) {
        intersections_t.push_back(t);
      }
    }

    return intersections_t;
  }

  std::vector<double> cubic_rect_intersections(const dcubic_bezier& cubic, const drect& rect) {
    std::vector<double> intersections_t;
    std::vector<double> intersections;

    const auto [a, b, c, d] = cubic.coefficients();

    for (int j = 0; j < 2; j++) {
      for (int k = 0; k < 2; k++) {
        const math::CubicSolutions roots = math::solve_cubic(a[k], b[k], c[k], d[k] - rect[j][k]);

        for (uint8_t i = 0; i < roots.count; i++) {
          const double t = roots.solutions[i];

          if (math::is_normalized(t)) {
            intersections.push_back(t);
          }
        }
      }
    }

    if (intersections.empty()) return intersections_t;

    std::sort(intersections.begin(), intersections.end());

    for (double t : intersections) {
      const double t_sq = t * t;
      const dvec2 p = a * t_sq * t + b * t_sq + c * t + d;

      if (is_point_in_rect(p, rect, math::geometric_epsilon<double>)) {
        intersections_t.push_back(t);
      }
    }

    return intersections_t;
  }

  double line_closest_to(const dline& line, const dvec2 p) {
    const dvec2 v = line.p1 - line.p0;
    const dvec2 w = p - line.p0;

    const double len_sq = math::squared_length(v);

    return math::is_almost_zero(len_sq) ? 0.0 : std::clamp(dot(v, w) / len_sq, 0.0, 1.0);
  }

  double quadratic_closest_to(const dquadratic_bezier& quad, const dvec2 p) {
    const auto [A, B, C] = quad.coefficients();

    double a = 0.0;
    double b = 0.0;
    double c = 0.0;
    double d = 0.0;

    for (int i = 0; i < 2; i++) {
      a += 2.0 * A[i] * A[i];
      b += 3.0 * A[i] * B[i];
      c += 2.0 * A[i] * C[i] + B[i] * B[i] - 2.0 * A[i] * p[i];
      d += B[i] * C[i] - B[i] * p[i];
    }

    math::CubicSolutions t_values = math::solve_cubic(a, b, c, d);

    double min_sq_distance = math::squared_distance(quad.p0, p);
    double min_t = 0.0;

    for (uint8_t i = 0; i < t_values.count; i++) {
      const double t = t_values.solutions[i];

      if (math::is_normalized(t)) {
        const dvec2 point = A * t * t + B * t + C;
        const double sq_distance = math::squared_distance(point, p);

        if (sq_distance < min_sq_distance) {
          min_t = t;
          min_sq_distance = sq_distance;
        }
      }
    }

    return min_t;
  }

  // TODO: test
  double cubic_closest_to_alt(const dcubic_bezier& cubic, const dvec2 p) {
    const auto [A, B, C, D] = cubic.coefficients();

    double a = 0.0;
    double b = 0.0;
    double c = 0.0;
    double d = 0.0;
    double e = 0.0;
    double f = 0.0;

    for (int i = 0; i < 2; i++) {
      a += 3.0 * A[i] * A[i];
      b += 5.0 * A[i] * B[i];
      c += 4.0 * A[i] * C[i] + 2.0 * B[i] * B[i];
      d += 3.0 * A[i] * D[i] + 3.0 * B[i] * C[i] - 3 * A[i] * p[i];
      e += 2.0 * B[i] * D[i] - 2.0 * B[i] * p[i] + C[i] * C[i];
      f += C[i] * D[i] - C[i] * p[i];
    }

    // math::QuinticSolutions t_values = math::solve_quintic(a, b, c, d, e, f);

    // double min_sq_distance = math::squared_distance(quad.p0, p);
    double min_t = 0.0;

    // for (uint8_t i = 0; i < t_values.count; i++) {
    //   const double t = t_values.solutions[i];

    //   if (math::is_normalized(t)) {
    //     const dvec2 point = A * t * t + B * t + C;
    //     const double sq_distance = math::squared_distance(point, p);

    //     if (sq_distance < min_sq_distance) {
    //       min_t = t;
    //       min_sq_distance = sq_distance;
    //     }
    //   }
    // }

    return min_t;
  }

  double cubic_closest_to(const dcubic_bezier& cubic, const dvec2 p) {
    const dvec2 A_sq = cubic.p0 * cubic.p0;
    const dvec2 B_sq = cubic.p1 * cubic.p1;
    const dvec2 C_sq = cubic.p2 * cubic.p2;
    const dvec2 D_sq = cubic.p3 * cubic.p3;

    const dvec2 AB = cubic.p0 * cubic.p1;
    const dvec2 AC = cubic.p0 * cubic.p2;
    const dvec2 AD = cubic.p0 * cubic.p3;
    const dvec2 BC = cubic.p1 * cubic.p2;
    const dvec2 BD = cubic.p1 * cubic.p3;
    const dvec2 CD = cubic.p2 * cubic.p3;

    const dvec2 Apos = cubic.p0 * p;
    const dvec2 Bpos = cubic.p1 * p;
    const dvec2 Cpos = cubic.p2 * p;
    const dvec2 Dpos = cubic.p3 * p;

    double a = 0.0;
    double b = 0.0;
    double c = 0.0;
    double d = 0.0;
    double e = 0.0;
    double f = 0.0;

    for (int i = 0; i < 2; ++i) {
      a +=
        6.0 * A_sq[i] -
        36.0 * AB[i] +
        36.0 * AC[i] -
        12.0 * AD[i] +
        54.0 * B_sq[i] -
        108.0 * BC[i] +
        36.0 * BD[i] +
        54.0 * C_sq[i] -
        36.0 * CD[i] +
        6.0 * D_sq[i];

      b +=
        -30.0 * A_sq[i] +
        150.0 * AB[i] -
        120.0 * AC[i] +
        30.0 * AD[i] -
        180.0 * B_sq[i] +
        270.0 * BC[i] -
        60.0 * BD[i] -
        90.0 * C_sq[i] +
        30.0 * CD[i];

      c +=
        60.0 * A_sq[i] -
        240.0 * AB[i] +
        144.0 * AC[i] -
        24.0 * AD[i] +
        216.0 * B_sq[i] -
        216.0 * BC[i] +
        24.0 * BD[i] +
        36.0 * C_sq[i];

      d +=
        -60.0 * A_sq[i] +
        180.0 * AB[i] -
        72.0 * AC[i] +
        6.0 * AD[i] +
        6.0 * Apos[i] -
        108.0 * B_sq[i] +
        54.0 * BC[i] -
        18.0 * Bpos[i] +
        18.0 * Cpos[i] -
        6.0 * Dpos[i];

      e +=
        30.0 * A_sq[i] -
        60.0 * AB[i] +
        12.0 * AC[i] -
        12.0 * Apos[i] +
        18.0 * B_sq[i] +
        24.0 * Bpos[i] -
        12.0 * Cpos[i];

      f +=
        -6.0 * A_sq[i] +
        6.0 * AB[i] +
        6.0 * Apos[i] -
        6.0 * Bpos[i];
    }

    double min_sq_distance = math::squared_distance(cubic.p0, p);
    double min_t = 0.0;

    for (int i = 0; i <= math::newton_raphson_iterations<int>; i++) {
      double t = static_cast<double>(i) / math::newton_raphson_iterations<double>;

      for (int j = 0; j < math::newton_raphson_iterations<int>; ++j) {
        const double t_sq = t * t;
        const double t_cu = t_sq * t;
        const double t_qu = t_cu * t;
        const double t_qui = t_qu * t;

        t -= (a * t_qui + b * t_qu + c * t_cu + d * t_sq + e * t + f) /
          (5.0 * a * t_qu + 4.0 * b * t_cu + 3.0 * c * t_sq + 2.0 * d * t + e);
      }

      if (t < 0.0 || t > 1.0) continue;

      const dvec2 point = cubic.p0 * (1.0 - t) * (1.0 - t) * (1.0 - t) +
        cubic.p1 * 3.0 * t * (1.0 - t) * (1.0 - t) +
        cubic.p2 * 3.0 * t * t * (1.0 - t) +
        cubic.p3 * t * t * t;

      const double sq_dist = math::squared_distance(point, p);

      if (sq_dist < min_sq_distance) {
        min_t = t;
        min_sq_distance = sq_dist;
      }
    }

    return min_t;
  }

}
