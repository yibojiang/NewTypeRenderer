#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <streambuf>
#include <vector>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "BVH.h"
#include <basic/SceneNode.h>
#include "HDRLoader/HDRloader.h"
#include "math/Matrix.h"

// #define WIREFRAME_ON
#define EXPLICIT_LIGHT_SAMPLE
// #define RUSSIAN_ROULETTE_TERMINATION

namespace new_type_renderer
{
    class Intersection
    {
    public:
        Intersection()
        {
            t = inf;
            object = nullptr;
        }

        Intersection(double t_, Object* object_)
        {
            t = t_;
            object = object_;
        }

        operator bool() { return object != nullptr; }
        double t;
        Object* object;
    };

    class Scene
    {
    public:
        SceneNode* root;
        std::vector<Object*> objects;
        std::vector<Object*> lights;
        HDRImage hdri;
        double envLightIntense;
        double envLightExp;
        double envRotate;
        bool hasHdri;

        Scene()
        {
            // root = new SceneNode();
            envLightIntense = 1.0;
            envLightExp = 1.0;
            hasHdri = false;
        }

        // void LoadHdri(std::string name)
        // {
        //     std::string path = QDir::currentPath();
        //     // std::string name = "/textures/env.hdr";
        //     std::string fullpath = path.toUtf8().constData() + name;
        //     hasHdri = HDRLoader::load(fullpath.c_str(), hdri);
        // }

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
            // intersect all objects, one after the other
            // for (std::vector<Object*>::iterator it = objects.begin(); it != objects.end(); ++it){
            for (uint32_t i = 0; i < objects.size(); ++i)
            {
                double t = objects[i]->intersect(ray);
                if (t > eps && t < closestIntersection.t)
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
            root->removeAllChildren();
            for (uint32_t i = 0; i < objects.size(); ++i)
            {
                delete objects[i];
            }

            lights.clear();
            objects.clear();
        }

        void updateTransform(SceneNode* transform, Matrix4x4 mt)
        {
            mt = mt * transform->getTransformMatrix();
            if (transform->GetObject())
            {
                transform->GetObject()->updateTransformMatrix(mt);

                if (transform->GetObject()->isMesh)
                {
                    addMesh(static_cast<Mesh*>(transform->GetObject()));
                }
                else
                {
                    add(transform->GetObject());
                }

                if (transform->GetObject()->getMaterial()->getEmission().Length() > eps)
                {
                    lights.push_back(transform->GetObject());
                }
            }

            for (unsigned int i = 0; i < transform->GetChildren().size(); ++i)
            {
                updateTransform(transform->GetChildren()[i], mt);
            }
        }

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

    typedef Vector4 Color;
    struct Image
    {
    public:
        Image(int w, int h) : width(w), height(h)
        {
            pixels = new Color(w * h);
        }

        void SetPixel(int i, int j, const Color& color)
        {
            assert(i <= height);
            assert(j <= width);
            pixels[i * height + j] = color;
        }

    private:
        int width;
        int height;

        Color* pixels;
    };


    class Raytracer
    {
    public:
        Scene scene;
        BVH bvh;
        int curSamples;
        int samples;
        double progress;
        bool isRendering;
        std::string scenePath;

        Raytracer(unsigned width, unsigned height, int _samples);
        ~Raytracer();
        Vector3 tracing(Ray& ray, int depth, int E);


        void renderIndirectProgressive(Vector3* colorArray, bool& abort, bool& restart, int& samples);

        void renderIndirect(double& time, Image& image);
        void renderDirect(double& time, Image& directImage, Image& normalImage, Image& boundingBoxImage);
        void testPixel(int x, int y);
        void testRaytracing(Ray& ray, int depth);
        unsigned short width;
        unsigned short height;
        void setResolution(const int& width, const int& height);
        void rotateCamera(float, float, float);
        void moveCamera(float, float);
        void scaleCamera(float);
        void setupScene(const std::string& scenePath);
        void unloadScene();
        Vector3 getEnvColor(const Vector3& dir) const;
        // Vector3 toneMapping(const Vector3 &radiance) const;
    };
}
