#include "Extents.h"

#include "BVH.h"

namespace new_type_renderer
{
    Extents::Extents()
    {
        for (uint8_t i = 0; i < SLABCOUNT; ++i)
        {
            dnear[i] = FLT_MAX;
            dfar[i] = -FLT_MAX;
        }
    }

    Vector3 Extents::getCentriod() const
    {
#if SLABCOUNT == 3
        return Vector3(-(dnear[0] + dfar[0]) * 0.5, -(dnear[1] + dfar[1]) * 0.5, -(dnear[2] + dfar[2]) * 0.5);
#endif
#if SLABCOUNT == 7
        Vector3 c1 = Vector3(-(dnear[0] + dfar[0]) * 0.5, -(dnear[1] + dfar[1]) * 0.5, -(dnear[2] + dfar[2]) * 0.5);
        Vector3 c2 = Vector3(-(dnear[3] + dfar[3]) * 0.5, -(dnear[4] + dfar[4]) * 0.5, -(dnear[5] + dfar[5]) * 0.5);

        return (c1 + c2) * 0.5;
#endif
    }

    Vector3 Extents::getBoundMin() const
    {
        return Vector3(-dfar[0], -dfar[1], -dfar[2]);
    }

    Vector3 Extents::getBoundMax() const
    {
        return Vector3(-dnear[0], -dnear[1], -dnear[2]);
    }

    void Extents::extendBy(Extents& extents)
    {
        for (int i = 0; i < SLABCOUNT; ++i)
        {
            dnear[i] = fmin(extents.dnear[i], dnear[i]);
            dfar[i] = fmax(extents.dfar[i], dfar[i]);
        }
    }

    float Extents::Intersect(const Ray& r) const
    {
        // returns distance, 0 if nohit    

        float tmin = -FLT_MAX;
        float tmax = FLT_MAX;
        for (uint8_t i = 0; i < SLABCOUNT; ++i)
        {
            double tNear = (-dnear[i] - r.origin.Dot(BVH::normals[i])) / r.dir.Dot(BVH::normals[i]);
            double tFar = (-dfar[i] - r.origin.Dot(BVH::normals[i])) / r.dir.Dot(BVH::normals[i]);

            // Swap near and far t.
            if (tNear > tFar)
            {
                std::swap(tFar, tNear);
            }

            if (tNear > tmin)
            {
                tmin = tNear;
            }

            if (tFar < tmax)
            {
                tmax = tFar;
            }
        }

        if (tmax < 0)
        {
            return 0;
        }

        if (tmin > tmax)
        {
            return 0;
        }

        if (tmin < 0)
        {
            return tmax;
        }

        return tmin;
    }

    float Extents::intersectNear(const Ray& r) const
    {
        // returns distance, 0 if nohit    

        float tmin = -FLT_MAX;
        float tmax = FLT_MAX;
        for (uint8_t i = 0; i < SLABCOUNT; ++i)
        {
            double tNear = (-dnear[i] - r.origin.Dot(BVH::normals[i])) / r.dir.Dot(BVH::normals[i]);
            double tFar = (-dfar[i] - r.origin.Dot(BVH::normals[i])) / r.dir.Dot(BVH::normals[i]);

            // Swap near and far t.
            if (tNear > tFar)
            {
                std::swap(tFar, tNear);
            }

            if (tNear > tmin)
            {
                tmin = tNear;
            }

            if (tFar < tmax)
            {
                tmax = tFar;
            }
        }

        if (tmax <= 0)
        {
            return 0;
        }

        if (tmin > tmax)
        {
            return 0;
        }

        // if (fabs(tmax-tmin)<eps){
        //     return -1;
        // }


        // if (tmin < eps){
        //     // return fmax(fmin(fabs(tmin), tmax), eps * 2);
        //     return fmin(fabs(tmin), tmax);
        //     // return -1;
        // }

        // inside the boundingbox
        if (tmin < eps)
        {
            // tmin = 1000;
            tmin = eps * 2;
            // tmin = -1;
        }
        //     return fmax(fmin(fabs(tmin), tmax), eps * 2);
        // }

        return tmin;
    }

    double Extents::intersectWireframe(const Ray& r) const
    {
        // returns distance, 0 if nohit    

        double tmin = -inf;
        double tmax = inf;
        for (uint8_t i = 0; i < SLABCOUNT; ++i)
        {
            double tNear = (-dnear[i] - r.origin.Dot(BVH::normals[i])) / r.dir.Dot(BVH::normals[i]);
            double tFar = (-dfar[i] - r.origin.Dot(BVH::normals[i])) / r.dir.Dot(BVH::normals[i]);

            // Swap near and far t.
            if (tNear > tFar)
            {
                // double tmp = tFar;
                // tFar = tNear;
                // tNear = tmp;
                std::swap(tFar, tNear);
            }

            if (tNear > tmin)
            {
                tmin = tNear;
            }

            if (tFar < tmax)
            {
                tmax = tFar;
            }
        }

        if (tmax < 0)
        {
            return 0;
        }

        if (tmin > tmax)
        {
            return 0;
        }

        double width = 0.4;
        int count = 0;
        double t = tmin;
        Vector3 hit = r.origin + r.dir * t;
        for (int i = 0; i < SLABCOUNT; ++i)
        {
            if (fabs(-dnear[i] - hit.Dot(BVH::normals[i])) < width)
            {
                count++;
            }
            else if (fabs(-dfar[i] - hit.Dot(BVH::normals[i])) < width)
            {
                count++;
            }

            if (count > 1)
            {
                return t;
            }
        }

        count = 0;
        t = tmax;
        hit = r.origin + r.dir * t;
        for (int i = 0; i < SLABCOUNT; ++i)
        {
            if (fabs(-dnear[i] - hit.Dot(BVH::normals[i])) < width)
            {
                count++;
            }
            else if (fabs(-dfar[i] - hit.Dot(BVH::normals[i])) < width)
            {
                count++;
            }

            if (count > 1)
            {
                return t;
            }
        }


        return 0;
    }
}
