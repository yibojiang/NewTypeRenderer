#pragma once

class Material
{
public:
    Material();
    ~Material();
    double diffuse;
    double reflect;
    double refract;
    double emit;
    double ior;
};