#pragma once
#include "renderer/Renderer.h"

namespace new_type_renderer
{
    class PathtracingRenderer : Renderer
    {
    public:
        void Init() override;

        void Render() override;

        void LoadScene(shared_ptr<Scene>& scene) override;

        void Update(const float elapsedTime) override;

    protected:
        int m_Width{ 640 };

        int m_Height{ 320 };

        BVH m_BVH;
    };
}
