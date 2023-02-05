#pragma once

#define SLABCOUNT 3
#include "math/Vector.h"

struct Ray;

namespace new_type_renderer
{
    struct Extents
    {
    public:
        
        Extents();

        Vector3 getCentriod() const;

        Vector3 getBoundMin() const;

        Vector3 getBoundMax() const;

        void extendBy(Extents&);

        float Intersect(const Ray& r) const;

        float intersectNear(const Ray& r) const;

        float intersectWireframe(const Ray& r) const;

        friend bool operator==(Extents a, const Extents& b)
        {
            if (a.getCentriod() == b.getCentriod())
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
