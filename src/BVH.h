#pragma once
#include "vec.h"
#include <queue>
#define SLABCOUNT 3
class Scene;
class Object;
struct Ray;
class Intersection;


// class Extents;

class Extents {
public:
    double dnear[SLABCOUNT], dfar[SLABCOUNT];
    Extents();
    ~Extents();
    vec3 getCentriod() const;
    // Extents group(Extents &);
    void extendBy(Extents &);
    double intersect(const Ray &r) const;
    double intersectWireframe(const Ray &r) const;
    friend bool operator==(Extents a, const Extents& b){
        if (a.getCentriod() == b.getCentriod()){
            return true;
        }
        else{
            return false;   
        }
    }

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
    void intersectTestWireframe(const Ray &r, Intersection &intersection) const;
};

class BVH{
public:
    // static const uint8_t slabCount = SLABCOUNT;
    static const vec3 normals[SLABCOUNT];
    Scene *scene;
    BVH();
    void setup(Scene &);
    Intersection intersect(const Ray&) const;
    Intersection intersectBoundingBox(const Ray&) const;
    Intersection intersectBVH(const Ray& ray) const;
    ~BVH();
    std::vector<Extents *> extentsList;
    OctreeNode octree;


};




