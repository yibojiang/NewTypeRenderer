#pragma once
#include "basic/Primitive.h"
#include "HDRLoader/HDRloader.h"
#include "math/Vector.h"
#include "basic/SceneNode.h"

namespace new_type_renderer
{
    struct Scene
    {
    public:
        Scene()
        {
            envLightIntense = 1.0;
            envLightExp = 1.0;
            hasHdri = false;
        }

        void LoadHdri(std::string name)
        {
            std::string path = "";
            // std::string name = "/textures/env.hdr";
            std::string fullpath = path + name;
            hasHdri = HDRLoader::load(fullpath.c_str(), hdri);
        }

        void add(Object* object)
        {
            objects.push_back(object);
        }

        void addMesh(Mesh* mesh)
        {
            for (uint32_t i = 0; i < mesh->faces.size(); ++i)
            {
                Triangle* triangle = mesh->faces[i];
                // triangle->setupVertices(mesh->faces[i]->p1, mesh->faces[i]->p2, mesh->faces[i]->p3);
                // Triangle *triangle = new Triangle(mesh->faces[i]->v1, mesh->faces[i]->v2, mesh->faces[i]->v3);
                // Triangle *triangle = new Triangle(mesh->faces[i]->p1, mesh->faces[i]->p2, mesh->faces[i]->p3);
                // if (mesh->getMaterial()){
                //     triangle->setMaterial(mesh->getMaterial());    
                // }

                if (!triangle->getMaterial())
                {
                    triangle->setMaterial(mesh->getMaterial());
                }

                triangle->name = mesh->name + '_' + std::to_string(i);
                // triangle->setupUVs(mesh->faces[i]->uv1, mesh->faces[i]->uv2, mesh->faces[i]->uv3);
                add(triangle);
            }
        }

        Intersection intersect(Ray& ray) const
        {
            Intersection closestIntersection;
            // Intersect all objects, one after the other
            // for (std::vector<Object*>::iterator it = objects.begin(); it != objects.end(); ++it){
            for (uint32_t i = 0; i < objects.size(); ++i)
            {
                double t = objects[i]->intersect(ray);
                if (t > FLT_EPSILON && t < closestIntersection.t)
                {
                    closestIntersection.t = t;
                    closestIntersection.object = objects[i];
                }
            }
            return closestIntersection;
        }


        ~Scene()
        {
            destroyScene();
        }

        void destroyScene()
        {
            root->RemoveAllChildren();
            for (uint32_t i = 0; i < objects.size(); ++i)
            {
                delete objects[i];
            }

            lights.clear();
            objects.clear();
        }

        void updateTransform(SceneNode* scene_node, Matrix4x4 mt)
        {
            mt = mt * scene_node->transform.TransformMatrix();
            auto obj_wekptr = scene_node->GetObject();

            if (auto obt_spt = scene_node->GetObject().lock())
            {
                obt_spt->updateTransformMatrix(mt);

                if (obt_spt->isMesh)
                {
                    addMesh(static_cast<Mesh*>(obt_spt.get()));
                }
                else
                {
                    add(obt_spt.get());
                }

                if (obt_spt->getMaterial()->getEmission().Length() > FLT_EPSILON)
                {
                    lights.push_back(obt_spt.get());
                }
            }

            for (unsigned int i = 0; i < scene_node->GetChildren().size(); ++i)
            {
                updateTransform(scene_node->GetChildren()[i].get(), mt);
            }
        }

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
        float fov;
        float near;
        float focalLength;
        float focusOn;
        float aperture;
        float focalDistance;
    };
}
