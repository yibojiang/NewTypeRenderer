#pragma once
#include "vec.h"

class Scene;
class Object;
struct Ray;
class Intersection;


class BVH{
public:
    static const uint8_t slabCount = 7;
    static const vec3 normals[slabCount];
    Scene *scene;
    BVH();
    void setup(Scene &);
    Intersection intersect(const Ray&) const;
    ~BVH();
    
};

class Extents {
public:
    double dnear[BVH::slabCount], dfar[BVH::slabCount];
    Extents();

    double intersect(const Ray &r);
};