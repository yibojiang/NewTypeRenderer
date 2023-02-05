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

        shared_ptr<SceneNode> AddChild(const Vector3& location);

        shared_ptr<SceneNode> AddChild(shared_ptr<SceneNode> child);

        void AddObject(weak_ptr<Object> obj);

        weak_ptr<Object> GetObject() const { return object; }

        std::vector<shared_ptr<SceneNode>>& GetChildren() { return children; }

        void RemoveAllChildren();

        void RemoveChild(shared_ptr<SceneNode> child);

        SceneNode();

        SceneNode(weak_ptr<Object> object);

    public:
        Transform transform{};

    private:
        std::vector<shared_ptr<SceneNode>> children{};

        weak_ptr<SceneNode> parent{};

        weak_ptr<Object> object{};
    };
}
