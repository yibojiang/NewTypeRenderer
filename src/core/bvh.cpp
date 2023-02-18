#include "BVH.h"
#include "OctreeNode.h"
#include "basic/Scene.h"

namespace new_type_renderer
{
    BVH::BVH()
    {
    }

    void BVH::Build(const shared_ptr<Scene>& scene)
    {
        this->m_Scene = scene;

        // Compute the bounding extent for the whole scene for octree
        Extents sceneExtents;
        for (uint32_t i = 0; i < scene->m_Objects.size(); ++i)
        {
            scene->m_Objects[i]->ComputeBounds();
            Extents e = scene->m_Objects[i]->GetBounds();
            sceneExtents.ExtendBy(e);
        }

        m_Octree = make_shared<OctreeNode>(sceneExtents.GetBoundMin(), sceneExtents.GetBoundMax());
        m_Octree->m_IsLeaf = false;

        // Construct bvh hierarchy.
        for (uint32_t i = 0; i < scene->m_Objects.size(); ++i)
        {
            m_Octree->AddObject(scene->m_Objects[i]);
        }

        m_Octree->ComputeExetents();
    }

    void BVH::Destroy()
    {
        m_Octree->DestroyAllNodes();
    }    

    Intersection BVH::IntersectBoundingBox(const Ray& ray) const
    {
        Intersection closestIntersection;
        for (uint8_t i = 0; i < m_Scene->m_Objects.size(); ++i)
        {
            float t = m_Scene->m_Objects[i]->GetBounds().IntersectWireframe(ray);
            if (t > FLT_EPSILON && t < closestIntersection.t)
            {
                closestIntersection.t = t;
                closestIntersection.object = m_Scene->m_Objects[i];
            }
        }
        return closestIntersection;
    }

    Intersection BVH::IntersectBVH(const Ray& ray) const
    {
        Intersection closestIntersection;
        m_Octree->IntersectTestWireframe(ray, closestIntersection);
        return closestIntersection;
    }


    void BVH::IntersectNode(Ray& r, const shared_ptr<OctreeNode>& node, Intersection& intersection,
                            std::priority_queue<HitNode>& hitNodes) const
    {
        if (node->m_IsLeaf)
        {
            for (unsigned int i = 0; i < node->m_Objects.size(); ++i)
            {
                double t = node->m_Objects[i]->Intersect(r);

                if (t > FLT_EPSILON && t < intersection.t)
                {
                    t = node->m_Objects[i]->Intersect(r);
                    intersection.object = node->m_Objects[i];
                    intersection.t = t;
                }
            }
        }
        else
        {
            for (int i = 0; i < 8; ++i)
            {
                if (node->m_Children[i])
                {
                    float test = node->m_Children[i]->m_Extents.IntersectNear(r);
                    if (test > FLT_EPSILON)
                    {
                        hitNodes.push(HitNode(node->m_Children[i], test));
                    }
                }
            }
        }

        if (hitNodes.size() == 0)
        {
            return;
        }

        const shared_ptr<OctreeNode> nearstNode = hitNodes.top().m_Node;
        double nearestHit = hitNodes.top().t;
        hitNodes.pop();

        if (nearestHit > intersection.t)
        {
            return;
        }

        IntersectNode(r, nearstNode, intersection, hitNodes);
    }

    Intersection BVH::Intersect(Ray& ray) const
    {
        std::priority_queue<HitNode> hitNodes;
        Intersection closestIntersection;

        float test = m_Octree->m_Extents.IntersectNear(ray);
        if (test > FLT_EPSILON)
        {
            IntersectNode(ray, m_Octree, closestIntersection, hitNodes);
        }

        return closestIntersection;
    }
}
