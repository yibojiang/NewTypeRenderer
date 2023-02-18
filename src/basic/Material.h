#pragma once

#include <memory>

#include "Shader.h"
#include "texture.h"
#include "math/Vector.h"

#define COOK_TORRANCE
namespace new_type_renderer
{
    using std::shared_ptr;

    struct Material
    {
    public:
        Material(float diffuse = 1.0f, float specular = 0.0f, int glossy = 1,
                 float reflection = 0.0f, float refract = 0.0f, float roughness = 0.0f,
                 float emission = 0.0f, float ior = 1.0f, shared_ptr<Shader> shader = nullptr)
        {
            m_Diffuse = diffuse;
            m_Specular = specular;
            m_Glossy = glossy;
            m_Reflection = reflection;
            m_Refract = refract;
            m_Roughness = roughness;
            m_Emission = emission;
            m_IOR = ior;
            m_UseBackground = false;
            m_UseDiffuseTexture = false;
            m_Shader = shader;
        }

        void Init()
        {
#ifdef COOK_TORRANCE
            float f0 = fabs((1.0 - m_IOR) / (1.0 + m_IOR));
            f0 = f0 * f0;
            m_F0 = f0 * (1 - m_Metallic) + m_Metallic;
            if (m_F0 > 1)
            {
                m_F0 = 1;
            }

            if (m_F0 < 0)
            {
                m_F0 = 0;
            }

            m_Diffuse = (1 - m_F0) * (1 - m_Metallic);
            m_Reflection = m_F0;
#endif

            if (m_Refract > 0)
            {
                m_Diffuse = 0;
                m_Reflection = 0;
            }

            float energy = m_Diffuse + m_Reflection + m_Refract;
            m_Diffuse = m_Diffuse / energy;
            m_Reflection = m_Reflection / energy;
            m_Refract = m_Refract / energy;
        }

        shared_ptr<Shader> GetShader()
        {
            return m_Shader;
        }

        void SetShader(shared_ptr<Shader> shader)
        {
            m_Shader = shader;
        }

        void SetBumpTexture(const std::string& name)
        {
            m_UseBumpTexture = true;
            m_BumpTexture.LoadImage(name);
        }

        void SetEmission(Color emissionColor)
        {
            m_EmissionColor = emissionColor * m_Emission;
        }

        void SetDiffuseTexture(const std::string& name)
        {
            m_UseDiffuseTexture = true;
            m_DiffuseTexture.LoadImage(name);
        }

        Color GetDiffuseColor(const Vector2& uv)
        {
            if (m_UseDiffuseTexture)
            {
                return m_DiffuseColor * m_DiffuseTexture.GetColor(uv);
            }

            return m_DiffuseColor;
        }

        Color GetReflectColor(const Vector2&)
        {
            return m_ReflectColor;
        }

        Color GetRefractColor(const Vector2&)
        {
            return m_RefractColor;
        }

        Color GetEmission()
        {
            return m_EmissionColor;
        }

    public:
        float m_Diffuse{ 0.0f };
        float m_Specular{ 0.0f };
        float m_Reflection{ 0.0f };
        float m_Refract{ 0.0f };

        float m_DiffuseRoughness{ 0.0f };
        float m_Roughness{ 0.0f };

        
        float m_Emission{ 0.0f };
        float m_IOR { 0.0f };
        float m_F0 { 0.0f };
        float m_Metallic { 0.0f };
        int m_Glossy{ 0 };

        Color m_ReflectColor{};
        Color m_DiffuseColor{};
        Color m_RefractColor;
        Color m_EmissionColor;

        bool m_UseBackground{ false };
        bool m_UseDiffuseTexture{ false };
        bool m_UseBumpTexture{ false };

        Texture m_DiffuseTexture{};
        Texture m_BumpTexture{};

        shared_ptr<Shader> m_Shader{};
    };
}
