#pragma once

class Material
{
public:
    Material();
    ~Material();
    double diffuse;
    double reflect;
    double refract;
    double emission;
    double ior;
    vec3 *texture;
};