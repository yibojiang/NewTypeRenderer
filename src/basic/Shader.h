#pragma once
#include <string>

namespace new_type_renderer
{
    struct Vector4;
    struct Matrix4x4;
    using std::string;

    class Shader
    {
    public:
        Shader();

        ~Shader();

        void SetMatrix4f(const string& location, const Matrix4x4& mat);

        void SetVector4f(const string& location, const Vector4& vec);
        void Bind();

        void UnBind();
        void CompilerLinkShader();

    private:
        bool m_Compiled{ false };

        unsigned int m_ShaderProgram{ 0 };

        unsigned int m_VertexShader = 0;

        unsigned int m_FragmentShader = 0;
    };
}
