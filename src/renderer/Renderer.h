#pragma once
#include "basic/Scene.h"

namespace new_type_renderer
{
    class Renderer
    {
    public:
        virtual void Init() = 0;

        virtual void Render() = 0;

        virtual void LoadScene(shared_ptr<Scene>& scene) = 0;

        virtual void Update(const float elapsedTime) = 0;

    protected:
        shared_ptr<Scene> m_Scene;
    };
}
