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
        // unsigned int mvpLocation = glGetUniformLocation(m_ShaderProgram, "u_MVP");
        unsigned int loc = glGetUniformLocation(m_ShaderProgram, location.c_str());
        glUniformMatrix4fv(loc, 1, false, &transposed.cols[0][0]);
    }

    void Shader::SetVector4f(const string& location, const Vector4& vec)
    {
        // unsigned int viewDirLocation = glGetUniformLocation(m_ShaderProgram, "u_ViewDir");
        unsigned int loc = glGetUniformLocation(m_ShaderProgram, location.c_str());
        glUniform4f(loc, vec.x, vec.y, vec.z, 0.0f);
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
            layout(location = 1) in vec3 normal;
            out vec4 v_Position;
            out vec3 v_Normal;
            out vec4 camPos;
            out vec3 viewDir;

            void main()
            {
                gl_Position = u_MVP * position;
                v_Position = position;
                v_Normal = normal;

                camPos = inverse(u_MVP) * gl_Position;
                viewDir = normalize(-camPos.xyz);
            }
         )glsl";

        m_VertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(m_VertexShader, 1, &vertexSource, NULL);
        glCompileShader(m_VertexShader);

        const char* fragmentSource = R"glsl(
        #version 330 core

        in vec4 camPos;
        in vec3 viewDir;
        in vec4 v_Position;
        in vec3 v_Normal;
        layout(location = 0) out vec4 outColor;

        void main()
        {
            float radiance = dot(-viewDir, v_Normal);
            outColor = vec4(radiance, radiance, radiance, 1);
            // outColor = v_Position;
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
        // if (validated != GL_TRUE) 
        // {
        //     char info_log[1024];
        //     GLsizei info_log_length;
        //     glGetProgramInfoLog(shaderProgram, 1024, &info_log_length, info_log);
        //     fprintf(stderr, "Program object validation failed: %s\n", info_log);
        //     __debugbreak();
        // }

        glLinkProgram(shaderProgram);
        glUseProgram(shaderProgram);

        glDeleteShader(m_VertexShader);
        glDeleteShader(m_FragmentShader);

        m_ShaderProgram = shaderProgram;

        m_Compiled = true;
    }
}
