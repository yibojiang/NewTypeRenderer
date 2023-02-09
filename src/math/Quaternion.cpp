#include "Quaternion.h"

#include "Matrix.h"

namespace new_type_renderer
{
    Quaternion::Quaternion()
    {
    }

    Quaternion::Quaternion(float qx, float qy, float qz, float qw): x(qx), y(qy), z(qz), w(qw)
    {
    }

    Quaternion::Quaternion(const Vector3& axis, const float angle)
    {
        const Vector3 normalized_axis = axis.Normalized();
        const float half_angle = angle / 2.0f;
        x = normalized_axis.x * sin(half_angle);
        y = normalized_axis.y * sin(half_angle);
        z = normalized_axis.z * sin(half_angle);
        w = cos(half_angle);
    }

    Quaternion::Quaternion(float roll, float pitch, float yaw)
    {
        const float t0 = cos(yaw * 0.5f);
        const float t1 = sin(yaw * 0.5f);
        const float t2 = cos(roll * 0.5f);
        const float t3 = sin(roll * 0.5f);
        const float t4 = cos(pitch * 0.5f);
        const float t5 = sin(pitch * 0.5f);
        w = t0 * t2 * t4 + t1 * t3 * t5;
        x = t0 * t3 * t4 - t1 * t2 * t5;
        y = t0 * t2 * t5 + t1 * t3 * t4;
        z = t1 * t2 * t4 - t0 * t3 * t5;
    }

    Quaternion::Quaternion(const Quaternion& quat)
    {
        x = quat.x;
        y = quat.y;
        z = quat.z;
        w = quat.w;
    }

    Quaternion Quaternion::operator*(const Vector3& v)
    {
        Quaternion qr;
        qr.w =  - (x * v.x) - (y * v.y) - (z * v.z);
        qr.x =    (w * v.x) + (y * v.z) - (z * v.y);
        qr.y =    (w * v.y) + (z * v.x) - (x * v.z);
        qr.z =    (w * v.z) + (x * v.y) - (y * v.x);
        return qr;
    }

    Quaternion Quaternion::operator*(const Quaternion& q2)
    {
        Quaternion qr;
        qr.w = (w * q2.w) - (x * q2.x) - (y * q2.y) - (z * q2.z);
        qr.x = (x * q2.w) + (w * q2.x) + (y * q2.z) - (z * q2.y);
        qr.y = (y * q2.w) + (w * q2.y) + (z * q2.x) - (x * q2.z);
        qr.z = (z * q2.w) + (w * q2.z) + (x * q2.y) - (y * q2.x);
        return qr;
    }

    float Quaternion::Magnitude() const
    {
        return sqrt(w * w + x * x + y * y + z * z);
    }

    void Quaternion::Conjugate()
    {
        x = -x;
        y = -y;
        z = -z;
    }

    Quaternion Quaternion::Conjugated() const
    {
        return { -x, -y, -z, w };
    }

    EulerRotation Quaternion::ToEulerAngle() const
    {
        EulerRotation rot;
        const float ysqr = y * y;

        // roll (x-axis rotation)
        const float t0 = +2.0f * (w * x + y * z);
        const float t1 = +1.0f - 2.0f * (x * x + ysqr);
        rot.x = atan2(t0, t1);

        // pitch (y-axis rotation)
        float t2 = +2.0f * (w * y - z * x);
        t2 = t2 > 1.0f ? 1.0f : t2;
        t2 = t2 < -1.0f ? -1.0f : t2;
        rot.y = asin(t2);

        // yaw (z-axis rotation)
        float t3 = +2.0f * (w * z + x * y);
        float t4 = +1.0f - 2.0f * (ysqr + z * z);
        rot.z = atan2(t3, t4);

        return rot;
    }

    Matrix4x4 Quaternion::ToMatrix() const
    {
        return Matrix4x4
        {
            1 - 2 * y * y - 2 * z * z, 2 * x * y - 2 * z * w, 2 * x * z + 2 * y * w, 0,
            2 * x * y + 2 * z * w, 1 - 2 * x * x - 2 * z * z, 2 * y * z - 2 * x * w, 0,
            2 * x * z - 2 * y * w, 2 * y * z + 2 * x * w, 1 - 2 * x * x - 2 * y * y, 0,
            0, 0, 0, 1
        };
    }
}
