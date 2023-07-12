
#pragma once


#include "F24Dot8.h"
#include "FloatPoint.h"

namespace Blaze {

    struct F24Dot8Point final {
        F24Dot8 X;
        F24Dot8 Y;
    };

#ifdef GK_PLATFORM_WINDOWS
    static F24Dot8Point FloatPointToF24Dot8Point(const FloatPoint p) {
#else
    static constexpr F24Dot8Point FloatPointToF24Dot8Point(const FloatPoint p) {
#endif
        return F24Dot8Point{
            DoubleToF24Dot8(p.X),
            DoubleToF24Dot8(p.Y)
        };
    }

#ifdef GK_PLATFORM_WINDOWS
    static F24Dot8Point FloatPointToF24Dot8Point(const double x, const double y) {
#else
    static constexpr F24Dot8Point FloatPointToF24Dot8Point(const double x, const double y) {
#endif
        return F24Dot8Point{
            DoubleToF24Dot8(x),
            DoubleToF24Dot8(y)
        };
    }


    STATIC_ASSERT(SIZE_OF(F24Dot8Point) == 8);

}
