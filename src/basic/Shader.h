#pragma once
#include <string>

#include "math/Vector.h"

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
        void SetVector3f(const string& location, const Vector3& vec);
        void SetTextureSampler(const string& location, const int sampler);
        void SetInt(const string& location, const int val);
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
