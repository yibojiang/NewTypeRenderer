#pragma once
#include "SceneNode.h"
#include "Primitive.h"

namespace new_type_renderer
{
    SceneNode::SceneNode()
    {
    }

    SceneNode::SceneNode(weak_ptr<Object> obj)
    {
        object = std::move(obj);
    }

    shared_ptr<SceneNode> SceneNode::AddChild(const Vector3& location)
    {
        auto child = make_unique<SceneNode>();
        child->transform.SetLocation(location);
        children.push_back(child);
        child->parent = make_shared<SceneNode>(*this);
        return child;
    }

    shared_ptr<SceneNode> SceneNode::AddChild(shared_ptr<SceneNode> child)
    {
        children.push_back(child);
        return child;
    }

    void SceneNode::AddObject(weak_ptr<Object> obj)
    {
        object = std::move(obj);
    }

    void SceneNode::RemoveAllChildren()
    {
        for (unsigned int i = 0; i < children.size(); ++i)
        {
            children[i]->removeAllChildren();
        }

        children.clear();
        delete this;
    }

    void SceneNode::RemoveChild(shared_ptr<SceneNode> child)
    {
        children.erase(std::remove(children.begin(), children.end(), child), children.end());
    }
}
