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
        BVH();

        ~BVH();

        void Setup(Scene& scene);

        void Destroy();

        Intersection Intersect(Ray&) const;

        Intersection IntersectBoundingBox(const Ray&) const;

        Intersection IntersectBVH(const Ray& ray) const;

        void IntersectNode(Ray& ray, const OctreeNode* node, Intersection& intersection, std::priority_queue<HitNode>&) const;
        
    public:
        Scene* m_Scene;

        std::vector<Extents*> m_Extents;

        OctreeNode* m_Octree;
    };
}
