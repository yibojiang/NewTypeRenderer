#pragma once
#include <vector>

#include "basic/Mesh.h"

namespace new_type_renderer
{
    class MeshDraw
    {
    public:
        MeshDraw(const shared_ptr<Mesh>& mesh, unsigned int shaderProgram);

        void Draw();

    public:
        unsigned int m_ShaderProgram{};

        shared_ptr<Mesh> m_Mesh;

        std::vector<float> m_Vertices{};

        std::vector<unsigned int> m_Indices{};

        unsigned int m_VAO{};

        unsigned int m_VBO{};

        unsigned int m_IBO{};
    };
}

