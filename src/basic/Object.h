#pragma once

#include "core/Extents.h"
#include "core/ray.h"
#include "basic/Material.h"

namespace new_type_renderer
{
    struct Matrix4x4;
    struct Material;

    class Object
    {
    public:
        std::string name;

        Object() {}

        virtual ~Object()
        {
        }

        virtual Vector3 GetHitNormal(const Vector3&hitLocation) const { return Vector3(1); }

        virtual float Intersect(Ray&) { return 0; }

        virtual float GetProjectAngleToSphere()
        {
            return M_PI;
        }


        void SetMaterial(Material* _material)
        {
            material = _material;
        }

        Material* GetMaterial() const
        {
            return material;
        }

        virtual Vector3 GetCentriod() const = 0;

        virtual void UpdateTransformMatrix(const Matrix4x4&) = 0;

        virtual void ComputeBounds() = 0;

        Extents GetBounds()
        {
            return bounds;
        }

    protected:
        Extents bounds{};

        Material* material;
    };
}
