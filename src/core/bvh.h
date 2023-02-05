#pragma once
#include "math/Vector.h"
#include <queue>
#include <string>

#include "OctreeNode.h"

namespace new_type_renderer
{
    struct Object;
    struct Ray;
    struct Intersection;
    class Scene;

    struct BVH
    {
    public:
        static const Vector3 normals[SLABCOUNT];

        BVH();

        ~BVH();

        void Setup(Scene& scene);

        void Destroy();

        Intersection Intersect(Ray&);

        Intersection IntersectBoundingBox(const Ray&) const;

        Intersection IntersectBVH(const Ray& ray) const;

        void IntersectNode(Ray& ray, const OctreeNode* node, Intersection& intersection, std::priority_queue<HitNode>&);
        
    public:
        Scene* scene;

        std::vector<Extents*> extents_list;

        OctreeNode* octree;
    };
}
