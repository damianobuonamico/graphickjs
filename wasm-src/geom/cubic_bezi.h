#pragma once

#include "vector.h"
#include "rect.h"

#include "../utils/console.h"

#include <vector>
#include <optional>
#include <algorithm>

namespace graphick::Math {


  // std::vector<float> cubic_extrema(const vec2 v1, const vec2 v2, const vec2 v3, const vec2 v4) {
  //   const vec2 a = 3.0f * (-v1 + 3.0f * v2 - 3.0f * v3 + v4);
  //   const vec2 b = 6.0f * (v1 - 2.0f * v2 + v3);
  //   const vec2 c = 3.0f * (v2 - v1);

  //   std::vector<float> roots = { 0.0f, 1.0f };

  //   for (int i = 0; i < 2; i++) {
  //     if (is_almost_zero(a[i])) {
  //       if (is_almost_zero(b[i])) continue;

  //       float t = -c[i] / b[i];
  //       if (t > 0.0f && t < 1.0f) {
  //         roots.push_back(t);
  //       }

  //       continue;
  //     }

  //     float delta = b[i] * b[i] - 4.0f * a[i] * c[i];

  //     if (is_almost_zero(delta)) {
  //       roots.push_back(-b[i] / (2.0f * a[i]));
  //     } else if (delta < 0.0f) {
  //       continue;
  //     } else {
  //       float sqrt_delta = std::sqrtf(delta);

  //       float t1 = (-b[i] + sqrt_delta) / (2.0f * a[i]);
  //       float t2 = (-b[i] - sqrt_delta) / (2.0f * a[i]);

  //       if (t1 > 0.0f && t1 < 1.0f) {
  //         roots.push_back(t1);
  //       }
  //       if (t2 > 0.0f && t2 < 1.0f) {
  //         roots.push_back(t2);
  //       }
  //     }
  //   }

  //   return roots;
  // }


  struct QuadraticBezier {
    vec2 p0;
    vec2 p1;
    vec2 p2;
  };

  struct CubicBezier {
    vec2 p0;
    vec2 p1;
    vec2 p2;
    vec2 p3;
  };

  static vec2 approx_quad_control(const CubicBezier& c, const float t) {
    const vec2 p1 = c.p0 + (c.p1 - c.p0) * 1.5;
    const vec2 p2 = c.p3 + (c.p2 - c.p3) * 1.5;

    return Math::lerp(p1, p2, t);
  }

  static std::pair<CubicBezier, CubicBezier> split(const CubicBezier& c, const float t) {
    vec2 p = Math::bezier(c.p0, c.p1, c.p2, c.p3, t);

    vec2 q0 = Math::lerp(c.p0, c.p1, t);
    vec2 q1 = Math::lerp(c.p1, c.p2, t);
    vec2 q2 = Math::lerp(c.p2, c.p3, t);

    vec2 r0 = Math::lerp(q0, q1, t);
    vec2 r1 = Math::lerp(q1, q2, t);

    return {
      { c.p0, q0, r0, p },
      { p, r1, q2, c.p3 }
    };
  }

  static std::vector<CubicBezier> split_into_n(const CubicBezier& c, const int n) {
    std::vector<CubicBezier> cubics;
    cubics.reserve(n);

    const float step = 1.0f / n;
    float t = 0.0f;

    for (int i = 0; i < n; ++i) {
      const float t1 = t + step;

      const auto& [p0, p1, p2, p3] = split_bezier(c.p0, c.p1, c.p2, c.p3, t, t1);

      cubics.push_back({ p0, p1, p2, p3 });

      t = t1;

      // const vec2 p0 = c.p0;
      // const vec2 p1 = c.p0 + (c.p1 - c.p0) * t;
      // const vec2 p2 = c.p0 + (c.p1 - c.p0) * t1;
      // const vec2 p3 = c.p0 + (c.p1 - c.p0) * 2.0f * t1 + (c.p2 - c.p1) * t1 * t1;
      // const vec2 p4 = c.p0 + (c.p1 - c.p0) * 3.0f * t1 + (c.p2 - c.p1) * 3.0f * t1 * t1 + (c.p3 - c.p2) * t1 * t1 * t1;

      // cubics.push_back({ p0, p1, p3, p4 });
      // t = t1;
    }

    return cubics;
  }

  /// Does this curve fit inside the given distance from the origin?
  ///
  /// Rust port of cu2qu [cubic_farthest_fit_inside](https://github.com/fonttools/fonttools/blob/3b9a73ff8379ab49d3ce35aaaaf04b3a7d9d1655/Lib/fontTools/cu2qu/cu2qu.py#L281)
  static bool fit_inside(const CubicBezier& c, const float distance) {
    if (Math::squared_length(c.p2) <= distance * distance && Math::squared_length(c.p1) <= distance * distance) {
      return true;
    }

    const vec2 mid = (c.p0 + 3.0f * (c.p1 + c.p2) + c.p3) * 0.125f;

    if (Math::squared_length(mid) > distance * distance) {
      return false;
    }

    // Split in two. Note that cu2qu here uses a 3/8 subdivision. I don't know why.
    const auto& [left, right] = split(c, 0.5f);
    return fit_inside(left, distance) && fit_inside(right, distance);
  }

  /// Computes the point where two lines, if extended to infinity, would cross.
  static std::optional<vec2> crossing_point(rect& l, rect& other) {
    const vec2 ab = l.max - l.min;
    const vec2 cd = other.max - other.min;
    const float pcd = Math::cross(ab, cd);

    if (pcd == 0.0f) {
      return std::nullopt;
    }

    const float h = Math::cross(ab, (l.min - other.min) / pcd);
    return other.min + cd * h;
  }

  static std::vector<vec2> try_approx_quadratic(const CubicBezier& c, const float accuracy) {
    std::optional<vec2> q1 = crossing_point(rect{ c.p0, c.p1 }, rect{ c.p2, c.p3 });

    if (q1.has_value()) {
      const vec2 c1 = Math::lerp(c.p0, q1.value(), 2.0f / 3.0f);
      const vec2 c2 = Math::lerp(c.p3, q1.value(), 2.0f / 3.0f);

      if (!fit_inside(CubicBezier{ vec2{ 0.0f, 0.0f }, c1 - c.p1, c2 - c.p2, vec2{ 0.0f, 0.0f } }, accuracy)) {
        return {};
      }

      return { c.p0, q1.value(), c.p3 };
    }
  }

  static std::vector<vec2> approx_spline_n(const CubicBezier& c, const int n, const float accuracy) {
    if (n == 1) {
      return try_approx_quadratic(c, accuracy);
    }

    std::vector<CubicBezier> cubics = split_into_n(c, n);

    // The above function guarantees that the iterator returns n items,
    // which is why we're unwrapping things with wild abandon.
    CubicBezier& next_cubic = cubics.front();
    vec2 next_q1 = approx_quad_control(next_cubic, 0.0f);
    vec2 q2 = c.p0;
    // TODO: implement vec2::zero();
    vec2 d1 = vec2{ 0.0f, 0.0f };
    std::vector<vec2> spline = { c.p0, next_q1 };

    for (int i = 1; i <= n; ++i) {
      const CubicBezier& current_cubic = next_cubic;
      const vec2 q0 = q2;
      const vec2 q1 = next_q1;

      if (i < n) {
        next_cubic = cubics[i];
        next_q1 = approx_quad_control(next_cubic, static_cast<float>(i) / static_cast<float>(n - 1));

        spline.push_back(next_q1);
        q2 = Math::midpoint(q1, next_q1);
      } else {
        q2 = current_cubic.p3;
      }

      const vec2 d0 = d1;
      d1 = q2 - current_cubic.p3;

      if (
        Math::length(d1) > accuracy ||
        !fit_inside(
          CubicBezier{
            d0,
            Math::lerp(q0, q1, 2.0f / 3.0f) - current_cubic.p1,
            Math::lerp(q2, q1, 2.0f / 3.0f) - current_cubic.p2,
            d1
          },
          accuracy
        )
      ) {
        // return {};
      }

      spline.push_back(current_cubic.p3);
    }

    // spline.push_back(c.p3);
    return spline;
  }

#if 0
  static std::vector<vec2> to_quads(const CubicBezier& c, const float accuracy) {
    // The maximum error, as a vector from the cubic to the best approximating
    // quadratic, is proportional to the third derivative, which is constant
    // across the segment. Thus, the error scales down as the third power of
    // the number of subdivisions. Our strategy then is to subdivide `t` evenly.
    //
    // This is an overestimate of the error because only the component
    // perpendicular to the first derivative is important. But the simplicity is
    // appealing.

    // This magic number is the square of 36 / sqrt(3).
    // See: http://caffeineowl.com/graphics/2d/vectorial/cubic2quad01.html
    const float max_hypot2 = 432.0f * accuracy * accuracy;

    const vec2 p1x2 = 3.0f * c.p1 - c.p0;
    const vec2 p2x2 = 3.0f * c.p2 - c.p3;

    const float err = Math::squared_length(p2x2 - p1x2);
    const int n = std::max(1, static_cast<int>(std::ceilf(std::powf(err / max_hypot2, 1.0f / 6.0f))));

    return approx_spline_n(c, n, accuracy);
  }
#endif

  static inline float cubic_to_quadratic_distance_at_t(const CubicBezier& cubic, const vec2 q, const float t) {
    const vec2 p_c = Math::bezier(cubic.p0, cubic.p1, cubic.p2, cubic.p3, t);
    const vec2 p_q = Math::quadratic(cubic.p0, q, cubic.p3, t);

    return Math::squared_distance(p_c, p_q);
  }

  struct ErrorParameter {
    float t;
    float e;
  };

  static ErrorParameter cubic_to_single_quadratic_max_error(const CubicBezier& cubic, const vec2 q) {
    const vec2 a_c = -cubic.p0 + 3 * cubic.p1 - 3 * cubic.p2 + cubic.p3;
    const vec2 b_c = 3 * cubic.p0 - 6 * cubic.p1 + 3 * cubic.p2;
    const vec2 c_c = -3 * cubic.p0 + 3 * cubic.p1;
    const vec2 d_c = cubic.p0;

    const vec2 a_q = cubic.p0 - 2 * q + cubic.p3;
    const vec2 b_q = 2 * (q - cubic.p0);
    const vec2 c_q = cubic.p0;

    const float a = a_c.x + a_c.y;
    const float b = b_c.x - a_q.x + b_c.y - a_q.y;
    const float c = c_c.x - b_q.x + c_c.y - b_q.y;
    //TODO: d should always be zero
    const float d = d_c.x - c_q.x + d_c.y - c_q.y;

    if (Math::is_almost_zero(a)) {
      if (Math::is_almost_zero(b)) {
        if (Math::is_almost_zero(c)) {
          // TODO: implement
          // return -d / 2.0f;
          return { 0.0f, 0.0f };
        }

        const float t = -d / c;

        if (!Math::is_normalized(t, false)) return { 0.0f, 0.0f };

        return { t, cubic_to_quadratic_distance_at_t(cubic, q, t) };
      } else {
        float det = c * c - 4.0f * b * d;

        if (det > 0) {
          det = std::sqrtf(det);

          const float t1 = (-c + det) / (2.0f * b);
          const float t2 = (-c - det) / (2.0f * b);

          float d1 = 0.0f;
          float d2 = 0.0f;

          if (Math::is_normalized(t1, false)) {
            d1 = cubic_to_quadratic_distance_at_t(cubic, q, t1);
          }

          if (Math::is_normalized(t2, false)) {
            d2 = cubic_to_quadratic_distance_at_t(cubic, q, t2);
          }

          if (d1 > d2) {
            return { t1, d1 };
          } else {
            return { t2, d2 };
          }
        }

        const float t = -c / (2.0f * b);

        if (!Math::is_normalized(t, false)) return { 0.0f, 0.0f };

        return { t, cubic_to_quadratic_distance_at_t(cubic, q, t) };
      }
    }

    float coeff1 = -27.0f * a * a * d + 9.0f * a * b * c - 2.0f * b * b * b;

    float det1 = b * b - 3.0f * a * c;
    float det2 = coeff1 * coeff1 - 4.0f * det1 * det1 * det1;

    float t1 = 0.0f;
    float t2 = 0.0f;
    float t3 = 0.0f;

    float d1 = 0.0f;
    float d2 = 0.0f;
    float d3 = 0.0f;

    if (det1 > 0.0f) {
      const float det3 = std::sqrtf(det1);

      t1 = (-b + det3) / (3.0f * a);
      t2 = (-b - det3) / (3.0f * a);

      if (Math::is_normalized(t1, false)) {
        d1 = cubic_to_quadratic_distance_at_t(cubic, q, t1);
      }

      if (Math::is_normalized(t2, false)) {
        d2 = cubic_to_quadratic_distance_at_t(cubic, q, t2);
      }
    }

    if (det2 > 0.0f) {
      const float coeff2 = std::cbrtf(std::sqrtf(det2) + coeff1);

      t3 = coeff2 / (2.0f * std::cbrtf(2.0f) * a) + std::cbrtf(2.0f) * det1 / (3.0f * a * coeff2) - b / (3.0f * a);

      if (Math::is_normalized(t3, false)) {
        d3 = cubic_to_quadratic_distance_at_t(cubic, q, t3);
      }
    }

    if (d1 > d2) {
      if (d1 > d3) {
        return { t1, d1 };
      } else {
        return { t3, d3 };
      }
    } else {
      if (d2 > d3) {
        return { t2, d2 };
      } else {
        return { t3, d3 };
      }
    }
  }

  // TODO: add max recursion depth
  static void approx_monotonic_cubic(const CubicBezier& c, const float tolerance, std::vector<vec2>& sink) {
    vec2 q;

    if (Math::is_almost_equal(c.p0, c.p1)) {
      q = c.p2;

      // sink.push_back(c.p2);
      // sink.push_back(c.p3);

      // return;
    } else if (Math::is_almost_equal(c.p2, c.p3)) {
      q = c.p1;

      // sink.push_back(c.p1);
      // sink.push_back(c.p3);

      // return;
    } else {
      float d = (c.p0.x - c.p1.x) * (c.p2.y - c.p3.y) - (c.p0.y - c.p1.y) * (c.p2.x - c.p3.x);

      if (Math::is_almost_zero(d, GK_POINT_EPSILON)) {
        const vec2 p1 = (c.p1 * 3.0 - c.p0) * 0.5;
        const vec2 p2 = (c.p2 * 3.0 - c.p3) * 0.5;

        q = Math::midpoint(p1, p2);

        // sink.push_back(Math::midpoint(p1, p2));
        // sink.push_back(c.p3);

        // return;
      } else {
        const float pre = (c.p0.x * c.p1.y - c.p0.y * c.p1.x);
        const float post = (c.p2.x * c.p3.y - c.p2.y * c.p3.x);

        q = (pre * (c.p2 - c.p3) - (c.p0 - c.p1) * post) / d;

        // sink.push_back((pre * (c.p2 - c.p3) - (c.p0 - c.p1) * post) / d);
        // sink.push_back(c.p3);
      }
    }

#if 0
    const ErrorParameter max_error = cubic_to_single_quadratic_max_error(c, q);
    const float max_size = std::max(Math::length(c.p3 - c.p0), Math::length(c.p3 - q));

    const vec2 d = Math::cubic_derivative(c.p0, c.p1, c.p2, c.p3, max_error.t);
    const vec2 dd = Math::cubic_second_derivative(c.p0, c.p1, c.p2, c.p3, max_error.t);
    const float numerator = d.x * dd.y - d.y * dd.x;
    const float denominator = std::powf(d.x * d.x + d.y * d.y, 1.5f);

    float curvature = Math::is_almost_zero(denominator) ? 100.0f : std::fabsf(numerator / denominator) * 10.0f;

    curvature = std::clamp(curvature, 0.001f, 100.0f);
    // const float curvature = Math::is_almost_zero(denominator) ? 100.0f : std::clamp(0.01f, curvature, 100.0f);

    // const vec2 p_c = Math::bezier(c.p0, c.p1, c.p2, c.p3, max_error_t);
    // const vec2 p_q = Math::quadratic(c.p0, q, c.p3, max_error_t);

    // TODO: maybe normalize error based on the length of the curve
    // if (max_error.e / std::max(max_size / 10.0f, GK_EPSILON) <= tolerance * tolerance) {
    if (max_error.e * curvature <= tolerance * tolerance) {
      sink.push_back(q);
      sink.push_back(c.p3);

      console::log("max_error", max_error.e);
      console::log("max_t", max_error.t);
      console::log("curvature", curvature);

      return;
    }

    const auto& [left, right] = split(c, max_error.t);

    approx_monotonic_cubic(left, tolerance, sink);
    approx_monotonic_cubic(right, tolerance, sink);
#else
    bool good_enough = true;
    int i;

    for (i = 1; i < 10; i++) {
      const float t = static_cast<float>(i) / 10.0f;
      const float e = cubic_to_quadratic_distance_at_t(c, q, t);

      if (e > tolerance * tolerance) {
        good_enough = false;
        break;
      }
    }

    if (good_enough) {
      sink.push_back(q);
      sink.push_back(c.p3);

      return;
    }

    const auto& [left, right] = split(c, static_cast<float>(i) / 10.0f);

    approx_monotonic_cubic(left, tolerance, sink);
    approx_monotonic_cubic(right, tolerance, sink);
#endif
  }

#if 0
  static vec2 _to_quad(const CubicBezier& c, const float t0, const float t1) {
    // Find a single control point for given segment of cubic Bezier curve
    // These control point is an interception of tangent lines to the boundary points
    // Let's denote that f(t) is a vector function of parameter t that defines the cubic Bezier curve,
    // f(t1) + f'(t1)*z1 is a parametric equation of tangent line to f(t1) with parameter z1
    // f(t2) + f'(t2)*z2 is the same for point f(t2) and the vector equation
    // f(t1) + f'(t1)*z1 = f(t2) + f'(t2)*z2 defines the values of parameters z1 and z2.
    // Defining fx(t) and fy(t) as the x and y components of vector function f(t) respectively
    // and solving the given system for z1 one could obtain that
    //
    //      -(fx(t2) - fx(t1))*fy'(t2) + (fy(t2) - fy(t1))*fx'(t2)
    // z1 = ------------------------------------------------------.
    //            -fx'(t1)*fy'(t2) + fx'(t2)*fy'(t1)
    //
    // Let's assign letter D to the denominator and note that if D = 0 it means that the curve actually
    // is a line. Substituting z1 to the equation of tangent line to the point f(t1), one could obtain that
    // cx = [fx'(t1)*(fy(t2)*fx'(t2) - fx(t2)*fy'(t2)) + fx'(t2)*(fx(t1)*fy'(t1) - fy(t1)*fx'(t1))]/D
    // cy = [fy'(t1)*(fy(t2)*fx'(t2) - fx(t2)*fy'(t2)) + fy'(t2)*(fx(t1)*fy'(t1) - fy(t1)*fx'(t1))]/D
    // where c = (cx, cy) is the control point of quadratic Bezier curve.

    const vec2 f1 = bezier(c.p0, c.p1, c.p2, c.p3, t0);
    const vec2 f2 = bezier(c.p0, c.p1, c.p2, c.p3, t1);
    const vec2 f1_ = cubic_derivative(c.p0, c.p1, c.p2, c.p3, t0);
    const vec2 f2_ = cubic_derivative(c.p0, c.p1, c.p2, c.p3, t1);

    const float D = -f1_.x * f2_.y + f2_.x * f1_.y;

    if (std::fabsf(D) < GK_POINT_EPSILON) {
      return midpoint(f1, f2); // straight line segment
    }

    const float qx = (f1_.x * (f2.y * f2_.x - f2.x * f2_.y) + f2_.x * (f1.x * f1_.y - f1.y * f1_.x)) / D;
    const float qy = (f1_.y * (f2.y * f2_.x - f2.x * f2_.y) + f2_.y * (f1.x * f1_.y - f1.y * f1_.x)) / D;

    return vec2{ qx, qy };
  }

  static float min_distance_sq_to_line(const vec2 point, const vec2 p1, const vec2 p2) {
    const vec2 p1p2 = p2 - p1;
    const float d = Math::dot(point - p1, p1p2);
    const float len_sq = Math::squared_length(p1p2);

    float param = 0;
    vec2 diff;

    if (len_sq != 0) param = d / len_sq;

    if (param <= 0) {
      diff = point - p1;
    } else if (param >= 1) {
      diff = point - p2;
    } else {
      diff = point - p1 + p1p2 * param;
    }

    return Math::squared_length(diff);
  }

  static bool _is_segment_approximation_close(const CubicBezier& c, const float t0, const float t1, const QuadraticBezier& q, const float tolerance) {
    // return cubic_to_single_quadratic_max_error(c, q.p1).e <= tolerance * tolerance;

    const float tolerance_sq = tolerance * tolerance;
    const int n = 10; // number of points

    std::vector<vec2> cubicPoints;
    std::vector<vec2> quadPoints;

    float distSq, minDistSq;
    float t, dt;
    int i, j;

    dt = (t1 - t0) / n;
    for (i = 0, t = t0; i <= n; i++, t += dt) {
      cubicPoints.push_back(bezier(c.p0, c.p1, c.p2, c.p3, t));
    }

    dt = 1 / n;
    for (i = 0, t = 0; i <= n; i++, t += dt) {
      quadPoints.push_back(quadratic(q.p0, q.p1, q.p2, t));
    }

    for (i = 1; i < static_cast<int>(cubicPoints.size()) - 1; i++) {
      minDistSq = std::numeric_limits<float>::infinity();

      for (j = 0; j < static_cast<int>(quadPoints.size()) - 1; j++) {
        distSq = min_distance_sq_to_line(cubicPoints[i], quadPoints[j], quadPoints[j + 1]);
        minDistSq = std::min(minDistSq, distSq);
      }

      if (minDistSq > tolerance_sq) return false;
    }

    for (i = 1; i < static_cast<int>(quadPoints.size()) - 1; i++) {
      minDistSq = std::numeric_limits<float>::infinity();

      for (j = 0; j < static_cast<int>(cubicPoints.size()) - 1; j++) {
        distSq = min_distance_sq_to_line(quadPoints[i], cubicPoints[j], cubicPoints[j + 1]);
        minDistSq = std::min(minDistSq, distSq);
      }

      if (minDistSq > tolerance_sq) return false;
    }

    return true;
  }

  static bool _is_approximation_close(const CubicBezier& c, const std::vector<vec2>& approximation, const float tolerance) {
    const int quads_count = approximation.size() / 2;
    const float dt = 1.0f / quads_count;

    for (int i = 0; i < quads_count; i++) {
      const vec2 p0 = approximation[i * 2];
      const vec2 p1 = approximation[i * 2 + 1];
      const vec2 p2 = approximation[i * 2 + 2];

      if (!_is_segment_approximation_close(c, i * dt, (i + 1) * dt, { p0, p1, p2 }, tolerance)) {
        return false;
      }
    }

    return true;
  }

  static void _to_quads(const CubicBezier& c, const float tolerance, std::vector<vec2>& sink) {
    // std::vector<vec2> approximation = { c.p0 };

    // approximation.push_back(_to_quad(c, 0.0f, 1.0f));
    // approximation.push_back(c.p3);

    // if (
    //   (dot(approximation[1] - c.p0, c.p1 - c.p0) < 0.0f || dot(approximation[1] - c.p3, c.p2 - c.p3) < 0.0f) ||
    //   !_is_approximation_close(c, approximation, tolerance)
    // ) {
    //   const auto& [left, right] = split(c, 0.5f);

    //   _to_quads(left, tolerance, sink);
    //   _to_quads(right, tolerance, sink);
    // } else {
    //   sink.insert(sink.end(), approximation.begin() + 1, approximation.end());
    // }

    std::vector<vec2> approximation;

    for (int i = 1; i <= GK_MAX_RECURSION; i++) {
      approximation = { c.p0 };

      for (int j = 0; j < i; j++) {
        const float t0 = static_cast<float>(j) / i;
        const float t1 = static_cast<float>(j + 1) / i;

        approximation.push_back(_to_quad(c, t0, t1));
        approximation.push_back(bezier(c.p0, c.p1, c.p2, c.p3, t1));
      }

      if (
        i == 1 && (dot(approximation[1] - c.p0, c.p1 - c.p0) < 0.0f || dot(approximation[1] - c.p3, c.p2 - c.p3) < 0.0f)
      ) {
        continue;
      }

      if (_is_approximation_close(c, approximation, tolerance)) {
        break;
      }
    }

    sink.insert(sink.end(), approximation.begin() + 1, approximation.end());
  }

  static std::vector<vec2> to_quads(const CubicBezier& c, const float tolerance) {
    std::vector<float> extrema = cubic_extrema(c.p0, c.p1, c.p2, c.p3);
    std::vector<vec2> quads = { c.p0 };

    const vec2 A = 3 * (-c.p0 + 3 * c.p1 - 3 * c.p2 + c.p3);
    const vec2 B = 6 * (c.p0 - 2 * c.p1 + c.p2);
    const vec2 C = -3 * (c.p0 - c.p1);

    // To get the inflections C'(t) cross C''(t) = at^2 + bt + c = 0 needs to be solved for 't'.
    // The first cooefficient of the quadratic formula is also the denominator.
    const float den = Math::cross(B, A);

    if (den != 0.0f) {
      // Two roots might exist, solve with quadratic formula ('tl' is real).
      float tc = Math::cross(A, C) / den;
      float tl = tc * tc + Math::cross(B, C) / den;

      // If 'tl < 0' there are two complex roots (no need to solve).
      // If 'tl == 0' there is a real double root at tc (cusp case).
      // If 'tl > 0' two real roots exist at 'tc - Sqrt(tl)' and 'tc + Sqrt(tl)'.
      if (tl > -GK_POINT_EPSILON) {
        tl = std::sqrtf(std::max(0.0f, tl));

        if (tc - tl > GK_POINT_EPSILON && tc - tl < 1.0f - GK_POINT_EPSILON)
          extrema.push_back(tc - tl);
        if (tc + tl > GK_POINT_EPSILON && tc + tl < 1.0f - GK_POINT_EPSILON)
          extrema.push_back(tc + tl);
      } else if (std::fabsf(tl) < GK_POINT_EPSILON) {
        if (tc > GK_POINT_EPSILON && tc < 1.0f - GK_POINT_EPSILON)
          extrema.push_back(tc);
      }
    } else {
      // One real root might exist, solve linear case ('tl' is NaN).
      float tc = -0.5f * Math::cross(C, B) / Math::cross(C, A);

      if (tc > GK_POINT_EPSILON && tc < 1.0f - GK_POINT_EPSILON)
        extrema.push_back(tc);
    }

    // 0 and 1 are already included.
    std::sort(extrema.begin(), extrema.end());

    for (size_t i = 0; i < extrema.size() - 1; i++) {
      float t0 = extrema[i];
      float t1 = extrema[i + 1];

      const auto& [p0, p1, p2, p3] = split_bezier(c.p0, c.p1, c.p2, c.p3, t0, t1);

      _to_quads({ p0, p1, p2, p3 }, tolerance, quads);
    }

    return quads;
  }
#elif 1
  static std::vector<vec2> to_quads(const CubicBezier& c, const float tolerance) {
    std::vector<float> extrema = cubic_extrema(c.p0, c.p1, c.p2, c.p3);
    std::vector<vec2> quads = { c.p0 };

    const vec2 A = 3 * (-c.p0 + 3 * c.p1 - 3 * c.p2 + c.p3);
    const vec2 B = 6 * (c.p0 - 2 * c.p1 + c.p2);
    const vec2 C = -3 * (c.p0 - c.p1);

    // To get the inflections C'(t) cross C''(t) = at^2 + bt + c = 0 needs to be solved for 't'.
    // The first cooefficient of the quadratic formula is also the denominator.
    const float den = Math::cross(B, A);

    if (den != 0.0f) {
      // Two roots might exist, solve with quadratic formula ('tl' is real).
      float tc = Math::cross(A, C) / den;
      float tl = tc * tc + Math::cross(B, C) / den;

      // If 'tl < 0' there are two complex roots (no need to solve).
      // If 'tl == 0' there is a real double root at tc (cusp case).
      // If 'tl > 0' two real roots exist at 'tc - Sqrt(tl)' and 'tc + Sqrt(tl)'.
      if (tl > -GK_POINT_EPSILON) {
        tl = std::sqrtf(std::max(0.0f, tl));

        if (tc - tl > GK_POINT_EPSILON && tc - tl < 1.0f - GK_POINT_EPSILON)
          extrema.push_back(tc - tl);
        if (tc + tl > GK_POINT_EPSILON && tc + tl < 1.0f - GK_POINT_EPSILON)
          extrema.push_back(tc + tl);
      } else if (std::fabsf(tl) < GK_POINT_EPSILON) {
        if (tc > GK_POINT_EPSILON && tc < 1.0f - GK_POINT_EPSILON)
          extrema.push_back(tc);
      }
    } else {
      // One real root might exist, solve linear case ('tl' is NaN).
      float tc = -0.5f * Math::cross(C, B) / Math::cross(C, A);

      if (tc > GK_POINT_EPSILON && tc < 1.0f - GK_POINT_EPSILON)
        extrema.push_back(tc);
    }

    const vec2 bbox_size = cubic_bounding_rect(c.p0, c.p1, c.p2, c.p3).size();
    const float dim = std::max(bbox_size.x, bbox_size.y) / 50.0f;

    // 0 and 1 are already included.
    std::sort(extrema.begin(), extrema.end());

    for (size_t i = 0; i < extrema.size() - 1; i++) {
      float t0 = extrema[i];
      float t1 = extrema[i + 1];

      const auto& [p0, p1, p2, p3] = split_bezier(c.p0, c.p1, c.p2, c.p3, t0, t1);

      approx_monotonic_cubic({ p0, p1, p2, p3 }, tolerance * dim, quads);

      // const vec2 q = approx_quad_control({ p0, p1, p2, p3 }, 0.0f);

      // quads.push_back(q);
      // quads.push_back(p3);
    }

    return quads;
  }
#else
  static vec2 to_quad(const CubicBezier& c) {
    const vec2 p1 = 0.5f * (c.p1 * 3.0f - c.p0);
    const vec2 p2 = 0.5f * (c.p2 * 3.0f - c.p3);

    return Math::midpoint(p1, p2);
  }

  static int number_of_quadratics(const CubicBezier& c, const float tolerance) {
    const vec2 p = c.p0 - 3.0f * c.p1 + 3.0f * c.p2 - c.p3;

    const float err = Math::squared_length(p);
    const int n = std::max(1, static_cast<int>(std::ceilf(std::powf(err / (432.0f * tolerance * tolerance), 1.0f / 6.0f))));

    return n;
  }

  static std::vector<vec2> to_quads(const CubicBezier& c, const float tolerance) {
    std::vector<float> extrema = cubic_extrema(c.p0, c.p1, c.p2, c.p3);
    std::vector<vec2> quads = { c.p0 };

    const vec2 A = 3 * (-c.p0 + 3 * c.p1 - 3 * c.p2 + c.p3);
    const vec2 B = 6 * (c.p0 - 2 * c.p1 + c.p2);
    const vec2 C = -3 * (c.p0 - c.p1);

    // To get the inflections C'(t) cross C''(t) = at^2 + bt + c = 0 needs to be solved for 't'.
    // The first cooefficient of the quadratic formula is also the denominator.
    const float den = Math::cross(B, A);

    if (den != 0.0f) {
      // Two roots might exist, solve with quadratic formula ('tl' is real).
      float tc = Math::cross(A, C) / den;
      float tl = tc * tc + Math::cross(B, C) / den;

      // If 'tl < 0' there are two complex roots (no need to solve).
      // If 'tl == 0' there is a real double root at tc (cusp case).
      // If 'tl > 0' two real roots exist at 'tc - Sqrt(tl)' and 'tc + Sqrt(tl)'.
      if (tl > -GK_POINT_EPSILON) {
        tl = std::sqrtf(std::max(0.0f, tl));

        if (tc - tl > GK_POINT_EPSILON && tc - tl < 1.0f - GK_POINT_EPSILON)
          extrema.push_back(tc - tl);
        if (tc + tl > GK_POINT_EPSILON && tc + tl < 1.0f - GK_POINT_EPSILON)
          extrema.push_back(tc + tl);
      } else if (std::fabsf(tl) < GK_POINT_EPSILON) {
        if (tc > GK_POINT_EPSILON && tc < 1.0f - GK_POINT_EPSILON)
          extrema.push_back(tc);
      }
    } else {
      // One real root might exist, solve linear case ('tl' is NaN).
      float tc = -0.5f * Math::cross(C, B) / Math::cross(C, A);

      if (tc > GK_POINT_EPSILON && tc < 1.0f - GK_POINT_EPSILON)
        extrema.push_back(tc);
    }

    // 0 and 1 are already included.
    std::sort(extrema.begin(), extrema.end());

    for (size_t i = 0; i < extrema.size() - 1; i++) {
      float t0 = extrema[i];
      float t1 = extrema[i + 1];

      const auto& [cp0, cp1, cp2, cp3] = split_bezier(c.p0, c.p1, c.p2, c.p3, t0, t1);

      const int n = number_of_quadratics(c, tolerance);
      const float step = 1.0f / n;

      t0 = 0.0f;

      for (int i = 0; i < n - 1; i++) {
        t1 = t0 + step;

        const auto& [p0, p1, p2, p3] = split_bezier(cp0, cp1, cp2, cp3, t0, t1);

        quads.push_back(to_quad({ p0, p1, p2, p3 }));
        quads.push_back(p3);

        t0 = t1;
      }

      const auto& [p0, p1, p2, p3] = split_bezier(cp0, cp1, cp2, cp3, t0, 1.0f);

      quads.push_back(to_quad({ p0, p1, p2, p3 }));
      quads.push_back(p3);


      // const vec2 q = approx_quad_control({ p0, p1, p2, p3 }, 0.0f);

      // quads.push_back(q);
      // quads.push_back(p3);
    }

    return quads;
  }
#endif

}
