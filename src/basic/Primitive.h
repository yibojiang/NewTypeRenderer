#pragma once

#include "material.h"
#include "Object.h"
#include "core/bvh.h"
#include "math/Matrix.h"

namespace new_type_renderer
{
    enum Refl_t { DIFF, SPEC, REFR }; // material types, used in radiance()


    class Plane : public Object
    {
    private:
        Vector3 normal;
    public:
        double off;

        Plane(Vector3 _nor, double _off)
        {
            normal = _nor.Normalize();
            off = _off;
            isMesh = false;
        }

        float Intersect(Ray& r) override
        {
            return (-r.origin.Dot(normal) - off) / (normal).Dot(r.dir);
        }

        Vector3 GetNormal(const Vector3&) const override
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


    class Sphere : public Object
    {
    public:
        double rad;
        Vector3 center;


        Matrix4x4 m;

        Sphere(double _rad)
        {
            rad = _rad;
            isMesh = false;
            center = Vector3(0, 0, 0);
        }

        Sphere(double rad, Vector3 pos)
        {
            this->rad = rad;
            this->center = pos;
        }

        float Intersect(Ray& r) override
        {
            // returns distance, 0 if nohit
            Vector3 op = center - r.origin; // Solve t^2*d.d + 2*t*(o-p).d + (o-p).(o-p)-R^2 = 0
            double t, eps = 1e-4, b = op.Dot(r.dir), det = b * b - op.Dot(op) + rad * rad;
            if (det < 0)
            {
                return 0;
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
            r.uv = Vector2((atan2(sp.x, sp.z)) / (2.0 * M_PI) + 0.5,
                           asin(sp.y) / M_PI + 0.5);

            r.normal = GetNormal(hit);
            // return (t = b - det) > eps ? t : ((t = b + det) > eps ? t : 0);
            return t;
        }

        Vector3 GetNormal(const Vector3& _pos) const override
        {
            return (_pos - center) / rad;
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
                Vector3 slabN = BVH::normals[i];
                double d = center.Dot(slabN);
                bounds.m_DistNear[i] = -rad - d;
                bounds.m_DistFar[i] = rad - d;
            }
        }

        virtual Vector3 GetCentriod() const override
        {
            return center;
        }


        // virtual Vector3 debug(Vector3 _pos) const{return Vector3(1);}
    };


    class Box : public Object
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

            isMesh = false;
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


        float Intersect(Ray& r) override
        {
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
                return 0;
            }

            // if tmin > tmax, ray doesn't Intersect AABB
            if (tmin > tmax)
            {
                return 0;
            }

            Vector3 hit = r.origin + r.dir * tmin;
            r.normal = GetNormal(hit);

            return tmin;
        }

        Vector3 GetNormal(const Vector3& _pos) const override
        {
            for (int i = 0; i < 3; ++i)
            {
                if (ApproximatelyEqual(_pos.Dot(normals[i]) + dnear[i], 0.0f))
                {
                    return normals[i];
                }

                if (ApproximatelyEqual(_pos.Dot(normals[i]) + dfar[i], 0.0f))
                {
                    return -normals[i];
                }

                if (ApproximatelyEqual(_pos.Dot(-normals[i]) + dnear[i], 0.0f))
                {
                    return -normals[i];
                }

                if (ApproximatelyEqual(_pos.Dot(-normals[i]) + dfar[i], 0.0f))
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
                bounds.m_DistNear[i] = FLT_MAX;
                bounds.m_DistFar[i] = -FLT_MAX;
                for (int j = 0; j < 8; ++j)
                {
                    // qDebug()<<p[j];
                    double d = -p[j].Dot(BVH::normals[i]);
                    if (d < bounds.m_DistNear[i])
                    {
                        bounds.m_DistNear[i] = d;
                    }
                    if (d > bounds.m_DistFar[i])
                    {
                        bounds.m_DistFar[i] = d;
                    }
                }
            }
        }

        virtual Vector3 GetCentriod() const override
        {
            return center;
        }
    };

    class Triangle : public Object
    {
    private:
    public:
        Vector2 uv1, uv2, uv3;
        Vector3 n1, n2, n3;
        Vector3 normal, u, v;
        Vector3 p1, p2, p3;
        double s, t;

        Triangle(Vector3 in_p1, Vector3 in_p2, Vector3 in_p3)
        {
            p1 = in_p1;
            p2 = in_p2;
            p3 = in_p3;
            u = p2 - p1;
            v = p3 - p1;
            normal = u.Cross(v).Normalize();
            isMesh = false;
        }

        ~Triangle() override
        {
        }

        void setupVertices(const Vector3 in_p1, const Vector3 in_p2, const Vector3 in_p3)
        {
            p1 = in_p1;
            p2 = in_p2;
            p3 = in_p3;
            u = p2 - p1;
            v = p3 - p1;
            normal = u.Cross(v).Normalize(); // Cross product
            isMesh = false;
        }

        void setupNormals(Vector3 in_n1, Vector3 in_n2, Vector3 in_n3)
        {
            n1 = in_n1.Normalize();
            n2 = in_n2.Normalize();
            n3 = in_n3.Normalize();
        }

        void setupUVs(Vector2 in_uv1, Vector2 in_uv2, Vector2 in_uv3)
        {
            uv1 = in_uv1;
            uv2 = in_uv2;
            uv3 = in_uv3;
        }

        Vector3 GetNormal(const Vector3&) const override
        {
            return normal;
            // return n1;
            // return n1 * (1 - s -t) + n2 * s + n3 * t;
        }

        float Intersect(Ray& r) override
        {
            // returns distance, 0 if nohit        
            // Vector3 nl = r.dir.Dot(normal) > 0 ? normal : normal * -1;
            // double dist = -r.origin.Dot(nl) + center.Dot(nl);
            // double tt = dist / r.dir.Dot(nl);

            // if (fabs(r.dir.Dot(normal)) < eps){
            //     return 0;
            // }


            // if (normal == Vector3(0,0,0)){             // triangle is degenerate
            //     return 0; 
            // }

            double dn = r.dir.Dot(normal);
            if (fabs(dn) < FLT_EPSILON)
            {
                // ray is  parallel to triangle plane
                return 0;
            }

            Vector3 center = (p1 + p2 + p3) / 3;
            double dist = -r.origin.Dot(normal) + center.Dot(normal);
            double tt = dist / dn;
            Vector3 hit = r.origin + r.dir * tt;


            u = p2 - p1;
            v = p3 - p1;

            // is I inside T?
            double uu, uv, vv, wu, wv, D;
            uu = u.Dot(u);
            uv = u.Dot(v);
            vv = v.Dot(v);

            Vector3 w = hit - p1;
            wu = w.Dot(u);
            wv = w.Dot(v);
            D = uv * uv - uu * vv;

            // get and test parametric coords
            // double s, t;
            s = (uv * wv - vv * wu) / D;
            if (s < 0.0 || s > 1.0)
            {
                // I is outside T
                return 0;
            }

            t = (uv * wu - uu * wv) / D;
            if (t < 0.0 || (s + t) > 1.0)
            {
                // I is outside T
                return 0;
            }

            // r.uv = (uv1 * s +  uv2 * (1 - s));
            r.uv = uv1 * (1 - s - t) + uv2 * s + uv3 * t;

            r.normal = GetNormal(hit);
            // r.normal = n1 * (1 - s -t) + n2 * s + n3 * t;
            // r.uv = Vector2(drand48(), drand48());

            // r.uv.x = fmin(1, r.uv.x);
            // r.uv.y = fmin(1, r.uv.y);
            // r.uv = uv1 * s +  uv2 * (1 - s);
            return tt;
        }

        void ComputeBounds() override
        {
            for (uint8_t i = 0; i < SLABCOUNT; ++i)
            {
                Vector3 slabN = BVH::normals[i];
                double d1 = -p1.Dot(slabN);
                double d2 = -p2.Dot(slabN);
                double d3 = -p3.Dot(slabN);
                bounds.m_DistNear[i] = fmin(d1, fmin(d2, d3));
                bounds.m_DistFar[i] = fmax(d1, fmax(d2, d3));

                // bounds.m_DistNear[i]-=eps;
                // bounds.m_DistFar[i]+=eps;
                // qDebug() << this->name.c_str() << "m_Near" << bounds.m_DistNear[i];
                // qDebug() << this->name.c_str() << "far" << bounds.m_DistFar[i];
                // if ( fabs(bounds.m_DistFar[i] - bounds.m_DistNear[i]) < eps){
                //     bounds.m_DistNear[i]-=eps;
                //     bounds.m_DistFar[i]+=eps;
                // }
            }
        }

        Vector3 GetCentriod() const override
        {
            return (p1 + p2 + p3) / 3.0;
        }

        void UpdateTransformMatrix(const Matrix4x4& m) override
        {
            Vector4 vp1(p1, 1.0);
            Vector4 vp2(p2, 1.0);
            Vector4 vp3(p3, 1.0);

            vp1 = m * vp1;
            vp2 = m * vp2;
            vp3 = m * vp3;

            p1 = Vector3(vp1.x, vp1.y, vp1.z);
            p2 = Vector3(vp2.x, vp2.y, vp2.z);
            p3 = Vector3(vp3.x, vp3.y, vp3.z);

            n1 = m * n1;
            n2 = m * n2;
            n3 = m * n3;
        }
    };

    // class Face
    // {

    // public:
    //     Vector3 v1, v2, v3;
    //     Vector3 n1, n2, n3;
    //     Vector2 uv1, uv2, uv3;

    //     void setupUVs(const Vector2 uv1,const Vector2 uv2,const Vector2 uv3){
    //         this->uv1 = uv1;
    //         this->uv2 = uv2;
    //         this->uv3 = uv3;

    //     }

    //     void setupVertices(const Vector3 v1,const Vector3 v2,const Vector3 v3){
    //         this->v1 = v1;
    //         this->v2 = v2;
    //         this->v3 = v3;
    //     }

    //     void setupNormals(const Vector3 n1,const Vector3 n2,const Vector3 n3){
    //         this->n1 = n1;
    //         this->n2 = n2;
    //         this->n3 = n3;
    //     }


    //     Face() {};
    //     ~Face() {};

    // };

    class Mesh : public Object
    {
    public:
        std::vector<Triangle*> faces;

        Mesh()
        {
            isMesh = true;
        }

        ~Mesh() override
        {
            for (uint32_t i = 0; i < faces.size(); ++i)
            {
                delete faces[i];
            }
        }

        void UpdateTransformMatrix(const Matrix4x4& m) override
        {
            for (uint32_t i = 0; i < faces.size(); ++i)
            {
                Vector4 p1(faces[i]->p1, 1.0);
                Vector4 p2(faces[i]->p2, 1.0);
                Vector4 p3(faces[i]->p3, 1.0);

                p1 = m * p1;
                p2 = m * p2;
                p3 = m * p3;

                faces[i]->p1 = Vector3(p1.x, p1.y, p1.z);
                faces[i]->p2 = Vector3(p2.x, p2.y, p2.z);
                faces[i]->p3 = Vector3(p3.x, p3.y, p3.z);

                faces[i]->u = faces[i]->p2 - faces[i]->p1;
                faces[i]->v = faces[i]->p3 - faces[i]->p1;
                faces[i]->normal = faces[i]->u.Cross(faces[i]->v).Normalize();

                faces[i]->n1 = m * faces[i]->n1;
                faces[i]->n2 = m * faces[i]->n2;
                faces[i]->n3 = m * faces[i]->n3;
            }
        }

        void ComputeBounds() override
        {

        }

        Vector3 GetCentriod() const override
        {
            return Vector3{};
        }

        void AddFace(Triangle* face)
        {
            faces.push_back(face);
        }
    };
}
