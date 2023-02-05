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
        bool isMesh;

        Object()
        {
        }

        virtual ~Object()
        {
        }

        virtual Vector3 getNormal(const Vector3&) const { return Vector3(1); }

        virtual double intersect(Ray&) { return 0; }

        virtual double getProjectAngleToSphere()
        {
            return M_PI;
        }


        void setMaterial(Material* in_material)
        {
            material = in_material;
        }

        Material* getMaterial()
        {
            return this->material;
        }

        virtual Vector3 getCentriod() const { return Vector3(); }


        virtual void updateTransformMatrix(const Matrix4x4&)
        {
        }

        virtual void computebounds()
        {
        }

        Extents getBounds()
        {
            return bounds;
        }

    protected:
        Extents bounds{};

        Material* material;
    };

    
}
