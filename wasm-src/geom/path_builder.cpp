/**
 * @file geom/path_builder.h
 * @brief Containes the PathBuilder class definition and its options.
 */

#include "path_builder.h"

#include "quadratic_bezier.h"
#include "quadratic_path.h"
#include "intersections.h"
#include "curve_ops.h"
#include "path.h"
#include "geom.h"

#include "../math/matrix.h"
#include "../math/math.h"

#include "offset/CubicCurveOffset.h"

namespace graphick::geom {

  /* -- Static Methods -- */

  /**
   * @brief Approximates a circular arc with a series of quadratic bezier curves.
   *
   * @param center The center of the arc.
   * @param from The start point of the arc.
   * @param to The end point of the arc.
   * @param radius The radius of the arc.
   * @param tolerance The error tolerance.
   * @param sink The output path.
   * @param reverse Whether to fill the arc angle or not
   */
  template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
  static void quadratic_arc(const dvec2 center, const dvec2 from, const dvec2 to, const double radius, const double tolerance, QuadraticPath<T>& sink, const bool reverse = false) {
    const double ang1 = std::atan2(from.y - center.y, from.x - center.x);
    const double ang2 = std::atan2(to.y - center.y, to.x - center.x);
    const double dphi = 4.0 * std::acos(std::sqrt(2.0 + tolerance - std::sqrt(tolerance * (2.0 + tolerance))) / std::sqrt(2.0));

    double diff = std::abs(ang2 - ang1);

    if (diff > math::pi<double>) diff = math::two_pi<double> -diff;
    if (reverse) diff = -diff;

    const double diff_abs = std::abs(diff);

    const int segments = static_cast<int>(std::ceil(diff_abs / dphi));
    const double inc = diff / segments;
    const double b = (std::cos(inc) - 1.0) / std::sin(inc);

    for (int i = 0; i <= segments; i++) {
      const double angle = ang1 + i * inc;
      const double sin = std::sin(angle);
      const double cos = std::cos(angle);

      const dvec2 p1 = center + radius * dvec2{ cos - b * sin, sin + b * cos };
      const dvec2 p2 = center + radius * dvec2{ cos, sin };

      sink.quadratic_to(math::Vec2<T>(p1), math::Vec2<T>(p2));
    }
  }

  /**
   * @brief Adds the given cap to the contour.
   *
   * @param from The start point of the cap.
   * @param to The end point of the cap.
   * @param n The normal of the cap.
   * @param radius The radius of the cap.
   * @param cap The cap type.
   * @param sink The contour to add the cap to.
   */
  template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
  static void add_cap(const dvec2 from, const dvec2 to, const dvec2 n, const double radius, const LineCap cap, QuadraticPath<T>& sink) {
    switch (cap) {
    case LineCap::Round: {
      // TODO: angular tolerance
      quadratic_arc(from + (to - from) / 2.0, from, to, radius, 0.01, sink);
      break;
    }
    case LineCap::Square: {
      dvec2 dir = { -n.y * radius, n.x * radius };

      sink.line_to(math::Vec2<T>(from + dir));
      sink.line_to(math::Vec2<T>(to + dir));
      sink.line_to(math::Vec2<T>(to));

      break;
    }
    default:
    case LineCap::Butt: {
      sink.line_to(math::Vec2<T>(to));
      break;
    }
    }
  }

  /**
   * @brief Adds the given join to the contour.
   *
   * @param from The start point of the join.
   * @param to The end point of the join.
   * @param pivot The pivot point of the join.
   * @param from_normal The normal of the start point.
   * @param to_normal The normal of the end point.
   * @param radius The radius of the join.
   * @param inv_miter_limit The inverse miter limit.
   * @param join The join type.
   * @param sink The contour to add the join to.
   * @param reverse Whether to consider the complementary angle.
   */
  template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
  static void add_join(const dvec2 from, const dvec2 to, const dvec2 pivot, const dvec2 from_normal, const dvec2 to_normal, const double radius, const double inv_miter_limit, LineJoin join, QuadraticPath<T>& sink, const bool reverse = false) {
    if (math::is_almost_equal(from, to, math::geometric_epsilon<double>)) {
      return;
    }

    const dvec2 a = from - pivot;
    const dvec2 b = to - pivot;

    double dot = a.x * b.x + a.y * b.y;
    double cross = a.x * b.y - a.y * b.x;

    if (reverse) cross = -cross;

    double ang = std::atan2(cross, dot);

    if (ang < 0.0) ang += math::two_pi<double>;
    if (ang >= math::pi<double>) join = LineJoin::Bevel;

    if (math::is_almost_zero(ang)) {
      return;
    }

    switch (join) {
    case LineJoin::Round: {
      // TODO: tolerance
      quadratic_arc(pivot, from, to, radius, 0.01, sink, reverse);
      break;
    }
    case LineJoin::Miter: {
      double dot = from_normal.x * to_normal.x + from_normal.y * to_normal.y;
      double sin_half = std::sqrt((1.0 + dot) * 0.5);

      if (sin_half < inv_miter_limit) {
        sink.line_to(math::Vec2<T>(to));
        break;
      } else {
        dvec2 mid = from_normal + to_normal;
        double l = radius / (sin_half * std::hypot(mid.x, mid.y));
        dvec2 p = pivot + mid * l;

        sink.line_to(math::Vec2<T>(p));
        sink.line_to(math::Vec2<T>(to));
        break;
      }
    }
    default:
    case LineJoin::Bevel: {
      sink.line_to(math::Vec2<T>(to));
      break;
    }
    }
  }

  /**
   * @brief Offsets a line segment.
   *
   * @param p0 The start point of the line.
   * @param p1 The end point of the line.
   * @param radius The radius to offset the line by.
   * @param sink The output path.
   * @return The normal of the end point.
   */
  template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
  static dvec2 offset_line(const dvec2 p0, const dvec2 p1, const double radius, StrokeOutline<T>& sink) {
    const dvec2 n = math::normal(p0, p1);
    const dvec2 nr = n * radius;

    sink.inner.line_to(math::Vec2<T>(p1 - nr));
    sink.outer.line_to(math::Vec2<T>(p1 + nr));

    return n;
  }

/**
   * @brief Offsets a monotonic quadratic bezier curve.
   *
   * @param p0 The start point of the curve.
   * @param p1 The control point of the curve.
   * @param p2 The end point of the curve.
   * @param radius The radius to offset the curve by.
   * @param tolerance The offset error tolerance.
   * @param sink The output path.
   * @return The normal of the end point.
   */
  template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
  static dvec2 offset_monotonic_quadratic(dvec2 p0, dvec2 p1, dvec2 p2, const double radius, const double tolerance, StrokeOutline<T>& sink) {
    dvec2 start_n = math::normal(p0, p1);

    while (true) {
      const dvec2 a = 2.0 * (p0 - 2.0 * p1 + p2);
      const dvec2 b = 2.0 * (p1 - p0);

      const double aob = math::dot(a, b);
      const double axb = math::cross(a, b);

      if (aob == 0.0) {
        break;
      }

      double t = tolerance * math::squared_length(b) / (std::abs(axb) - tolerance * aob);

      if (!(t > math::geometric_epsilon<double> && t < 1.0 - math::geometric_epsilon<double>)) {
        t = 1.0;
      }

      const auto& [left, right] = split(dquadratic_bezier{ p0, p1, p2 }, t);

      const dvec2 end_n = math::normal(left.p1, left.p2);
      const dvec2 n = start_n + end_n;

      const dvec2 nr1 = n * (2.0 * radius / math::squared_length(n));
      const dvec2 nr2 = end_n * radius;

      sink.inner.quadratic_to(math::Vec2<T>(left.p1 - nr1), math::Vec2<T>(left.p2 - nr2));
      sink.outer.quadratic_to(math::Vec2<T>(left.p1 + nr1), math::Vec2<T>(left.p2 + nr2));

      start_n = end_n;

      if (t == 1.0) {
        break;
      }

      p0 = left.p2;
      p1 = right.p1;
    }

    return start_n;
  }

  /**
   * @brief Offsets a quadratic bezier curve.
   *
   * https://github.com/blend2d/blend2d/blob/master/src/blend2d/pathstroke.cpp
   *
   * @param p0 The start point of the curve.
   * @param p1 The control point of the curve.
   * @param p2 The end point of the curve.
   * @param radius The radius to offset the curve by.
   * @param tolerance The offset error tolerance.
   * @param sink The output path.
   * @return The normal of the end point.
   */
  template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
  static dvec2 offset_quadratic(dvec2 p0, dvec2 p1, dvec2 p2, const double radius, const double tolerance, StrokeOutline<T>& sink) {
    const dvec2 v1 = p1 - p0;
    const dvec2 v2 = p2 - p1;

    const double cross = math::cross(v2, v1);

    if (math::is_almost_zero(cross, tolerance)) {
      const double dot = math::dot(-v1, v2);

      /* Check if the control point lies outside of the start/end points. */
      if (dot > 0.0) {
        /* Rotate all points to x-axis. */
        const double r1 = math::squared_length(v1);
        const double r2 = math::dot(p2 - p0, v1);

        /* Parameter of the cusp if it's within (0, 1). */
        const double t = r1 / (2.0 * r1 - r2);

        if (math::is_normalized(t, false)) {
          const dvec2 p = quadratic({ p0, p1, p2 }, t);
          const dvec2 n = math::normal(p, p2);

          offset_line(p0, p, radius, sink);
          add_cap(dvec2(sink.outer.points.back()), p + n * radius, n, radius, LineCap::Round, sink.outer);
          add_cap(dvec2(sink.inner.points.back()), p - n * radius, -n, radius, LineCap::Round, sink.inner);

          return offset_line(p, p2, radius, sink);
        }
      }

      return offset_line(p0, p2, radius, sink);
    }

    const dvec2 a = 2.0 * (v2 - v1);
    const dvec2 b = 2.0 * (p1 - p0);

    const double bxa = math::cross(b, a);
    const double boa = math::dot(b, a);

    if (bxa == 0) {
      return dvec2{ 0.0, 0.0 };
    }

    const double alen2 = math::squared_length(a);
    const double blen2 = math::squared_length(b);

    const double fac = -1.0 / alen2;
    const double sqrt_ = std::sqrt(boa * boa - alen2 * (blen2 - std::cbrt(radius * radius * bxa * bxa)));

    const double t1 = fac * (boa + sqrt_);
    const double t2 = fac * (boa - sqrt_);

    const uint8_t code = static_cast<uint8_t>(math::is_normalized(t1, false)) |
      (static_cast<uint8_t>(math::is_normalized(t2, false)) << 1);

    switch (code) {
    case 1: {
      const auto& [left, right] = split(dquadratic_bezier{ p0, p1, p2 }, t1);
      offset_monotonic_quadratic(left.p0, left.p1, left.p2, radius, tolerance, sink);
      return offset_monotonic_quadratic(right.p0, right.p1, right.p2, radius, tolerance, sink);
    }
    case 2: {
      const auto& [left, right] = split(dquadratic_bezier{ p0, p1, p2 }, t2);
      offset_monotonic_quadratic(left.p0, left.p1, left.p2, radius, tolerance, sink);
      return offset_monotonic_quadratic(right.p0, right.p1, right.p2, radius, tolerance, sink);
    }
    case 3: {
      const auto& [left, center, right] = split(dquadratic_bezier{ p0, p1, p2 }, t1, t2);
      offset_monotonic_quadratic(left.p0, left.p1, left.p2, radius, tolerance, sink);
      offset_monotonic_quadratic(center.p0, center.p1, center.p2, radius, tolerance, sink);
      return offset_monotonic_quadratic(right.p0, right.p1, right.p2, radius, tolerance, sink);
    }
    case 0:
    default: {
      return offset_monotonic_quadratic(p0, p1, p2, radius, tolerance, sink);
    }
    }

    return dvec2::zero();
  }

  template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
  static dvec2 offset_quadratic(const dquadratic_bezier& quad, const double radius, const double tolerance, StrokeOutline<T>& sink) {
    const dvec2 P = quad.sample(0.5);
    const dvec2 C = circle_center(quad.p0, P, quad.p2);

    const double roh = math::distance(quad.p0, C);
    const double roh_outer = roh + radius;
    const double roh_inner = roh - radius;

    const dvec2 outer_p0 = C + (quad.p0 - C) * (roh_outer / roh);
    const dvec2 outer_p2 = C + (quad.p2 - C) * (roh_outer / roh);
    const dvec2 inner_p0 = C + (quad.p0 - C) * (roh_inner / roh);
    const dvec2 inner_p2 = C + (quad.p2 - C) * (roh_inner / roh);

    quadratic_arc(C, outer_p0, outer_p2, roh_outer, tolerance, sink.outer);
    quadratic_arc(C, inner_p0, inner_p2, roh_inner, tolerance, sink.inner);

    return math::normal(quad.p1, quad.p2);

    // const dvec2 G = (2.0 * quad.p0 + quad.p1 + quad.p2) / 4.0;
    // const dvec2 A = math::midpoint(quad.p0, G);
    // const dvec2 B = math::midpoint(G, quad.p2);

    // const dvec2 na = vec2(quad.p1.y - quad.p0.y, quad.p0.x - quad.p1.x);
    // const dvec2 nb = vec2(quad.p2.y - quad.p1.y, quad.p1.x - quad.p2.x);
    // const dvec2 nc = vec2(quad.p2.y - quad.p0.y, quad.p0.x - quad.p2.x);


    // const dline l1 = { quad.p0, quad.p0 - na };
    // const dline l2 = { quad.p2, quad.p2 - nb };

    // const dline g1 = { math::midpoint(quad.p0, G), math::midpoint(quad.p0, G) - vec2(quad.p1.y - quad.p2.y, quad.p2.x - quad.p1.x) };

  }

#if 0
  // TODO: doc
  constexpr double curve_approximately_straight_epsilon = 1e-5;
  constexpr double curve_completely_straight_epsilon = 1e-15;
  constexpr double curve_point_clump_epsilon = 1e-14;
  constexpr double curve_redundant_tangent_epsilon = 1e-12;
  constexpr double arc_center_comparison_epsilon = 1e-8;
  constexpr double maximum_arc_radius = 1e6;
  constexpr double arc_probe_positions[] = {
    0.2,
    0.4,
    0.6,
    0.8
  };

  inline static double unit_turn(const dvec2 p1, const dvec2 p2, const dvec2 p3) {
    return math::cross(math::normalize(p2 - p1), math::normalize(p3 - p1));
  }

  static dline start_tangent(const dcubic_bezier& cubic) {
    if (math::is_almost_equal(cubic.p0, cubic.p1)) {
      if (math::is_almost_equal(cubic.p0, cubic.p2)) {
        return { cubic.p0, cubic.p3 };
      }

      return { cubic.p0, cubic.p2 };
    }

    return { cubic.p0, cubic.p1 };
  }

  static dline end_tangent(const dcubic_bezier& cubic) {
    if (math::is_almost_equal(cubic.p2, cubic.p3)) {
      if (math::is_almost_equal(cubic.p1, cubic.p2)) {
        return { cubic.p3, cubic.p0 };
      }

      return { cubic.p3, cubic.p1 };
    }

    return { cubic.p3, cubic.p2 };
  }

  /**
   * Represents curve tangents as two line segments and some precomputed data.
   */
  struct CurveTangentData final {
    explicit CurveTangentData(const dcubic_bezier& cubic) {
      start_tangent = geom::start_tangent(cubic);

      end_tangent = geom::end_tangent(cubic);

      turn_1 = unit_turn(start_tangent.p0, start_tangent.p1, end_tangent.p0);
      turn_2 = unit_turn(start_tangent.p0, end_tangent.p1, end_tangent.p0);

      start_unit_normal = math::normal(start_tangent.p0, start_tangent.p1);
      end_unit_normal = math::normal(end_tangent.p0, end_tangent.p1);
    }

    dline start_tangent;
    dline end_tangent;

    dvec2 start_unit_normal;
    dvec2 end_unit_normal;

    double turn_1 = 0.0;
    double turn_2 = 0.0;
  };

  static bool is_curve_approximately_straight(const CurveTangentData& d) {
    const dvec2 min = math::min(d.start_tangent.p0, d.end_tangent.p0);
    const dvec2 max = math::max(d.start_tangent.p0, d.end_tangent.p0);

    const dvec2 p1 = d.start_tangent.p1;
    const dvec2 p2 = d.end_tangent.p1;

    return (
      // Is p1 located between p0 and p3?
      min.x <= p1.x &&
      min.y <= p1.y &&
      max.x >= p1.x &&
      max.y >= p1.y &&
      // Is p2 located between p0 and p3?
      min.x <= p2.x &&
      min.y <= p2.y &&
      max.x >= p2.x &&
      max.y >= p2.y &&
      // Are all points collinear?
      math::is_almost_zero(d.turn_1, curve_approximately_straight_epsilon) &&
      math::is_almost_zero(d.turn_2, curve_approximately_straight_epsilon)
    );
  }

  inline static bool is_curve_completely_straight(const CurveTangentData& d) {
    return (
      math::is_almost_zero(d.turn_1, curve_completely_straight_epsilon) &&
      math::is_almost_zero(d.turn_2, curve_completely_straight_epsilon)
    );
  }

  /**
   * Returns true if an attempt to approximate a curve with given tangents
   * should be made.
   */
  static bool can_try_arc_offset(const CurveTangentData& d) {
    // Arc approximation is only attempted if curve is not considered
    // approximately straight. But it can be attemped for curves which have
    // their control points on the different sides of line connecting points
    // p0 and p3.
    //
    // We need to make sure we don't try to do arc approximation for these S
    // type curves because such curves cannot be approximated by arcs in such
    // cases.

    return (
      (d.turn_1 >= curve_approximately_straight_epsilon && d.turn_2 >= curve_approximately_straight_epsilon) ||
      (d.turn_1 <= -curve_approximately_straight_epsilon && d.turn_2 <= -curve_approximately_straight_epsilon)
    );
  }

  static math::CubicSolutions<double> ray_intersections(const dcubic_bezier& cubic, const dvec2 pa, const dvec2 pb) {
    const dvec2 v = pb - pa;

    const double ax = (cubic.p0.y - pa.y) * v.x - (cubic.p0.x - pa.x) * v.y;
    const double bx = (cubic.p1.y - pa.y) * v.x - (cubic.p1.x - pa.x) * v.y;
    const double cx = (cubic.p2.y - pa.y) * v.x - (cubic.p2.x - pa.x) * v.y;
    const double dx = (cubic.p3.y - pa.y) * v.x - (cubic.p3.x - pa.x) * v.y;

    const double a = dx;
    const double b = cx * 3.0;
    const double c = bx * 3.0;

    const double D = ax;
    const double A = a - (D - c + b);
    const double B = b + (3.0 * D - 2.0 * c);
    const double C = c - (3.0 * D);

    return math::solve_cubic_normalized(A, B, C, D);
  }

  /**
 * Returns true if circular arc with given parameters approximate curve close
 * enough.
 *
 * @param arc_center Point where arc center is located.
 *
 * @param arc_radius Radius of arc.
 *
 * @param curve Curve being approximated with arc.
 *
 * @param tolerance Maximum allowed error.
 */
  static bool is_good_arc(const dvec2 arc_center, const double arc_radius,
    const dcubic_bezier& curve, const double tolerance,
    const double t_from, const double t_to
  ) {
    if (arc_radius > maximum_arc_radius) {
      return false;
    }

    const double e = std::min(tolerance, arc_radius / 3.0);

    // Calculate value equal to slightly more than half of maximum error.
    // Slightly more to minimize false negatives due to finite precision in
    // circle-line intersection test.
    const double me = (e * (0.5 + 1e-4));

    for (int i = 0; i < 4; i++) {
      const double t = arc_probe_positions[i];

      // Find t on a given curve.
      const double curve_t = math::lerp(t_from, t_to, t);

      // Find point and normal at this position.
      const dvec2 point = curve.sample(curve_t);
      const dvec2 d = curve.derivative(curve_t);
      const dvec2 n = math::normalize(dvec2{ -d.y, d.x });

      // Create line segment which has its center at curve on point and
      // extends by half of maximum allowed error to both directions from
      // curve point along normal.
      const dline segment(point + (n * me), point - (n * me));

      // Test if intersection exists.
      if (line_circle_intersection_points(segment, arc_center, arc_radius).empty()) {
        return false;
      }
    }

    return true;
  }

  template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
  static bool try_arc_approximation(const dcubic_bezier& cubic, const CurveTangentData& d, const double radius, const double tolerance, StrokeOutline<T>& outline) {
    if (!can_try_arc_offset(d)) {
      return false;
    }

    // Cast ray from curve end points to start and end tangent directions.
    const dvec2 vector_from = math::normalize(d.start_tangent.p1 - d.start_tangent.p0);
    const dvec2 vector_to = math::normalize(d.end_tangent.p1 - d.end_tangent.p0);
    const double denom = vector_to.x * vector_from.y - vector_to.y * vector_from.x;

    // Should not happen as we already elliminated parallel case.
    if (math::is_almost_zero(denom)) {
      return false;
    }

    const dvec2 asv = d.start_tangent.p0;
    const dvec2 bsv = d.end_tangent.p0;
    const double u = ((bsv.y - asv.y) * vector_to.x - (bsv.x - asv.x) * vector_to.y) / denom;
    const double v = ((bsv.y - asv.y) * vector_from.x - (bsv.x - asv.x) * vector_from.y) / denom;

    if (u < 0.0 || v < 0.0) {
        // Intersection is on the wrong side.
      return false;
    }

    const dvec2 V = asv + (u * vector_from);

    // If start or end tangents extend too far beyond intersection, return
    // early since it will not result in good approximation.
    if (
      math::squared_distance(cubic.p0, V) < (math::squared_distance(d.start_tangent.p0, d.start_tangent.p1) * 0.25) ||
      math::squared_distance(cubic.p3, V) < (math::squared_distance(d.end_tangent.p0, d.end_tangent.p1) * 0.25)
    ) {
      return false;
    }

    const double p3VDistance = math::distance(cubic.p3, V);
    const double p0VDistance = math::distance(cubic.p0, V);
    const double p0p3Distance = math::distance(cubic.p0, cubic.p3);

    const dvec2 G = (p3VDistance * cubic.p0 + p0VDistance * cubic.p3 + p0p3Distance * V) / (p3VDistance + p0VDistance + p0p3Distance);

    // const dline p0G(cubic.p0, G);
    // const dline Gp3(G, cubic.p3);

    const dvec2 p0G_mid = math::midpoint(cubic.p0, G);
    const dvec2 Gp3_mid = math::midpoint(G, cubic.p3);

    const dline E(p0G_mid, p0G_mid - dvec2(G.y - cubic.p0.y, cubic.p0.x - G.x));
    const dline E1(d.start_tangent.p0, d.start_tangent.p0 -
      dvec2(d.start_tangent.p1.y - d.start_tangent.p0.y, d.start_tangent.p0.x - d.start_tangent.p1.x));

    const std::optional<dvec2> C1 = line_line_intersection_point(E, E1);

    if (!C1.has_value()) {
      return false;
    }

    const math::CubicSolutions<double> intersections = ray_intersections(cubic, C1.value(), G);

    if (intersections.count != 1) {
      return false;
    }

    const double tG = intersections.solutions[0];
    const double dist0 = math::distance(G, cubic.sample(tG));

    if (dist0 > tolerance) {
      return false;
    }

    const dline F(Gp3_mid, Gp3_mid - dvec2(cubic.p3.y - G.y, G.x - cubic.p3.x));
    const dline F1(d.end_tangent.p0, d.end_tangent.p0 +
      dvec2(d.end_tangent.p1.y - d.end_tangent.p0.y, d.end_tangent.p0.x - d.end_tangent.p1.x));

    const std::optional<dvec2> C2 = line_line_intersection_point(F, F1);

    if (!C2.has_value()) {
      return false;
    }

    if (math::is_almost_equal(C1.value(), C2.value(), arc_center_comparison_epsilon)) {
      const double r = math::distance(C1.value(), cubic.p0);

      if (is_good_arc(C1.value(), r, cubic, tolerance, 0, 1)) {
        const bool clockwise = geom::clockwise(std::vector<dvec2>{ cubic.p0, V, cubic.p3 });

        quadratic_arc(C1.value(), cubic.p0, cubic.p3, r, tolerance, outline.inner, !clockwise);
        return true;
      }
    } else {
      const double r1 = math::distance(C1.value(), cubic.p0);

      if (!is_good_arc(C1.value(), r1, cubic, tolerance, 0, tG)) {
        return false;
      }

      const double r2 = math::distance(C2.value(), cubic.p3);

      if (!is_good_arc(C2.value(), r2, cubic, tolerance, tG, 1)) {
        return false;
      }

      const bool clockwise = geom::clockwise(std::vector<dvec2>{ cubic.p0, V, cubic.p3 });

      quadratic_arc(C1.value(), cubic.p0, V, r1, tolerance, outline.inner, !clockwise);
      quadratic_arc(C2.value(), V, cubic.p3, r2, tolerance, outline.inner, !clockwise);

      return true;
    }

    return false;
  }

  static math::CubicSolutions<double> find_max_curvature(const dcubic_bezier& cubic) {
    const double axx = cubic.p1.x - cubic.p0.x;
    const double bxx = cubic.p2.x - 2.0 * cubic.p1.x + cubic.p0.x;
    const double cxx = cubic.p3.x + 3.0 * (cubic.p1.x - cubic.p2.x) - cubic.p0.x;

    const double cox0 = cxx * cxx;
    const double cox1 = 3.0 * bxx * cxx;
    const double cox2 = 2.0 * bxx * bxx + cxx * axx;
    const double cox3 = axx * bxx;

    const double ayy = cubic.p1.y - cubic.p0.y;
    const double byy = cubic.p2.y - 2.0 * cubic.p1.y + cubic.p0.y;
    const double cyy = cubic.p3.y + 3.0 * (cubic.p1.y - cubic.p2.y) - cubic.p0.y;

    const double coy0 = cyy * cyy;
    const double coy1 = 3.0 * byy * cyy;
    const double coy2 = 2.0 * byy * byy + cyy * ayy;
    const double coy3 = ayy * byy;

    const double coe0 = cox0 + coy0;
    const double coe1 = cox1 + coy1;
    const double coe2 = cox2 + coy2;
    const double coe3 = cox3 + coy3;

    return math::solve_cubic(coe0, coe1, coe2, coe3);
  }

  /**
   * If all points of the curve are collinear, a shortcut must be made because
   * general offsetting algorithm does not handle such curves very well. In case
   * where are points are collinear, lines between cusps are offset to direction
   * of their normals and at the points where curve has a cusps, semi-circles
   * are added to the output.
   */
  template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
  static void offset_linear_cuspy_curve(const dcubic_bezier& cubic,
    const double radius, const math::CubicSolutions<double>& maximum_curvature_points, StrokeOutline<T>& outline
  ) {
    const dline start_tangent = geom::start_tangent(cubic);
    const dvec2 normal = math::normal(start_tangent.p0, start_tangent.p1);

    dvec2 previous_point = start_tangent.P0;
    dvec2 previous_offset_point = previous_point + (normal * offset);

    for (uint8_t i = 0; i < maximum_curvature_points.count; i++) {
      // Skip 0 and 1!
      const double t = maximum_curvature_points.solutions[i];
      const dvec2 derived = cubic.derivative(t);
      const double length_squared = math::squared_length(derived);

      if (length_squared <= 1e-9) {
        // Cusp. Since we know all curve points are on the same line, some
        // of maximum curvature points will have nearly zero length
        // derivative vectors.
        const dvec2 point_at_cusp = cubic.sample(t);

        // Draw line from previous point to point at cusp.
        const dline l(previous_point, point_at_cusp);
        const dvec2 n = math::normal(l.p0, l.p1);
        const dvec2 to = point_at_cusp + (n * radius);

        outline.outer.line_to(math::Vec2<T>(to));

        // Draw semi circle at cusp.
        const dvec2 arcTo = point_at_cusp - (n * radius);

        quadratic_arc(point_at_cusp, to, arcTo, radius, 0.01, outline.outer, false);

        previous_point = point_at_cusp;
        previous_offset_point = arcTo;
      }
    }

    const dline endTangent = end_tangent(cubic);
    const dvec2 normal2 = math::normal(endTangent.P0, endTangent.P1);

    outline.outer.line_to(math::Vec2<T>(endTangent.P0 - (normal2 * offset)));
  }

  template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
  static void do_approximate_bezier(const dcubic_bezier& cubic, const CurveTangentData& d, const double radius, const double tolerance, StrokeOutline<T>& outline) {
    // First find maximum curvature positions.
    const math::CubicSolutions<double> maximum_curvature_positions = find_max_curvature(cubic);

    // Handle special case where the input curve is a straight line, but
    // control points do not necessary lie on line segment between curve
    // points P0 and P3.
    if (is_curve_completely_straight(d)) {
      // TODO: implement


      // OffsetLinearCuspyCurve(curve, builder, offset,
      //     maximumCurvaturePositions, numMaximumCurvaturePositions);
      return;
    }

    // Now find inflection point positions.
    double inflections[2];

    const int numInflections = curve.FindInflections(inflections);

    // Merge maximum curvature and inflection point positions.
    double t[5];

    const int count0 = MergeCurvePositions(t, 0, inflections, numInflections, 1e-7);

    const int count = MergeCurvePositions(t, count0, maximumCurvaturePositions,
        numMaximumCurvaturePositions, 1e-5);

    Quicksort(t, count, [](const double a, const double b) -> bool {
      return a < b;
    });

    if (count == 0) {
        // No initial subdivision suggestions.
      ApproximateBezier(curve, d, builder, offset, maximumError);
    } else {
      double previousT = 0;

      for (int i = 0; i < count; i++) {
        const double T = t[i];
        const dvec2 derivative = curve.DerivedAt(T);
        const double length_squared = derivative.LengthSquared();

        if (length_squared < CuspDerivativeLengthSquared) {
            // Squared length of derivative becomes tiny. This is where
            // the cusp is. The goal here is to find a spon on curve,
            // located before T which has large enough derivative and draw
            // circular arc to the next point on curve with large enough
            // derivative.

          const double t1 = FindPositionOnCurveWithLargeEnoughDerivative(
              curve, previousT, T);

          ASSERT(t1 < T);

          const CubicCurve k = curve.GetSubcurve(previousT, t1);
          const CurveTangentData nd(k);

          ApproximateBezier(k, nd, builder, offset, maximumError);

          const double t2 = FindPositionOnCurveWithLargeEnoughDerivativeStart(
              curve, T, i == (count - 1) ? 1.0 : t[i + 1]);

          ASSERT(t2 > T);

          builder.CuspPoint = curve.PointAt(T);
          builder.NeedsCuspArc = true;
          builder.CuspArcClockwise = dvec2::IsTriangleClockwise(
              k.P3, builder.CuspPoint, curve.PointAt(t2));

          previousT = t2;
        } else {
            // Easy, feed subcurve between previous and current t values
            // to offset approximation function.

          const CubicCurve k = curve.GetSubcurve(previousT, T);
          const CurveTangentData nd(k);

          ApproximateBezier(k, nd, builder, offset, maximumError);

          previousT = T;
        }
      }

      ASSERT(previousT < 1.0);

      const CubicCurve k = curve.GetSubcurve(previousT, 1.0);
      const CurveTangentData nd(k);

      ApproximateBezier(k, nd, builder, offset,
          maximumError);
    }
  }

  template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
  static void offset_cubic(const dcubic_bezier& cubic, const double radius, const double tolerance, StrokeOutline<T>& outline) {
    const drect bounds = cubic.approx_bounding_rect();
    const dvec2 delta = bounds.max - bounds.min;

    if (math::is_almost_zero(delta, curve_point_clump_epsilon)) {
      return;
    }

    // Select biggest dimension.
    const double m = std::max(delta.x, delta.y) / 2.0;

    // Calculate scaled radius.
    const double sr = radius / m;

    // TODO: decide how to handle this case
    // if (math::is_almost_zero(sr)) {
    //   return;
    // }

    // Calculate "normalized" curve which kind of fits into range from -1 to 1.
    const dvec2 t = (bounds.min + bounds.max) / 2.0;

    dvec2 p0 = cubic.p0 - t;
    dvec2 p1 = cubic.p1 - t;
    dvec2 p2 = cubic.p2 - t;
    dvec2 p3 = cubic.p3 - t;

    // Flatten ends of curve if control points are too close to end points.
    if (math::squared_distance(p0, p1) < curve_redundant_tangent_epsilon) {
      p1 = p0;
    }

    if (math::squared_distance(p3, p2) < curve_redundant_tangent_epsilon) {
      p2 = p3;
    }

    const dcubic_bezier c(p0 / m, p1 / m, p2 / m, p3 / m);
    const CurveTangentData d(c);

    if (is_curve_approximately_straight(d)) {
      if (is_curve_completely_straight(d)) {
        // Curve is extremely close to being straight, use simple line
        // translation.
        const dvec2 n = math::normal(c.p0, c.p3);
        const dvec2 nr = n * sr;

        outline.inner.line_to(math::Vec2<T>((c.p3 - nr) * m + t));
        outline.outer.line_to(math::Vec2<T>((c.p3 + nr) * m + t));
      } else {
        // Curve is almost straight. Translate start and end tangents
        // separately and generate a cubic curve.
        const dvec2 start_nr = d.start_unit_normal * sr;
        const dvec2 end_nr = d.end_unit_normal * sr;

        cubic_to_quadratics(CubicBezier<T>{
          math::Vec2<T>((d.start_tangent.p0 - start_nr)* m + t),
            math::Vec2<T>((d.start_tangent.p1 - start_nr)* m + t),
            math::Vec2<T>((d.end_tangent.p1 + end_nr)* m + t),
            math::Vec2<T>((d.end_tangent.p0 + end_nr)* m + t)
        }, T(2e-2), outline.inner);

        cubic_to_quadratics(CubicBezier<T>{
          math::Vec2<T>((d.start_tangent.p0 + start_nr)* m + t),
            math::Vec2<T>((d.start_tangent.p1 + start_nr)* m + t),
            math::Vec2<T>((d.end_tangent.p1 - end_nr)* m + t),
            math::Vec2<T>((d.end_tangent.p0 - end_nr)* m + t)
        }, T(2e-2), outline.outer);
      }
    } else {
      // Arbitrary curve.
      // Try arc approximation first in case this curve was intended to
      // approximate circle. If that is indeed true, we avoid a lot of
      // expensive calculations like finding inflection and maximum
      // curvature points.
      if (!try_arc_approximation(c, d, sr, tolerance, outline)) {
        do_approximate_bezier(c, d, sr, tolerance, outline);
      }
    }
  }
#endif

  /**
   * @brief Flattens a quadratic bezier curve and outputs the line segments to a sink vector.
   *
   * This method uses a fast flattening algorithm that will create lots of extra lines if the curve is too large.
   *
   * @tparam T The type of the output coordinates.
   * @param quad The quadratic bezier curve to flatten.
   * @param tolerance The tolerance to use when flattening the curve.
   * @param sink_callback The callback to output the lines to.
   */
  template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
  static void fast_flatten(
    const dquadratic_bezier& quad,
    const double tolerance,
    std::function<void(const math::Vec2<T>, const math::Vec2<T>)> sink_callback
  ) {
    const auto& [a, b, c] = quad.coefficients();
    const double dt = std::sqrt((2.0 * tolerance) / math::length(quad.p0 - 2.0 * quad.p1 + quad.p2));

    dvec2 last = quad.p0;
    double t = dt;

    while (t < 1.0) {
      const double t_sq = t * t;
      const dvec2 p = a * t_sq + b * t + c;

      sink_callback(math::Vec2<T>(last), math::Vec2<T>(p));

      last = p;
      t += dt;
    }

    sink_callback(math::Vec2<T>(last), math::Vec2<T>(quad.p2));
  }

  /**
   * @brief Recursively flattens a quadratic bezier curve and outputs the line segments to a sink vector.
   *
   * This method uses a recursive flattening algorithm that will create less extra lines than the fast flattening algorithm.
   *
   * @tparam T The type of the output coordinates.
   * @param quad The quadratic bezier curve to flatten.
   * @param clip The rectangle to clip the curve to.
   * @param tolerance_sq The squared tolerance to use when flattening the curve.
   * @param sink_callback The callback to output the lines to.
   * @param depth The current recursion depth.
   */
  template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
  static void recursive_flatten(
    const dquadratic_bezier& quad,
    const drect& clip,
    const double tolerance_sq,
    std::function<void(const math::Vec2<T>, const math::Vec2<T>)> sink_callback,
    uint8_t depth = 0
  ) {
    if (depth > math::max_recursion_depth<uint8_t>) {
      sink_callback(math::Vec2<T>(quad.p0), math::Vec2<T>(quad.p2));
      return;
    }

    const drect bounds = quad.approx_bounding_rect();

    if (!does_rect_intersect_rect(bounds, clip)) {
      return;
    }

    depth += 1;

    const dvec2 p01 = (quad.p0 + quad.p1) * 0.5;
    const dvec2 p12 = (quad.p1 + quad.p2) * 0.5;
    const dvec2 p012 = (p01 + p12) * 0.5;

    const double den = math::squared_distance(quad.p0, quad.p2);
    const double num = std::abs(
      (quad.p2.x - quad.p0.x) * (quad.p0.y - p012.y) -
      (quad.p0.x - p012.x) * (quad.p2.y - quad.p0.y)
    );

    const double sq_error = num * num / den;

    if (sq_error < tolerance_sq) {
      sink_callback(math::Vec2<T>(quad.p0), math::Vec2<T>(quad.p2));
      return;
    }

    recursive_flatten(dquadratic_bezier{ quad.p0, p01, p012 }, clip, tolerance_sq, sink_callback, depth);
    recursive_flatten(dquadratic_bezier{ p012, p12, quad.p2 }, clip, tolerance_sq, sink_callback, depth);
  }

  /* -- PathBuilder -- */

  template <typename T, typename _>
  PathBuilder<T, _>::PathBuilder(
    const QuadraticPath<T>& path,
    const math::Mat2x3<T>& transform,
    const math::Rect<T>* bounding_rect
  ) :
    m_path(path),
    m_transform(dmat2x3(transform))
  {
    drect bounds = bounding_rect ? drect(*bounding_rect) : drect(path.approx_bounding_rect());
    m_bounding_rect = m_transform * bounds;
  }

  template <typename T, typename _>
  void PathBuilder<T, _>::flatten(
    const math::Rect<T>& clip,
    const T tolerance,
    std::function<void(const math::Vec2<T>, const math::Vec2<T>)> sink_callback
  ) const {
    if (m_path.empty()) {
      return;
    }

    const drect clipping_rect = drect(clip);
    const double coverage = rect_rect_intersection_area(m_bounding_rect, clipping_rect) / m_bounding_rect.area();

    if (coverage <= 0.0) {
      return;
    } else if (coverage <= 0.5) {
      flatten_clipped(drect(clip), static_cast<double>(tolerance) * static_cast<double>(tolerance), sink_callback);
    } else {
      flatten_unclipped(static_cast<double>(tolerance), sink_callback);
    }
  }

  template <typename T, typename _>
  StrokeOutline<T> PathBuilder<T, _>::stroke(const StrokingOptions<T>& options, const T tolerance) const {
    if (m_path.empty()) {
      return {};
    }

    dquadratic_bezier quad = { m_transform * dvec2(m_path[0]), m_transform * dvec2(m_path[1]), m_transform * dvec2(m_path[2]) };

    const double radius = static_cast<double>(options.width) * 0.5;
    const double inv_miter_limit = 1.0 / static_cast<double>(options.miter_limit);

    StrokeOutline<T> outline;

    if (m_path.size() == 1 && (math::is_almost_equal(quad.p0, quad.p1) && math::is_almost_equal(quad.p1, quad.p2))) {
      if (options.cap != LineCap::Butt) {
        return outline;
      }

      dvec2 n = { 0.0, 1.0 };
      dvec2 nr = n * radius;
      dvec2 start = quad.p0 + nr;
      dvec2 rstart = quad.p0 - nr;

      outline.outer.move_to(math::Vec2<T>(start));

      add_cap(start, rstart, n, radius, options.cap, outline.outer);
      add_cap(rstart, start, -n, radius, options.cap, outline.outer);

      return outline;
    }

    dvec2 pivot = quad.p0;
    dvec2 last_p1 = quad.p1;
    dvec2 last_n = math::normal(quad.p0, quad.p1);

    if (m_path.closed()) {
      outline.inner.move_to(math::Vec2<T>(quad.p0 - last_n * radius));
      outline.outer.move_to(math::Vec2<T>(quad.p0 + last_n * radius));
    } else {
      const dvec2 start = quad.p0 - last_n * radius;

      outline.inner.move_to(math::Vec2<T>(start));
      outline.outer.move_to(math::Vec2<T>(start));

      add_cap(start, quad.p0 + last_n * radius, -last_n, radius, options.cap, outline.outer);
    }

    for (size_t i = 0; i < m_path.size(); i++) {
      quad = {
        m_transform * dvec2(m_path[i * 2]),
        m_transform * dvec2(m_path[i * 2 + 1]),
        m_transform * dvec2(m_path[i * 2 + 2])
      };

      const dvec2 start_n = math::normal(quad.p0, quad.p1);
      const dvec2 start_nr = start_n * radius;

      const dvec2 a = quad.p1 - quad.p0;
      const dvec2 b = last_p1 - quad.p0;

      double cos = math::dot(a, b) / (math::length(a) * math::length(b));

      // if (cos > 0 || std::abs(cos) < 1.0 - math::geometric_epsilon<double>) {
      const dvec2 inner_start = quad.p0 - start_nr;
      const dvec2 outer_start = quad.p0 + start_nr;

      add_join(dvec2(outline.inner.points.back()), inner_start, pivot, -last_n, -start_n, radius, inv_miter_limit, options.join, outline.inner, true);
      add_join(dvec2(outline.outer.points.back()), outer_start, pivot, last_n, start_n, radius, inv_miter_limit, options.join, outline.outer);
    // }

      if (math::is_almost_equal(quad.p1, quad.p2)) {
        // Linear segment.

        outline.inner.line_to(math::Vec2<T>(quad.p2 - start_nr));
        outline.outer.line_to(math::Vec2<T>(quad.p2 + start_nr));

        last_n = start_n;
        last_p1 = quad.p0;
      } else {
        // Quadratic segment.

        // outline.inner.line_to(math::Vec2<T>(quad.p2 - start_nr));
        // outline.outer.line_to(math::Vec2<T>(quad.p2 + start_nr));

        // last_n = start_n;
        last_n = offset_quadratic(quad.p0, quad.p1, quad.p2, radius, tolerance, outline);
        // last_n = offset_quadratic(quad, radius, tolerance, outline);
        last_p1 = quad.p1;
      }

      pivot = quad.p2;
    }

    if (m_path.closed()) {
      const dvec2 start_n = math::normal(dvec2(m_path[0]), dvec2(m_path[1]));

      add_join(dvec2(outline.inner.points.back()), dvec2(outline.inner.points.front()), pivot, -last_n, -start_n, radius, inv_miter_limit, options.join, outline.inner, true);
      add_join(dvec2(outline.outer.points.back()), dvec2(outline.outer.points.front()), pivot, last_n, start_n, radius, inv_miter_limit, options.join, outline.outer);
    } else {
      add_cap(dvec2(outline.outer.points.back()), dvec2(outline.inner.points.back()), last_n, radius, options.cap, outline.outer);

      outline.outer.points.insert(outline.outer.points.end(), outline.inner.points.rbegin() + 1, outline.inner.points.rend());
      outline.inner.points.clear();
    }

    return outline;
  }

  template <typename T, typename _>
  StrokeOutline<T> PathBuilder<T, _>::stroke(const Path<T, std::enable_if<true>>& path, const StrokingOptions<T>& options, const T tolerance) const {
    if (path.empty()) {
      return {};
    }

    Path<T>::Segment segment = path.front();

    const double radius = static_cast<double>(options.width) * 0.5;
    const double inv_miter_limit = 1.0 / static_cast<double>(options.miter_limit);

    StrokeOutline<T> outline;

    if (path.size() == 1 && segment.is_point()) {
      if (options.cap != LineCap::Butt) {
        return outline;
      }

      dvec2 p0 = m_transform * dvec2(segment.p0);

      dvec2 n = { 0.0, 1.0 };
      dvec2 nr = n * radius;
      dvec2 start = p0 + nr;
      dvec2 rstart = p0 - nr;

      outline.outer.move_to(math::Vec2<T>(start));

      add_cap(start, rstart, n, radius, options.cap, outline.outer);
      add_cap(rstart, start, -n, radius, options.cap, outline.outer);

      return outline;
    }

    dvec2 p0 = m_transform * dvec2(segment.p0);
    dvec2 p1 = m_transform * dvec2(segment.p1);

    dvec2 pivot = p0;
    dvec2 last_p1 = p1;
    dvec2 last_n = math::normal(p0, p1);

    if (path.closed()) {
      outline.inner.move_to(math::Vec2<T>(p0 - last_n * radius));
      outline.outer.move_to(math::Vec2<T>(p0 + last_n * radius));
    } else {
      const dvec2 start = p0 - last_n * radius;

      outline.inner.move_to(math::Vec2<T>(start));
      outline.outer.move_to(math::Vec2<T>(start));

      add_cap(start, p0 + last_n * radius, -last_n, radius, options.cap, outline.outer);
    }

    for (uint32_t i = 1, j = 1; i < path.m_commands_size; i++) {
      const dvec2 p0 = m_transform * dvec2(path.m_points[j - 1]);
      const dvec2 p1 = m_transform * dvec2(path.m_points[j]);

      const dvec2 start_n = math::normal(p0, p1);
      const dvec2 start_nr = start_n * radius;

      const dvec2 inner_start = p0 - start_nr;
      const dvec2 outer_start = p0 + start_nr;

      add_join(dvec2(outline.inner.points.back()), inner_start, pivot, -last_n, -start_n, radius, inv_miter_limit, options.join, outline.inner, true);
      add_join(dvec2(outline.outer.points.back()), outer_start, pivot, last_n, start_n, radius, inv_miter_limit, options.join, outline.outer);

      switch (path.get_command(i)) {
      case Path<T>::Command::Line: {
        outline.inner.line_to(math::Vec2<T>(p1 - start_nr));
        outline.outer.line_to(math::Vec2<T>(p1 + start_nr));

        last_n = start_n;

        j += 1;
        break;
      }
      case Path<T>::Command::Quadratic: {
        j += 2;
        break;
      }
      case Path<T>::Command::Cubic: {
        const dvec2 p2 = m_transform * dvec2(path.m_points[j + 1]);
        const dvec2 p3 = m_transform * dvec2(path.m_points[j + 2]);

        // TODO: don't recalculate normals
        const dvec2 end_n = math::normal(p2, p3);
        const dvec2 end_nr = end_n * radius;

        {
          CubicCurveBuilder builder((quadratic_path&)outline.outer);

          OffsetCurve(dcubic_bezier{ p0, p1, p2, p3 }, radius, tolerance, builder);
        }

        {
          CubicCurveBuilder builder((quadratic_path&)outline.inner);
          OffsetCurve(dcubic_bezier{ p0, p1, p2, p3 }, -radius, tolerance, builder);
        }

        last_n = end_n;

        j += 3;
        break;
      }
      default:
      case Path<T>::Command::Move:
        j += 1;
        break;
      }
    }

    if (path.closed()) {
      const dvec2 start_n = math::normal(dvec2(path.m_points[0]), dvec2(path.m_points[1]));

      add_join(dvec2(outline.inner.points.back()), dvec2(outline.inner.points.front()), pivot, -last_n, -start_n, radius, inv_miter_limit, options.join, outline.inner, true);
      add_join(dvec2(outline.outer.points.back()), dvec2(outline.outer.points.front()), pivot, last_n, start_n, radius, inv_miter_limit, options.join, outline.outer);
    } else {
      add_cap(dvec2(outline.outer.points.back()), dvec2(outline.inner.points.back()), last_n, radius, options.cap, outline.outer);

      outline.outer.points.insert(outline.outer.points.end(), outline.inner.points.rbegin() + 1, outline.inner.points.rend());
      outline.inner.points.clear();
    }

    return outline;
  }

  template <typename T, typename _>
  void PathBuilder<T, _>::flatten_clipped(
    const drect& clip,
    const double tolerance_sq,
    std::function<void(const math::Vec2<T>, const math::Vec2<T>)> sink_callback
  ) const {
    dvec2 p0 = m_transform * dvec2(m_path[0]);

    for (size_t i = 0; i < m_path.size(); i++) {
      const dvec2 p1 = m_transform * dvec2(m_path[i * 2 + 1]);
      const dvec2 p2 = m_transform * dvec2(m_path[i * 2 + 2]);

      if (math::is_almost_equal(p1, p2)) {
        sink_callback(math::Vec2<T>(p0), math::Vec2<T>(p2));
      } else {
        recursive_flatten(dquadratic_bezier{ p0, p1, p2 }, clip, tolerance_sq, sink_callback);
      }

      p0 = p2;
    }
  }

  template <typename T, typename _>
  void PathBuilder<T, _>::flatten_unclipped(
    const double tolerance,
    std::function<void(const math::Vec2<T>, const math::Vec2<T>)> sink_callback
  ) const {
    dvec2 p0 = m_transform * dvec2(m_path[0]);

    for (size_t i = 0; i < m_path.size(); i++) {
      const dvec2 p1 = m_transform * dvec2(m_path[i * 2 + 1]);
      const dvec2 p2 = m_transform * dvec2(m_path[i * 2 + 2]);

      if (p1 == p2) {
        sink_callback(math::Vec2<T>(p0), math::Vec2<T>(p2));
      } else {
        fast_flatten(dquadratic_bezier{ p0, p1, p2 }, tolerance, sink_callback);
      }

      p0 = p2;
    }
  }

  /* -- Template instantiations -- */

  template class PathBuilder<float>;
  template class PathBuilder<double>;

}
