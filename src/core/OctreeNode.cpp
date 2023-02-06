#include "OctreeNode.h"
#include "Ray.h"
#include "basic/Object.h"

namespace new_type_renderer
{
    int OctreeNode::maxDepth = 0;

    OctreeNode::OctreeNode(OctreeNode* parent)
    {
        this->parent = parent;
        isLeaf = true;
        for (int i = 0; i < 8; ++i)
        {
            children[i] = nullptr;
        }
    }

    OctreeNode::OctreeNode()
    {
        this->parent = nullptr;
        isLeaf = true;
        for (int i = 0; i < 8; ++i)
        {
            children[i] = nullptr;
        }
    }

    void OctreeNode::destroyAllNodes()
    {
        for (int i = 0; i < 8; ++i)
        {
            if (this->children[i])
            {
                this->children[i]->destroyAllNodes();
            }
        }

        delete this;
    }

    OctreeNode::~OctreeNode()
    {
    }

    void OctreeNode::traverse()
    {
        if (isLeaf)
        {
            return;
        }

        for (int i = 0; i < 8; ++i)
        {
            if (children[i])
            {
                children[i]->traverse();
            }
        }
    }

    // friend bool Extents::operator==(Extents a, const Extents& b){
    //     return false;
    // }

    Extents OctreeNode::computeExetents()
    {
        if (isLeaf)
        {
            name = objects[0]->name + "_boundingbox";
            for (unsigned int i = 0; i < objects.size(); ++i)
            {
                Extents e = objects[i]->GetBounds();
                extents.ExtendBy(e);
            }

            return extents;
        }
        for (unsigned int i = 0; i < 8; ++i)
        {
            if (children[i])
            {
                Extents e = children[i]->computeExetents();
                extents.ExtendBy(e);
            }
        }

        return extents;
    }

    void OctreeNode::addObject(shared_ptr<Object>& obj)
    {
        int debugDepth = 90;

        if (depth > maxDepth)
        {
            maxDepth = depth;
        }

        Extents e = obj->GetBounds();
        Vector3 center = (boundMin + boundMax) * 0.5;
        Vector3 pos = obj->GetCentriod() - center;


        int childIdx = -1;

        std::vector<int> vec;
        vec.push_back(0);
        vec.push_back(1);
        vec.push_back(2);
        vec.push_back(3);
        vec.push_back(4);
        vec.push_back(5);
        vec.push_back(6);
        vec.push_back(7);


        if (pos.z > 0)
        {
            vec.erase(std::remove(vec.begin(), vec.end(), 2), vec.end());
            vec.erase(std::remove(vec.begin(), vec.end(), 3), vec.end());
            vec.erase(std::remove(vec.begin(), vec.end(), 6), vec.end());
            vec.erase(std::remove(vec.begin(), vec.end(), 7), vec.end());
        }
        else if (pos.z < 0)
        {
            vec.erase(std::remove(vec.begin(), vec.end(), 0), vec.end());
            vec.erase(std::remove(vec.begin(), vec.end(), 1), vec.end());
            vec.erase(std::remove(vec.begin(), vec.end(), 5), vec.end());
            vec.erase(std::remove(vec.begin(), vec.end(), 4), vec.end());
        }

        if (pos.x > 0)
        {
            vec.erase(std::remove(vec.begin(), vec.end(), 1), vec.end());
            vec.erase(std::remove(vec.begin(), vec.end(), 2), vec.end());
            vec.erase(std::remove(vec.begin(), vec.end(), 5), vec.end());
            vec.erase(std::remove(vec.begin(), vec.end(), 6), vec.end());
        }
        else if (pos.x < 0)
        {
            vec.erase(std::remove(vec.begin(), vec.end(), 0), vec.end());
            vec.erase(std::remove(vec.begin(), vec.end(), 3), vec.end());
            vec.erase(std::remove(vec.begin(), vec.end(), 4), vec.end());
            vec.erase(std::remove(vec.begin(), vec.end(), 7), vec.end());
        }

        if (pos.y > 0)
        {
            vec.erase(std::remove(vec.begin(), vec.end(), 4), vec.end());
            vec.erase(std::remove(vec.begin(), vec.end(), 5), vec.end());
            vec.erase(std::remove(vec.begin(), vec.end(), 6), vec.end());
            vec.erase(std::remove(vec.begin(), vec.end(), 7), vec.end());
        }
        else if (pos.y < 0)
        {
            vec.erase(std::remove(vec.begin(), vec.end(), 0), vec.end());
            vec.erase(std::remove(vec.begin(), vec.end(), 1), vec.end());
            vec.erase(std::remove(vec.begin(), vec.end(), 2), vec.end());
            vec.erase(std::remove(vec.begin(), vec.end(), 3), vec.end());
        }

        for (unsigned int i = 0; i < vec.size(); ++i)
        {
            childIdx = vec[i];

            if (!children[vec[i]])
            {
                childIdx = vec[i];
                break;
            }
        }

        if (!children[childIdx])
        {
            children[childIdx] = new OctreeNode(this);
            children[childIdx]->name = std::to_string(depth) + "_" + std::to_string(childIdx);
            children[childIdx]->depth = depth + 1;
            children[childIdx]->objects.push_back(obj);
            children[childIdx]->isLeaf = true;

            // TODO: caculate the child node bounding min and boundg max
            if (childIdx == 0)
            {
                children[childIdx]->boundMin = center;
                children[childIdx]->boundMax = this->boundMax;
            }
            else if (childIdx == 1)
            {
                children[childIdx]->boundMin = Vector3(boundMin.x, center.y, center.z);
                children[childIdx]->boundMax = Vector3(center.x, boundMax.y, boundMax.z);
            }
            else if (childIdx == 2)
            {
                children[childIdx]->boundMin = Vector3(boundMin.x, center.y, boundMin.z);
                children[childIdx]->boundMax = Vector3(center.x, boundMax.y, center.z);
            }
            else if (childIdx == 3)
            {
                children[childIdx]->boundMin = Vector3(center.x, center.y, boundMin.z);
                children[childIdx]->boundMax = Vector3(boundMax.x, boundMax.y, center.z);
            }
            else if (childIdx == 4)
            {
                children[childIdx]->boundMin = Vector3(center.x, boundMin.y, center.z);
                children[childIdx]->boundMax = Vector3(boundMax.x, center.y, boundMax.z);
            }
            else if (childIdx == 5)
            {
                children[childIdx]->boundMin = Vector3(boundMin.x, boundMin.y, center.z);
                children[childIdx]->boundMax = Vector3(center.x, center.y, boundMax.z);
            }
            else if (childIdx == 6)
            {
                children[childIdx]->boundMin = boundMin;
                children[childIdx]->boundMax = center;
            }
            else if (childIdx == 7)
            {
                children[childIdx]->boundMin = Vector3(center.x, boundMin.y, boundMin.z);
                children[childIdx]->boundMax = Vector3(boundMax.x, center.y, center.z);
            }
        }
        else
        {
            // if it is a leaf node
            if (children[childIdx]->isLeaf)
            {
                if (depth >= 50)
                {
                    children[childIdx]->objects.push_back(obj);
                }
                else
                {
                    auto childObj = children[childIdx]->objects[0];
                    children[childIdx]->addObject(obj);
                    children[childIdx]->objects.clear();
                    children[childIdx]->addObject(childObj);
                    children[childIdx]->isLeaf = false;
                }
            }
            else
            {
                children[childIdx]->addObject(obj);
            }
        }
    }

    void OctreeNode::intersectTestWireframe(const Ray& r, Intersection& intersection) const
    {
        if (this->depth > 4)
        {
            return;
        }

        double t = this->extents.IntersectWireframe(r);

        // Hit the bounding box.
        if (t > FLT_EPSILON && t < intersection.t)
        {
            intersection.t = t;
        }

        if (!this->isLeaf)
        {
            for (int i = 0; i < 8; ++i)
            {
                if (this->children[i])
                {
                    this->children[i]->intersectTestWireframe(r, intersection);
                }
            }
        }
    }


    void OctreeNode::intersectTest(Ray& r, Intersection& intersection) const
    {
        float test = this->extents.Intersect(r);

        // Hit the bounding box.
        if (test > FLT_EPSILON)
        {
            if (this->isLeaf)
            {
                for (unsigned int i = 0; i < this->objects.size(); ++i)
                {
                    double t = this->objects[i]->Intersect(r);
                    if (t > FLT_EPSILON && t < intersection.t)
                    {
                        t = this->objects[i]->Intersect(r);
                        intersection.object = this->objects[i];
                        intersection.t = t;
                    }
                }
            }
            else
            {
                // add the child node into a priority list, and check distance
                for (int i = 0; i < 8; ++i)
                {
                    if (this->children[i])
                    {
                        this->children[i]->intersectTest(r, intersection);
                    }
                }
            }
        }
    }
}
