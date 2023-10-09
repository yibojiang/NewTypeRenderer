#include "Shader.h"

#include "GL/glew.h"
#include "math/Matrix.h"

namespace new_type_renderer
{
    Shader::Shader()
    {
    }

    Shader::~Shader()
    {
        glDeleteProgram(m_ShaderProgram);
    }

    void Shader::SetMatrix4f(const string& location, const Matrix4x4& mat)
    {
        Matrix4x4 transposed = mat.Transposed();
        unsigned int loc = glGetUniformLocation(m_ShaderProgram, location.c_str());
        glUniformMatrix4fv(loc, 1, false, &transposed.cols[0][0]);
    }

    void Shader::SetVector4f(const string& location, const Vector4& vec)
    {
        unsigned int loc = glGetUniformLocation(m_ShaderProgram, location.c_str());
        glUniform4f(loc, vec.x, vec.y, vec.z, 0.0f);
    }

    void Shader::SetVector3f(const string& location, const Vector3& vec)
    {
        unsigned int loc = glGetUniformLocation(m_ShaderProgram, location.c_str());
        glUniform3f(loc, vec.x, vec.y, vec.z);
    }

    void Shader::SetTextureSampler(const string& location, const int sampler)
    {
        unsigned int loc = glGetUniformLocation(m_ShaderProgram, location.c_str());
        glUniform1i(loc, sampler);
    }

    void Shader::Bind()
    {
        glUseProgram(m_ShaderProgram);
    }

    void Shader::UnBind()
    {
        glUseProgram(0);
    }

    void Shader::CompilerLinkShader()
    {
        if (m_Compiled)
        {
            return;
        }

        // Compile shader
        const char* vertexSource = R"glsl(
            #version 330 core

            uniform mat4 u_MVP;
            layout(location = 0) in vec4 position;
            layout(location = 1) in vec4 normal;
            layout(location = 2) in vec2 textureCoords;
            out vec4 v_Position;
            out vec3 v_Normal;
            out vec3 viewDir;
            out vec2 v_TextureCoords;

            void main()
            {
                gl_Position = u_MVP * position;
                v_Position = position;
                v_Normal = normalize(normal.xyz);
                v_TextureCoords = textureCoords;
            }
         )glsl";

        m_VertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(m_VertexShader, 1, &vertexSource, NULL);
        glCompileShader(m_VertexShader);

        const char* fragmentSource = R"glsl(
        #version 330 core

        in vec4 v_Position;
        in vec3 v_Normal;
        in vec2 v_TextureCoords;
        layout(location = 0) out vec4 outColor;
        uniform vec3 u_ViewDir;
        uniform sampler2D u_DiffuseTexture;

        void main()
        {
            float radiance = dot(-u_ViewDir, v_Normal);
            vec4 rad = vec4(radiance, radiance, radiance, 1);
            if (v_TextureCoords.x >= 0.0f && v_TextureCoords.x <= 1.0f && v_TextureCoords.y >= 0.0f && v_TextureCoords.y <= 1.0f)
            {
                outColor = texture(u_DiffuseTexture, v_TextureCoords) * rad;
            }
            else
            {
                outColor = rad;
            }

            //vec4 texColor = texture(u_DiffuseTexture, v_TextureCoords);
            //outColor = texColor * vec4(radiance, radiance, radiance, 1);
            //outColor = vec4(v_TextureCoords.x, v_TextureCoords.y, 1, 1);
        }
        )glsl";

        m_FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(m_FragmentShader, 1, &fragmentSource, NULL);
        glCompileShader(m_FragmentShader);

        unsigned int shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, m_VertexShader);
        glAttachShader(shaderProgram, m_FragmentShader);

        glValidateProgram(shaderProgram);
        GLint validated;
        glGetProgramiv(shaderProgram, GL_VALIDATE_STATUS, &validated);

        glLinkProgram(shaderProgram);
        glUseProgram(shaderProgram);

        glDeleteShader(m_VertexShader);
        glDeleteShader(m_FragmentShader);

        m_ShaderProgram = shaderProgram;

        m_Compiled = true;
    }
}
