#include "Transform.h"

namespace new_type_renderer
{
    Transform::Transform()
    {
    }

    Vector3 Transform::TransformLocation(const Vector3& position) const
    {
        const Vector4 result = TransformMatrix() * Vector4{position.x, position.y, position.z, 1.0f};
        return Vector3{result.x, result.y, result.z};
    }

    Vector3 Transform::TransformDirection(const Vector3& direction) const
    {
        const Vector4 result = TransformMatrix() * Vector4{direction.x, direction.y, direction.z, 0.0f};
        return Vector3{result.x, result.y, result.z};
    }

    Matrix4x4 Transform::TransformMatrix() const
    {
        const Matrix4x4 t = Matrix4x4::FromTranslate(location.x, location.y, location.z);
        const Matrix4x4 r = orientation.ToMatrix();
        const Matrix4x4 s = Matrix4x4::FromScale(scale.x, scale.y, scale.z);
        return s * t * r;
    }

    void Transform::SetLocation(const Vector3& position)
    {
        location = position;
    }

    void Transform::SetScale(const Vector3& s)
    {
        scale = s;
    }

    void Transform::SetRotation(const EulerRotation& rot)
    {
        orientation = Quaternion{ rot.x, rot.y, rot.z };
    }

    Vector3 Transform::GetLocation() const
    {
        return location;
    }

    Vector3 Transform::GetScale() const
    {
        return scale;
    }

    Quaternion Transform::GetOrientation() const
    {
        return orientation;
    }

    void Transform::SetOrientation(const Quaternion& o)
    {
        orientation = o;
    }

    EulerRotation Transform::GetRotation() const
    {
        EulerRotation rot;
        return orientation.ToEulerAngle();
    }

    void Transform::Rotate(EulerRotation rot)
    {
        Quaternion roll{Vector3(1, 0, 0), rot.x};
        Quaternion pitch{Vector3(0, 1, 0), rot.y};
        Quaternion yaw{Vector3(0, 0, 1), rot.z};
        orientation = yaw * pitch * roll * orientation;
    }
}
