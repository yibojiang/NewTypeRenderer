#include "Transform.h"

#include "utility/Log.h"

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
        const Matrix4x4 t = Matrix4x4::FromTranslate(m_Location.x, m_Location.y, m_Location.z);
        const Matrix4x4 r = m_Orientation.ToMatrix();
        const Matrix4x4 s = Matrix4x4::FromScale(m_Scale.x, m_Scale.y, m_Scale.z);
        return t * r * s;
    }

    void Transform::SetLocation(const Vector3& position)
    {
        m_Location = position;
    }

    void Transform::SetScale(const Vector3& s)
    {
        m_Scale = s;
    }

    void Transform::SetRotation(const EulerRotation& rot)
    {
        m_Orientation = Quaternion{ rot.x, rot.y, rot.z };
    }

    Vector3 Transform::GetLocation() const
    {
        return m_Location;
    }

    Vector3 Transform::GetScale() const
    {
        return m_Scale;
    }

    Quaternion Transform::GetOrientation() const
    {
        return m_Orientation;
    }

    void Transform::SetOrientation(const Quaternion& o)
    {
        m_Orientation = o;
    }

    EulerRotation Transform::GetRotation() const
    {
        EulerRotation rot;
        return m_Orientation.ToEulerAngle();
    }

    void Transform::Rotate(EulerRotation rot)
    {
        Quaternion roll{Vector3(1, 0, 0), rot.x};
        Quaternion pitch{Vector3(0, 1, 0), rot.y};
        Quaternion yaw{Vector3(0, 0, 1), rot.z};
        m_Orientation = yaw * pitch * roll * m_Orientation;
    }
}
