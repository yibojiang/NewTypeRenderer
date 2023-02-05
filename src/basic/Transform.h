#pragma once
#include "math/Matrix.h"
#include "math/Quaternion.h"

namespace new_type_renderer
{
    struct Transform
    {
    public:
        Transform();

        Vector3 TransformLocation(const Vector3& position)
        {
            Vector4 v{position.x, position.y, position.z, 1.0f};
            Matrix4x4 mat = TransformMatrix();
            Vector4 result = mat * v;
            return Vector3{result.x, result.y, result.z};
        }

        Vector3 TransformDirection(const Vector3& direction)
        {
            Vector4 v{direction.x, direction.y, direction.z, 0.0f};
            Matrix4x4 mat = TransformMatrix();
            const Vector4 result = mat * v;
            return Vector3{result.x, result.y, result.z};
        }

        Matrix4x4 TransformMatrix() const
        {
            const Matrix4x4 t = Matrix4x4::FromTranslate(location.x, location.y, location.z);
            const Matrix4x4 r = orientation.ToMatrix();
            const Matrix4x4 s = Matrix4x4::FromScale(scale.x, scale.y, scale.z);
            return s * t * r;
        }

        void SetLocation(const Vector3& position)
        {
            location = position;
        }

        void SetScale(const Vector3& s)
        {
            scale = s;
        }

        void SetRotation(const Quaternion& o)
        {
            orientation = o;
        }

        Vector3 GetLocation() const
        {
            return location;
        }

        Vector3 GetScale() const
        {
            return scale;
        }

        Quaternion GetOrientation() const
        {
            return orientation;
        }

        EulerRotation GetRotation() const
        {
            EulerRotation rot;
            return orientation.ToEulerAngle();
        }

        void Rotate(EulerRotation rot)
        {
            Quaternion roll{ Vector3(1, 0, 0), rot.x };
            Quaternion pitch{ Vector3(0, 1, 0), rot.y };
            Quaternion yaw{ Vector3(0, 0, 1), rot.z };
            orientation = yaw * pitch * roll * orientation;
        }

    public:
        Quaternion orientation { 0.0f, 0.0f, 0.f ,1.0f };

        Vector3 location{ 0.0f, 0.0f, 0.0f };

        Vector3 scale{ 1.0f, 1.0f, 1.0f };
    };
}
