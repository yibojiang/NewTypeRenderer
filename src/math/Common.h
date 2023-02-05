#pragma once
#include <math.h>
#include <limits>
#include <cmath>
#include <memory>

namespace new_type_renderer
{
    using namespace std;
    // relative epsilon comparison
    // https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
    inline bool ApproximatelyEqual(float a, float b, float epsilon = FLT_EPSILON)
    {
        const float diff = fabs(a - b);
        const float largest = fabs(b) > fabs(a) ? fabs(b) : fabs(a);
        if (diff <= largest * epsilon)
        {
            return true;
        }
        return false;
    }

    inline bool EssentiallyEqual(float a, float b, float epsilon = FLT_EPSILON)
    {
        const float diff = fabs(a - b);
        const float smallest = fabs(b) > fabs(a) ? fabs(a) : fabs(b);
        if (diff <= smallest * epsilon)
        {
            return true;
        }
        return false;
    }

    inline bool DefinitelyGreaterThan(float a, float b, float epsilon = FLT_EPSILON)
    {
        return (a - b) > ((fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon);
    }

    inline bool DefinitelyLessThan(float a, float b, float epsilon = FLT_EPSILON)
    {
        return (b - a) > ((fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon);
    }
}
