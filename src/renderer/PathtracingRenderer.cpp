#include "PathtracingRenderer.h"

namespace new_type_renderer
{
    void PathtracingRenderer::Init()
    {

    }

    void PathtracingRenderer::Render()
    {

    }

    void PathtracingRenderer::LoadScene(shared_ptr<Scene>& scene)
    {
        // Load scene
        m_Scene = scene;

        // Setup BVH
        m_BVH.Build(scene);
    }

    void PathtracingRenderer::Update()
    {

    }
}
