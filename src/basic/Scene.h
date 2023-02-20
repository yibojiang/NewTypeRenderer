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
    using std::vector;

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

        Camera& GetCamera()
        {
            return m_Camera;
        }

        shared_ptr<SceneNode>& GetRootNode()
        {
            return m_Root;
        }

        vector<shared_ptr<Shape>>& GetAllShapes()
        {
            return m_Shapes;
        }

        vector<shared_ptr<Shape>>& GetAllLights()
        {
            return m_Lights;
        }

        const unique_ptr<HDRImage>& GetHDRImage() const
        {
            return m_HDRI;
        }

    public:
        float m_EnvLightIntense;

        float m_EnvLightExp;

        float m_EnvRotate;

        BVH bvh;

    private:
        shared_ptr<SceneNode> m_Root;

        vector<shared_ptr<Shape>> m_Shapes;

        vector<shared_ptr<Shape>> m_Lights;

        bool m_HasHdri;

        unique_ptr<HDRImage> m_HDRI;

        Camera m_Camera;
    };
}
