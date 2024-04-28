
#include "FloatPoint.h"


TrianglePointOrientation FloatPoint::DetermineTriangleOrientation(
    const FloatPoint &p0, const FloatPoint &p1, const FloatPoint &p2)
{
    const double turn = Turn(p0, p1, p2);

    if (FuzzyIsZero(turn)) {
        return TrianglePointOrientation::Collinear;
    } else if (turn > 0.0) {
        return TrianglePointOrientation::Clockwise;
    }

    return TrianglePointOrientation::CounterClockwise;
}


bool FloatPoint::IsEqual(const FloatPoint &point, const double epsilon) const
{
    return IsEqualWithEpsilon(X, point.X, epsilon) &&
        IsEqualWithEpsilon(Y, point.Y, epsilon);
}


FloatPoint FloatPoint::UnitVector() const
{
    const double mag2 = LengthSquared();

    if (mag2 != 0.0 && mag2 != 1.0) {
        const double length = Sqrt(mag2);

        return FloatPoint(X / length, Y / length);
    }

    return *this;
}
