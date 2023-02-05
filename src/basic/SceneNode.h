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

        std::shared_ptr<SceneNode> AddChild(const Vector3& location);

        std::shared_ptr<SceneNode> AddChild(std::shared_ptr<SceneNode> child);

        void AddObject(std::weak_ptr<Object> obj);

        std::weak_ptr<Object> GetObject() const { return m_Object; }

        std::vector<std::shared_ptr<SceneNode>>& GetChildren() { return m_Children; }

        void RemoveAllChildren();

        void RemoveChild(std::shared_ptr<SceneNode> child);

    public:
        Transform m_Transform{};

    protected:
        std::vector<std::shared_ptr<SceneNode>> m_Children{};

        std::weak_ptr<SceneNode> m_Parent{};

        std::weak_ptr<Object> m_Object{};
    };
}
