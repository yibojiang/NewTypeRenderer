#include "BVH.h"
#include "OctreeNode.h"
#include "basic/Scene.h"

namespace new_type_renderer
{
    BVH::BVH()
    {
    }

    void BVH::Setup(Scene& scene)
    {
        this->scene = &scene;
        Extents sceneExtents;
        for (uint32_t i = 0; i < scene.objects.size(); ++i)
        {
            scene.objects[i]->ComputeBounds();
            Extents e = scene.objects[i]->GetBounds();
            sceneExtents.ExtendBy(e);
        }

        octree = new OctreeNode();
        octree->extents = sceneExtents;
        octree->depth = 0;

        octree->boundMin = sceneExtents.GetBoundMin();
        octree->boundMax = sceneExtents.GetBoundMax();

        // Construct bvh hierarchy.
        for (uint32_t i = 0; i < scene.objects.size(); ++i)
        {
            octree->addObject(scene.objects[i]);
        }
        octree->isLeaf = false;
        octree->computeExetents();      
    }

    BVH::~BVH()
    {
        for (uint32_t i = 0; i < extents_list.size(); ++i)
        {
            delete extents_list[i];
        }
    }

    void BVH::Destroy()
    {
        octree->destroyAllNodes();
    }    

    Intersection BVH::IntersectBoundingBox(const Ray& ray) const
    {
        Intersection closestIntersection;
        for (uint8_t i = 0; i < scene->objects.size(); ++i)
        {
            float t = scene->objects[i]->GetBounds().IntersectWireframe(ray);
            if (t > FLT_EPSILON && t < closestIntersection.t)
            {
                closestIntersection.t = t;
                closestIntersection.object = scene->objects[i];
            }
        }
        return closestIntersection;
    }

    Intersection BVH::IntersectBVH(const Ray& ray) const
    {
        // std::priority_queue<OctreeNode> closeNode;
        Intersection closestIntersection;
        octree->intersectTestWireframe(ray, closestIntersection);
        return closestIntersection;
    }


    void BVH::IntersectNode(Ray& r, const OctreeNode* node, Intersection& intersection,
                            std::priority_queue<HitNode>& hitNodes) const
    {
        if (node->isLeaf)
        {
            for (unsigned int i = 0; i < node->objects.size(); ++i)
            {
                double t = node->objects[i]->Intersect(r);

                if (t > FLT_EPSILON && t < intersection.t)
                {
                    t = node->objects[i]->Intersect(r);
                    intersection.object = node->objects[i];
                    intersection.t = t;
                }
            }
        }
        else
        {
            for (int i = 0; i < 8; ++i)
            {
                if (node->children[i])
                {
                    float test = node->children[i]->extents.IntersectNear(r);
                    if (test > FLT_EPSILON)
                    {
                        hitNodes.push(HitNode(node->children[i], test));
                    }
                }
            }
        }

        if (hitNodes.size() == 0)
        {
            return;
        }

        const OctreeNode* nearstNode = hitNodes.top().node;
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


        double test = octree->extents.IntersectNear(ray);
        if (test > FLT_EPSILON)
        {
            IntersectNode(ray, octree, closestIntersection, hitNodes);
        }


        return closestIntersection;
    }


#if SLABCOUNT == 7
    const Vector3 BVH::normals[SLABCOUNT] = {
        Vector3(1, 0, 0),
        Vector3(0, 1, 0),
        Vector3(0, 0, 1),
        Vector3(sqrt(3.0) / 3.0, sqrt(3.0) / 3.0, sqrt(3.0) / 3.0),
        Vector3(-sqrt(3.0) / 3.0, sqrt(3.0) / 3.0, sqrt(3.0) / 3.0),
        Vector3(-sqrt(3.0) / 3.0, -sqrt(3.0) / 3.0, sqrt(3.0) / 3.0),
        Vector3(sqrt(3.0) / 3.0, -sqrt(3.0) / 3.0, sqrt(3.0) / 3.0)
    };
#endif

#if SLABCOUNT == 3
    const Vector3 BVH::normals[SLABCOUNT] = {
        Vector3(1, 0, 0),
        Vector3(0, 1, 0),
        Vector3(0, 0, 1)
    };
#endif
}
