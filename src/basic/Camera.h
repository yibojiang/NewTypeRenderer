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

    public:
        float m_FOV{ 60.f };

        float m_Near{ 0.1f };

        float m_FocalLength{ 100.0f };

        float m_FocusOn{ false };

        float m_Aperture{ 1.0f };

        float m_FocalDistance{ 1000.0f };

        Matrix4x4 m_CameraMatrix;
    };
}
