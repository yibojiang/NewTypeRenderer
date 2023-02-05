#pragma once
#include "Object.h"

namespace new_type_renderer
{
    class Camera : public Object
    {
    public:
        Camera();

        Vector3 GetCentriod() const override;

        void UpdateTransformMatrix(const Matrix4x4&) override;

        void ComputeBounds() override;

    public:
        float fov{ 60.f };

        float near{ 0.1f };

        float focal_length{ 100.0f };

        float focus_on{ false };

        float aperture{ 1.0f };

        float focal_distance{ 1000.0f };
    };
}
