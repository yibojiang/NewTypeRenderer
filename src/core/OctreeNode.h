#pragma once
#include <vector>

#include "Extents.h"

namespace new_type_renderer
{
    // https://en.cppreference.com/w/cpp/memory/enable_shared_from_this
    struct OctreeNode : public std::enable_shared_from_this<OctreeNode>
    {
    public:
        OctreeNode();

        OctreeNode(Extents boundingExtent);

        ~OctreeNode();

        void AddObject(shared_ptr<Object>& object);

        void Traverse();

        void DestroyAllNodes();

        void IntersectTest(Ray& r, Intersection& intersection) const;

        void IntersectTestWireframe(const Ray& r, Intersection& intersection) const;

        static constexpr int m_MaxDepth{ 50 };

        Extents ComputeExetents();

    public:
        std::string m_Name{};
        int m_Depth{ 0 };
        bool m_IsLeaf{ false };
        weak_ptr<OctreeNode> m_Parent{};
        shared_ptr<OctreeNode> m_Children[8]{};

        vector<shared_ptr<Object>> m_Objects{};
        Vector3 m_BoundMin{};
        Vector3 m_BoundMax{};

        Extents m_Extents;
    };

    struct HitNode
    {
        shared_ptr<OctreeNode> m_Node;
        float t;

        HitNode(const shared_ptr<OctreeNode>& n, float thit)
        {
            m_Node = n;
            t = thit;
        }

        friend bool operator<(const HitNode& a, const HitNode& b)
        {
            return a.t > b.t;
        }
    };
}
