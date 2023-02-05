#pragma once
#include "math/Matrix.h"
#include "math/Quaternion.h"

namespace new_type_renderer
{
    struct Transform
    {
    public:
        Transform();

        Vector3 TransformLocation(const Vector3& position) const;

        Vector3 TransformDirection(const Vector3& direction) const;

        Matrix4x4 TransformMatrix() const;

        void SetLocation(const Vector3& position);

        void SetScale(const Vector3& s);

        void SetRotation(const Quaternion& o);

        Vector3 GetLocation() const;

        Vector3 GetScale() const;

        Quaternion GetOrientation() const;

        void SetOrientation(const Quaternion& o);

        EulerRotation GetRotation() const;

        void Rotate(EulerRotation rot);

    public:
        Quaternion orientation{0.0f, 0.0f, 0.f, 1.0f};

        Vector3 location{0.0f, 0.0f, 0.0f};

        Vector3 scale{1.0f, 1.0f, 1.0f};
    };
}
