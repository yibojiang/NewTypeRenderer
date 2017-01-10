#pragma once

#include <math.h>   
#include <iostream>
// #include <fstream>
#include <vector>
#include "sys/time.h"
#include <QImage>
#include <QDebug>
#include <QCoreApplication>

#include "vec.h"
#include "primitive.h"
#include "transform.h"
#include "modelloader.h"
#include <QColor>
#define WIREFRAME_ON
#define EXPLICIT_LIGHT_SAMPLE


class Intersection {
    public:
    Intersection() { t = inf; object = nullptr; }
    Intersection(double t_, Object* object_) { t = t_; object = object_; }
    operator bool() { return object != nullptr; }
    double t;
    Object* object;

};

class Scene
{
public:
    Transform *root;
    std::vector<Object*> objects;
    std::vector<Object*> lights;
    Scene(){
        root = new Transform();

    }
    void add(Object* object) {
        objects.push_back(object);
    }

    void addMesh(Mesh* mesh) {

        for (uint32_t i = 0; i < mesh->faces.size(); ++i) {
            Triangle *triangle = new Triangle(mesh->faces[i]->v1, mesh->faces[i]->v2, mesh->faces[i]->v3);
            triangle->setMaterial(mesh->getMaterial());
            triangle->name = mesh->name + '_' + std::to_string(i);
            add((Object*)triangle);
        }
    }

    Intersection intersect(const Ray& ray) const {
        Intersection closestIntersection;
        // intersect all objects, one after the other
        // for (std::vector<Object*>::iterator it = objects.begin(); it != objects.end(); ++it){
        for (uint32_t i = 0; i < objects.size(); ++i) {
            double t = objects[i]->intersect(ray);
            if (t > eps && t < closestIntersection.t) {
                closestIntersection.t = t;
                closestIntersection.object = objects[i];
            }
        }
        return closestIntersection;
    }

    

    ~Scene(){
        // printf("destroy scene\n");
        for (uint32_t i = 0; i < objects.size(); ++i) {
            delete objects[i];
        }
    }

    void destroyScene(){
        for (uint32_t i = 0; i < objects.size(); ++i) {
            delete objects[i];
        }

        objects.clear();
    }

    void updateTransform(Transform* transform, mat4 mt) {

        mt = mt * transform->getTransformMatrix();
        if (transform->object){
            transform->object->updateTransformMatrix(mt);

            qDebug()<<"add mesh:" << transform->object->name.c_str();
            if (transform->object->isMesh){
                addMesh((Mesh*)transform->object);    
            }
            else{
                add(transform->object);    
            }

            if (transform->object->getEmission().length() > eps ){
                lights.push_back(transform->object);
                qDebug() << "add light:" << transform->object->name.c_str();
            }
        }

        for (unsigned int i = 0; i < transform->children.size(); ++i){
            updateTransform(transform->children[i], mt);
        }
    }

    mat3 ca;
    vec3 ro;
    vec3 ta;
    float fov;
    float near;
};



class Raytracer{
public:
    Scene scene;
    BVH bvh;
    int curSamples;
    int samples;
    double progress;
    bool isRendering;

    Raytracer(unsigned width, unsigned height,int _samples);
    ~Raytracer();
    vec3 tracing(const Ray &ray, int depth, int E);
    
    
    void renderIndirectProgressive(vec3 *colorArray, int &samples);
    
    void renderIndirect(double &time, QImage &image);
    void renderDirect(double &time, QImage &directImage, QImage &normalImage, QImage &boundingBoxImage);
    unsigned short width;
    unsigned short height;
    void setResolution(const int &width, const int &height);
    void rotateCamera(float, float, float);
    void setupScene();
};

