#pragma once

#include "texture.h"
class Material
{
public:
    Material(const float& diffuse=1, const float& specular=0, const float& refract=0, float roughness=0,
        float emission=0, float ior=1){

        this->diffuse = diffuse;
        this->specular = specular;
        this->refract = refract;
        this->roughness = roughness;
        this->emission = emission;
        this->ior = ior;
        this->energy = this->diffuse + this->specular + this->refract;
        this->useDiffuseTexture = false;
    }
    ~Material(){}

    float diffuse;
    float specular;
    float refract;

    float diffuseRoughness;
    float roughness;
    float emission;
    float ior;
    float energy;


    vec3 diffuseColor;
    vec3 reflectColor;
    vec3 emissionColor;
    bool useDiffuseTexture;

    Texture diffuseTexture;
    Texture bumpTexture;

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
            return diffuseTexture.getColor3(uv);
        }
      
        return diffuseColor;
    }

    vec3 getEmission(){
        return  emissionColor;
    }
};