#include "OctreeNode.h"
#include "Ray.h"
#include "basic/Object.h"

namespace new_type_renderer
{
    int OctreeNode::maxDepth = 0;

    OctreeNode::OctreeNode()
    {
        m_IsLeaf = true;
        for (int i = 0; i < 8; ++i)
        {
            m_Children[i] = nullptr;
        }
    }

    void OctreeNode::DestroyAllNodes()
    {
        for (int i = 0; i < 8; ++i)
        {
            if (m_Children[i])
            {
                m_Children[i]->DestroyAllNodes();
            }
        }
    }

    OctreeNode::~OctreeNode()
    {
    }

    void OctreeNode::Traverse()
    {
        if (m_IsLeaf)
        {
            return;
        }

        for (int i = 0; i < 8; ++i)
        {
            if (m_Children[i])
            {
                m_Children[i]->Traverse();
            }
        }
    }

    Extents OctreeNode::ComputeExetents()
    {
        if (m_IsLeaf)
        {
            m_Name = m_Objects[0]->name + "_boundingbox";
            for (unsigned int i = 0; i < m_Objects.size(); ++i)
            {
                Extents e = m_Objects[i]->GetBounds();
                m_Extents.ExtendBy(e);
            }

            return m_Extents;
        }
        for (unsigned int i = 0; i < 8; ++i)
        {
            if (m_Children[i])
            {
                Extents e = m_Children[i]->ComputeExetents();
                m_Extents.ExtendBy(e);
            }
        }

        return m_Extents;
    }

    void OctreeNode::AddObject(shared_ptr<Object>& obj)
    {
        int debugDepth = 90;

        if (m_Depth > maxDepth)
        {
            maxDepth = m_Depth;
        }

        Extents e = obj->GetBounds();
        Vector3 center = (m_BoundMin + m_BoundMax) * 0.5;
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

            if (!m_Children[vec[i]])
            {
                childIdx = vec[i];
                break;
            }
        }

        if (!m_Children[childIdx])
        {
            m_Children[childIdx] = make_shared<OctreeNode>();
            m_Children[childIdx]->m_Parent = shared_from_this();
            m_Children[childIdx]->m_Name = std::to_string(m_Depth) + "_" + std::to_string(childIdx);
            m_Children[childIdx]->m_Depth = m_Depth + 1;
            m_Children[childIdx]->m_Objects.push_back(obj);
            m_Children[childIdx]->m_IsLeaf = true;

            // TODO: caculate the child node bounding min and boundg max
            if (childIdx == 0)
            {
                m_Children[childIdx]->m_BoundMin = center;
                m_Children[childIdx]->m_BoundMax = this->m_BoundMax;
            }
            else if (childIdx == 1)
            {
                m_Children[childIdx]->m_BoundMin = Vector3(m_BoundMin.x, center.y, center.z);
                m_Children[childIdx]->m_BoundMax = Vector3(center.x, m_BoundMax.y, m_BoundMax.z);
            }
            else if (childIdx == 2)
            {
                m_Children[childIdx]->m_BoundMin = Vector3(m_BoundMin.x, center.y, m_BoundMin.z);
                m_Children[childIdx]->m_BoundMax = Vector3(center.x, m_BoundMax.y, center.z);
            }
            else if (childIdx == 3)
            {
                m_Children[childIdx]->m_BoundMin = Vector3(center.x, center.y, m_BoundMin.z);
                m_Children[childIdx]->m_BoundMax = Vector3(m_BoundMax.x, m_BoundMax.y, center.z);
            }
            else if (childIdx == 4)
            {
                m_Children[childIdx]->m_BoundMin = Vector3(center.x, m_BoundMin.y, center.z);
                m_Children[childIdx]->m_BoundMax = Vector3(m_BoundMax.x, center.y, m_BoundMax.z);
            }
            else if (childIdx == 5)
            {
                m_Children[childIdx]->m_BoundMin = Vector3(m_BoundMin.x, m_BoundMin.y, center.z);
                m_Children[childIdx]->m_BoundMax = Vector3(center.x, center.y, m_BoundMax.z);
            }
            else if (childIdx == 6)
            {
                m_Children[childIdx]->m_BoundMin = m_BoundMin;
                m_Children[childIdx]->m_BoundMax = center;
            }
            else if (childIdx == 7)
            {
                m_Children[childIdx]->m_BoundMin = Vector3(center.x, m_BoundMin.y, m_BoundMin.z);
                m_Children[childIdx]->m_BoundMax = Vector3(m_BoundMax.x, center.y, center.z);
            }
        }
        else
        {
            // if it is a leaf node
            if (m_Children[childIdx]->m_IsLeaf)
            {
                if (m_Depth >= 50)
                {
                    m_Children[childIdx]->m_Objects.push_back(obj);
                }
                else
                {
                    auto childObj = m_Children[childIdx]->m_Objects[0];
                    m_Children[childIdx]->AddObject(obj);
                    m_Children[childIdx]->m_Objects.clear();
                    m_Children[childIdx]->AddObject(childObj);
                    m_Children[childIdx]->m_IsLeaf = false;
                }
            }
            else
            {
                m_Children[childIdx]->AddObject(obj);
            }
        }
    }

    void OctreeNode::IntersectTestWireframe(const Ray& r, Intersection& intersection) const
    {
        if (this->m_Depth > 4)
        {
            return;
        }

        double t = this->m_Extents.IntersectWireframe(r);

        // Hit the bounding box.
        if (t > FLT_EPSILON && t < intersection.t)
        {
            intersection.t = t;
        }

        if (!this->m_IsLeaf)
        {
            for (int i = 0; i < 8; ++i)
            {
                if (this->m_Children[i])
                {
                    this->m_Children[i]->IntersectTestWireframe(r, intersection);
                }
            }
        }
    }


    void OctreeNode::IntersectTest(Ray& r, Intersection& intersection) const
    {
        float test = this->m_Extents.Intersect(r);

        // Hit the bounding box.
        if (test > FLT_EPSILON)
        {
            if (this->m_IsLeaf)
            {
                for (unsigned int i = 0; i < this->m_Objects.size(); ++i)
                {
                    double t = this->m_Objects[i]->Intersect(r);
                    if (t > FLT_EPSILON && t < intersection.t)
                    {
                        t = this->m_Objects[i]->Intersect(r);
                        intersection.object = this->m_Objects[i];
                        intersection.t = t;
                    }
                }
            }
            else
            {
                // add the child node into a priority list, and check distance
                for (int i = 0; i < 8; ++i)
                {
                    if (this->m_Children[i])
                    {
                        this->m_Children[i]->IntersectTest(r, intersection);
                    }
                }
            }
        }
    }
}
