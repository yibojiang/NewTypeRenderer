#pragma once

#define SLABCOUNT 3
#include "math/Vector.h"

#include "Ray.h"

namespace new_type_renderer
{
    /*
     * Bounding volume for a set of given m_Objects, the bounding volume
     * is represented as a collections of parallel planed. Each pair is
     * defined by 2 distance value, m_DistNear and m_DistFar
     */ 
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
        float m_DistNear[SLABCOUNT]{};

        float m_DistFar[SLABCOUNT]{};

#if SLABCOUNT == 7
        static Vector3 m_Normals[SLABCOUNT];
#elif SLABCOUNT == 3
        static Vector3 m_Normals[SLABCOUNT];
#endif
    };
}
