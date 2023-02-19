#pragma once
#include <thread>

#include "basic/Image.h"
#include "renderer/Renderer.h"

namespace new_type_renderer
{
    using std::atomic;
    using std::thread;

    class PathtracingRenderer : Renderer
    {
    public:
        PathtracingRenderer(int width, int height): m_Width(width), m_Height(height), m_DirectImage(width, height){}

        void Init() override;

        void DispatchRenderTask_RenderThread();

        void Render_WorkderThread();

        void Render() override;

        void LoadScene(shared_ptr<Scene>& scene) override;

        void Update(const float elapsedTime) override;

        void SetNumThreads(const int numThread)
        {
            m_NumThreads = numThread;
        }

        bool IsRendering() const
        {
            return m_IsRendering;
        }

        float GetProgress() const
        {
            const int totalPixels = m_Width * m_Height;
            return m_Progress * 1.0f / totalPixels;
        }

        Image& GetImage();

    protected:
        int m_Width{ 256 };

        int m_Height{ 160 };

        Image m_DirectImage{ m_Width , m_Height };

    private:
        BVH m_BVH;

        atomic<int> m_Progress{ 0 };

        atomic<bool> m_IsRendering{ false };

        thread m_RenderingThread;

        int m_NumThreads{ 8 };
    };
}
