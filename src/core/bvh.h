#pragma once
#include "math/Vector.h"
#include <queue>
#include <string>

#include "basic/Object.h"


class Scene;
class Object;
struct Ray;
class Intersection;

namespace new_type_renderer
{
    class OctreeNode
    {
    public:
        OctreeNode();
        OctreeNode(OctreeNode* parent);
        ~OctreeNode();
        std::string name;
        int depth;
        bool isLeaf;
        OctreeNode* parent;
        OctreeNode* children[8];

        std::vector<Object*> objects;
        Vector3 boundMin;
        Vector3 boundMax;

        Extents extents;

        void addObject(Object*);

        void addObject1(Object*);

        void traverse();

        void destroyAllNodes();

        void intersectTest(Ray& r, Intersection& intersection) const;

        void intersectTestWireframe(const Ray& r, Intersection& intersection) const;

        static int maxDepth;

        Extents computeExetents();
    };

    struct HitNode
    {
        const OctreeNode* node;
        double t;

        HitNode(const OctreeNode* n, double thit)
        {
            node = n;
            t = thit;
        }

        friend bool operator<(const HitNode& a, const HitNode& b)
        {
            return a.t > b.t;
        }
    };

    struct BVH
    {
    public:
        static const Vector3 normals[SLABCOUNT];

        BVH();
        void setup(Scene&);
        void destroy();
        Intersection intersect(Ray&);
        Intersection intersectBoundingBox(const Ray&) const;
        Intersection intersectBVH(const Ray& ray) const;
        void intersectNode(Ray& ray, const OctreeNode* node, Intersection& intersection, std::priority_queue<HitNode>&);

        ~BVH();
        std::vector<Extents*> extentsList;
        OctreeNode* octree;

    public:
        Scene* scene;
    };
}
