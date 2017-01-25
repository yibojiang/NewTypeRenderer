#pragma once

#include "texture.h"
class Material
{
public:
    Material(const float& diffuse=1, const float& reflection=0, const float& refract=0, float roughness=0,
        float emission=0, float ior=1){
        this->diffuse = diffuse;
        this->reflection = reflection;
        this->refract = refract;
        this->roughness = roughness;
        this->emission = emission;
        this->ior = ior;
        this->energy = this->diffuse + this->reflection + this->refract;
        this->useDiffuseTexture = false;
    }
    ~Material(){}

    float diffuse;
    float reflection;
    float refract;

    float diffuseRoughness;
    float roughness;
    float emission;
    float ior;
    float energy;


    vec3 diffuseColor;
    vec3 reflectColor;
    vec3 refractColor;
    vec3 emissionColor;
    bool useDiffuseTexture;
    bool useBumpTexture;

    Texture diffuseTexture;
    Texture bumpTexture;

    void setBumpTexture(const std::string& name){
        useBumpTexture = true;
        bumpTexture.loadImage(name);
    }

    void setEmission(vec3 emissionColor){
        this->emissionColor = emissionColor * this->emission;
    }

    void setDiffuseTexture(const std::string& name){
        useDiffuseTexture = true;
        qDebug() << name.c_str();
        diffuseTexture.loadImage(name);
    }

    vec3 getDiffuseColor(const vec2& uv){
        if (useDiffuseTexture){
            return this->diffuseColor * diffuseTexture.getColor3(uv);
        }
      
        return diffuseColor;
    }

    vec3 getReflectColor(const vec2&){
        return reflectColor;
    }

    vec3 getRefractColor(const vec2&){
        return refractColor;
    }

    vec3 getEmission(){
        return  emissionColor;
    }
};