#pragma once
#include <vector>
#include <memory>
#include "Transform.h"
#include "Object.h"

namespace new_type_renderer
{
    struct Vector3;

    class SceneNode
    {
    public:
        SceneNode();

        SceneNode(shared_ptr<SceneNode>& parent);

        std::shared_ptr<SceneNode> AddChild(const Vector3& location);

        std::shared_ptr<SceneNode> AddChild(std::shared_ptr<SceneNode> child);

        void AddObject(const std::shared_ptr<Object>& obj);

        std::shared_ptr<Object>& GetObject() { return m_Object; }

        std::vector<std::shared_ptr<SceneNode>>& GetChildren() { return m_Children; }

        void RemoveAllChildren();

        void RemoveChild(std::shared_ptr<SceneNode> child);

        // Return object from this node and child nodes.
        void GetAllObjects(std::vector<shared_ptr<Object>>& objects) const;

    public:
        Transform m_Transform{};

    protected:
        std::vector<std::shared_ptr<SceneNode>> m_Children{};

        std::weak_ptr<SceneNode> m_Parent{};

        std::shared_ptr<Object> m_Object{};
    };
}
