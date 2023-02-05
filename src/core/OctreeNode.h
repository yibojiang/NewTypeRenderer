#pragma once
#include <vector>

#include "Extents.h"

namespace new_type_renderer
{
    struct OctreeNode
    {
    public:
        OctreeNode();

        OctreeNode(OctreeNode* parent);

        ~OctreeNode();

        void addObject(Object*);

        void traverse();

        void destroyAllNodes();

        void intersectTest(Ray& r, Intersection& intersection) const;

        void intersectTestWireframe(const Ray& r, Intersection& intersection) const;

        static int maxDepth;

        Extents computeExetents();

    public:
        std::string name{};
        int depth{ 0 };
        bool isLeaf{ false };
        OctreeNode* parent{ nullptr };
        OctreeNode* children[8]{};

        vector<Object*> objects{};
        Vector3 boundMin{};
        Vector3 boundMax{};

        Extents extents;
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
}
