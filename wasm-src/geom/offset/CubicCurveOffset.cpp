
#include "CubicCurveOffset.h"
#include "Quicksort.h"


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
        const FloatPoint& translation)
        : Builder(builder),
        Scale(scale),
        Translation(translation)
    {
    }

    CubicCurveBuilder& Builder;
    FloatPoint PreviousPoint;
    FloatPoint PreviousPointT;
    FloatPoint CuspPoint;
    bool NeedsCuspArc = false;
    bool CuspArcClockwise = false;
    const double Scale = 1;
    const FloatPoint Translation;
};


/**
 * Called once when the first point of output is calculated.
 */
static void MoveTo(OutputBuilder& builder, const FloatPoint& to)
{
    builder.PreviousPoint = to;
    builder.PreviousPointT = (to * builder.Scale) + builder.Translation;
}


/**
 * Called when a new line needs to be added to the output. Line starts at the
 * last point of previously added segment or point set by a call to MoveTo.
 */
static void LineTo(OutputBuilder& builder, const FloatPoint& to)
{
    const FloatPoint previous = builder.PreviousPoint;

    if (previous != to) {
        const FloatPoint t = (to * builder.Scale) + builder.Translation;

        builder.Builder.AddLine(builder.PreviousPointT, t);

        builder.PreviousPoint = to;
        builder.PreviousPointT = t;
    }
}


/**
 * Called when a new cubic curve needs to be added to the output. Curve starts
 * at the last point of previously added segment or point set by a call to
 * MoveTo.
 */
static void CubicTo(OutputBuilder& builder, const FloatPoint& cp1,
    const FloatPoint& cp2, const FloatPoint& to)
{
    const FloatPoint previous = builder.PreviousPoint;

    if (previous != cp1 || previous != cp2 || previous != to) {
        const FloatPoint c1 = (cp1 * builder.Scale) + builder.Translation;
        const FloatPoint c2 = (cp2 * builder.Scale) + builder.Translation;
        const FloatPoint t = (to * builder.Scale) + builder.Translation;

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
static CubicCurve FindUnitCubicCurveForArc(const FloatPoint& p0,
    const FloatPoint& p3)
{
    const double ax = p0.X;
    const double ay = p0.Y;
    const double bx = p3.X;
    const double by = p3.Y;
    const double q1 = ax * ax + ay * ay;
    const double q2 = q1 + ax * bx + ay * by;
    const double k2 = (4.0 / 3.0) * (Sqrt(2.0 * q1 * q2) - q2) /
        (ax * by - ay * bx);
    const double x1 = p0.X - k2 * p0.Y;
    const double y1 = p0.Y + k2 * p0.X;
    const double x2 = p3.X + k2 * p3.Y;
    const double y2 = p3.Y - k2 * p3.X;

    return CubicCurve(p0, FloatPoint(x1, y1), FloatPoint(x2, y2), p3);
}


/**
 * Called when a circular arc needs to be added to the output. Arc starts at
 * the last point of previously added segment or point set by a call to MoveTo
 * and goes to a given end point.
 */
static void ArcTo(OutputBuilder& builder, const FloatPoint& center,
    const FloatPoint& to, const bool clockwise)
{
    const FloatPoint arcFrom = builder.PreviousPoint;

    const double arcRadius = center.DistanceTo(arcFrom);

    if (arcRadius < MinimumArcRadius) {
        return;
    }

    const FloatLine centerToCurrentPoint(center, arcFrom);
    const FloatLine centerToEndPoint(center, to);
    const double startAngle = Deg2Rad(centerToCurrentPoint.Angle());

    double sweepAngle = centerToCurrentPoint.GetRadiansToLine(centerToEndPoint);

    if (FuzzyIsZero(sweepAngle)) {
        return;
    }

    const TrianglePointOrientation determinedOrientation =
        FloatPoint::DetermineTriangleOrientation(center, arcFrom, to);

    if (determinedOrientation != TrianglePointOrientation::Collinear) {
        // If our three points are not collinear, we check if they are
        // clockwise. If we see that their orientation is opposite of what we
        // are told to draw, we draw large arc.
        const bool determinedClockwise =
            determinedOrientation == TrianglePointOrientation::Clockwise;

        if (determinedClockwise != clockwise) {
            sweepAngle = (M_PI * 2.0) - sweepAngle;
        }
    }

    const int nSteps = Ceil(sweepAngle / (M_PI / 2.0));
    const double step = sweepAngle / nSteps * (clockwise ? -1.0 : 1.0);

    double s = -Sin(startAngle);
    double c = Cos(startAngle);

    for (int i = 1; i <= nSteps; i++) {
        const double a1 = startAngle + step * double(i);

        const double s1 = -Sin(a1);
        const double c1 = Cos(a1);

        const CubicCurve unitCurve = FindUnitCubicCurveForArc(
            FloatPoint(c, s), FloatPoint(c1, s1));

        const FloatPoint p1 = (unitCurve.P1 * arcRadius) + center;
        const FloatPoint p2 = (unitCurve.P2 * arcRadius) + center;

        if (i < nSteps) {
            const FloatPoint p3 = (unitCurve.P3 * arcRadius) + center;

            CubicTo(builder, p1, p2, p3);
        } else {
            // Last point. Make sure we end with it. This is quite important
            // thing to do.
            CubicTo(builder, p1, p2, to);
        }

        s = s1;
        c = c1;
    }
}


static void MaybeAddCuspArc(OutputBuilder& builder, const FloatPoint& toPoint)
{
    if (builder.NeedsCuspArc) {
        builder.NeedsCuspArc = false;

        ArcTo(builder, builder.CuspPoint, toPoint, builder.CuspArcClockwise);

        builder.CuspPoint = FloatPoint();
        builder.CuspArcClockwise = false;
    }
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
static bool AcceptOffset(const CubicCurve& original,
    const CubicCurve& parallel, const double offset,
    const double maximumError)
{
    // Using shape control method, sometimes output curve becomes completely
    // off in some situations involving start and end tangents being almost
    // parallel. These two checks are to prevent accepting such curves as good.
    if (FloatPoint::IsTriangleClockwise(original.P0, original.P1, original.P3) !=
        FloatPoint::IsTriangleClockwise(parallel.P0, parallel.P1, parallel.P3)) {
        return false;
    }

    if (FloatPoint::IsTriangleClockwise(original.P0, original.P2, original.P3) !=
        FloatPoint::IsTriangleClockwise(parallel.P0, parallel.P2, parallel.P3)) {
        return false;
    }

    double intersections[3];

    for (int i = 0; i < ARRAY_SIZE(SimpleOffsetProbePositions); i++) {
        const double t = SimpleOffsetProbePositions[i];
        const FloatPoint op0 = original.PointAt(t);
        const FloatPoint n = original.NormalVector(t);

        const int nRoots = parallel.FindRayIntersections(op0, op0 + n,
            intersections);

        if (nRoots != 1) {
            return false;
        }

        const FloatPoint p0 = parallel.PointAt(*intersections);
        const double d = op0.DistanceTo(p0);
        const double error = Abs(d - Abs(offset));

        if (error > maximumError) {
            return false;
        }
    }

    return true;
}


static void ArcOffset(OutputBuilder& builder, const double offset,
    const FloatPoint& center, const FloatPoint& from, const FloatPoint& to,
    const bool clockwise)
{
    FloatLine l1(center, from);
    FloatLine l2(center, to);

    if (clockwise) {
        l1.ExtendByLengthFront(offset);
        l2.ExtendByLengthFront(offset);
    } else {
        l1.ExtendByLengthFront(-offset);
        l2.ExtendByLengthFront(-offset);
    }

    MaybeAddCuspArc(builder, l1.P1);

    // Determine if it is clockwise again since arc orientation may have
    // changed if arc radius was smaller than offset.
    //
    // Also it is important to use previous point to determine orientation
    // instead of the point we just calculated as the start of circular arc
    // because for small arcs a small numeric error can result in incorrect
    // arc orientation.
    ArcTo(builder, center, l2.P1, FloatPoint::IsTriangleClockwise(center,
        builder.PreviousPoint, l2.P1));
}


/**
 * Represents curve tangents as two line segments and some precomputed data.
 */
struct CurveTangentData final {
    explicit CurveTangentData(const CubicCurve& curve);

    const FloatLine StartTangent;
    const FloatLine EndTangent;
    const double Turn1 = 0;
    const double Turn2 = 0;
    const FloatPoint StartUnitNormal;
    const FloatPoint EndUnitNormal;
};


static double UnitTurn(const FloatPoint& point1, const FloatPoint& point2,
    const FloatPoint& point3)
{
    return ((point2 - point1).UnitVector()).Cross((point3 - point1).UnitVector());
}


CurveTangentData::CurveTangentData(const CubicCurve& curve)
    : StartTangent(curve.StartTangent()),
    EndTangent(curve.EndTangent()),
    Turn1(UnitTurn(StartTangent.P0, StartTangent.P1, EndTangent.P0)),
    Turn2(UnitTurn(StartTangent.P0, EndTangent.P1, EndTangent.P0)),
    StartUnitNormal(StartTangent.UnitNormalVector()),
    EndUnitNormal(EndTangent.UnitNormalVector())
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
static bool CurveIsTooTiny(const CubicCurve& curve)
{
    const double lengthsSquared =
        curve.P0.DistanceToSquared(curve.P1) +
        curve.P1.DistanceToSquared(curve.P2) +
        curve.P2.DistanceToSquared(curve.P3);

    return lengthsSquared <= MaximumTinyCurvePolygonPerimeterSquared;
}


/**
 * Attempts to perform simple curve offsetting and returns true if it succeeds
 * to generate good enough parallel curve.
 */
static bool TrySimpleCurveOffset(const CubicCurve& curve,
    const CurveTangentData& d, OutputBuilder& builder, const double offset,
    const double maximumError)
{
    if (!CanTrySimpleOffset(d)) {
        return false;
    }

    const FloatPoint d1 = curve.P1 - curve.P0;
    const FloatPoint d2 = curve.P2 - curve.P3;
    const double div = d1.Cross(d2);

    if (FuzzyIsZero(div)) {
        return false;
    }

    // Start point.
    const FloatPoint p0 = d.StartTangent.P0 +
        (d.StartTangent.UnitNormalVector() * offset);

    // End point.
    const FloatPoint p3 = d.EndTangent.P0 -
        (d.EndTangent.UnitNormalVector() * offset);

    // Middle point.
    const FloatPoint mp = curve.PointAt(0.5);
    const FloatPoint mpN = curve.UnitNormalVector(0.5);
    const FloatPoint p = mp + (mpN * offset);

    const FloatPoint bxby = (8.0 / 3.0) * (p - (0.5 * (p0 + p3)));

    const double a = bxby.Cross(d2) / div;
    const double b = d1.Cross(bxby) / div;

    const FloatPoint p1(p0.X + a * d1.X, p0.Y + a * d1.Y);
    const FloatPoint p2(p3.X + b * d2.X, p3.Y + b * d2.Y);

    const CubicCurve candidate(p0, p1, p2, p3);

    if (CurveIsTooTiny(candidate)) {
        // If curve is too tiny, tell caller there was a great success.
        return true;
    }

    if (!AcceptOffset(curve, candidate, offset, maximumError)) {
        return false;
    }

    MaybeAddCuspArc(builder, candidate.P0);

    CubicTo(builder, candidate.P1, candidate.P2, candidate.P3);

    return true;
}


static bool DoubleArrayContainsMergePosition(const double* a, const int count,
    const double value, const double epsilon)
{
    for (int i = 0; i < count; i++) {
        const double v = a[i];

        if (IsEqualWithEpsilon(value, v, epsilon)) {
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

        if (IsZeroWithEpsilon(v, epsilon)) {
            continue;
        }

        if (IsEqualWithEpsilon(v, 1.0, epsilon)) {
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
static bool LineCircleIntersect(const FloatLine& line,
    const FloatPoint& circleCenter, const double circleRadius)
{
    ASSERT(circleRadius >= 0);

    const FloatPoint d = line.P1 - line.P0;
    const FloatPoint g = line.P0 - circleCenter;
    const double a = d.Dot(d);
    const double b = 2.0 * g.Dot(d);
    const double crSquared = circleRadius * circleRadius;
    const double c = g.Dot(g) - crSquared;
    const double discriminant = b * b - 4.0 * a * c;

    if (discriminant > 0) {
        const double dsq = Sqrt(discriminant);
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
static bool GoodArc(const FloatPoint& arcCenter, const double arcRadius,
    const CubicCurve& curve, const double maximumError,
    const double tFrom, const double tTo)
{
    if (arcRadius > MaximumArcRadius) {
        return false;
    }

    const double e = Min(maximumError, arcRadius / 3.0);

    // Calculate value equal to slightly more than half of maximum error.
    // Slightly more to minimize false negatives due to finite precision in
    // circle-line intersection test.
    const double me = (e * (0.5 + 1e-4));

    for (int i = 0; i < ARRAY_SIZE(ArcProbePositions); i++) {
        const double t = ArcProbePositions[i];

        // Find t on a given curve.
        const double curveT = InterpolateLinear(t, tFrom, tTo);

        // Find point and normal at this position.
        const FloatPoint point = curve.PointAt(curveT);
        const FloatPoint n = curve.UnitNormalVector(curveT);

        // Create line segment which has its center at curve on point and
        // extends by half of maximum allowed error to both directions from
        // curve point along normal.
        const FloatLine segment(point + (n * me), point - (n * me));

        // Test if intersection exists.
        if (!LineCircleIntersect(segment, arcCenter, arcRadius)) {
            return false;
        }
    }

    return true;
}


/**
 * Attempts to use circular arc offsetting method on a given curve.
 */
static bool TryArcApproximation(const CubicCurve& curve,
    const CurveTangentData& d, OutputBuilder& builder, const double offset,
    const double maximumError)
{
    if (!CanTryArcOffset(d)) {
        return false;
    }

    // Cast ray from curve end points to start and end tangent directions.
    const FloatPoint vectorFrom = d.StartTangent.UnitVector();
    const FloatPoint vectorTo = d.EndTangent.UnitVector();
    const double denom = vectorTo.X * vectorFrom.Y - vectorTo.Y * vectorFrom.X;

    // Should not happen as we already elliminated parallel case.
    if (FuzzyIsZero(denom)) {
        return false;
    }

    const FloatPoint asv = d.StartTangent.P0;
    const FloatPoint bsv = d.EndTangent.P0;
    const double u = ((bsv.Y - asv.Y) * vectorTo.X - (bsv.X - asv.X) * vectorTo.Y) / denom;
    const double v = ((bsv.Y - asv.Y) * vectorFrom.X - (bsv.X - asv.X) * vectorFrom.Y) / denom;

    if (u < 0.0 || v < 0.0) {
        // Intersection is on the wrong side.
        return false;
    }

    const FloatPoint V = asv + (u * vectorFrom);

    // If start or end tangents extend too far beyond intersection, return
    // early since it will not result in good approximation.
    if (curve.P0.DistanceToSquared(V) < (d.StartTangent.LengthSquared() * 0.25) ||
        curve.P3.DistanceToSquared(V) < (d.EndTangent.LengthSquared() * 0.25)) {
        return false;
    }

    const double P3VDistance = curve.P3.DistanceTo(V);
    const double P0VDistance = curve.P0.DistanceTo(V);
    const double P0P3Distance = curve.P0.DistanceTo(curve.P3);
    const FloatPoint G = (P3VDistance * curve.P0 + P0VDistance * curve.P3 + P0P3Distance * V) / (P3VDistance + P0VDistance + P0P3Distance);

    const FloatLine P0G(curve.P0, G);
    const FloatLine GP3(G, curve.P3);

    const FloatLine E(P0G.MidPoint(), P0G.MidPoint() - P0G.NormalVector());
    const FloatLine E1(d.StartTangent.P0, d.StartTangent.P0 -
        d.StartTangent.NormalVector());

    const LineIntersectionSimple C1 = E.IntersectSimple(E1);

    if (!C1.Found) {
        return false;
    }

    double intersections[3];

    const int nRoots = curve.FindRayIntersections(C1.IntersectionPoint, G, intersections);

    if (nRoots != 1) {
        return false;
    }

    const double tG = *intersections;
    const double dist0 = G.DistanceTo(curve.PointAt(tG));

    if (dist0 > maximumError) {
        return false;
    }

    const FloatLine F(GP3.MidPoint(), GP3.MidPoint() - GP3.NormalVector());
    const FloatLine F1(d.EndTangent.P0, d.EndTangent.P0 +
        d.EndTangent.NormalVector());

    const LineIntersectionSimple C2 = F.IntersectSimple(F1);

    if (!C2.Found) {
        return false;
    }

    if (C1.IntersectionPoint.IsEqual(C2.IntersectionPoint, ArcCenterComparisonEpsilon)) {
        const double radius = C1.IntersectionPoint.DistanceTo(curve.P0);

        if (GoodArc(C1.IntersectionPoint, radius, curve, maximumError, 0, 1)) {
            const bool clockwise = FloatPoint::IsTriangleClockwise(curve.P0,
                V, curve.P3);

            ArcOffset(builder, offset, C1.IntersectionPoint, curve.P0,
                curve.P3, clockwise);
            return true;
        }
    } else {
        const double radius1 = C1.IntersectionPoint.DistanceTo(curve.P0);

        if (!GoodArc(C1.IntersectionPoint, radius1, curve, maximumError, 0, tG)) {
            return false;
        }

        const double radius2 = C2.IntersectionPoint.DistanceTo(curve.P3);

        if (!GoodArc(C2.IntersectionPoint, radius2, curve, maximumError, tG, 1)) {
            return false;
        }

        const bool clockwise = FloatPoint::IsTriangleClockwise(curve.P0, V,
            curve.P3);

        ArcOffset(builder, offset, C1.IntersectionPoint, curve.P0, G, clockwise);
        ArcOffset(builder, offset, C2.IntersectionPoint, G, curve.P3, clockwise);

        return true;
    }

    return false;
}


static bool IsCurveApproximatelyStraight(const CurveTangentData& d)
{
    const double minx = Min(d.StartTangent.X0(), d.EndTangent.X0());
    const double miny = Min(d.StartTangent.Y0(), d.EndTangent.Y0());
    const double maxx = Max(d.StartTangent.X0(), d.EndTangent.X0());
    const double maxy = Max(d.StartTangent.Y0(), d.EndTangent.Y0());

    const double x1 = d.StartTangent.X1();
    const double y1 = d.StartTangent.Y1();
    const double x2 = d.EndTangent.X1();
    const double y2 = d.EndTangent.Y1();

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
        IsZeroWithEpsilon(d.Turn1,
            ApproximatelyStraightCurveTestApsilon) &&
        IsZeroWithEpsilon(d.Turn2,
            ApproximatelyStraightCurveTestApsilon);
}


static bool CurveIsCompletelyStraight(const CurveTangentData& d)
{
    return
        IsZeroWithEpsilon(d.Turn1, CompletelyStraightCurveTestApsilon) &&
        IsZeroWithEpsilon(d.Turn2, CompletelyStraightCurveTestApsilon);
}


/**
 * Main function for approximating offset of a curve without cusps.
 */
static void ApproximateBezier(const CubicCurve& curve,
    const CurveTangentData& d, OutputBuilder& builder, const double offset,
    const double maximumError)
{
    if (!curve.IsPoint(CurvePointClumpTestEpsilon)) {
        if (IsCurveApproximatelyStraight(d)) {
            if (CurveIsCompletelyStraight(d)) {
                // Curve is extremely close to being straight.
                const FloatLine line(curve.P0, curve.P1);
                const FloatPoint normal = line.UnitNormalVector();

                MaybeAddCuspArc(builder, line.P0 + (normal * offset));

                LineTo(builder, line.P1 + (normal * offset));
            } else {
                const FloatPoint p1o = d.StartTangent.P0 + (offset * d.StartUnitNormal);
                const FloatPoint p2o = d.StartTangent.P1 + (offset * d.StartUnitNormal);
                const FloatPoint p3o = d.EndTangent.P1 - (offset * d.EndUnitNormal);
                const FloatPoint p4o = d.EndTangent.P0 - (offset * d.EndUnitNormal);

                MaybeAddCuspArc(builder, p1o);

                CubicTo(builder, p2o, p3o, p4o);
            }
        } else {
            if (!TrySimpleCurveOffset(curve, d, builder, offset, maximumError)) {
                if (!TryArcApproximation(curve, d, builder, offset, maximumError)) {
                    // Split in half and continue.
                    CubicCurve a;
                    CubicCurve b;

                    curve.Split(a, b);

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
    const CubicCurve& curve, const double previousT, const double currentT)
{
    ASSERT(currentT > previousT);

    static constexpr double kPrecision = CuspDerivativeLengthSquared * 2.0;

    double t = Max(InterpolateLinear(previousT, currentT, 0.8), currentT - 0.05);

    for (int i = 0; i < NearCuspPointSearchMaxIterationCount; i++) {
        const FloatPoint derivative = curve.DerivedAt(t);
        const double lengthSquared = derivative.LengthSquared();

        if (lengthSquared < kPrecision) {
            return t;
        }

        const double a = t + currentT;

        t = a / 2.0;
    }

    return t;
}


static double FindPositionOnCurveWithLargeEnoughDerivativeStart(
    const CubicCurve& curve, const double currentT, const double nextT)
{
    ASSERT(currentT < nextT);

    static constexpr double kPrecision = CuspDerivativeLengthSquared * 2.0;

    double t = Min(InterpolateLinear(currentT, nextT, 0.2), currentT + 0.05);

    for (int i = 0; i < NearCuspPointSearchMaxIterationCount; i++) {
        const FloatPoint derivative = curve.DerivedAt(t);
        const double lengthSquared = derivative.LengthSquared();

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
static void OffsetLinearCuspyCurve(const CubicCurve& curve,
    OutputBuilder& builder, const double offset,
    const double* maximumCurvaturePoints, const int maximumCurvaturePointCount)
{
    const FloatLine startTangent = curve.StartTangent();
    const FloatPoint normal = startTangent.UnitNormalVector();

    FloatPoint previousPoint = startTangent.P0;
    FloatPoint previousOffsetPoint = previousPoint + (normal * offset);

    MoveTo(builder, previousOffsetPoint);

    for (int i = 0; i < maximumCurvaturePointCount; i++) {
        // Skip 0 and 1!
        const double t = maximumCurvaturePoints[i];
        const FloatPoint derived = curve.DerivedAt(t);
        const double lengthSquared = derived.LengthSquared();

        if (lengthSquared <= 1e-9) {
            // Cusp. Since we know all curve points are on the same line, some
            // of maximum curvature points will have nearly zero length
            // derivative vectors.
            const FloatPoint pointAtCusp = curve.PointAt(t);

            // Draw line from previous point to point at cusp.
            const FloatLine l(previousPoint, pointAtCusp);
            const FloatPoint n = l.UnitNormalVector();
            const FloatPoint to = pointAtCusp + (n * offset);

            LineTo(builder, to);

            // Draw semi circle at cusp.
            const FloatPoint arcTo = pointAtCusp - (n * offset);

            ArcTo(builder, pointAtCusp, arcTo,
                FloatPoint::IsTriangleClockwise(previousPoint,
                    previousOffsetPoint, pointAtCusp));

            previousPoint = pointAtCusp;
            previousOffsetPoint = arcTo;
        }
    }

    const FloatLine endTangent = curve.EndTangent();
    const FloatPoint normal2 = endTangent.UnitNormalVector();

    LineTo(builder, endTangent.P0 - (normal2 * offset));
}


static void DoApproximateBezier(const CubicCurve& curve,
    const CurveTangentData& d, OutputBuilder& builder, const double offset,
    const double maximumError)
{
    // First find maximum curvature positions.
    double maximumCurvaturePositions[3];

    const int numMaximumCurvaturePositions = curve.FindMaxCurvature(
        maximumCurvaturePositions);

    // Handle special case where the input curve is a straight line, but
    // control points do not necessary lie on line segment between curve
    // points P0 and P3.
    if (CurveIsCompletelyStraight(d)) {
        OffsetLinearCuspyCurve(curve, builder, offset,
            maximumCurvaturePositions, numMaximumCurvaturePositions);
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
            const FloatPoint derivative = curve.DerivedAt(T);
            const double lengthSquared = derivative.LengthSquared();

            if (lengthSquared < CuspDerivativeLengthSquared) {
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
                builder.CuspArcClockwise = FloatPoint::IsTriangleClockwise(
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


/**
 * Flattens ends of curves if control points are too close to end points.
 */
static CubicCurve FixRedundantTangents(const CubicCurve& curve)
{
    FloatPoint p1 = curve.P1;
    FloatPoint p2 = curve.P2;

    if (curve.P0.DistanceToSquared(p1) < 1e-12) {
        p1 = curve.P0;
    }

    if (curve.P3.DistanceToSquared(p2) < 1e-12) {
        p2 = curve.P3;
    }

    return CubicCurve(curve.P0, p1, p2, curve.P3);
}


void OffsetCurve(const CubicCurve& curve, const double offset,
    const double maximumError, CubicCurveBuilder& builder)
{
    builder.Reset();

    const double minx = Min4(curve.P0.X, curve.P1.X, curve.P2.X, curve.P3.X);
    const double maxx = Max4(curve.P0.X, curve.P1.X, curve.P2.X, curve.P3.X);
    const double miny = Min4(curve.P0.Y, curve.P1.Y, curve.P2.Y, curve.P3.Y);
    const double maxy = Max4(curve.P0.Y, curve.P1.Y, curve.P2.Y, curve.P3.Y);

    const double dx = maxx - minx;
    const double dy = maxy - miny;

    if (dx < CurvePointClumpTestEpsilon && dy < CurvePointClumpTestEpsilon) {
        return;
    }

    // Select bigger of width and height.
    const double m = Max(dx, dy) / 2.0;

    // Calculate scaled offset.
    const double so = offset / m;

    if (FuzzyIsZero(so)) {
        builder.AddCubic(curve.P0, curve.P1, curve.P2, curve.P3);
        return;
    }

    // Calculate "normalized" curve which kind of fits into range from -1 to 1.
    const double tx = (minx + maxx) / 2.0;
    const double ty = (miny + maxy) / 2.0;
    const FloatPoint t(tx, ty);

    const FloatPoint p0 = curve.P0 - t;
    const FloatPoint p1 = curve.P1 - t;
    const FloatPoint p2 = curve.P2 - t;
    const FloatPoint p3 = curve.P3 - t;

    const CubicCurve sc(p0 / m, p1 / m, p2 / m, p3 / m);

    const CubicCurve c = FixRedundantTangents(sc);

    OutputBuilder b(builder, m, t);

    const CurveTangentData d(c);

    if (IsCurveApproximatelyStraight(d)) {
        if (CurveIsCompletelyStraight(d)) {
            // Curve is extremely close to being straight, use simple line
            // translation.
            const FloatLine line(c.P0, c.P3);
            const FloatPoint normal = line.UnitNormalVector();
            const FloatLine translated = line.Translated(normal * so);

            MoveTo(b, translated.P0);

            LineTo(b, translated.P1);
        } else {
            // Curve is almost straight. Translate start and end tangents
            // separately and generate a cubic curve.
            const FloatPoint p1o = d.StartTangent.P0 + (so * d.StartUnitNormal);
            const FloatPoint p2o = d.StartTangent.P1 + (so * d.StartUnitNormal);
            const FloatPoint p3o = d.EndTangent.P1 - (so * d.EndUnitNormal);
            const FloatPoint p4o = d.EndTangent.P0 - (so * d.EndUnitNormal);

            MoveTo(b, p1o);

            CubicTo(b, p2o, p3o, p4o);
        }
    } else {
        // Arbitrary curve.
        MoveTo(b, d.StartTangent.P0 + (so * d.StartUnitNormal));

        // Try arc approximation first in case this curve was intended to
        // approximate circle. If that is indeed true, we avoid a lot of
        // expensive calculations like finding inflection and maximum
        // curvature points.
        if (!TryArcApproximation(c, d, b, so, maximumError)) {
            DoApproximateBezier(c, d, b, so, maximumError);
        }
    }
}
