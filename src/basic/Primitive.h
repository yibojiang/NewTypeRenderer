#pragma once

#include "material.h"
#include "Shape.h"
#include "core/bvh.h"
#include "math/Matrix.h"

namespace new_type_renderer
{
    enum Refl_t { DIFF, SPEC, REFR }; // material types, used in radiance()


    class Plane : public Shape, public enable_shared_from_this<Plane>
    {
    private:
        Vector3 normal;
    public:
        double off;

        Plane(Vector3 _nor, double _off)
        {
            normal = _nor.Normalize();
            off = _off;
        }

        Intersection Intersect(Ray& r) override
        {
            Intersection intersection;
            intersection.m_Distance = (-r.origin.Dot(normal) - off) / (normal).Dot(r.dir);
            if (intersection.m_Distance > 0)
            {
                intersection.m_HitObject = weak_from_this();
            }
            
            return intersection;
        }

        Vector3 GetHitNormal(const Vector3&hitLocation) const override
        {
            return normal;
        }

        void UpdateTransformMatrix(const Matrix4x4& m) override
        {
            auto normalDir = Vector4(normal, 0);
            normalDir = m * normalDir;
            normalDir.Normalize();
            normal = Vector3(normalDir.x, normalDir.y, normalDir.z);

            auto origin = Vector4(normal * off, 1);
            origin = m * origin;
            auto origin3 = Vector3(origin.x, origin.y, origin.z);
            off = origin3.Dot(normal);
        }
    };


    class Sphere : public Shape, public enable_shared_from_this<Sphere>
    {
    public:
        double rad;
        Vector3 center;


        Matrix4x4 m;

        Sphere(double _rad)
        {
            rad = _rad;
            center = Vector3(0, 0, 0);
        }

        Sphere(double rad, Vector3 pos)
        {
            this->rad = rad;
            this->center = pos;
        }

        Intersection Intersect(Ray& r) override
        {
            Intersection intersection;
            // returns distance, 0 if nohit
            Vector3 op = center - r.origin; // Solve t^2*d.d + 2*t*(o-p).d + (o-p).(o-p)-R^2 = 0
            double t, eps = 1e-4, b = op.Dot(r.dir), det = b * b - op.Dot(op) + rad * rad;
            if (det < 0)
            {
                return intersection;
            }
            det = sqrt(det);

            t = (t = b - det) > eps ? t : ((t = b + det) > eps ? t : 0);

            // double t0 = b + det;
            // double t1 = b - det;
            // if (t0 > t1) std::swap(t0, t1); 

            // if (t0 < 0) { 
            //     t0 = t1; // if t0 is negative, let's use t1 instead 
            //     if (t0 < 0) return 0; // both t0 and t1 are negative 
            // } 

            // t = t0;

            Vector3 hit = r.origin + r.dir * t;
            Vector3 sp = (this->m * (hit - center)).Normalized();

            // float u = (atan( sp.x, sp.z)) / ( 2.0 * M_PI ) + 0.5;
            // float v = asin( sp.y ) / M_PI + 0.5;
            intersection.m_UV = Vector2((atan2(sp.x, sp.z)) / (2.0 * M_PI) + 0.5,
                           asin(sp.y) / M_PI + 0.5);

            intersection.m_Normal = GetHitNormal(hit);
            intersection.m_HitObject = weak_from_this();
            intersection.m_Distance = (t = b - det) > eps ? t : ((t = b + det) > eps ? t : 0);
            // return (t = b - det) > eps ? t : ((t = b + det) > eps ? t : 0);
            return intersection;
        }

        Vector3 GetHitNormal(const Vector3& hitLocation) const override
        {
            return (hitLocation - center) / rad;
        }

        void UpdateTransformMatrix(const Matrix4x4& m) override
        {
            this->m = m;
            auto pos = Vector4(center, 1);
            pos = m * pos;
            center = Vector3(pos.x, pos.y, pos.z);

            Vector3 dir = m * Vector3(1, 0, 0);
            rad = dir.Length();
        }

        void ComputeBounds() override
        {
            // bounds = new Extents();
            for (uint8_t i = 0; i < SLABCOUNT; ++i)
            {
                Vector3 slabN = Extents::m_Normals[i];
                double d = center.Dot(slabN);
                m_Bounds.m_DistNear[i] = -rad - d;
                m_Bounds.m_DistFar[i] = rad - d;
            }
        }

        virtual Vector3 GetCentriod() const override
        {
            return center;
        }


        // virtual Vector3 debug(Vector3 _pos) const{return Vector3(1);}
    };


    class Box : public Shape, public enable_shared_from_this<Box>
    {
    private:
        float dnear[3];
        float dfar[3];
    public:
        Vector3 p[8];
        Vector3 center;
        Vector3 size;
        Vector3 normals[3];

        Box(const Vector3& _size)
        {
            size = _size;
            normals[0] = Vector3(1, 0, 0);
            normals[1] = Vector3(0, 1, 0);
            normals[2] = Vector3(0, 0, 1);

            Vector3 half_size = size * 0.5;
            p[0] = center + Vector3(half_size.x, half_size.y, half_size.z);
            p[1] = center + Vector3(-half_size.x, half_size.y, half_size.z);
            p[2] = center + Vector3(half_size.x, -half_size.y, half_size.z);
            p[3] = center + Vector3(half_size.x, half_size.y, -half_size.z);
            p[4] = center + Vector3(-half_size.x, -half_size.y, half_size.z);
            p[5] = center + Vector3(half_size.x, -half_size.y, -half_size.z);
            p[6] = center + Vector3(-half_size.x, half_size.y, -half_size.z);
            p[7] = center + Vector3(-half_size.x, -half_size.y, -half_size.z);
            updatePlane();
        }

        void updatePlane()
        {
            for (int i = 0; i < 3; ++i)
            {
                dnear[i] = FLT_MAX;
                dfar[i] = -FLT_MAX;

                for (int j = 0; j < 8; ++j)
                {
                    double d = -p[j].Dot(normals[i]);
                    if (d > dfar[i])
                    {
                        dfar[i] = d;
                    }

                    if (d < dnear[i])
                    {
                        dnear[i] = d;
                    }
                }
            }
        }

        // AABB
        // double Intersect(const Ray &r) { // returns distance, 0 if nohit
        //     Vector3 invdir(1.0 / r.dir.x, 1.0 / r.dir.y, 1.0 / r.dir.z);

        //     // lb is the corner of AABB with minimal coordinates - left bottom, rt is maximal corner
        //     // r.org is origin of ray
        //     double t1 = (bounds[0].x - r.origin.x) * invdir.x;
        //     double t2 = (bounds[1].x - r.origin.x) * invdir.x;
        //     double t3 = (bounds[0].y - r.origin.y) * invdir.y;
        //     double t4 = (bounds[1].y - r.origin.y) * invdir.y;
        //     double t5 = (bounds[0].z - r.origin.z) * invdir.z;
        //     double t6 = (bounds[1].z - r.origin.z) * invdir.z;

        //     double tmin = fmax(fmax(fmin(t1, t2), fmin(t3, t4)), fmin(t5, t6)); // max tnear 
        //     double tmax = fmin(fmin(fmax(t1, t2), fmax(t3, t4)), fmax(t5, t6)); // min tfar

        //     // if tmax < 0, ray (line) is intersecting AABB, but whole AABB is behing us
        //     if (tmax < 0){
        //         return 0;
        //     }

        //     // if tmin > tmax, ray doesn't Intersect AABB
        //     if (tmin > tmax){
        //         return 0;
        //     }
        //     return tmin;
        // } 


        Intersection Intersect(Ray& r) override
        {
            Intersection intersection;
            // returns distance, 0 if nohit
            double t1 = (-dnear[0] - r.origin.Dot(normals[0])) / r.dir.Dot(normals[0]);
            double t2 = (-dfar[0] - r.origin.Dot(normals[0])) / r.dir.Dot(normals[0]);
            double t3 = (-dnear[1] - r.origin.Dot(normals[1])) / r.dir.Dot(normals[1]);
            double t4 = (-dfar[1] - r.origin.Dot(normals[1])) / r.dir.Dot(normals[1]);
            double t5 = (-dnear[2] - r.origin.Dot(normals[2])) / r.dir.Dot(normals[2]);
            double t6 = (-dfar[2] - r.origin.Dot(normals[2])) / r.dir.Dot(normals[2]);
            double tmin = fmax(fmax(fmin(t1, t2), fmin(t3, t4)), fmin(t5, t6)); // max t m_Near 
            double tmax = fmin(fmin(fmax(t1, t2), fmax(t3, t4)), fmax(t5, t6)); // min t far
            // if tmax < 0, ray (line) is intersecting AABB, but whole AABB is behing us
            if (tmax < 0)
            {
                return intersection;
            }

            // if tmin > tmax, ray doesn't Intersect AABB
            if (tmin > tmax)
            {
                return intersection;
            }

            Vector3 hit = r.origin + r.dir * tmin;
            intersection.m_Normal = GetHitNormal(hit);
            intersection.m_HitObject = weak_from_this();
            intersection.m_Distance = tmin;
            return intersection;
        }

        Vector3 GetHitNormal(const Vector3& hitLocation) const override
        {
            for (int i = 0; i < 3; ++i)
            {
                if (ApproximatelyEqual(hitLocation.Dot(normals[i]) + dnear[i], 0.0f))
                {
                    return normals[i];
                }

                if (ApproximatelyEqual(hitLocation.Dot(normals[i]) + dfar[i], 0.0f))
                {
                    return -normals[i];
                }

                if (ApproximatelyEqual(hitLocation.Dot(-normals[i]) + dnear[i], 0.0f))
                {
                    return -normals[i];
                }

                if (ApproximatelyEqual(hitLocation.Dot(-normals[i]) + dfar[i], 0.0f))
                {
                    return normals[i];
                }
            }
            return Vector3();
        }

        void UpdateTransformMatrix(const Matrix4x4& m) override
        {
            for (int i = 0; i < 3; ++i)
            {
                // Vector4 n = m*  Vector4(normals[i], 0);
                normals[i] = m * normals[i];
                normals[i].Normalize();
            }

            for (int i = 0; i < 8; ++i)
            {
                auto p4 = Vector4(p[i], 1);
                p4 = m * p4;
                p[i] = Vector3(p4.x, p4.y, p4.z);
            }

            // size = m * size;
            Vector4 center4 = m * Vector4(center, 1);
            center = Vector3(center4.x, center4.y, center4.z);
            updatePlane();
        }

        void ComputeBounds() override
        {
            for (uint8_t i = 0; i < SLABCOUNT; ++i)
            {
                m_Bounds.m_DistNear[i] = FLT_MAX;
                m_Bounds.m_DistFar[i] = -FLT_MAX;
                for (int j = 0; j < 8; ++j)
                {
                    // qDebug()<<p[j];
                    double d = -p[j].Dot(Extents::m_Normals[i]);
                    if (d < m_Bounds.m_DistNear[i])
                    {
                        m_Bounds.m_DistNear[i] = d;
                    }
                    if (d > m_Bounds.m_DistFar[i])
                    {
                        m_Bounds.m_DistFar[i] = d;
                    }
                }
            }
        }

        virtual Vector3 GetCentriod() const override
        {
            return center;
        }
    };
}
