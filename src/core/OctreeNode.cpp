#include "OctreeNode.h"
#include "Ray.h"
#include "basic/Object.h"

namespace new_type_renderer
{
    OctreeNode::OctreeNode()
    {
        m_IsLeaf = true;
        for (int i = 0; i < 8; ++i)
        {
            m_Children[i] = nullptr;
        }
    }

    OctreeNode::OctreeNode(Vector3 boundMin, Vector3 boundMax): m_BoundMin(boundMin), m_BoundMax(boundMax)
    {
        
        m_Depth = 0;
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
            for (int i = 0; i < m_Objects.size(); ++i)
            {
                Extents e = m_Objects[i]->GetBounds();
                m_Extents.ExtendBy(e);
            }

            return m_Extents;
        }

        for (int i = 0; i < 8; ++i)
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
        const Vector3 center = (m_BoundMin + m_BoundMax) * 0.5;

        // Decide which subdiv node the mesh would attach to base on the relative location to bound center
        const Vector3 relativePos = obj->GetCentriod() - center;

        int selectChildId = -1;

        std::vector<int> candidateChildIds {
            0, 1, 2, 3, 4, 5, 6, 7
        };

        // filter out the candidates by the relative location
        if (relativePos.z > 0)
        {
            candidateChildIds.erase(std::remove(candidateChildIds.begin(), candidateChildIds.end(), 2), candidateChildIds.end());
            candidateChildIds.erase(std::remove(candidateChildIds.begin(), candidateChildIds.end(), 3), candidateChildIds.end());
            candidateChildIds.erase(std::remove(candidateChildIds.begin(), candidateChildIds.end(), 6), candidateChildIds.end());
            candidateChildIds.erase(std::remove(candidateChildIds.begin(), candidateChildIds.end(), 7), candidateChildIds.end());
        }
        else if (relativePos.z < 0)
        {
            candidateChildIds.erase(std::remove(candidateChildIds.begin(), candidateChildIds.end(), 0), candidateChildIds.end());
            candidateChildIds.erase(std::remove(candidateChildIds.begin(), candidateChildIds.end(), 1), candidateChildIds.end());
            candidateChildIds.erase(std::remove(candidateChildIds.begin(), candidateChildIds.end(), 5), candidateChildIds.end());
            candidateChildIds.erase(std::remove(candidateChildIds.begin(), candidateChildIds.end(), 4), candidateChildIds.end());
        }

        if (relativePos.x > 0)
        {
            candidateChildIds.erase(std::remove(candidateChildIds.begin(), candidateChildIds.end(), 1), candidateChildIds.end());
            candidateChildIds.erase(std::remove(candidateChildIds.begin(), candidateChildIds.end(), 2), candidateChildIds.end());
            candidateChildIds.erase(std::remove(candidateChildIds.begin(), candidateChildIds.end(), 5), candidateChildIds.end());
            candidateChildIds.erase(std::remove(candidateChildIds.begin(), candidateChildIds.end(), 6), candidateChildIds.end());
        }
        else if (relativePos.x < 0)
        {
            candidateChildIds.erase(std::remove(candidateChildIds.begin(), candidateChildIds.end(), 0), candidateChildIds.end());
            candidateChildIds.erase(std::remove(candidateChildIds.begin(), candidateChildIds.end(), 3), candidateChildIds.end());
            candidateChildIds.erase(std::remove(candidateChildIds.begin(), candidateChildIds.end(), 4), candidateChildIds.end());
            candidateChildIds.erase(std::remove(candidateChildIds.begin(), candidateChildIds.end(), 7), candidateChildIds.end());
        }

        if (relativePos.y > 0)
        {
            candidateChildIds.erase(std::remove(candidateChildIds.begin(), candidateChildIds.end(), 4), candidateChildIds.end());
            candidateChildIds.erase(std::remove(candidateChildIds.begin(), candidateChildIds.end(), 5), candidateChildIds.end());
            candidateChildIds.erase(std::remove(candidateChildIds.begin(), candidateChildIds.end(), 6), candidateChildIds.end());
            candidateChildIds.erase(std::remove(candidateChildIds.begin(), candidateChildIds.end(), 7), candidateChildIds.end());
        }
        else if (relativePos.y < 0)
        {
            candidateChildIds.erase(std::remove(candidateChildIds.begin(), candidateChildIds.end(), 0), candidateChildIds.end());
            candidateChildIds.erase(std::remove(candidateChildIds.begin(), candidateChildIds.end(), 1), candidateChildIds.end());
            candidateChildIds.erase(std::remove(candidateChildIds.begin(), candidateChildIds.end(), 2), candidateChildIds.end());
            candidateChildIds.erase(std::remove(candidateChildIds.begin(), candidateChildIds.end(), 3), candidateChildIds.end());
        }

        for (int i = 0; i < candidateChildIds.size(); ++i)
        {
            selectChildId = candidateChildIds[i];

            // Choose the first candidate child node to insert the mesh
            if (!m_Children[candidateChildIds[i]])
            {
                selectChildId = candidateChildIds[i];
                break;
            }
        }

        auto& selectedChildNode = m_Children[selectChildId];
        if (!m_Children[selectChildId])
        {
            selectedChildNode = make_shared<OctreeNode>();
            selectedChildNode->m_Parent = shared_from_this();
            selectedChildNode->m_Name = std::to_string(m_Depth) + "_" + std::to_string(selectChildId);
            selectedChildNode->m_Depth = m_Depth + 1;
            selectedChildNode->m_Objects.push_back(obj);
            selectedChildNode->m_IsLeaf = true;

            // TODO: caculate the child node bounding min and bounding max
            if (selectChildId == 0)
            {
                selectedChildNode->m_BoundMin = center;
                selectedChildNode->m_BoundMax = this->m_BoundMax;
            }
            else if (selectChildId == 1)
            {
                selectedChildNode->m_BoundMin = Vector3(m_BoundMin.x, center.y, center.z);
                selectedChildNode->m_BoundMax = Vector3(center.x, m_BoundMax.y, m_BoundMax.z);
            }
            else if (selectChildId == 2)
            {
                selectedChildNode->m_BoundMin = Vector3(m_BoundMin.x, center.y, m_BoundMin.z);
                selectedChildNode->m_BoundMax = Vector3(center.x, m_BoundMax.y, center.z);
            }
            else if (selectChildId == 3)
            {
                selectedChildNode->m_BoundMin = Vector3(center.x, center.y, m_BoundMin.z);
                selectedChildNode->m_BoundMax = Vector3(m_BoundMax.x, m_BoundMax.y, center.z);
            }
            else if (selectChildId == 4)
            {
                selectedChildNode->m_BoundMin = Vector3(center.x, m_BoundMin.y, center.z);
                selectedChildNode->m_BoundMax = Vector3(m_BoundMax.x, center.y, m_BoundMax.z);
            }
            else if (selectChildId == 5)
            {
                selectedChildNode->m_BoundMin = Vector3(m_BoundMin.x, m_BoundMin.y, center.z);
                selectedChildNode->m_BoundMax = Vector3(center.x, center.y, m_BoundMax.z);
            }
            else if (selectChildId == 6)
            {
                selectedChildNode->m_BoundMin = m_BoundMin;
                selectedChildNode->m_BoundMax = center;
            }
            else if (selectChildId == 7)
            {
                selectedChildNode->m_BoundMin = Vector3(center.x, m_BoundMin.y, m_BoundMin.z);
                selectedChildNode->m_BoundMax = Vector3(m_BoundMax.x, center.y, center.z);
            }
        }
        else
        {
            // Leaf node contains the object, need to further split the child node
            if (selectedChildNode->m_IsLeaf)
            {
                /*
                 * If the tree reach max depth, just put them into the same node, otherwise
                 * split the nodes and the objects into the child nodes of the split nodes
                 */
                if (m_Depth >= m_MaxDepth)
                {
                    m_Children[selectChildId]->m_Objects.push_back(obj);
                }
                else
                {
                    // Mark the selected child node as the leaf node as it needs to be split
                    selectedChildNode->m_IsLeaf = false;

                    // Split the nodes as the child node has object
                    auto childObj = selectedChildNode->m_Objects[0];

                    selectedChildNode->AddObject(obj);

                    // Needs to clear the object list as only the leaf node contains the object
                    selectedChildNode->m_Objects.clear();
                    selectedChildNode->AddObject(childObj);
                    
                }
            }
            else
            {
                // Recursively call to add the object to the target child node
                selectedChildNode->AddObject(obj);
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
        if (t > FLT_EPSILON && t < intersection.m_Distance)
        {
            intersection.m_Distance = t;
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
                    Intersection geomIntersection = m_Objects[i]->Intersect(r);
                    if (geomIntersection.GetHitDistance() > FLT_EPSILON && geomIntersection < intersection)
                    {
                        intersection = geomIntersection;
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
