#include "Extents.h"

#include "BVH.h"

namespace new_type_renderer
{
#if SLABCOUNT == 7
    Vector3 Extents::m_Normals[SLABCOUNT]{
        Vector3(1, 0, 0),
        Vector3(0, 1, 0),
        Vector3(0, 0, 1),
        Vector3(sqrt(3.0) / 3.0, sqrt(3.0) / 3.0, sqrt(3.0) / 3.0),
        Vector3(-sqrt(3.0) / 3.0, sqrt(3.0) / 3.0, sqrt(3.0) / 3.0),
        Vector3(-sqrt(3.0) / 3.0, -sqrt(3.0) / 3.0, sqrt(3.0) / 3.0),
        Vector3(sqrt(3.0) / 3.0, -sqrt(3.0) / 3.0, sqrt(3.0) / 3.0)
    };
#elif SLABCOUNT == 3
    Vector3 Extents::m_Normals[SLABCOUNT]{
        Vector3(1, 0, 0),
        Vector3(0, 1, 0),
        Vector3(0, 0, 1)
    };
#endif

    Extents::Extents()
    {
        for (uint8_t i = 0; i < SLABCOUNT; ++i)
        {
            m_DistNear[i] = DBL_MAX;
            m_DistFar[i] = -DBL_MAX;
        }
    }

    Vector3 Extents::GetCentriod() const
    {
#if SLABCOUNT == 3
        return Vector3(-(m_DistNear[0] + m_DistFar[0]) * 0.5, -(m_DistNear[1] + m_DistFar[1]) * 0.5, -(m_DistNear[2] + m_DistFar[2]) * 0.5);
#endif
#if SLABCOUNT == 7
        Vector3 c1 = Vector3(-(m_DistNear[0] + m_DistFar[0]) * 0.5, -(m_DistNear[1] + m_DistFar[1]) * 0.5, -(m_DistNear[2] + m_DistFar[2]) * 0.5);
        Vector3 c2 = Vector3(-(m_DistNear[3] + m_DistFar[3]) * 0.5, -(m_DistNear[4] + m_DistFar[4]) * 0.5, -(m_DistNear[5] + m_DistFar[5]) * 0.5);

        return (c1 + c2) * 0.5;
#endif
    }

    Vector3 Extents::GetBoundMin() const
    {
        return Vector3(-m_DistFar[0], -m_DistFar[1], -m_DistFar[2]);
    }

    Vector3 Extents::GetBoundMax() const
    {
        return Vector3(-m_DistNear[0], -m_DistNear[1], -m_DistNear[2]);
    }

    void Extents::ExtendBy(Extents& extents)
    {
        for (int i = 0; i < SLABCOUNT; ++i)
        {
            m_DistNear[i] = fmin(extents.m_DistNear[i], m_DistNear[i]);
            m_DistFar[i] = fmax(extents.m_DistFar[i], m_DistFar[i]);
        }
    }

    double Extents::Intersect(const Ray& r) const
    {
        // returns distance, 0 if no hit
        double tmin = -DBL_MAX;
        double tmax = DBL_MAX;
        for (uint8_t i = 0; i < SLABCOUNT; ++i)
        {
            double tNear = (-m_DistNear[i] - r.origin.Dot(Extents::m_Normals[i])) / r.dir.Dot(Extents::m_Normals[i]);
            double tFar = (-m_DistFar[i] - r.origin.Dot(Extents::m_Normals[i])) / r.dir.Dot(Extents::m_Normals[i]);

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

    double Extents::IntersectNear(const Ray& r) const
    {
        // return distance to the plane, 0 if missed
        double tmin = -DBL_MAX;
        double tmax = DBL_MAX;
        for (uint8_t i = 0; i < SLABCOUNT; ++i)
        {
            double tNear = (-m_DistNear[i] - r.origin.Dot(Extents::m_Normals[i])) / r.dir.Dot(Extents::m_Normals[i]);
            double tFar = (-m_DistFar[i] - r.origin.Dot(Extents::m_Normals[i])) / r.dir.Dot(Extents::m_Normals[i]);

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
        // if (tmin < DBL_MAX)
        // {
        //     // tmin = 1000;
        //     tmin = DBL_MAX * 2;
        //     // tmin = -1;
        // }
        //     return fmax(fmin(fabs(tmin), tmax), eps * 2);
        // }

        return tmin;
    }

    double Extents::IntersectWireframe(const Ray& r) const
    {
        // returns distance, 0 if nohit    

        double tmin = -DBL_MAX;
        double tmax = DBL_MAX;
        for (uint8_t i = 0; i < SLABCOUNT; ++i)
        {
            double tNear = (-m_DistNear[i] - r.origin.Dot(Extents::m_Normals[i])) / r.dir.Dot(Extents::m_Normals[i]);
            double tFar = (-m_DistFar[i] - r.origin.Dot(Extents::m_Normals[i])) / r.dir.Dot(Extents::m_Normals[i]);

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
            if (fabs(-m_DistNear[i] - hit.Dot(Extents::m_Normals[i])) < width)
            {
                count++;
            }
            else if (fabs(-m_DistFar[i] - hit.Dot(Extents::m_Normals[i])) < width)
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
            if (fabs(-m_DistNear[i] - hit.Dot(Extents::m_Normals[i])) < width)
            {
                count++;
            }
            else if (fabs(-m_DistFar[i] - hit.Dot(Extents::m_Normals[i])) < width)
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
