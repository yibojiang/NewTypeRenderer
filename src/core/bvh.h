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

        void Setup(const shared_ptr<Scene>& scene);

        void Destroy();

        Intersection Intersect(Ray&) const;

        Intersection IntersectBoundingBox(const Ray&) const;

        Intersection IntersectBVH(const Ray& ray) const;

        void IntersectNode(Ray& ray, const shared_ptr<OctreeNode>& node, Intersection& intersection, std::priority_queue<HitNode>&) const;
        
    public:
        shared_ptr <Scene> m_Scene;

        std::vector<Extents*> m_Extents;

        shared_ptr<OctreeNode> m_Octree;
    };
}
