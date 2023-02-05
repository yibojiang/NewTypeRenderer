#pragma once

#include "texture.h"
#include "math/Vector.h"
#define COOK_TORRANCE

namespace new_type_renderer
{
    struct Material
    {
    public:
        Material(float in_diffuse = 1.0f, float in_specular = 0.0f, int in_glossy = 1,
                 float in_reflection = 0.0f, float in_refract = 0.0f, float in_roughness = 0.0f,
                 float in_emission = 0.0f, float in_ior = 1.0f)
        {
            diffuse = in_diffuse;
            specular = in_specular;
            glossy = in_glossy;
            reflection = in_reflection;
            refract = in_refract;
            roughness = in_roughness;
            emission = in_emission;
            ior = in_ior;
            useBackground = false;
            useDiffuseTexture = false;
        }

        void init()
        {
#ifdef COOK_TORRANCE
            float f0 = fabs((1.0 - this->ior) / (1.0 + this->ior));
            f0 = f0 * f0;
            this->F0 = f0 * (1 - this->metallic) + this->metallic;
            if (this->F0 > 1)
            {
                this->F0 = 1;
            }

            if (this->F0 < 0)
            {
                this->F0 = 0;
            }

            this->diffuse = (1 - this->F0) * (1 - metallic);
            this->reflection = this->F0;

#endif

            if (this->refract > 0)
            {
                this->diffuse = 0;
                this->reflection = 0;
            }

            float energy = this->diffuse + this->reflection + this->refract;
            this->diffuse = this->diffuse / energy;
            this->reflection = this->reflection / energy;
            this->refract = this->refract / energy;
        }

        void setBumpTexture(const std::string& name)
        {
            useBumpTexture = true;
            bumpTexture.LoadImage(name);
        }

        void setEmission(Color emissionColor)
        {
            this->emissionColor = emissionColor * this->emission;
        }

        void setDiffuseTexture(const std::string& name)
        {
            useDiffuseTexture = true;
            diffuseTexture.LoadImage(name);
        }

        Color getDiffuseColor(const Vector2& uv)
        {
            if (useDiffuseTexture)
            {
                return diffuseColor * diffuseTexture.GetColor(uv);
            }

            return diffuseColor;
        }

        Color getReflectColor(const Vector2&)
        {
            return reflectColor;
        }

        Color getRefractColor(const Vector2&)
        {
            return refractColor;
        }

        Color getEmission()
        {
            return emissionColor;
        }

    public:
        float diffuse{ 0.0f };
        float specular{ 0.0f };
        float reflection{ 0.0f };
        float refract{ 0.0f };

        float diffuseRoughness{ 0.0f };
        float roughness{ 0.0f };

        
        float emission{ 0.0f };
        float ior { 0.0f };
        float F0 { 0.0f };
        float metallic { 0.0f };
        int glossy{ 0 };

        Color reflectColor{};
        Color diffuseColor{};
        Color refractColor;
        Color emissionColor;

        bool useBackground{ false };
        bool useDiffuseTexture{ false };
        bool useBumpTexture{ false };

        Texture diffuseTexture{};
        Texture bumpTexture{};
    };
}
