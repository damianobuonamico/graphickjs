
#include "CubicCurveOffset.h"

#include "../intersections.h"
#include "../curve_ops.h"
#include "../geom.h"

#include "../../math/math.h"

#include <algorithm>

namespace graphick::geom {

/**
 * A maximum number of iterations for searching for the closest point to cusp
 * that has the first derivative long enough for finding start or end points
 * of a circular arc for cusp.
 *
 * Smaller value means faster search, but worse accuracy when handling
 * cusp-like points of the curve.
 */
  static constexpr int NearCuspPointSearchMaxIterationCount = 18;


  /**
   * After an attempt to find an offset curve is made, squared lengths of all
   * edges of the polygon enclosing curve is calculated and added together. If
   * this length is equal to or less than this number, the resulting curve will
   * be discarded immediately without attempting to add it to the output.
   *
   * Smaller value means smaller curves will be accepted for output.
   */
  static constexpr double MaximumTinyCurvePolygonPerimeterSquared = 1e-7;


  /**
   * If a good circular arc approximation of a curve is found, but its radius is
   * very close to the offset amount, the scaled arc can collapse to a point or
   * almost a point. This is an epsilon for testing if the arc is large enough.
   * Arcs with radius smaller than this value will not be added to the output.
   *
   * Smaller value means smaller arcs will be accepted for output.
   */
  static constexpr double MinimumArcRadius = 1e-8;


  /**
   * An upper limit of arc radius. Circular arcs with calculated radius greater
   * than this value will not be considered as accepted approximations of curve
   * segments.
   */
  static constexpr double MaximumArcRadius = 1e+6;


  /**
   * Offsetter does not attempt to find exact cusp locations and does not
   * consider cusp only to be where the derivative vector length is exactly
   * zero.
   *
   * Smaller values means that sharper curve edges are considered cusps.
   */
  static constexpr double CuspDerivativeLengthSquared = 1.5e-4;


  /**
   * If X and Y components of all points are equal when compared with this
   * epsilon, the curve is considered a point.
   */
  static constexpr double CurvePointClumpTestEpsilon = 1e-14;


  /**
   * Epsilon used to compare coordinates of circular arc centers to see if they
   * can be merged into a single circular arc.
   */
  static constexpr double ArcCenterComparisonEpsilon = 1e-8;


  /**
   * When testing if curve is almost straight, cross products of unit vectors
   * are calculated as follows
   *
   *     Turn1 = (P0 → P1) ⨯ (P0 → P3)
   *     Turn2 = (P1 → P2) ⨯ (P0 → P3)
   *
   * Where P0, P1, P2 and P3 are curve points and (X → Y) are unit vectors going
   * from X to a direction of Y.
   *
   * Then these values are compared with zero. If they both are close to zero,
   * curve is considered approximately straight. This is the epsilon used for
   * comparing Turn1 and Turn2 values to zero.
   *
   * Bigger value means less straight curves are considered approximately
   * straight.
   */
  static constexpr double ApproximatelyStraightCurveTestApsilon = 1e-5;


  /**
   * The logic is the same as for ApproximatelyStraightCurveTestApsilon value.
   * This value is used to determine if curve is completely straight, not just
   * approximately straight.
   *
   * Bigger value means less straight curves are considered completely straight.
   * This value should be smaller than ApproximatelyStraightCurveTestApsilon.
   */
  static constexpr double CompletelyStraightCurveTestApsilon = 1e-15;


  /**
   * A list of positions on curve for testing if circular arc approximation of a
   * curve is good enough. All values must be from 0 to 1. Positions 0 and 1
   * should not be included here because distances at 0 and 1 are either
   * guaranteed to be exactly on the curve being approximated (on the sides of
   * the input curve) or they are tested before using another methods (ray cast
   * from arc center to triangle incenter point).
   */
  static constexpr double ArcProbePositions[] = {
      0.2,
      0.4,
      0.6,
      0.8
  };


  /**
   * A list of positions on curve for testing if candidate offset curve is good
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
  static constexpr double SimpleOffsetProbePositions[] = {
      0.25,
      0.5,
      0.75
  };


  /**
   * Keeps data needed to generate a set of output segments.
   */
  struct OutputBuilder final {
    constexpr OutputBuilder(CubicCurveBuilder& builder, const double scale,
        const dvec2 translation)
      : Builder(builder),
      Scale(scale),
      Translation(translation),
      PreviousPoint(0, 0),
      PreviousPointT(0, 0),
      CuspPoint(0, 0)
    {
    }

    CubicCurveBuilder& Builder;
    dvec2 PreviousPoint;
    dvec2 PreviousPointT;
    dvec2 CuspPoint;
    bool NeedsCuspArc = false;
    bool CuspArcClockwise = false;
    const double Scale = 1;
    const dvec2 Translation;
  };


  /**
   * Called once when the first point of output is calculated.
   */
  static void MoveTo(OutputBuilder& builder, const dvec2 to)
  {
    builder.PreviousPoint = to;
    builder.PreviousPointT = (to * builder.Scale) + builder.Translation;
  }


  /**
   * Called when a new line needs to be added to the output. Line starts at the
   * last point of previously added segment or point set by a call to MoveTo.
   */
  static void LineTo(OutputBuilder& builder, const dvec2 to)
  {
    const dvec2 previous = builder.PreviousPoint;

    if (previous != to) {
      const dvec2 t = (to * builder.Scale) + builder.Translation;

      builder.Builder.AddLine(builder.PreviousPointT, t);

      builder.PreviousPoint = to;
      builder.PreviousPointT = t;
    }
  }

  static void QuadraticTo(OutputBuilder& builder, const dvec2 cp, const dvec2 to)
  {
    const dvec2 previous = builder.PreviousPoint;

    if (previous != cp || previous != to) {
      const dvec2 c = (cp * builder.Scale) + builder.Translation;
      const dvec2 t = (to * builder.Scale) + builder.Translation;

      builder.Builder.AddQuadratic(builder.PreviousPointT, c, t);

      builder.PreviousPoint = to;
      builder.PreviousPointT = t;
    }
  }

  /**
   * Called when a new cubic curve needs to be added to the output. Curve starts
   * at the last point of previously added segment or point set by a call to
   * MoveTo.
   */
  static void CubicTo(OutputBuilder& builder, const dvec2 cp1,
      const dvec2 cp2, const dvec2 to)
  {
    const dvec2 previous = builder.PreviousPoint;

    if (previous != cp1 || previous != cp2 || previous != to) {
      const dvec2 c1 = (cp1 * builder.Scale) + builder.Translation;
      const dvec2 c2 = (cp2 * builder.Scale) + builder.Translation;
      const dvec2 t = (to * builder.Scale) + builder.Translation;

      builder.Builder.AddCubic(builder.PreviousPointT, c1, c2, t);

      builder.PreviousPoint = to;
      builder.PreviousPointT = t;
    }
  }


  /**
   * Returns unit cubic curve for given circular arc parameters. Arc center is
   * assumed to be at 0, 0.
   *
   * @param p0 Starting point of circular arc. Both components must be in range
   * from -1 to 1.
   *
   * @param p3 End point of circular arc. Both components must be in range from
   * -1 to 1.
   */
  static dcubic_bezier FindUnitCubicCurveForArc(const dvec2 p0,
      const dvec2 p3)
  {
    const double ax = p0.x;
    const double ay = p0.y;
    const double bx = p3.x;
    const double by = p3.y;
    const double q1 = ax * ax + ay * ay;
    const double q2 = q1 + ax * bx + ay * by;
    const double k2 = (4.0 / 3.0) * (std::sqrt(2.0 * q1 * q2) - q2) /
      (ax * by - ay * bx);
    const double x1 = p0.x - k2 * p0.y;
    const double y1 = p0.y + k2 * p0.x;
    const double x2 = p3.x + k2 * p3.y;
    const double y2 = p3.y - k2 * p3.x;

    return dcubic_bezier(p0, dvec2(x1, y1), dvec2(x2, y2), p3);
  }

  static double GetRadiansToLine(const dline& line1, const dline& line2) {
    if (math::is_almost_equal(line1.p0, line1.p1) || math::is_almost_equal(line2.p0, line2.p1)) {
      return 0;
    }

    const dvec2 d1 = line1.p1 - line1.p0;
    const dvec2 d2 = line2.p1 - line2.p0;

    const double c = math::dot(d1, d2) / (line1.length() * line2.length());

    // FLT_EPSILON instead of DBL_EPSILON is used deliberately.
    static constexpr double kMinRange = -1.0 - FLT_EPSILON;
    static constexpr double kMaxRange = 1.0 + FLT_EPSILON;

    // Return 0 instead of PI if c is outside range.
    if (c >= kMinRange && c <= kMaxRange) {
      return std::acos(math::clamp(c, -1.0, 1.0));
    }

    return 0;
  }

  static void ArcTo(OutputBuilder& builder, const dvec2 center,
      const dvec2 to, const bool clockwise) {
    const dvec2 from = builder.PreviousPoint;
    const double radius = math::distance(center, from);
    const double tolerance = 0.1;

    const double ang1 = std::atan2(from.y - center.y, from.x - center.x);
    const double ang2 = std::atan2(to.y - center.y, to.x - center.x);
    const double dphi = 4.0 * std::acos(std::sqrt(2.0 + tolerance - std::sqrt(tolerance * (2.0 + tolerance))) / std::sqrt(2.0));

    double diff = std::abs(ang2 - ang1);

    if (diff > math::pi<double>) diff = math::two_pi<double> -diff;
    if (!clockwise) diff = -diff;

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

      QuadraticTo(builder, p1, p2);
    }
  }

  static void MaybeAddCuspArc(OutputBuilder& builder, const dvec2 toPoint)
  {
    if (builder.NeedsCuspArc) {
      builder.NeedsCuspArc = false;

      ArcTo(builder, builder.CuspPoint, toPoint, builder.CuspArcClockwise);

      builder.CuspPoint = dvec2();
      builder.CuspArcClockwise = false;
    }
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
   * Returns true if the curve is close enough to be considered parallel to the
   * original curve.
   *
   * @param original The original curve.
   *
   * @param parallel Candidate parallel curve to be tested.
   *
   * @param offset Offset from original curve to candidate parallel curve.
   *
   * @param maximumError Maximum allowed error.
   */
  static bool AcceptOffset(const dcubic_bezier& original,
      const dcubic_bezier& parallel, const double offset,
      const double maximumError)
  {
      // Using shape control method, sometimes output curve becomes completely
      // off in some situations involving start and end tangents being almost
      // parallel. These two checks are to prevent accepting such curves as good.
    if (clockwise(original.p0, original.p1, original.p3) !=
        clockwise(parallel.p0, parallel.p1, parallel.p3)) {
      return false;
    }

    if (clockwise(original.p0, original.p2, original.p3) !=
        clockwise(parallel.p0, parallel.p2, parallel.p3)) {
      return false;
    }

    for (int i = 0; i < ARRAY_SIZE(SimpleOffsetProbePositions); i++) {
      const double t = SimpleOffsetProbePositions[i];
      const dvec2 op0 = original.sample(t);
      const dvec2 n = original.raw_normal(t);

      const math::CubicSolutions<double> intersections = ray_intersections(parallel, op0, op0 + n);

      if (intersections.count != 1) {
        return false;
      }

      const dvec2 p0 = parallel.sample(intersections.solutions[0]);
      const double d = math::distance(op0, p0);
      const double error = std::abs(d - std::abs(offset));

      if (error > maximumError) {
        return false;
      }
    }

    return true;
  }


  static void ArcOffset(OutputBuilder& builder, const double offset,
    const dvec2 center, const dvec2 from, const dvec2 to,
    const bool clockwise)
  {
    dline l1(center, from);
    dline l2(center, to);

    dvec2 v1 = math::normalize(l1.p1 - l1.p0);
    dvec2 v2 = math::normalize(l2.p1 - l2.p0);

    if (clockwise) {
      l1.p1 += v1 * offset;
      l2.p1 += v2 * offset;
    } else {
      l1.p1 -= v1 * offset;
      l2.p1 -= v2 * offset;
    }

    MaybeAddCuspArc(builder, l1.p1);

    // Determine if it is clockwise again since arc orientation may have
    // changed if arc radius was smaller than offset.
    //
    // Also it is important to use previous point to determine orientation
    // instead of the point we just calculated as the start of circular arc
    // because for small arcs a small numeric error can result in incorrect
    // arc orientation.
    ArcTo(builder, center, l2.p1, geom::clockwise(center,
      builder.PreviousPoint, l2.p1));
  }


  /**
   * Represents curve tangents as two line segments and some precomputed data.
   */
  struct CurveTangentData final {
    explicit CurveTangentData(const dcubic_bezier& curve);

    const dline StartTangent;
    const dline EndTangent;
    const double Turn1 = 0;
    const double Turn2 = 0;
    const dvec2 StartUnitNormal;
    const dvec2 EndUnitNormal;
  };


  static double UnitTurn(const dvec2 point1, const dvec2 point2,
      const dvec2 point3)
  {
    return math::cross(math::normalize(point2 - point1), math::normalize(point3 - point1));
  }


  CurveTangentData::CurveTangentData(const dcubic_bezier& curve)
    : StartTangent(curve.start_tangent()),
    EndTangent(curve.end_tangent()),
    Turn1(UnitTurn(StartTangent.p0, StartTangent.p1, EndTangent.p0)),
    Turn2(UnitTurn(StartTangent.p0, EndTangent.p1, EndTangent.p0)),
    StartUnitNormal(StartTangent.normal()),
    EndUnitNormal(EndTangent.normal())
  {
  }


  /**
   * Returns true if an attempt to approximate a curve with given tangents
   * should be made.
   */
  static bool CanTryArcOffset(const CurveTangentData& d)
  {
      // Arc approximation is only attempted if curve is not considered
      // approximately straight. But it can be attemped for curves which have
      // their control points on the different sides of line connecting points
      // P0 and P3.
      //
      // We need to make sure we don't try to do arc approximation for these S
      // type curves because such curves cannot be approximated by arcs in such
      // cases.

    static constexpr double P = ApproximatelyStraightCurveTestApsilon;
    static constexpr double N = -P;

    return
      (d.Turn1 >= P && d.Turn2 >= P) ||
      (d.Turn1 <= N && d.Turn2 <= N);
  }


  /**
   * Returns true if an attempt to use simple offsetting for a curve with given
   * tangents should be made.
   */
  static bool CanTrySimpleOffset(const CurveTangentData& d)
  {
      // Arc approximation is only attempted if curve is not considered
      // approximately straight. But it can be attemped for curves which have
      // their control points on the different sides of line connecting points
      // P0 and P3.
      //
      // We need to make sure we don't try to do arc approximation for these S
      // type curves because the shape control method behaves really badly with
      // S shape curves.

    return
      (d.Turn1 >= 0 && d.Turn2 >= 0) ||
      (d.Turn1 <= 0 && d.Turn2 <= 0);
  }


  /**
   * Returns true if curve is considered too small to be added to offset output.
   */
  static bool CurveIsTooTiny(const dcubic_bezier& curve)
  {
    const double lengthsSquared =
      math::squared_distance(curve.p0, curve.p1) +
      math::squared_distance(curve.p1, curve.p2) +
      math::squared_distance(curve.p2, curve.p3);

    return lengthsSquared <= MaximumTinyCurvePolygonPerimeterSquared;
  }


  /**
   * Attempts to perform simple curve offsetting and returns true if it succeeds
   * to generate good enough parallel curve.
   */
  static bool TrySimpleCurveOffset(const dcubic_bezier& curve,
      const CurveTangentData& d, OutputBuilder& builder, const double offset,
      const double maximumError)
  {
    if (!CanTrySimpleOffset(d)) {
      return false;
    }

    const dvec2 d1 = curve.p1 - curve.p0;
    const dvec2 d2 = curve.p2 - curve.p3;
    const double div = math::cross(d1, d2);

    if (math::is_almost_zero(div)) {
      return false;
    }

    // Start point.
    const dvec2 p0 = d.StartTangent.p0 +
      (d.StartTangent.normal() * offset);

  // End point.
    const dvec2 p3 = d.EndTangent.p0 -
      (d.EndTangent.normal() * offset);

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

    if (CurveIsTooTiny(candidate)) {
        // If curve is too tiny, tell caller there was a great success.
      return true;
    }

    if (!AcceptOffset(curve, candidate, offset, maximumError)) {
      return false;
    }

    MaybeAddCuspArc(builder, candidate.p0);

    CubicTo(builder, candidate.p1, candidate.p2, candidate.p3);

    return true;
  }


  static bool DoubleArrayContainsMergePosition(const double* a, const int count,
      const double value, const double epsilon)
  {
    for (int i = 0; i < count; i++) {
      const double v = a[i];

      if (math::is_almost_equal(value, v, epsilon)) {
        return true;
      }
    }

    return false;
  }


  static int MergeCurvePositions(double t[5], const int t_count, const double* s,
      const int count, const double epsilon)
  {
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
   * Returns true if a given line segment intersects with circle. Only
   * intersection within line segment is considered.
   *
   * @param line Line segment.
   *
   * @param circleCenter Position of the circle center.
   *
   * @param circleRadius Circle radius. Must not be negative.
   */
  static bool LineCircleIntersect(const dline& line,
      const dvec2 circleCenter, const double circleRadius)
  {
    ASSERT(circleRadius >= 0);

    const dvec2 d = line.p1 - line.p0;
    const dvec2 g = line.p0 - circleCenter;
    const double a = math::dot(d, d);
    const double b = 2.0 * math::dot(g, d);
    const double crSquared = circleRadius * circleRadius;
    const double c = math::dot(g, g) - crSquared;
    const double discriminant = b * b - 4.0 * a * c;

    if (discriminant > 0) {
      const double dsq = std::sqrt(discriminant);
      const double a2 = a * 2.0;
      const double t1 = (-b - dsq) / a2;
      const double t2 = (-b + dsq) / a2;

      return (t1 >= 0.0 && t1 <= 1.0) || (t2 >= 0.0 && t2 <= 1.0);
    }

    return false;
  }


  /**
   * Returns true if circular arc with given parameters approximate curve close
   * enough.
   *
   * @param arcCenter Point where arc center is located.
   *
   * @param arcRadius Radius of arc.
   *
   * @param curve Curve being approximated with arc.
   *
   * @param maximumError Maximum allowed error.
   */
  static bool GoodArc(const dvec2 arcCenter, const double arcRadius,
      const dcubic_bezier& curve, const double maximumError,
      const double tFrom, const double tTo)
  {
    if (arcRadius > MaximumArcRadius) {
      return false;
    }

    const double e = std::min(maximumError, arcRadius / 3.0);

    // Calculate value equal to slightly more than half of maximum error.
    // Slightly more to minimize false negatives due to finite precision in
    // circle-line intersection test.
    const double me = (e * (0.5 + 1e-4));

    for (int i = 0; i < ARRAY_SIZE(ArcProbePositions); i++) {
      const double t = ArcProbePositions[i];

      // Find t on a given curve.
      const double curveT = math::lerp(t, tFrom, tTo);

      // Find point and normal at this position.
      const dvec2 point = curve.sample(curveT);
      const dvec2 n = curve.normal(curveT);

      // Create line segment which has its center at curve on point and
      // extends by half of maximum allowed error to both directions from
      // curve point along normal.
      const dline segment(point + (n * me), point - (n * me));

      // Test if intersection exists.
      if (!LineCircleIntersect(segment, arcCenter, arcRadius)) {
        return false;
      }
    }

    return true;
  }

  static std::optional<dvec2> IntersectSimple(const dline& l1, const dline& l2) {
    const dvec2 a = l1.p1 - l1.p0;
    const dvec2 b = l2.p0 - l2.p1;
    const double denominator = a.y * b.x - a.x * b.y;

    if (denominator == 0) {
      return std::nullopt;
    }

    const dvec2 c = l1.p0 - l2.p0;
    const double reciprocal = 1.0 / denominator;
    const double na = (b.y * c.x - b.x * c.y) * reciprocal;

    return l1.p0 + a * na;
  }

  /**
   * Attempts to use circular arc offsetting method on a given curve.
   */
  static bool TryArcApproximation(const dcubic_bezier& curve,
      const CurveTangentData& d, OutputBuilder& builder, const double offset,
      const double maximumError)
  {
    if (!CanTryArcOffset(d)) {
      return false;
    }

    // Cast ray from curve end points to start and end tangent directions.
    const dvec2 vectorFrom = d.StartTangent.direction();
    const dvec2 vectorTo = d.EndTangent.direction();
    const double denom = vectorTo.x * vectorFrom.y - vectorTo.y * vectorFrom.x;

    // Should not happen as we already elliminated parallel case.
    if (math::is_almost_zero(denom)) {
      return false;
    }

    const dvec2 asv = d.StartTangent.p0;
    const dvec2 bsv = d.EndTangent.p0;
    const double u = ((bsv.y - asv.y) * vectorTo.x - (bsv.x - asv.x) * vectorTo.y) / denom;
    const double v = ((bsv.y - asv.y) * vectorFrom.x - (bsv.x - asv.x) * vectorFrom.y) / denom;

    if (u < 0.0 || v < 0.0) {
        // Intersection is on the wrong side.
      return false;
    }

    const dvec2 V = asv + (u * vectorFrom);

    // If start or end tangents extend too far beyond intersection, return
    // early since it will not result in good approximation.
    if (math::squared_distance(curve.p0, V) < (d.StartTangent.squared_length() * 0.25) ||
        math::squared_distance(curve.p3, V) < (d.EndTangent.squared_length() * 0.25)) {
      return false;
    }

    const double P3VDistance = math::distance(curve.p3, V);
    const double P0VDistance = math::distance(curve.p0, V);
    const double P0P3Distance = math::distance(curve.p0, curve.p3);
    const dvec2 G = (P3VDistance * curve.p0 + P0VDistance * curve.p3 + P0P3Distance * V) / (P3VDistance + P0VDistance + P0P3Distance);

    const dline P0G(curve.p0, G);
    const dline GP3(G, curve.p3);

    const dline E(P0G.midpoint(), P0G.midpoint() - P0G.raw_normal());
    const dline E1(d.StartTangent.p0, d.StartTangent.p0 -
        d.StartTangent.raw_normal());

    const std::optional<dvec2> C1 = IntersectSimple(E, E1);

    if (!C1.has_value()) {
      return false;
    }

    const math::CubicSolutions<double> intersections = ray_intersections(curve, C1.value(), G);

    if (intersections.count != 1) {
      return false;
    }

    const double tG = intersections.solutions[0];
    const double dist0 = math::distance(G, curve.sample(tG));

    if (dist0 > maximumError) {
      return false;
    }

    const dline F(GP3.midpoint(), GP3.midpoint() - GP3.raw_normal());
    const dline F1(d.EndTangent.p0, d.EndTangent.p0 +
        d.EndTangent.raw_normal());

    const std::optional<dvec2> C2 = IntersectSimple(F, F1);

    if (!C2.has_value()) {
      return false;
    }


    if (math::is_almost_equal(C1.value(), C2.value(), ArcCenterComparisonEpsilon)) {
      const double radius = math::distance(C1.value(), curve.p0);

      if (GoodArc(C1.value(), radius, curve, maximumError, 0, 1)) {
        const bool clockwise = geom::clockwise(curve.p0,
            V, curve.p3);

        ArcOffset(builder, offset, C1.value(), curve.p0,
            curve.p3, clockwise);
        return true;
      }
    } else {
      const double radius1 = math::distance(C1.value(), curve.p0);

      if (!GoodArc(C1.value(), radius1, curve, maximumError, 0, tG)) {
        return false;
      }

      const double radius2 = math::distance(C2.value(), curve.p3);

      if (!GoodArc(C2.value(), radius2, curve, maximumError, tG, 1)) {
        return false;
      }

      const bool clockwise = geom::clockwise(curve.p0, V,
          curve.p3);

      ArcOffset(builder, offset, C1.value(), curve.p0, G, clockwise);
      ArcOffset(builder, offset, C2.value(), G, curve.p3, clockwise);

      return true;
    }

    return false;
  }


  static bool IsCurveApproximatelyStraight(const CurveTangentData& d)
  {
    const double minx = std::min(d.StartTangent.p0.x, d.EndTangent.p0.x);
    const double miny = std::min(d.StartTangent.p0.y, d.EndTangent.p0.y);
    const double maxx = std::max(d.StartTangent.p0.x, d.EndTangent.p0.x);
    const double maxy = std::max(d.StartTangent.p0.y, d.EndTangent.p0.y);

    const double x1 = d.StartTangent.p1.x;
    const double y1 = d.StartTangent.p1.y;
    const double x2 = d.EndTangent.p1.x;
    const double y2 = d.EndTangent.p1.y;

    return
        // Is P1 located between P0 and P3?
      minx <= x1 &&
      miny <= y1 &&
      maxx >= x1 &&
      maxy >= y1 &&
      // Is P2 located between P0 and P3?
      minx <= x2 &&
      miny <= y2 &&
      maxx >= x2 &&
      maxy >= y2 &&
      // Are all points collinear?
      math::is_almost_zero(d.Turn1,
          ApproximatelyStraightCurveTestApsilon) &&
      math::is_almost_zero(d.Turn2,
          ApproximatelyStraightCurveTestApsilon);
  }


  static bool CurveIsCompletelyStraight(const CurveTangentData& d)
  {
    return
      math::is_almost_zero(d.Turn1, CompletelyStraightCurveTestApsilon) &&
      math::is_almost_zero(d.Turn2, CompletelyStraightCurveTestApsilon);
  }


  /**
   * Main function for approximating offset of a curve without cusps.
   */
  static void ApproximateBezier(const dcubic_bezier& curve,
      const CurveTangentData& d, OutputBuilder& builder, const double offset,
      const double maximumError)
  {
    if (!curve.is_point(CurvePointClumpTestEpsilon)) {
      if (IsCurveApproximatelyStraight(d)) {
        if (CurveIsCompletelyStraight(d)) {
            // Curve is extremely close to being straight.
          const dline line(curve.p0, curve.p1);
          const dvec2 normal = line.normal();

          MaybeAddCuspArc(builder, line.p0 + (normal * offset));

          LineTo(builder, line.p1 + (normal * offset));
        } else {
          const dvec2 p1o = d.StartTangent.p0 + (offset * d.StartUnitNormal);
          const dvec2 p2o = d.StartTangent.p1 + (offset * d.StartUnitNormal);
          const dvec2 p3o = d.EndTangent.p1 - (offset * d.EndUnitNormal);
          const dvec2 p4o = d.EndTangent.p0 - (offset * d.EndUnitNormal);

          MaybeAddCuspArc(builder, p1o);

          CubicTo(builder, p2o, p3o, p4o);
        }
      } else {
        if (!TrySimpleCurveOffset(curve, d, builder, offset, maximumError)) {
          if (!TryArcApproximation(curve, d, builder, offset, maximumError)) {
            // Split in half and continue.
            const auto& [a, b] = split(curve, 0.5);

            const CurveTangentData da(a);

            ApproximateBezier(a, da, builder, offset, maximumError);

            const CurveTangentData db(b);

            ApproximateBezier(b, db, builder, offset, maximumError);
          }
        }
      }
    }
  }


  static double FindPositionOnCurveWithLargeEnoughDerivative(
      const dcubic_bezier& curve, const double previousT, const double currentT)
  {
    ASSERT(currentT > previousT);

    static constexpr double kPrecision = CuspDerivativeLengthSquared * 2.0;

    double t = std::max(math::lerp(previousT, currentT, 0.8), currentT - 0.05);

    for (int i = 0; i < NearCuspPointSearchMaxIterationCount; i++) {
      const dvec2 derivative = curve.derivative(t);
      const double lengthSquared = math::squared_length(derivative);

      if (lengthSquared < kPrecision) {
        return t;
      }

      const double a = t + currentT;

      t = a / 2.0;
    }

    return t;
  }


  static double FindPositionOnCurveWithLargeEnoughDerivativeStart(
      const dcubic_bezier& curve, const double currentT, const double nextT)
  {
    ASSERT(currentT < nextT);

    static constexpr double kPrecision = CuspDerivativeLengthSquared * 2.0;

    double t = std::min(math::lerp(currentT, nextT, 0.2), currentT + 0.05);

    for (int i = 0; i < NearCuspPointSearchMaxIterationCount; i++) {
      const dvec2 derivative = curve.derivative(t);
      const double lengthSquared = math::squared_length(derivative);

      if (lengthSquared < kPrecision) {
        return t;
      }

      const double a = currentT + t;

      t = a / 2.0;
    }

    return t;
  }


  /**
   * If all points of the curve are collinear, a shortcut must be made because
   * general offsetting algorithm does not handle such curves very well. In case
   * where are points are collinear, lines between cusps are offset to direction
   * of their normals and at the points where curve has a cusps, semi-circles
   * are added to the output.
   */
  static void OffsetLinearCuspyCurve(const dcubic_bezier& curve,
      OutputBuilder& builder, const double offset,
      const double* maximumCurvaturePoints, const int maximumCurvaturePointCount)
  {
    const dline startTangent = curve.start_tangent();
    const dvec2 normal = startTangent.normal();

    dvec2 previousPoint = startTangent.p0;
    dvec2 previousOffsetPoint = previousPoint + (normal * offset);

    MoveTo(builder, previousOffsetPoint);

    for (int i = 0; i < maximumCurvaturePointCount; i++) {
        // Skip 0 and 1!
      const double t = maximumCurvaturePoints[i];
      const dvec2 derived = curve.derivative(t);
      const double lengthSquared = math::squared_length(derived);

      if (lengthSquared <= 1e-9) {
          // Cusp. std::since we know all curve points are on the same line, some
          // of maximum curvature points will have nearly zero length
          // derivative vectors.
        const dvec2 pointAtCusp = curve.sample(t);

        // Draw line from previous point to point at cusp.
        const dline l(previousPoint, pointAtCusp);
        const dvec2 n = l.normal();
        const dvec2 to = pointAtCusp + (n * offset);

        LineTo(builder, to);

        // Draw semi circle at cusp.
        const dvec2 arcTo = pointAtCusp - (n * offset);

        ArcTo(builder, pointAtCusp, arcTo,
            clockwise(previousPoint,
              previousOffsetPoint, pointAtCusp));

        previousPoint = pointAtCusp;
        previousOffsetPoint = arcTo;
      }
    }

    const dline endTangent = curve.end_tangent();
    const dvec2 normal2 = endTangent.normal();

    LineTo(builder, endTangent.p0 - (normal2 * offset));
  }


  static void DoApproximateBezier(const dcubic_bezier& curve,
      const CurveTangentData& d, OutputBuilder& builder, const double offset,
      const double maximumError)
  {
    // First find maximum curvature positions.
    const math::CubicSolutions<double> maximumCurvaturePositions = geom::max_curvature(curve);
    const int numMaximumCurvaturePositions = maximumCurvaturePositions.count;

    // Handle special case where the input curve is a straight line, but
    // control points do not necessary lie on line segment between curve
    // points P0 and P3.
    if (CurveIsCompletelyStraight(d)) {
      OffsetLinearCuspyCurve(curve, builder, offset,
          maximumCurvaturePositions.solutions, numMaximumCurvaturePositions);
      return;
    }

    // Now find inflection point positions.
    const math::QuadraticSolutions<double> inflections = geom::inflections(curve);

    const int numInflections = inflections.count;

    // Merge maximum curvature and inflection point positions.
    double t[5];

    const int count0 = MergeCurvePositions(t, 0, inflections.solutions, numInflections, 1e-7);

    const int count = MergeCurvePositions(t, count0, maximumCurvaturePositions.solutions,
        numMaximumCurvaturePositions, 1e-5);

    std::sort(t, t + count);

    if (count == 0) {
        // No initial subdivision suggestions.
      ApproximateBezier(curve, d, builder, offset, maximumError);
    } else {
      double previousT = 0;

      for (int i = 0; i < count; i++) {
        const double T = t[i];
        const dvec2 derivative = curve.derivative(T);
        const double lengthSquared = math::squared_length(derivative);

        if (lengthSquared < CuspDerivativeLengthSquared) {
            // Squared length of derivative becomes tiny. This is where
            // the cusp is. The goal here is to find a spon on curve,
            // located before T which has large enough derivative and draw
            // circular arc to the next point on curve with large enough
            // derivative.

          const double t1 = FindPositionOnCurveWithLargeEnoughDerivative(
              curve, previousT, T);

          ASSERT(t1 < T);

          const dcubic_bezier k = extract(curve, previousT, t1);
          const CurveTangentData nd(k);

          ApproximateBezier(k, nd, builder, offset, maximumError);

          const double t2 = FindPositionOnCurveWithLargeEnoughDerivativeStart(
              curve, T, i == (count - 1) ? 1.0 : t[i + 1]);

          ASSERT(t2 > T);

          builder.CuspPoint = curve.sample(T);
          builder.NeedsCuspArc = true;
          builder.CuspArcClockwise = clockwise(
              k.p3, builder.CuspPoint, curve.sample(t2));

          previousT = t2;
        } else {
            // Easy, feed subcurve between previous and current t values
            // to offset approximation function.

          const dcubic_bezier k = extract(curve, previousT, T);
          const CurveTangentData nd(k);

          ApproximateBezier(k, nd, builder, offset, maximumError);

          previousT = T;
        }
      }

      ASSERT(previousT < 1.0);

      const dcubic_bezier k = extract(curve, previousT, 1.0);
      const CurveTangentData nd(k);

      ApproximateBezier(k, nd, builder, offset,
          maximumError);
    }
  }


  /**
   * Flattens ends of curves if control points are too close to end points.
   */
  static dcubic_bezier FixRedundantTangents(const dcubic_bezier& curve)
  {
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


  void OffsetCurve(const dcubic_bezier& curve, const double offset,
      const double maximumError, CubicCurveBuilder& builder)
  {
    const double minx = std::min({ curve.p0.x, curve.p1.x, curve.p2.x, curve.p3.x });
    const double maxx = std::max({ curve.p0.x, curve.p1.x, curve.p2.x, curve.p3.x });
    const double miny = std::min({ curve.p0.y, curve.p1.y, curve.p2.y, curve.p3.y });
    const double maxy = std::max({ curve.p0.y, curve.p1.y, curve.p2.y, curve.p3.y });

    const double dx = maxx - minx;
    const double dy = maxy - miny;

    if (dx < CurvePointClumpTestEpsilon && dy < CurvePointClumpTestEpsilon) {
      return;
    }

    // Select bigger of width and height.
    const double m = std::max(dx, dy) / 2.0;

    // Calculate scaled offset.
    const double so = offset / m;

    if (math::is_almost_zero(so)) {
      builder.AddCubic(curve.p0, curve.p1, curve.p2, curve.p3);
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

    const dcubic_bezier c = FixRedundantTangents(sc);

    OutputBuilder b(builder, m, t);

    const CurveTangentData d(c);

    if (IsCurveApproximatelyStraight(d)) {
      if (CurveIsCompletelyStraight(d)) {
          // Curve is extremely close to being straight, use simple line
          // translation.
        const dline line(c.p0, c.p3);
        const dvec2 normal = line.normal();
        const dline translated = { line.p0 + normal * so, line.p1 + normal * so };

        MoveTo(b, translated.p0);

        LineTo(b, translated.p1);
      } else {
          // Curve is almost straight. Translate start and end tangents
          // separately and generate a cubic curve.
        const dvec2 p1o = d.StartTangent.p0 + (so * d.StartUnitNormal);
        const dvec2 p2o = d.StartTangent.p1 + (so * d.StartUnitNormal);
        const dvec2 p3o = d.EndTangent.p1 - (so * d.EndUnitNormal);
        const dvec2 p4o = d.EndTangent.p0 - (so * d.EndUnitNormal);

        MoveTo(b, p1o);

        CubicTo(b, p2o, p3o, p4o);
      }
    } else {
        // Arbitrary curve.
      MoveTo(b, d.StartTangent.p0 + (so * d.StartUnitNormal));

      // Try arc approximation first in case this curve was intended to
      // approximate circle. If that is indeed true, we avoid a lot of
      // expensive calculations like finding inflection and maximum
      // curvature points.
      if (!TryArcApproximation(c, d, b, so, maximumError)) {
        DoApproximateBezier(c, d, b, so, maximumError);
      }
    }
  }
}
