#pragma once
#include "SceneNode.h"
#include "Primitive.h"

namespace new_type_renderer
{
    SceneNode::SceneNode()
    {
    }

    SceneNode::SceneNode(shared_ptr<SceneNode>& parent) : m_Parent(parent)
    {
    }

    shared_ptr<SceneNode> SceneNode::AddChild(const Vector3& location)
    {
        auto child = make_shared<SceneNode>();
        child->m_Transform.SetLocation(location);
        m_Children.push_back(child);
        return child;
    }

    shared_ptr<SceneNode> SceneNode::AddChild(shared_ptr<SceneNode> child)
    {
        m_Children.push_back(child);
        return child;
    }

    void SceneNode::AddObject(const shared_ptr<Object>& obj)
    {
        m_Object = obj;
    }

    void SceneNode::RemoveAllChildren()
    {
        for (unsigned int i = 0; i < m_Children.size(); ++i)
        {
            m_Children[i]->RemoveAllChildren();
        }

        m_Children.clear();
    }

    void SceneNode::RemoveChild(shared_ptr<SceneNode> child)
    {
        m_Children.erase(std::remove(m_Children.begin(), m_Children.end(), child), m_Children.end());
    }

    void SceneNode::GetAllObjects(std::vector<shared_ptr<Object>>& objects) const
    {
        if (m_Object != nullptr)
        {
            objects.push_back(m_Object);
        }
        
        for (int i = 0; i < m_Children.size(); i++)
        {
            m_Children[i]->GetAllObjects(objects);
        }
    }
}
