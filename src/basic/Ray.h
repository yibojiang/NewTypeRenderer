#pragma once
#include "math/Vector.h"

namespace new_type_renderer
{
    struct Ray
    {
    public:
        Ray(const Vector3& ro) : origin(ro) {}

        Ray(const Vector3& ro, const Vector3& rd) : origin(ro), dir(rd) {}

    public:
        Vector3 origin{};
        Vector3 dir{};
        Vector2 uv{};
        Vector3 normal{};
    };
}

