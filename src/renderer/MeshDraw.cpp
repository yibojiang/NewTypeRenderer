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

            m_Vertices.push_back(mesh->m_TextureCoords[j].x);
            m_Vertices.push_back(mesh->m_TextureCoords[j].y);
        }

        for (int j = 0; j < mesh->m_Indices.size(); j++)
        {
            m_Indices.push_back(mesh->m_Indices[j]);
        }

        material->GetShader()->Bind();

        if (material->GetUseDiffuseTexture())
        {
            glGenTextures(1, &m_textureID);
            unsigned int slot = 1;
            
            glBindTexture(GL_TEXTURE_2D, m_textureID);
            const Texture& texture = material->GetDiffuseTexture();
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture.GetWidth(), texture.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.GetImage());

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        glGenVertexArrays(1, &m_VAO);
        glBindVertexArray(m_VAO);

        glGenBuffers(1, &m_VBO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(float), m_Vertices.data(), GL_STATIC_DRAW);

        glGenBuffers(1, &m_IBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(unsigned int), m_Indices.data(), GL_STATIC_DRAW);

        const int verticeStride = 8;
        unsigned int positionAttribIndex = 0;
        unsigned int normalAttribIndex = 1;
        unsigned int texCoordAttribIndex = 2;

        int positionOffset = 0 * sizeof(float);
        int normalOffset = 3 * sizeof(float);
        int texCoordOffset = 6 * sizeof(float);

        glEnableVertexAttribArray(positionAttribIndex);
        glVertexAttribPointer(positionAttribIndex, 3, GL_FLOAT, GL_FALSE, sizeof(float) * verticeStride, (void*)positionOffset);

        glEnableVertexAttribArray(normalAttribIndex);
        glVertexAttribPointer(normalAttribIndex, 3, GL_FLOAT, GL_FALSE, sizeof(float) * verticeStride, (void*)normalOffset);

        glEnableVertexAttribArray(texCoordAttribIndex);
        glVertexAttribPointer(texCoordAttribIndex, 2, GL_FLOAT, GL_FALSE, sizeof(float) * verticeStride, (void*)texCoordOffset);

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

        if (m_Material->GetUseDiffuseTexture())
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_textureID);
            m_Material->GetShader()->SetTextureSampler("u_DiffuseTexture", 0);
        }

        glDrawElements(GL_TRIANGLES, m_Indices.size(), GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}
