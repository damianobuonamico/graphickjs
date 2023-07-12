
#pragma once


#include "Utils.h"

namespace Blaze {

    /**
     * Fill rule for filling a Bézier path.
     */
    enum class FillRule : uint8 {
        NonZero = 0,
        EvenOdd
    };

}
