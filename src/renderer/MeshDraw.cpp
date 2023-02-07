#include "MeshDraw.h"

#include "GL/glew.h"

namespace new_type_renderer
{
    MeshDraw::MeshDraw(const shared_ptr<Mesh>& mesh, unsigned int shaderProgram) : m_Mesh(mesh), m_ShaderProgram(shaderProgram)
    {
        LOG_INFO("previewing mesh %s", mesh->name.c_str());

        for (int j = 0; j < mesh->m_Positions.size(); j++)
        {
            m_Positions.push_back(mesh->m_Positions[j].x);
            m_Positions.push_back(mesh->m_Positions[j].y);
            m_Positions.push_back(mesh->m_Positions[j].z);
        }

        for (int j = 0; j < mesh->m_Indices.size(); j++)
        {
            m_Indices.push_back(mesh->m_Indices[j]);
        }

        glGenVertexArrays(1, &m_VAO);
        glBindVertexArray(m_VAO);

        glGenBuffers(1, &m_VBO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, m_Positions.size() * sizeof(float), m_Positions.data(), GL_STATIC_DRAW);

        // unsigned int vertexShader, fragmentShader = 0;
        // m_ShaderProgram = CompilerLinkShader(vertexShader, fragmentShader);

        glGenBuffers(1, &m_IBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(unsigned int), m_Indices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
        glEnableVertexAttribArray(0);

        // Unbind buffer
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void MeshDraw::Draw()
    {
        glBindVertexArray(m_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
        glDrawElements(GL_TRIANGLES, m_Indices.size(), GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
}
