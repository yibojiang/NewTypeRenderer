#pragma once

#define SLABCOUNT 3
#include "math/Vector.h"

#include "Ray.h"

namespace new_type_renderer
{
    struct Extents
    {
    public:
        
        Extents();

        Vector3 GetCentriod() const;

        Vector3 GetBoundMin() const;

        Vector3 GetBoundMax() const;

        void ExtendBy(Extents&);

        float Intersect(const Ray& r) const;

        float IntersectNear(const Ray& r) const;

        float IntersectWireframe(const Ray& r) const;

        friend bool operator==(Extents a, const Extents& b)
        {
            if (a.GetCentriod() == b.GetCentriod())
            {
                return true;
            }
            return false;
        }

    public:
        float dnear[SLABCOUNT]{};

        float dfar[SLABCOUNT]{};
    };
}
