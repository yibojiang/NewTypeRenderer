#pragma once

#include "core/Extents.h"
#include "core/ray.h"
#include "basic/Material.h"

namespace new_type_renderer
{
    struct Matrix4x4;
    struct Material;

    class Shape
    {
    public:
        std::string name;

        Shape() {}

        virtual ~Shape()
        {
        }

        virtual Vector3 GetHitNormal(const Vector3&hitLocation) const { return Vector3(1); }

        virtual Intersection Intersect(Ray&) { return Intersection{}; }

        virtual float GetProjectAngleToSphere()
        {
            return M_PI;
        }


        void SetMaterial(shared_ptr<Material>& material)
        {
            m_Material = material;
        }

        shared_ptr<Material>& GetMaterial()
        {
            return m_Material;
        }

        virtual Vector3 GetCentriod() const = 0;

        virtual void UpdateTransformMatrix(const Matrix4x4&) = 0;

        virtual void ComputeBounds() = 0;

        Extents GetBounds()
        {
            return m_Bounds;
        }

    protected:
        Extents m_Bounds{};

        shared_ptr<Material> m_Material;
    };
}