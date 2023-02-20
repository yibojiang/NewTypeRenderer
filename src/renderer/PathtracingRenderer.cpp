#include "PathtracingRenderer.h"

#include <thread>

#include "utility/Profiler.h"

namespace new_type_renderer
{
    using std::thread;

    void PathtracingRenderer::DispatchRenderTask_RenderThread()
    {
        m_Progress = 0;
        LOG_INFO("Start Rendering ...");
        m_DirectImage.Empty();
        m_IsRendering = true;
        {
            PROFILE("Pathtracing");
            vector<thread> threads;
            for (int i = 0; i < m_NumThreads; i++)
            {
                threads.emplace_back(thread{ &PathtracingRenderer::Render_WorkderThread, this });
            }

            for (int i = 0; i < m_NumThreads; i++)
            {
                threads[i].join();
            }
        }
        m_IsRendering = false;
        LOG_INFO("Finish Rendering");
    }

    void PathtracingRenderer::Render_WorkderThread()
    {
        Camera& camera = m_Scene->GetCamera();
        const Vector3 foward = camera.GetForward();
        const Vector3 right = camera.GetRight();
        const Vector3 up = Vector3{ 0, 1, 0 };

        const Matrix3x3 cameraRotMatrix
        {
            right.x, up.x, foward.x,
            right.y, up.y, foward.y,
            right.z, up.z, foward.z
        };

        const float tanHalfFOV = tanf(ToRadian(camera.GetFOV() * 0.5f));
        float ratio = static_cast<float>(m_Width) / m_Height;

        const int totalPixels = m_Width * m_Height;
        while (m_Progress < totalPixels)
        {
            int y = m_Progress / m_Width;
            int x = m_Progress % m_Width;

            float u = x * 1.0 / m_Width;
            float v = (m_Height - y) * 1.0 / m_Height;
            u = (u * 2.0 - 1.0);
            v = (v * 2.0 - 1.0);
            u = u * ratio;

            Vector3 rayDir = cameraRotMatrix * Vector3{ u, v, 1.0f / tanHalfFOV };
            Ray ray(camera.GetLocation(), rayDir);
            Intersection intersection = m_BVH.Intersect(ray);
            shared_ptr<Shape> hitObj = intersection.GetHitObject().lock();
            if (hitObj != nullptr)
            {
                Color color = hitObj->GetMaterial()->GetDiffuseColor(intersection.GetUV());
                Vector3 normal = intersection.GetNormal();
                Vector3 LightDir{ 1, -2, 1 };
                LightDir.Normalize();
                float intense = Saturate(normal.Dot(-LightDir));
                m_DirectImage.SetPixel(x, y, color * intense * 255);
            }
            
            ++m_Progress;
        }
    }

    void PathtracingRenderer::Init()
    {

    }

    void PathtracingRenderer::Render()
    {
        if (m_IsRendering)
        {
            return;
        }

        // dispatch the render to render thread so it won't block the main thread
        m_RenderingThread = thread{ &PathtracingRenderer::DispatchRenderTask_RenderThread, this };
        m_RenderingThread.detach();
    }

    void PathtracingRenderer::LoadScene(shared_ptr<Scene>& scene)
    {
        // Load scene
        m_Scene = scene;

        {
            PROFILE("Build BVH");
            // Setup BVH
            m_BVH.Build(scene);
        }
    }

    void PathtracingRenderer::Update(const float elapsedTime)
    {

    }

    Image& PathtracingRenderer::GetImage()
    {
        return m_DirectImage;
    }
}
