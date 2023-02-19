#pragma once
#include "Object.h"
#include "SceneNode.h"
#include "Transform.h"

namespace new_type_renderer
{
    class Camera
    {
    public:
        Camera();

        Vector3 GetLocation() const { return m_Location; }

        void SetLocation(const Vector3& location) { m_Location = location; }

        EulerRotation GetRotation() const { return m_Rotation; }
         
        void SetRotation(const EulerRotation& rotation) { m_Rotation = rotation; }

        Vector3 GetLookAt() { return m_LookAt; }

        Vector3 GetForward() const { return m_Forward;  }

        Vector3 GetRight() const { return m_Right; }

        Matrix4x4 GetViewMatrix() const
        {
            return Matrix4x4::CreateViewMatrix(m_Location, m_Right, m_Up, m_Forward);
        }

        float GetFOV() const
        {
            return m_FOV;
        }

    public:
        float m_FOV{ 60.f };

        float m_Near{ 0.1f };

        float m_Far{ 1000.0f };

        float m_FocalLength{ 100.0f };

        float m_FocusOn{ false };

        float m_Aperture{ 1.0f };

        float m_FocalDistance{ 1000.0f };

        Vector3 m_LookAt{};

        Vector3 m_Location{};

        EulerRotation m_Rotation{};

        float m_AngleV{};

        float m_AngleH{};

        float m_LastAngleV{};

        float m_LastAngleH{};

        Vector3 m_Up{ 0.0f, 1.0f, 0.0f };

        Vector3 m_Right{ 1.0f, 0.0f, 0.0f};

        Vector3 m_Forward{ 0.0f, 0.0f, 1.0f };
    };
}
