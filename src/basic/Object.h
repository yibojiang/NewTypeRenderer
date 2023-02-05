#pragma once
#include "material.h"
#include "core/Extents.h"
#include "math/Common.h"

namespace new_type_renderer
{
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


        virtual void setMaterial(Material* material)
        {
            this->material = material;
            this->material->init();
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
