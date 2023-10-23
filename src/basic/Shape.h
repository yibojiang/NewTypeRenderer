#pragma once

#include "core/Extents.h"
#include "core/ray.h"
#include "basic/Material.h"
#include "math/Matrix.h"

namespace new_type_renderer
{
    struct Material;
    class Mesh;

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

        virtual void UpdateWorldTransformMatrix(const Matrix4x4&) = 0;

        const Matrix4x4& GetWorldTransform() const { return m_WorldTransform; }

        virtual void ComputeBounds() = 0;

        Extents GetBounds()
        {
            return m_Bounds;
        }

    protected:
        Extents m_Bounds{};

        shared_ptr<Material> m_Material;

        Matrix4x4 m_WorldTransform;
    };
}
