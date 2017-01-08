#pragma once
#include "vec.h"
#include <queue>
class Scene;
class Object;
struct Ray;
class Intersection;
// class Extents;

class Extents {
public:
    double dnear[7], dfar[7];
    Extents();
    ~Extents();
    vec3 getCentriod();
    // Extents group(Extents &);
    void extendBy(Extents &);
    double intersect(const Ray &r)const;

};

class OctreeNode{
public:
    OctreeNode();
    OctreeNode(OctreeNode*);
    ~OctreeNode();
    int depth;
    bool isLeaf;
    OctreeNode *parent;
    OctreeNode *children[8];
    Object *object;
    Extents extents;
    void addObject(Object *);
    void traverse();


    void intersectTest(const Ray &r, Intersection &intersection) const;
};

class BVH{
public:
    static const uint8_t slabCount = 7;
    static const vec3 normals[slabCount];
    Scene *scene;
    BVH();
    void setup(Scene &);
    Intersection intersect(const Ray&) const;
    Intersection intersectBoundingBox(const Ray&) const;
    
    ~BVH();
    std::vector<Extents *> extentsList;
    OctreeNode octree;


};




