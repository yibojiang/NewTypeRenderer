#pragma once

#define _USE_MATH_DEFINES

#include <sstream>
#include "Common.h"

namespace new_type_renderer
{
    struct Vector2
    {
        float x, y;

        Vector2(float _x = 0, float _y = 0)
        {
            x = _x;
            y = _y;
        }

        Vector2 operator+(const Vector2& b) const { return Vector2{ x + b.x, y + b.y }; }

        Vector2 operator-(const Vector2& b) const { return Vector2{ x - b.x, y - b.y }; }

        Vector2 operator*(const Vector2& b) const { return Vector2{ x * b.x, y * b.y }; }

        Vector2 operator*(const float b) const { return Vector2{ x * b, y * b }; }

        Vector2 operator/(const float b) const { return Vector2{ x / b, y / b }; }

        float Dot(const Vector2& b) const { return x * b.x + y * b.y; }

        float Length() const { return sqrt(x * x + y * y); }

        Vector2& Normalize() { return *this = *this / Length(); }

        Vector2 Normalized() const { return Vector2(*this / Length()); }

        Vector2 Reflect(const Vector2& normal) const { return normal * (Dot(normal) * 2) - *this; }

        std::string ToString() const
        {
            std::stringstream ss;
            ss << '(' << x << ',' << y << ')';
            return ss.str();
        }
    };

    struct Vector3
    {
        float x, y, z;

        Vector3(float _x = 0, float _y = 0, float _z = 0)
        {
            x = _x;
            y = _y;
            z = _z;
        }

        Vector3 operator+(const Vector3& b) const { return Vector3{ x + b.x, y + b.y, z + b.z }; }

        Vector3 operator-(const Vector3& b) const { return Vector3{ x - b.x, y - b.y, z - b.z }; }

        Vector3 operator*(const Vector3& b) const { return Vector3{ x * b.x, y * b.y, z * b.z }; }

        Vector3 operator*(const float b) const { return Vector3{ x * b, y * b, z * b }; }

        Vector3 operator/(const float b) const { return Vector3{ x / b, y / b, z / b }; }

        bool operator==(const Vector3& b) const
        {
            return ApproximatelyEqual(x, b.x) && ApproximatelyEqual(y, b.y) && ApproximatelyEqual(z, b.z);
        }

        Vector3 operator-() const { return Vector3(-x, -y, -z); }

        static Vector3 Abs(const Vector3& v)
        {
            return Vector3(fabs(v.x), fabs(v.y), fabs(v.z));
        }

        float Dot(const Vector3& b) const { return x * b.x + y * b.y + z * b.z; }

        float Length() const { return sqrt(x * x + y * y + z * z); }

        Vector3& Normalize() { return *this = *this / Length(); }

        Vector3 Normalized() const { return Vector3(*this / Length()); }

        Vector3 Cross(Vector3& b) const { return Vector3{ y * b.z - z * b.y, z * b.x - x * b.z, x * b.y - y * b.x }; }

        Vector3 Reflect(Vector3& normal) const { return *this - normal * (Dot(normal) * 2); }

        std::string ToString() const
        {
            std::stringstream ss;
            ss << '(' << x << ',' << y << ',' << z << ')';
            return ss.str();
        }

        friend std::ostream& operator<<(std::ostream& stream, const Vector3& v)
        {
            stream << '(' << v.x << ',' << v.y << ',' << v.z << ')';
            return stream;
        }

        Vector3& Lerp(Vector3& v, float t)
        {
            x = x * (1 - t) + (v.x - x) * t;
            y = y * (1 - t) + (v.y - y) * t;
            z = z * (1 - t) + (v.z - z) * t;
            return *this;
        }
    };

    struct Vector4
    {
        float x, y, z, w;

        Vector4(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f)
        {
            x = _x;
            y = _y;
            z = _z;
            w = 1.0;
        }

        Vector4(Vector3 _v, float _w)
        {
            x = _v.x;
            y = _v.y;
            z = _v.z;
            w = _w == 0.0f ? 0.0f : 1.0f;
        }

        Vector4(float _x, float _y, float _z, float _w)
        {
            x = _x;
            y = _y;
            z = _z;
            w = _w == 0.0f ? 0.0f : 1.0f;
        }

        Vector4 operator+(const Vector4& b) const { return Vector4{ x + b.x, y + b.y, z + b.z }; }

        Vector4 operator-(const Vector4& b) const { return Vector4{ x - b.x, y - b.y, z - b.z }; }

        Vector4 operator*(const float b) const { return Vector4{ x * b, y * b, z * b, w * b }; }

        Vector4 operator*(const Vector4 b) const { return Vector4{ x * b.x, y * b.y, z * b.z, w * b.w }; }

        Vector4 operator/(const float b) const { return Vector4{ x / b, y / b, z / b }; }

        bool operator==(const Vector4 b) const
        {
            return ApproximatelyEqual(x, b.x) && ApproximatelyEqual(y, b.y) && ApproximatelyEqual(z, b.z) &&
                ApproximatelyEqual(w, b.w);
        }

        float Dot(const Vector4& b) const { return x * b.x + y * b.y + z * b.z; }

        float Length() const { return sqrt(x * x + y * y + z * z); }

        Vector4& Normalize() { return *this = *this / Length(); }

        Vector4 Normalized() const { return Vector4(*this / Length()); }

        Vector4 Cross(const Vector4& b) const { return Vector4{ y * b.z - z * b.y, z * b.x - x * b.z, x * b.y - y * b.x }; }

        Vector4 Reflect(const Vector4& normal) const { return normal * (Dot(normal) * 2) - *this; }

        std::string ToString() const
        {
            std::stringstream ss;
            ss << '(' << x << ',' << y << ',' << z << ',' << w << ')';
            return ss.str();
        }
    };

    inline Vector3 Clamp(Vector3& v, Vector3& a, Vector3& b)
    {
        return Vector3(Clamp(v.x, a.x, b.x), Clamp(v.y, a.y, b.y), Clamp(v.z, a.z, b.z));
    }

    inline Vector3 Clamp(Vector3& v, Vector3 a, Vector3 b)
    {
        return Vector3(Clamp(v.x, a.x, b.x), Clamp(v.y, a.y, b.y), Clamp(v.z, a.z, b.z));
    }

    inline Vector4 Clamp(Vector4& v, Vector4 a, Vector4 b)
    {
        return Vector4(Clamp(v.x, a.x, b.x), Clamp(v.y, a.y, b.y), Clamp(v.z, a.z, b.z), Clamp(v.w, a.w, b.w));
    }
}
