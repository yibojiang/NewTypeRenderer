#pragma once
#include "math/Vector.h"

namespace new_type_renderer
{
    class Object;

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

    
    struct Intersection
    {
    public:
        Intersection()
        {
        }

        Intersection(double d, Object* obj) : t(d), object(obj)
        {
        }

        operator bool() {return object != nullptr; }

        Object* GetHitObject()
        {
            return object;
        }

        float GetHitDistance()
        {
            return t;
        }

    public:
        float t{ FLT_MAX };

        Object* object{ nullptr };
    };
}

