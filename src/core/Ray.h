#pragma once
#include "math/Vector.h"

namespace new_type_renderer
{
    class Shape;

    struct Ray
    {
    public:
        Ray(const Vector3& ro) : origin(ro) {}

        Ray(const Vector3& ro, const Vector3& rd) : origin(ro), dir(rd) {}

    public:
        Vector3 origin{};

        Vector3 dir{};
    };

    
    struct Intersection
    {
    public:
        Intersection()
        {

        }

        Intersection(float d, weak_ptr<Shape> obj) : m_Distance(d), m_HitObject(obj)
        {
        }

        bool operator<(Intersection& other)
        {
            return m_Distance < other.GetHitDistance();
        }

        weak_ptr<Shape>& GetHitObject()
        {
            return m_HitObject;
        }

        double GetHitDistance() const
        {
            return m_Distance;
        }

        Vector2 GetUV() const
        {
            return m_UV;
        }

        Vector3 GetNormal() const
        {
            return m_Normal;
        }

    public:
        weak_ptr<Shape> m_HitObject{};

        Vector2 m_UV{};

        Vector3 m_Normal{};

        double m_Distance{ DBL_MAX };
    };
}

