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

        bool LoadFromJson(std::string json_file_name);        

        void LoadHdri(std::string name);

        void Add(Object* object);

        void AddMesh(Mesh* mesh);

        Intersection Intersect(Ray& ray) const;

        void DestroyScene();

        void UpdateTransform(SceneNode* scene_node, Matrix4x4 mt);

    public:

        SceneNode* root;

        std::vector<Object*> objects;

        std::vector<Object*> lights;

        HDRImage hdri;

        float envLightIntense;

        float envLightExp;

        float envRotate;

        bool hasHdri;

        Matrix3x3 ca;

        Vector3 ro;

        Vector3 ta;

        float cameraScale;

        Vector3 up;

        std::string scene_path;

        BVH bvh;

        Camera camera;

        SceneNode camera_node;
    };
}
