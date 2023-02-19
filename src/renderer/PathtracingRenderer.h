#pragma once
#include "basic/Image.h"
#include "renderer/Renderer.h"

namespace new_type_renderer
{
    class PathtracingRenderer : Renderer
    {
    public:
        PathtracingRenderer(int width, int height, int numThread = 8): m_Width(width), m_Height(height), m_NumThread(numThread), m_DirectImage(width, height){}

        void Init() override;

        void RenderWorkderThread();

        void Render() override;

        void LoadScene(shared_ptr<Scene>& scene) override;

        void Update(const float elapsedTime) override;

    protected:
        int m_Width{ 256 };

        int m_Height{ 160 };

        Image m_DirectImage{ m_Width , m_Height };

        

    private:
        BVH m_BVH;

        int m_NumThread{ 8 };

        std::atomic<int> m_Progress{ 0 };
    };
}
