#pragma once
#include <math.h>
#include <limits>
#include <cmath>
#include <memory>

namespace new_type_renderer
{
    using namespace std;

    /*
     * float comparison by using relative epsilon comparison, there are other technique such as ULP
     * checking the distance between the integer represented of the float
     * https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
     */ 
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

    template<typename T>
    T Clamp(T v, T a, T b)
    {
        if (v < a) return a;
        if (v > b) return b;
        return v;
    }

    template<typename T>
    T Clamp01(T v)
    {
        if (v < 0) return 0;
        if (v > 1) return 1;
        return v;
    }

    inline int ChiGGX(float v)
    {
        return v > 0.0f ? 1 : 0;
    }

    inline float Saturate(float v)
    {
        return Clamp(v, 0.0f, 1.0f);
    }

    inline float Random01()
    {
        return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    }

    inline float ToRadian(const float degree)
    {
        return degree * M_PI / 180.0f;
    }

    inline float ToDegree(const float rad)
    {
        return rad * 180.0f / M_PI;
    }
}
