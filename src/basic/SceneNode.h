#pragma once
#include <vector>

#include "Primitive.h"
#include "Transform.h"
#include "math/Matrix.h"
#include "math/Quaternion.h"

class Object;

namespace new_type_renderer
{
    class SceneNode
    {
    public:
        void removeAllChildren();

        unique_ptr<SceneNode> AddChild(const Vector3& location);

        unique_ptr<SceneNode> AddChild(unique_ptr<SceneNode> child);

        void AddObject(weak_ptr<Object> obj);

        weak_ptr<Object> GetObject() const { return object; }

        std::vector<unique_ptr<SceneNode>>& GetChildren() { return children; }

        void RemoveAllChildren();

        void RemoveChild(unique_ptr<SceneNode> child);

        SceneNode();

        SceneNode(weak_ptr<Object> object);

        ~SceneNode();

    private:
        std::vector<unique_ptr<SceneNode>> children{};

        SceneNode* parent{ nullptr };

        weak_ptr<Object> object{};

        Transform transform{};
    };
}
