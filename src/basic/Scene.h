#pragma once

#include "math/Vector.h"
#include "Camera.h"
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

        void Add(Object* object);

        void AddMesh(Mesh* mesh);

        Intersection Intersect(Ray& ray) const;

        void DestroyScene();

        void UpdateTransform(SceneNode* scene_node, Matrix4x4 mt);

    public:

        SceneNode* m_Root;

        std::vector<Object*> m_Objects;

        std::vector<Object*> m_Lights;

        HDRImage m_HDRI;

        float m_EnvLightIntense;

        float m_EnvLightExp;

        float m_EnvRotate;

        bool m_HasHdri;

        Matrix3x3 ca;

        Vector3 ro;

        Vector3 ta;

        float cameraScale;

        Vector3 up;

        BVH bvh;

        Camera camera;
    };
}
