#include "BVH.h"
#include "raytracer.h"
BVH::BVH(){

}

void BVH::setup(Scene &scene){
    this->scene = &scene;
    for (uint32_t i = 0; i < scene.objects.size(); ++i) {
        scene.objects[i]->computebounds();
    }

}

BVH::~BVH(){
    
}

Intersection BVH::intersect(const Ray& ray) const{
    Intersection closestIntersection;
    for (uint32_t i = 0; i < scene->objects.size(); ++i) {
        double t = scene->objects[i]->getBounds().intersect(ray);
        // std::cout << t << std::endl;
        // double t = (*it)->intersect(ray);
        if (t > eps && t < closestIntersection.t) {
            closestIntersection.t = t;
            closestIntersection.object = scene->objects[i];
        }
    }
    return closestIntersection;   

}

const vec3 BVH::normals[BVH::slabCount] = {
    vec3(1, 0, 0),
    vec3(0, 1, 0),
    vec3(0, 0, 1),
    vec3(sqrt(3.0)/3.0, sqrt(3.0)/3.0, sqrt(3.0)/3.0),
    vec3(-sqrt(3.0)/3.0, sqrt(3.0)/3.0, sqrt(3.0)/3.0),
    vec3(-sqrt(3.0)/3.0, -sqrt(3.0)/3.0, sqrt(3.0)/3.0),
    vec3(sqrt(3.0)/3.0, -sqrt(3.0)/3.0, sqrt(3.0)/3.0)

};

Extents::Extents(){
    for (uint8_t i = 0; i < BVH::slabCount; ++i){
        dnear[i] = inf;
        dfar[i] = -inf;
    }
}

double Extents::intersect(const Ray &r) { // returns distance, 0 if nohit
    // vec3 invdir(1.0 / r.dir.x, 1.0 / r.dir.y, 1.0 / r.dir.z);
    double tmin = -inf;
    double tmax = inf;
    for (uint8_t i = 0; i < BVH::slabCount; ++i){
        double tNear = ( -dnear[i] - r.origin.dot(BVH::normals[i])) / r.dir.dot(BVH::normals[i]);
        double tFar = ( -dfar[i] - r.origin.dot(BVH::normals[i])) / r.dir.dot(BVH::normals[i]);

        double tmp;
        if (tNear > tFar){
            tmp = tFar;
            tFar = tNear;
            tFar = tmp;
        }

        if (tNear > tmin){
            tmin = tNear;
        }

        if (tFar < tmax){
            tmax = tFar;
        }
    }

    if (tmax < 0){
        return 0;
    }

    if (tmin > tmax){
        return 0;
    }

    return tmin;
}


