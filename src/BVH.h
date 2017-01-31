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
    vec3 getBoundMin() const;
    vec3 getBoundMax() const;
    void extendBy(Extents &);
    double intersect(const Ray &r) const;
    double intersectNear(const Ray &r) const;
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
    OctreeNode(OctreeNode *parent);
    ~OctreeNode();
    std::string name;
    int depth;
    bool isLeaf;
    OctreeNode *parent;
    OctreeNode *children[8];
    
    std::vector<Object*> objects;
    vec3 boundMin;
    vec3 boundMax;

    // Object *object;
    Extents extents;
    
    void addObject(Object *);
    void addObject1(Object *);
    void traverse();

    void destroyAllNodes();


    void intersectTest(Ray &r, Intersection &intersection) const;
    void intersectTestWireframe(const Ray &r, Intersection &intersection) const;
    static int maxDepth;

    Extents computeExetents();
};

struct HitNode
{
    const OctreeNode *node;
    double t;
    HitNode(const OctreeNode *n, double thit){
        node = n;
        t = thit;
    }
    
    friend bool operator<(const HitNode &a, const HitNode &b){
        return a.t > b.t;
    }
};

class BVH{
public:
    // static const uint8_t slabCount = SLABCOUNT;
    static const vec3 normals[SLABCOUNT];
    Scene *scene;
    bool logOn;

    
    BVH();
    void setup(Scene &);
    void destroy();
    Intersection intersect(Ray&);
    Intersection intersectBoundingBox(const Ray&) const;
    Intersection intersectBVH(const Ray& ray) const;
    void intersectNode(Ray& ray, const OctreeNode *node, Intersection &intersection, std::priority_queue<HitNode> &);

    ~BVH();
    std::vector<Extents *> extentsList;
    OctreeNode *octree;


};




