#pragma once
#include "math/Vector.h"
#include <queue>
#include <string>

#include "OctreeNode.h"

namespace new_type_renderer
{
    struct Shape;
    struct Ray;
    struct Intersection;
    class Scene;

    struct BVH
    {
    public:
        BVH();

        void Build(const shared_ptr<Scene>& scene);

        void Destroy();

        Intersection Intersect(Ray&) const;

        Intersection IntersectBoundingBox(const Ray&) const;

        Intersection IntersectBVH(const Ray& ray) const;

        void IntersectNode(Ray& ray, const shared_ptr<OctreeNode>& node, Intersection& intersection, std::priority_queue<HitNode>&) const;

        void SetDirt(const bool isDirted = true)
        {
            m_Dirted = isDirted;
        }
        
    public:
        shared_ptr <Scene> m_Scene;

        /*
         * Octree is used to used to construct the BVH by combining the mesh nodes
         */
        shared_ptr<OctreeNode> m_Octree;

        bool m_Dirted{ true };
    };
}
