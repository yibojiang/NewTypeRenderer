#pragma once

namespace new_type_renderer
{
    struct Vector3;
    struct Vector4;
    typedef Vector4 EulerRotation;
    struct Matrix4x4;

    /*
     * Quaternion detail
     * https://www.youtube.com/watch?v=MZuYmG1GBFk
     * https://www.youtube.com/watch?v=d4EgbgTm0Bg
     * https://www.youtube.com/watch?v=zjMuIxRvygQ
     */ 
    struct Quaternion
    {
    public:
        Quaternion();

        Quaternion(float qx, float qy, float qz, float qw);

        Quaternion(const Vector3& axis, const float angle);

        Quaternion(float roll, float pitch, float yaw);

        Quaternion(const Quaternion& quat);

        Quaternion operator*(const Vector3& v);

        Quaternion operator*(const Quaternion& q2);

        float Magnitude() const;

        void Conjugate();

        Quaternion Conjugated() const;

        EulerRotation ToEulerAngle() const;

        Matrix4x4 ToMatrix() const;

    public:
        float x{ 0.0f };
        float y{ 0.0f };
        float z{ 0.0f };
        float w{ 1.0f };
    };
}
