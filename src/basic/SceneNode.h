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

        std::weak_ptr<Object> GetObject() const { return object; }

        std::vector<std::shared_ptr<SceneNode>>& GetChildren() { return children; }

        void RemoveAllChildren();

        void RemoveChild(std::shared_ptr<SceneNode> child);

    public:
        Transform transform{};

    protected:
        std::vector<std::shared_ptr<SceneNode>> children{};

        std::weak_ptr<SceneNode> parent{};

        std::weak_ptr<Object> object{};
    };
}
