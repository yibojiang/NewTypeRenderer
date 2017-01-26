#pragma once

#include "texture.h"
class Material
{
public:
    Material(const float& diffuse=1, const float& specular=0, const float& reflection=0, const float& refract=0, float roughness=0,
        float emission=0, float ior=1){
        this->diffuse = diffuse;
        this->reflection = reflection;
        this->refract = refract;
        this->roughness = roughness;
        this->emission = emission;
        this->ior = ior;
        this->specular = specular;
        
        // this->energy = 1;
        // float energy = this->diffuse + this->reflection + this->refract;
        // this->diffuse = this->diffuse/energy;
        // this->reflection = this->reflection/energy;
        // this->refract = this->refract/energy;
        this->useDiffuseTexture = false;

        
        // this->F0 = vec3(1, 1, 1);
        // qDebug(this-F0);


    }

    void init(){
        float energy = this->diffuse + this->reflection + this->refract;
        this->diffuse = this->diffuse/energy;
        this->reflection = this->reflection/energy;
        this->refract = this->refract/energy;

        float f0 = fabs ((1.0 - this->ior) / (1.0 + this->ior));
        f0 = f0 * f0;
        
        this->F0 = vec3(f0, f0, f0);
        this->F0.lerp(reflectColor, this->metallic);
        // qDebug() << "f0" << this->F0;
        // qDebug() << "metallic" << this->metallic;
        
    }

    ~Material(){}

    float diffuse;
    float specular;
    float reflection;
    float refract;

    float diffuseRoughness;
    float roughness;
        
    vec3 reflectColor;
    float emission;
    float ior;
    // float energy;
    vec3 F0;
    float metallic;


    vec3 diffuseColor;
    
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