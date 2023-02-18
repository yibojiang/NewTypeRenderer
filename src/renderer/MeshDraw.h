#pragma once
#include <vector>

#include "basic/Mesh.h"

namespace new_type_renderer
{
    using std::vector;

    class MeshDraw
    {
    public:
        MeshDraw(const shared_ptr<Mesh>& mesh, const shared_ptr<Material>& material);

        shared_ptr<Mesh> GetMesh() { return m_Mesh; }

        int GetVertexCount() const;

        void Draw(Matrix4x4& mvp);

    public:
        shared_ptr<Mesh> m_Mesh{};

        shared_ptr<Material> m_Material{};

        vector<float> m_Vertices{};

        vector<unsigned int> m_Indices{};

        unsigned int m_VAO{};

        unsigned int m_VBO{};

        unsigned int m_IBO{};
    };
}

