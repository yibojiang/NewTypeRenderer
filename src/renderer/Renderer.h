#pragma once
#include "basic/Scene.h"

namespace new_type_renderer
{
    class Renderer
    {
    public:
        virtual void Init() = 0;

        virtual void Render() = 0;

        virtual void LoadScene(const Scene& scene) = 0;

    private:
        Scene m_Scene;
    };
}
