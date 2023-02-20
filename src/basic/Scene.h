#pragma once

#include "math/Vector.h"
#include "Camera.h"
#include "Mesh.h"
#include "basic/Primitive.h"
#include "basic/SceneNode.h"
#include "core/BVH.h"
#include "HDRLoader/HDRloader.h"

namespace new_type_renderer
{
    struct Scene
    {
    public:
        Scene();

        ~Scene();

        bool LoadFromJson(const std::string& fullPath);

        void LoadHdri(const std::string& name);

        void Add(shared_ptr<Shape>& object);

        void AddMesh(shared_ptr<Mesh>& mesh);

        Intersection Intersect(Ray& ray) const;

        void DestroyScene();

        void UpdateTransform(shared_ptr<SceneNode>& sceneNode, Matrix4x4 mt);

    public:
        shared_ptr<SceneNode> m_Root;

        std::vector<shared_ptr<Shape>> m_Objects;

        std::vector<shared_ptr<Shape>> m_Lights;

        HDRImage m_HDRI;

        float m_EnvLightIntense;

        float m_EnvLightExp;

        float m_EnvRotate;

        bool m_HasHdri;
        
        BVH bvh;

        Camera m_Camera;
    };
}
