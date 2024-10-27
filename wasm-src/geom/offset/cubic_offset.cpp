/**
 * @file geom/offset/cubic_offset.cpp
 * @brief Implementation of cubic offsetting.
 *
 * The following implementation is an adaptation of the algorithm in
 * <https://github.com/aurimasg/cubic-bezier-offsetter/blob/main/C%2B%2B/CubicCurveOffset.cpp>.
 */

#include "offset.h"

#include "../curve_ops.h"
#include "../geom.h"
#include "../intersections.h"

#include "../../math/math.h"

#include <algorithm>

namespace graphick::geom {

/**
 * @brief The maximum iteration count for locating the nearest point to the cusp,
 * which has a sufficiently long first derivative to identify the starting or
 * ending points of a circular arc for the cusp.
 */
static constexpr int near_cusp_point_search_max_iteration_count = 18;

/**
 * @brief Once the process to identify an offset curve is initiated, the squared lengths
 * of all the edges of the encompassing polygon are computed and summed. If this total
 * length is less than or equal to a certain value, the derived curve is instantly
 * rejected and not added to the output.
 */
static constexpr double max_tiny_curve_polygon_perimeter_squared = 1e-7;

/**
 * @brief If a good circular arc approximation of a curve is found, but its radius is
 * very close to the offset amount, the scaled arc can collapse to a point or
 * almost a point. This is an epsilon for testing if the arc is large enough.
 * Arcs with radius smaller than this value will not be added to the output.
 */
static constexpr double min_arc_radius = 1e-8;

/**
 * @brief An upper limit of arc radius. Circular arcs with calculated radius greater
 * than this value will not be considered as accepted approximations of curve
 * segments.
 */
static constexpr double max_arc_radius = 1e+6;

/**
 * @brief Offsetter does not attempt to find exact cusp locations and does not
 * consider cusp only to be where the derivative vector length is exactly
 * zero.
 */
static constexpr double cusp_derivative_length_squared = 1.5e-4;

/**
 * @brief If X and Y components of all points are equal when compared with this
 * epsilon, the curve is considered a point.
 */
static constexpr double curve_point_clump_test_epsilon = 1e-14;

/**
 * @brief Epsilon used to compare coordinates of circular arc centers to see if they
 * can be merged into a single circular arc.
 */
static constexpr double arc_center_comparison_epsilon = 1e-8;

/**
 * @brief When testing if curve is almost straight, cross products of unit vectors
 * are calculated as follows:
 *
 *     turn1 = (p0 → p1) ⨯ (p0 → p3)
 *     turn2 = (p1 → p2) ⨯ (p0 → p3)
 *
 * Where p0, p1, p2 and p3 are curve points and (X → Y) are unit vectors going
 * from X to a direction of Y.
 *
 * Then these values are compared with zero. If they both are close to zero,
 * curve is considered approximately straight. This is the epsilon used for
 * comparing turn1 and turn2 values to zero.
 */
static constexpr double approximately_straight_curve_test_epsilon = 1e-5;

/**
 * @brief The logic is the same as for approximately_straight_curve_test_epsilon value.
 * This value is used to determine if curve is completely straight, not just
 * approximately straight.
 */
static constexpr double completely_straight_curve_test_epsilon = 1e-15;

/**
 * @brief A list of positions on curve for testing if circular arc approximation of a
 * curve is good enough. All values must be from 0 to 1. Positions 0 and 1
 * should not be included here because distances at 0 and 1 are either
 * guaranteed to be exactly on the curve being approximated (on the sides of
 * the input curve) or they are tested before using another methods (ray cast
 * from arc center to triangle incenter point).
 */
static constexpr double arc_probe_positions[] = {0.2, 0.4, 0.6, 0.8};

/**
 * @brief A list of positions on curve for testing if candidate offset curve is good
 * enough. From points on original curve at each of these positions a ray is
 * cast to normal direction and intersection is found with offset candidate.
 * Then distance is checked to see if it is within maximum error. 0 and 1
 * should not be added in this list because candidate points at 0 and 1 is
 * guaranteed to be at the right place already.
 *
 * Note that testing involves cubic root finding which is not very cheap
 * operation. Adding more testing points increases precision, but also
 * increases the time spent for testing if candidate is good.
 */
static constexpr double simple_offset_probe_positions[] = {0.25, 0.5, 0.75};

/**
 * @brief Keeps data needed to generate a set of output segments.
 *
 * @struct OutputBuilder
 */
template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
struct OutputBuilder {
  constexpr OutputBuilder(CubicPath<T>& path, const double scale, const dvec2 translation) :
    path(path),
    scale(scale),
    translation(translation),
    previous_point(dvec2::zero()),
    previous_point_t(dvec2::zero()),
    cusp_point(dvec2::zero()) { }

  CubicPath<T>& path;

  dvec2 previous_point;
  dvec2 previous_point_t;
  dvec2 cusp_point;

  bool needs_cusp_arc = false;
  bool cusp_arc_clockwise = false;

  const double scale;
  const dvec2 translation;
};

/**
 * @brief Called once when the first point of output is calculated.
 *
 * @param builder The output builder.
 * @param p0 The first point of the output.
 */
template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
static void move_to(OutputBuilder<T>& builder, const dvec2 p0) {
  builder.previous_point = p0;
  builder.previous_point_t = (p0 * builder.scale) + builder.translation;
}

/**
 * @brief Called when a new line needs to be added to the output.
 *
 * @param builder The output builder.
 * @param p1 The end point of the line.
 */
template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
static void line_to(OutputBuilder<T>& builder, const dvec2 p1) {
  const dvec2 previous = builder.previous_point;

  if (previous != p1) {
    const dvec2 t = (p1 * builder.scale) + builder.translation;

    builder.path.line_to(math::Vec2<T>(t));

    builder.previous_point = p1;
    builder.previous_point_t = t;
  }
}

/**
 * @brief Called when a new quadratic curve needs to be added to the output.
 *
 * @param builder The output builder.
 * @param p1 The control point of the quadratic curve.
 * @param p2 The end point of the quadratic curve.
 */
template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
static void quadratic_to(OutputBuilder<T>& builder, const dvec2 p1, const dvec2 p2) {
  const dvec2 previous = builder.previous_point;

  if (previous != p1 || previous != p2) {
    const dvec2 c = (p1 * builder.scale) + builder.translation;
    const dvec2 t = (p2 * builder.scale) + builder.translation;

    builder.path.quadratic_to(math::Vec2<T>(c), math::Vec2<T>(t));

    builder.previous_point = p2;
    builder.previous_point_t = t;
  }
}

/**
 * @brief Called when a new cubic curve needs to be added to the output.
 *
 * @param builder The output builder.
 * @param p1 The first control point of the cubic curve.
 * @param p2 The second control point of the cubic curve.
 * @param p3 The end point of the cubic curve.
 */
template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
static void cubic_to(OutputBuilder<T>& builder, const dvec2 p1, const dvec2 p2, const dvec2 p3) {
  const dvec2 previous = builder.previous_point;

  if (previous != p1 || previous != p2 || previous != p3) {
    const dvec2 c1 = (p1 * builder.scale) + builder.translation;
    const dvec2 c2 = (p2 * builder.scale) + builder.translation;
    const dvec2 t = (p3 * builder.scale) + builder.translation;

    builder.path.cubic_to(math::Vec2<T>(c1), math::Vec2<T>(c2), math::Vec2<T>(t));

    builder.previous_point = p3;
    builder.previous_point_t = t;
  }
}

/**
 * @brief Called when a new arc needs to be added to the output.
 *
 * @param builder The output builder.
 * @param center The center of the arc.
 * @param to The end point of the arc.
 * @param clockwise Whether the arc is clockwise.
 */
template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
static void arc_to(OutputBuilder<T>& builder, const dvec2 center, const dvec2 to, const bool clockwise) {
  const dvec2 previous = builder.previous_point;

  if (previous != center || previous != to) {
    const dvec2 c = (center * builder.scale) + builder.translation;
    const dvec2 t = (to * builder.scale) + builder.translation;

    builder.path.arc_to(math::Vec2<T>(c), math::Vec2<T>(t), clockwise);

    builder.previous_point = to;
    builder.previous_point_t = t;
  }
}

/**
 * @brief Check if the approximation needs to add a cusp arc.
 *
 * @param builder The output builder.
 * @param to The end of the possible cusp.
 */
template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
static void maybe_add_cusp_arc(OutputBuilder<T>& builder, const dvec2 to) {
  if (builder.needs_cusp_arc) {
    builder.needs_cusp_arc = false;

    arc_to(builder, builder.cusp_point, to, builder.cusp_arc_clockwise);

    builder.cusp_point = dvec2::zero();
    builder.cusp_arc_clockwise = false;
  }
}

/**
 * @brief Returns the t values of the intersections between a cubic bezier and an infinite line.
 *
 * @param cubic The cubic bezier.
 * @param pa The start point of the line.
 * @param pb The end point of the line.
 * @return The t values of the intersections between a cubic bezier and an infinite line.
 */
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
 * @brief Returns true if the curve is close enough to be considered parallel to the original curve.
 *
 * @param original The original curve.
 * @param parallel Candidate parallel curve to be tested.
 * @param offset Offset from original curve to candidate parallel curve.
 * @param tolerance Maximum allowed error.
 * @return True if the curve is close enough to be considered parallel to the original curve.
 */
static bool accept_offset(
  const dcubic_bezier& original,
  const dcubic_bezier& parallel,
  const double offset,
  const double tolerance
) {
  // Using shape control method, sometimes output curve becomes completely
  // off in some situations involving start and end tangents being almost
  // parallel. These two checks are to prevent accepting such curves as good.
  if (geom::clockwise(original.p0, original.p1, original.p3) != geom::clockwise(parallel.p0, parallel.p1, parallel.p3)) {
    return false;
  }

  if (geom::clockwise(original.p0, original.p2, original.p3) != geom::clockwise(parallel.p0, parallel.p2, parallel.p3)) {
    return false;
  }

  for (int i = 0; i < sizeof(simple_offset_probe_positions) / sizeof(simple_offset_probe_positions[0]); i++) {
    const double t = simple_offset_probe_positions[i];
    const dvec2 op0 = original.sample(t);
    const dvec2 n = original.raw_normal(t);

    const math::CubicSolutions<double> intersections = ray_intersections(parallel, op0, op0 + n);

    if (intersections.count != 1) {
      return false;
    }

    const dvec2 p0 = parallel.sample(intersections.solutions[0]);
    const double d = math::distance(op0, p0);
    const double error = std::abs(d - std::abs(offset));

    if (error > tolerance) {
      return false;
    }
  }

  return true;
}

/**
 * @brief Creates a circular arc offset of a cubic bezier curve.
 *
 * @param cubic The input cubic bezier curve.
 * @param offset The offset amount. Can be negative.
 * @param center The center of the circular arc.
 * @param from The start point of the circular arc.
 * @param to The end point of the circular arc.
 * @param clockwise Whether the arc is clockwise.
 */
template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
static void arc_offset(
  OutputBuilder<T>& builder,
  const double offset,
  const dvec2 center,
  const dvec2 from,
  const dvec2 to,
  const bool clockwise
) {
  dline l1(center, from);
  dline l2(center, to);

  const dvec2 v1 = math::normalize(l1.p1 - l1.p0);
  const dvec2 v2 = math::normalize(l2.p1 - l2.p0);

  if (clockwise) {
    l1.p1 += v1 * offset;
    l2.p1 += v2 * offset;
  } else {
    l1.p1 -= v1 * offset;
    l2.p1 -= v2 * offset;
  }

  maybe_add_cusp_arc(builder, l1.p1);

  // Determine if it is clockwise again since arc orientation may have
  // changed if arc radius was smaller than offset.
  //
  // Also it is important to use previous point to determine orientation
  // instead of the point we just calculated as the start of circular arc
  // because for small arcs a small numeric error can result in incorrect
  // arc orientation.
  arc_to(builder, center, l2.p1, geom::clockwise(center, builder.previous_point, l2.p1));
}

/**
 * @brief Returns the unit turn of three points.
 *
 * @param p1 The first point.
 * @param p2 The second point.
 * @param p3 The third point.
 * @return The unit turn.
 */
static double unit_turn(const dvec2 p1, const dvec2 p2, const dvec2 p3) {
  return math::cross(math::normalize(p2 - p1), math::normalize(p3 - p1));
}

/**
 * @brief Represents curve tangents as two line segments and some precomputed data.
 *
 * @struct CurveTangentData
 */
struct CurveTangentData {
  CurveTangentData(const dcubic_bezier& curve) :
    start_tangent(curve.start_tangent()),
    end_tangent(curve.end_tangent()),
    turn1(unit_turn(start_tangent.p0, start_tangent.p1, end_tangent.p0)),
    turn2(unit_turn(start_tangent.p0, end_tangent.p1, end_tangent.p0)),
    start_unit_normal(start_tangent.normal()),
    end_unit_normal(end_tangent.normal()) { }

  const dline start_tangent;
  const dline end_tangent;

  const dvec2 start_unit_normal;
  const dvec2 end_unit_normal;

  const double turn1;
  const double turn2;
};

/**
 * @brief Returns true if an attempt to approximate a curve with given tangents should be made.
 *
 * @param d The curve tangent data.
 * @return True if an attempt to approximate a curve with given tangents should be made.
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

  static constexpr double P = approximately_straight_curve_test_epsilon;
  static constexpr double N = -P;

  return ((d.turn1 >= P && d.turn2 >= P) || (d.turn1 <= N && d.turn2 <= N));
}

/**
 * @brief Returns true if an attempt to use simple offsetting for a curve with given
 * tangents should be made.
 *
 * @param d The curve tangent data.
 * @return True if an attempt to use simple offsetting for a curve with given tangents should be made.
 */
static bool can_try_simple_offset(const CurveTangentData& d) {
  // Arc approximation is only attempted if curve is not considered
  // approximately straight. But it can be attemped for curves which have
  // their control points on the different sides of line connecting points
  // p0 and p3.
  //
  // We need to make sure we don't try to do arc approximation for these S
  // type curves because the shape control method behaves really badly with
  // S shape curves.

  return ((d.turn1 >= 0 && d.turn2 >= 0) || (d.turn1 <= 0 && d.turn2 <= 0));
}

/**
 * @brief Returns true if curve is considered too small to be added to offset output.
 *
 * @param curve The curve to check.
 * @return True if curve is considered too small to be added to offset output.
 */
static bool curve_is_too_tiny(const dcubic_bezier& curve) {
  const double lengthsSquared = math::squared_distance(curve.p0, curve.p1) + math::squared_distance(curve.p1, curve.p2) +
    math::squared_distance(curve.p2, curve.p3);

  return lengthsSquared <= max_tiny_curve_polygon_perimeter_squared;
}

/**
 * @brief Attempts to perform simple curve offsetting and returns true if it succeeds
 * to generate good enough parallel curve.
 *
 * @param curve The input curve.
 * @param d The curve tangent data.
 * @param builder The output builder.
 * @param offset The offset amount.
 * @param tolerance The maximum allowed error.
 * @return True if it succeeds to generate good enough parallel curve.
 */
template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
static bool try_simple_curve_offset(
  const dcubic_bezier& curve,
  const CurveTangentData& d,
  OutputBuilder<T>& builder,
  const double offset,
  const double tolerance
) {
  if (!can_try_simple_offset(d)) {
    return false;
  }

  const dvec2 d1 = curve.p1 - curve.p0;
  const dvec2 d2 = curve.p2 - curve.p3;
  const double div = math::cross(d1, d2);

  if (math::is_almost_zero(div)) {
    return false;
  }

  // Start point.
  const dvec2 p0 = d.start_tangent.p0 + (d.start_tangent.normal() * offset);

  // End point.
  const dvec2 p3 = d.end_tangent.p0 - (d.end_tangent.normal() * offset);

  // Middle point.
  const dvec2 mp = curve.sample(0.5);
  const dvec2 mpN = curve.normal(0.5);
  const dvec2 p = mp + (mpN * offset);

  const dvec2 bxby = (8.0 / 3.0) * (p - (0.5 * (p0 + p3)));

  const double a = math::cross(bxby, d2) / div;
  const double b = math::cross(d1, bxby) / div;

  const dvec2 p1(p0.x + a * d1.x, p0.y + a * d1.y);
  const dvec2 p2(p3.x + b * d2.x, p3.y + b * d2.y);

  const dcubic_bezier candidate(p0, p1, p2, p3);

  if (curve_is_too_tiny(candidate)) {
    // If curve is too tiny, tell caller there was a great success.
    return true;
  }

  if (!accept_offset(curve, candidate, offset, tolerance)) {
    return false;
  }

  maybe_add_cusp_arc(builder, candidate.p0);

  cubic_to(builder, candidate.p1, candidate.p2, candidate.p3);

  return true;
}

/**
 * @brief Check if the array contains a given merge value.
 *
 * @param a The array.
 * @param count The number of elements in the array.
 * @param value The value to check.
 * @param epsilon The epsilon value.
 * @return True if the array contains a given merge value.
 */
static bool DoubleArrayContainsMergePosition(const double* a, const int count, const double value, const double epsilon) {
  for (int i = 0; i < count; i++) {
    const double v = a[i];

    if (math::is_almost_equal(value, v, epsilon)) {
      return true;
    }
  }

  return false;
}

/**
 * @brief Merges curve positions into an array.
 *
 * @param t The array to merge into.
 * @param t_count The number of elements in the array.
 * @param s The array to merge from.
 * @param count The number of elements in the array to merge from.
 * @param epsilon The epsilon value.
 * @return The new number of elements in the array.
 */
static int merge_curve_positions(double t[5], const int t_count, const double* s, const int count, const double epsilon) {
  int rc = t_count;

  for (int i = 0; i < count; i++) {
    const double v = s[i];

    if (math::is_almost_zero(v, epsilon)) {
      continue;
    }

    if (math::is_almost_equal(v, 1.0, epsilon)) {
      continue;
    }

    if (DoubleArrayContainsMergePosition(t, rc, v, epsilon)) {
      continue;
    }

    t[rc++] = v;
  }

  return rc;
}

/**
 * @brief Returns true if circular arc with given parameters approximate curve close enough.
 *
 * @param arc_center Point where arc center is located.
 * @param arc_radius Radius of arc.
 * @param curve Curve being approximated with arc.
 * @param tolerance Maximum allowed error.
 */
static bool good_arc(
  const dvec2 arc_center,
  const double arc_radius,
  const dcubic_bezier& curve,
  const double tolerance,
  const double t_from,
  const double t_to
) {
  if (arc_radius > max_arc_radius) {
    return false;
  }

  const double e = std::min(tolerance, arc_radius / 3.0);

  // Calculate value equal to slightly more than half of maximum error.
  // Slightly more to minimize false negatives due to finite precision in
  // circle-line intersection test.
  const double me = (e * (0.5 + 1e-4));

  for (int i = 0; i < sizeof(arc_probe_positions) / sizeof(arc_probe_positions[0]); i++) {
    const double t = arc_probe_positions[i];

    // Find t on a given curve.
    const double curve_t = math::lerp(t, t_from, t_to);

    // Find point and normal at this position.
    const dvec2 point = curve.sample(curve_t);
    const dvec2 n = curve.normal(curve_t);

    // Create line segment which has its center at curve on point and
    // extends by half of maximum allowed error to both directions from
    // curve point along normal.
    const dline segment(point + (n * me), point - (n * me));

    // Test if intersection exists.
    if (!does_line_intersect_circle(segment, arc_center, arc_radius)) {
      return false;
    }
  }

  return true;
}

/**
 * @brief Attempts to use circular arc offsetting method on a given curve.
 *
 * @param curve The input curve.
 * @param d The curve tangent data.
 * @param builder The output builder.
 * @param offset The offset amount.
 * @param tolerance The maximum allowed error.
 * @return True if it succeeds to generate good enough parallel curve.
 */
template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
static bool try_arc_approximation(
  const dcubic_bezier& curve,
  const CurveTangentData& d,
  OutputBuilder<T>& builder,
  const double offset,
  const double tolerance
) {
  if (!can_try_arc_offset(d)) {
    return false;
  }

  // Cast ray from curve end points to start and end tangent directions.
  const dvec2 vectorFrom = d.start_tangent.direction();
  const dvec2 vectorTo = d.end_tangent.direction();
  const double denom = vectorTo.x * vectorFrom.y - vectorTo.y * vectorFrom.x;

  // Should not happen as we already elliminated parallel case.
  if (math::is_almost_zero(denom)) {
    return false;
  }

  const dvec2 asv = d.start_tangent.p0;
  const dvec2 bsv = d.end_tangent.p0;
  const double u = ((bsv.y - asv.y) * vectorTo.x - (bsv.x - asv.x) * vectorTo.y) / denom;
  const double v = ((bsv.y - asv.y) * vectorFrom.x - (bsv.x - asv.x) * vectorFrom.y) / denom;

  if (u < 0.0 || v < 0.0) {
    // Intersection is on the wrong side.
    return false;
  }

  const dvec2 V = asv + (u * vectorFrom);

  // If start or end tangents extend too far beyond intersection, return
  // early since it will not result in good approximation.
  if (math::squared_distance(curve.p0, V) < (d.start_tangent.squared_length() * 0.25) ||
      math::squared_distance(curve.p3, V) < (d.end_tangent.squared_length() * 0.25)) {
    return false;
  }

  const double p3VDistance = math::distance(curve.p3, V);
  const double p0VDistance = math::distance(curve.p0, V);
  const double p0p3Distance = math::distance(curve.p0, curve.p3);
  const dvec2 G =
    (p3VDistance * curve.p0 + p0VDistance * curve.p3 + p0p3Distance * V) / (p3VDistance + p0VDistance + p0p3Distance);

  const dline p0G(curve.p0, G);
  const dline Gp3(G, curve.p3);

  const dline E(p0G.midpoint(), p0G.midpoint() - p0G.raw_normal());
  const dline E1(d.start_tangent.p0, d.start_tangent.p0 - d.start_tangent.raw_normal());

  const std::optional<dvec2> C1 = line_line_intersection_point_infinite(E, E1);

  if (!C1.has_value()) {
    return false;
  }

  const math::CubicSolutions<double> intersections = ray_intersections(curve, C1.value(), G);

  if (intersections.count != 1) {
    return false;
  }

  const double tG = intersections.solutions[0];
  const double dist0 = math::distance(G, curve.sample(tG));

  if (dist0 > tolerance) {
    return false;
  }

  const dline F(Gp3.midpoint(), Gp3.midpoint() - Gp3.raw_normal());
  const dline F1(d.end_tangent.p0, d.end_tangent.p0 + d.end_tangent.raw_normal());

  const std::optional<dvec2> C2 = line_line_intersection_point_infinite(F, F1);

  if (!C2.has_value()) {
    return false;
  }

  if (math::is_almost_equal(C1.value(), C2.value(), arc_center_comparison_epsilon)) {
    const double radius = math::distance(C1.value(), curve.p0);

    if (good_arc(C1.value(), radius, curve, tolerance, 0, 1)) {
      const bool clockwise = geom::clockwise(curve.p0, V, curve.p3);

      arc_offset(builder, offset, C1.value(), curve.p0, curve.p3, clockwise);

      return true;
    }
  } else {
    const double radius1 = math::distance(C1.value(), curve.p0);

    if (!good_arc(C1.value(), radius1, curve, tolerance, 0, tG)) {
      return false;
    }

    const double radius2 = math::distance(C2.value(), curve.p3);

    if (!good_arc(C2.value(), radius2, curve, tolerance, tG, 1)) {
      return false;
    }

    const bool clockwise = geom::clockwise(curve.p0, V, curve.p3);

    arc_offset(builder, offset, C1.value(), curve.p0, G, clockwise);
    arc_offset(builder, offset, C2.value(), G, curve.p3, clockwise);

    return true;
  }

  return false;
}

/**
 * @brief Returns true if curve is considered approximately straight.
 *
 * @param d The curve tangent data.
 * @return True if curve is considered approximately straight.
 */
static bool is_curve_approximately_straight(const CurveTangentData& d) {
  const double minx = std::min(d.start_tangent.p0.x, d.end_tangent.p0.x);
  const double miny = std::min(d.start_tangent.p0.y, d.end_tangent.p0.y);
  const double maxx = std::max(d.start_tangent.p0.x, d.end_tangent.p0.x);
  const double maxy = std::max(d.start_tangent.p0.y, d.end_tangent.p0.y);

  const double x1 = d.start_tangent.p1.x;
  const double y1 = d.start_tangent.p1.y;
  const double x2 = d.end_tangent.p1.x;
  const double y2 = d.end_tangent.p1.y;

  return (
    // Is p1 located between p0 and p3?
    minx <= x1 && miny <= y1 && maxx >= x1 && maxy >= y1 &&
    // Is P2 located between p0 and p3?
    minx <= x2 && miny <= y2 && maxx >= x2 && maxy >= y2 &&
    // Are all points collinear?
    math::is_almost_zero(d.turn1, approximately_straight_curve_test_epsilon) &&
    math::is_almost_zero(d.turn2, approximately_straight_curve_test_epsilon)
  );
}

/**
 * @brief Returns true if the control points of the curve are almost collinear.
 *
 * @param c The curve.
 * @return True if curve is considered approximately straight.
 */
static bool is_curve_approximately_straight(const geom::dcubic_bezier& c) {
  return geom::collinear(c.p0, c.p1, c.p3, 5e-3) && geom::collinear(c.p0, c.p2, c.p3, 5e-3);
}

/**
 * @brief Returns true if curve is considered completely straight.
 *
 * @param d The curve tangent data.
 * @return True if curve is considered completely straight.
 */
static bool is_curve_completely_straight(const CurveTangentData& d) {
  return (
    math::is_almost_zero(d.turn1, completely_straight_curve_test_epsilon) &&
    math::is_almost_zero(d.turn2, completely_straight_curve_test_epsilon)
  );
}

/**
 * @brief Main function for approximating offset of a curve without cusps.
 *
 * @param curve The curve to approximate.
 * @param d The curve tangent data.
 * @param builder The output builder.
 * @param offset The offset amount.
 * @param tolerance The maximum allowed error.
 */
template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
static void approximate_bezier(
  const dcubic_bezier& curve,
  const CurveTangentData& d,
  OutputBuilder<T>& builder,
  const double offset,
  const double tolerance
) {
  if (!curve.is_point(curve_point_clump_test_epsilon)) {
    if (is_curve_approximately_straight(d)) {
      if (is_curve_completely_straight(d)) {
        // Curve is extremely close to being straight.
        const dline line(curve.p0, curve.p1);
        const dvec2 normal = line.normal();

        maybe_add_cusp_arc(builder, line.p0 + (normal * offset));

        line_to(builder, line.p1 + (normal * offset));
      } else {
        const dvec2 p1o = d.start_tangent.p0 + (offset * d.start_unit_normal);
        const dvec2 p2o = d.start_tangent.p1 + (offset * d.start_unit_normal);
        const dvec2 p3o = d.end_tangent.p1 - (offset * d.end_unit_normal);
        const dvec2 p4o = d.end_tangent.p0 - (offset * d.end_unit_normal);

        maybe_add_cusp_arc(builder, p1o);

        cubic_to(builder, p2o, p3o, p4o);
      }
    } else {
      if (!try_simple_curve_offset(curve, d, builder, offset, tolerance)) {
        if (!try_arc_approximation(curve, d, builder, offset, tolerance)) {
          // Split in half and continue.
          const auto& [a, b] = split(curve, 0.5);

          const CurveTangentData da(a);

          approximate_bezier(a, da, builder, offset, tolerance);

          const CurveTangentData db(b);

          approximate_bezier(b, db, builder, offset, tolerance);
        }
      }
    }
  }
}

/**
 * @brief Find position on curve where derivative is large enough to appoximate a cusp.
 *
 * @param curve The curve.
 * @param previous_t The previous t value.
 * @param current_t The current t value.
 * @return The position on curve where derivative is large enough to appoximate a cusp.
 */
static double find_position_on_curve_with_large_enough_derivative(
  const dcubic_bezier& curve,
  const double previous_t,
  const double current_t
) {
  GK_ASSERT(current_t > previous_t, "Current t must be greater than previous t.");

  static constexpr double k_precision = cusp_derivative_length_squared * 2.0;

  double t = std::max(math::lerp(previous_t, current_t, 0.8), current_t - 0.05);

  for (int i = 0; i < near_cusp_point_search_max_iteration_count; i++) {
    const dvec2 derivative = curve.derivative(t);
    const double length_squared = math::squared_length(derivative);

    if (length_squared < k_precision) {
      return t;
    }

    const double a = t + current_t;

    t = a / 2.0;
  }

  return t;
}

/**
 * @brief Find position on curve where derivative is large enough to appoximate a cusp.
 *
 * @param curve The curve.
 * @param current_t The current t value.
 * @param next_t The next t value.
 * @return The position on curve where derivative is large enough to appoximate a cusp.
 */
static double find_position_on_curve_with_large_enough_derivative_start(
  const dcubic_bezier& curve,
  const double current_t,
  const double next_t
) {
  GK_ASSERT(current_t < next_t, "Current t must be less than next t.");

  static constexpr double k_precision = cusp_derivative_length_squared * 2.0;

  double t = std::min(math::lerp(current_t, next_t, 0.2), current_t + 0.05);

  for (int i = 0; i < near_cusp_point_search_max_iteration_count; i++) {
    const dvec2 derivative = curve.derivative(t);
    const double length_squared = math::squared_length(derivative);

    if (length_squared < k_precision) {
      return t;
    }

    const double a = current_t + t;

    t = a / 2.0;
  }

  return t;
}

/**
 * @brief If all points of the curve are collinear, a shortcut must be made because
 * general offsetting algorithm does not handle such curves very well. In case
 * where are points are collinear, lines between cusps are offset to direction
 * of their normals and at the points where curve has a cusps, semi-circles
 * are added to the output.
 *
 * @param curve The curve to offset.
 * @param builder The output builder.
 * @param offset The offset amount.
 * @param max_curvature_points The maximum curvature points.
 * @param max_curvature_point_count The maximum curvature point count.
 */
template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
static void offset_linear_cuspy_curve(
  const dcubic_bezier& curve,
  OutputBuilder<T>& builder,
  const double offset,
  const double* max_curvature_points,
  const int max_curvature_point_count
) {
  const dline start_tangent = curve.start_tangent();
  const dvec2 normal = start_tangent.normal();

  dvec2 previous_point = start_tangent.p0;
  dvec2 previous_offset_point = previous_point + (normal * offset);

  move_to(builder, previous_offset_point);

  for (int i = 0; i < max_curvature_point_count; i++) {
    // Skip 0 and 1!
    const double t = max_curvature_points[i];
    const dvec2 derived = curve.derivative(t);
    const double length_squared = math::squared_length(derived);

    if (length_squared <= 1e-9) {
      // Cusp. Since we know all curve points are on the same line, some
      // of maximum curvature points will have nearly zero length
      // derivative vectors.
      const dvec2 point_at_cusp = curve.sample(t);

      // Draw line from previous point to point at cusp.
      const dline l(previous_point, point_at_cusp);
      const dvec2 n = l.normal();
      const dvec2 to = point_at_cusp + (n * offset);

      line_to(builder, to);

      // Draw semi circle at cusp.
      const dvec2 arc_to_pos = point_at_cusp - (n * offset);

      arc_to(builder, point_at_cusp, arc_to_pos, geom::clockwise(previous_point, previous_offset_point, point_at_cusp));

      previous_point = point_at_cusp;
      previous_offset_point = arc_to_pos;
    }
  }

  const dline end_tangent = curve.end_tangent();
  const dvec2 normal2 = end_tangent.normal();

  line_to(builder, end_tangent.p0 - (normal2 * offset));
}

/**
 * @brief Approximates offset of a cubic bezier curve.
 *
 * @param curve The curve to approximate.
 * @param d The curve tangent data.
 * @param builder The output builder.
 * @param offset The offset amount.
 * @param tolerance The maximum allowed error.
 */
template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
static void do_approximate_bezier(
  const dcubic_bezier& curve,
  const CurveTangentData& d,
  OutputBuilder<T>& builder,
  const double offset,
  const double tolerance
) {
  // First find maximum curvature positions.
  const math::CubicSolutions<double> max_curvature_positions = geom::max_curvature(curve);

  // Handle special case where the input curve is a straight line, but
  // control points do not necessary lie on line segment between curve
  // points p0 and p3.
  if (is_curve_completely_straight(d)) {
    offset_linear_cuspy_curve(curve, builder, offset, max_curvature_positions.solutions, max_curvature_positions.count);
    return;
  }

  // Now find inflection point positions.
  const math::QuadraticSolutions<double> inflections = geom::inflections(curve);

  // Merge maximum curvature and inflection point positions.
  double t[5];

  const int count0 = merge_curve_positions(t, 0, inflections.solutions, inflections.count, 1e-7);
  const int count = merge_curve_positions(t, count0, max_curvature_positions.solutions, max_curvature_positions.count, 1e-5);

  std::sort(t, t + count);

  if (count == 0) {
    // No initial subdivision suggestions.
    approximate_bezier(curve, d, builder, offset, tolerance);
  } else {
    double previous_t = 0;

    for (int i = 0; i < count; i++) {
      const double ti = t[i];
      const dvec2 derivative = curve.derivative(ti);
      const double length_squared = math::squared_length(derivative);

      if (length_squared < cusp_derivative_length_squared) {
        // Squared length of derivative becomes tiny. This is where
        // the cusp is. The goal here is to find a spon on curve,
        // located before T which has large enough derivative and draw
        // circular arc to the next point on curve with large enough
        // derivative.

        const double t1 = find_position_on_curve_with_large_enough_derivative(curve, previous_t, ti);

        GK_ASSERT(t1 < ti, "t1 must be less than t.");

        const dcubic_bezier k = extract(curve, previous_t, t1);
        const CurveTangentData nd(k);

        approximate_bezier(k, nd, builder, offset, tolerance);

        const double t2 = find_position_on_curve_with_large_enough_derivative_start(curve, ti, i == (count - 1) ? 1.0 : t[i + 1]);

        GK_ASSERT(t2 > ti, "t2 must be greater than t.");

        builder.cusp_point = curve.sample(ti);
        builder.needs_cusp_arc = true;
        builder.cusp_arc_clockwise = clockwise(k.p3, builder.cusp_point, curve.sample(t2));

        previous_t = t2;
      } else {
        // Easy, feed subcurve between previous and current t values
        // to offset approximation function.

        const dcubic_bezier k = extract(curve, previous_t, ti);
        const CurveTangentData nd(k);

        approximate_bezier(k, nd, builder, offset, tolerance);

        previous_t = ti;
      }
    }

    GK_ASSERT(previous_t < 1.0, "Previous t must be less than 1.");

    const dcubic_bezier k = extract(curve, previous_t, 1.0);
    const CurveTangentData nd(k);

    approximate_bezier(k, nd, builder, offset, tolerance);
  }
}

/**
 * @brief Flattens ends of curves if control points are too close to end points.
 *
 * @param curve The curve to fix.
 * @return The fixed curve.
 */
static dcubic_bezier fix_redundant_tangents(const dcubic_bezier& curve) {
  dvec2 p1 = curve.p1;
  dvec2 p2 = curve.p2;

  if (math::squared_distance(curve.p0, p1) < 1e-12) {
    p1 = curve.p0;
  }

  if (math::squared_distance(curve.p3, p2) < 1e-12) {
    p2 = curve.p3;
  }

  return dcubic_bezier(curve.p0, p1, p2, curve.p3);
}

template <typename T, typename _>
void offset_cubic(const dcubic_bezier& curve, const double offset, const double tolerance, CubicPath<T>& sink) {
  const double minx = std::min({curve.p0.x, curve.p1.x, curve.p2.x, curve.p3.x});
  const double maxx = std::max({curve.p0.x, curve.p1.x, curve.p2.x, curve.p3.x});
  const double miny = std::min({curve.p0.y, curve.p1.y, curve.p2.y, curve.p3.y});
  const double maxy = std::max({curve.p0.y, curve.p1.y, curve.p2.y, curve.p3.y});

  const double dx = maxx - minx;
  const double dy = maxy - miny;

  if (dx < curve_point_clump_test_epsilon && dy < curve_point_clump_test_epsilon) {
    return;
  }

  // Select bigger of width and height.
  const double m = std::max(dx, dy) / 2.0;

  // Calculate scaled offset.
  const double so = offset / m;

  if (math::is_almost_zero(so)) {
    sink.cubic_to(math::Vec2<T>(curve.p1), math::Vec2<T>(curve.p2), math::Vec2<T>(curve.p3));
    return;
  }

  // Calculate "normalized" curve which kind of fits into range from - 1 to 1.
  const double tx = (minx + maxx) / 2.0;
  const double ty = (miny + maxy) / 2.0;
  const dvec2 t(tx, ty);

  const dvec2 p0 = curve.p0 - t;
  const dvec2 p1 = curve.p1 - t;
  const dvec2 p2 = curve.p2 - t;
  const dvec2 p3 = curve.p3 - t;

  const dcubic_bezier sc(p0 / m, p1 / m, p2 / m, p3 / m);

  const dcubic_bezier c = fix_redundant_tangents(sc);

  OutputBuilder<T> b(sink, m, t);

  const CurveTangentData d(c);

  if (is_curve_approximately_straight(c)) {
    // Rotate curve so that it is aligned with x-axis.
    const double angle = math::is_almost_equal(c.p0.y, c.p3.y, math::geometric_epsilon<double>) ? 0.0 : math::atan2(c.p0, c.p3);
    const double sin = std::sin(angle);
    const double cos = std::cos(angle);

    dcubic_bezier r = c;

    if (!math::is_almost_zero(angle)) {
      r.p1 = math::rotate(c.p1, c.p0, -sin, cos);
      r.p2 = math::rotate(c.p2, c.p0, -sin, cos);
      r.p3 = math::rotate(c.p3, c.p0, -sin, cos);
    }

    const drect bounds = r.bounding_rect();

    // Size-agnostic straight line.
    if (std::abs(bounds.min.y - bounds.max.y) < math::geometric_epsilon<double>) {
      std::vector<double> lines = {r.p0.x};

      if (std::abs(r.p0.x - bounds.min.x) <= std::abs(r.p0.x - bounds.max.x)) {
        if (!math::is_almost_equal(bounds.min.x, r.p0.x, 1e-2)) lines.push_back(bounds.min.x);
        if (!math::is_almost_equal(bounds.max.x, r.p3.x, 1e-2)) lines.push_back(bounds.max.x);
      } else {
        if (!math::is_almost_equal(bounds.max.x, r.p0.x, 1e-2)) lines.push_back(bounds.max.x);
        if (!math::is_almost_equal(bounds.min.x, r.p3.x, 1e-2)) lines.push_back(bounds.min.x);
      }

      lines.push_back(r.p3.x);

      for (int i = 0; i < lines.size() - 1; i++) {
        const dvec2 p0 = {lines[i], r.p0.y};
        const dvec2 p1 = {lines[i + 1], r.p0.y};
        const dvec2 n = math::normal(p0, p1) * so;
        const dvec2 from = math::rotate(p0 + n, c.p0, sin, cos);
        const dvec2 to = math::rotate(p1 + n, c.p0, sin, cos);

        if (!math::is_almost_equal(from, b.previous_point, math::geometric_epsilon<double>)) {
          if (i == 0) {
            line_to(b, from);
          } else {
            arc_to(b, math::rotate(p0, c.p0, sin, cos), from, true);
          }
        }

        line_to(b, to);
      }

      return;
    }
  }

  // Arbitrary curve.
  move_to(b, d.start_tangent.p0 + (so * d.start_unit_normal));

  // Try arc approximation first in case this curve was intended to
  // approximate circle. If that is indeed true, we avoid a lot of
  // expensive calculations like finding inflection and maximum
  // curvature points.
  if (!try_arc_approximation(c, d, b, so, tolerance)) {
    do_approximate_bezier(c, d, b, so, tolerance);
  }
}

/* -- Template Instantiation -- */

template void offset_cubic(const dcubic_bezier&, const double, const double, CubicPath<float>&);
template void offset_cubic(const dcubic_bezier&, const double, const double, CubicPath<double>&);

}  // namespace graphick::geom
