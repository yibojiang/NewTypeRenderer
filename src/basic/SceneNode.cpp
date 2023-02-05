#pragma once
#include "SceneNode.h"
#include "Primitive.h"

namespace new_type_renderer
{
    SceneNode::SceneNode()
    {
    }

    shared_ptr<SceneNode> SceneNode::AddChild(const Vector3& location)
    {
        auto child = make_shared<SceneNode>();
        child->m_Transform.SetLocation(location);
        m_Children.push_back(child);
        child->m_Parent = make_shared<SceneNode>(*this);
        return child;
    }

    shared_ptr<SceneNode> SceneNode::AddChild(shared_ptr<SceneNode> child)
    {
        m_Children.push_back(child);
        return child;
    }

    void SceneNode::AddObject(weak_ptr<Object> obj)
    {
        m_Object = std::move(obj);
    }

    void SceneNode::RemoveAllChildren()
    {
        for (unsigned int i = 0; i < m_Children.size(); ++i)
        {
            m_Children[i]->RemoveAllChildren();
        }

        m_Children.clear();
        delete this;
    }

    void SceneNode::RemoveChild(shared_ptr<SceneNode> child)
    {
        m_Children.erase(std::remove(m_Children.begin(), m_Children.end(), child), m_Children.end());
    }
}
