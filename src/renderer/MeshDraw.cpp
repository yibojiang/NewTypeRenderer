#include "MeshDraw.h"

#include "GL/glew.h"

namespace new_type_renderer
{
    MeshDraw::MeshDraw(const shared_ptr<Mesh>& mesh, const shared_ptr<Material>& material) : m_Mesh(mesh), m_Material(material)
    {
        LOG_INFO("previewing mesh %s", mesh->name.c_str());
        if (material != nullptr)
        {
            material->GetShader()->CompilerLinkShader();
        }

        for (int j = 0; j < mesh->m_Positions.size(); j++)
        {
            m_Vertices.push_back(mesh->m_Positions[j].x);
            m_Vertices.push_back(mesh->m_Positions[j].y);
            m_Vertices.push_back(mesh->m_Positions[j].z);

            m_Vertices.push_back(mesh->m_VertexNormals[j].x);
            m_Vertices.push_back(mesh->m_VertexNormals[j].y);
            m_Vertices.push_back(mesh->m_VertexNormals[j].z);
        }

        for (int j = 0; j < mesh->m_Indices.size(); j++)
        {
            m_Indices.push_back(mesh->m_Indices[j]);
        }

        material->GetShader()->Bind();

        glGenVertexArrays(1, &m_VAO);
        glBindVertexArray(m_VAO);

        glGenBuffers(1, &m_VBO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(float), m_Vertices.data(), GL_STATIC_DRAW);

        glGenBuffers(1, &m_IBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(unsigned int), m_Indices.data(), GL_STATIC_DRAW);

        const int verticeStride = 6;
        unsigned int positionAttribIndex = 0;
        unsigned int normalAttribIndex = 1;

        int positionOffset = 0 * sizeof(float);
        int normalOffset = 3 * sizeof(float);

        glEnableVertexAttribArray(positionAttribIndex);
        glVertexAttribPointer(positionAttribIndex, 3, GL_FLOAT, GL_FALSE, sizeof(float) * verticeStride, (void*)positionOffset);

        glEnableVertexAttribArray(normalAttribIndex);
        glVertexAttribPointer(normalAttribIndex, 3, GL_FLOAT, GL_FALSE, sizeof(float) * verticeStride, (void*)normalOffset);

        // Unbind buffer
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    int MeshDraw::GetVertexCount() const
    {
        return m_Vertices.size();
    }

    void MeshDraw::Draw(Matrix4x4& mvp, Vector3& viewDir)
    {
        if (m_Material && m_Material->m_Shader)
        {
            m_Material->m_Shader->SetMatrix4f("u_MVP", mvp);
            m_Material->m_Shader->SetVector3f("u_ViewDir", viewDir);
        }
        
        glBindVertexArray(m_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
        glDrawElements(GL_TRIANGLES, m_Indices.size(), GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
}
