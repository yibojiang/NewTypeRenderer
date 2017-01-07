#pragma once
#include "vec.h"

class Scene;
class Object;
struct Ray;
class Intersection;
class Extents;



class OctreeNode{
public:
    OctreeNode();
    OctreeNode(OctreeNode*);
    ~OctreeNode();
    OctreeNode *parent;
    OctreeNode *children[8];
    Object *object;
    Extents *extents;
    void addObject(Object *);
    void Traverse();
};

class BVH{
public:
    static const uint8_t slabCount = 7;
    static const vec3 normals[slabCount];
    Scene *scene;
    BVH();
    void setup(Scene &);
    Intersection intersect(const Ray&) const;
    ~BVH();
    std::vector<Extents *> extentsList;
    OctreeNode octree;
};

class Extents {
public:
    double dnear[BVH::slabCount], dfar[BVH::slabCount];
    Extents();
    ~Extents();
    vec3 getCentriod();
    // Extents group(Extents &);
    void extendBy(Extents &);
    double intersect(const Ray &r);
};


