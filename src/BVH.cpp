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
    // lb is the corner of AABB with minimal coordinates - left bottom, rt is maximal corner
    // r.org is origin of ray
    // double t1 = (-dnear[0] - r.origin.dot(BVH::normals[0])) / r.dir.dot(BVH::normals[0]);
    // double t2 = (-dfar[0] - r.origin.dot(BVH::normals[0])) / r.dir.dot(BVH::normals[0]);
    // double t3 = (-dnear[1] - r.origin.dot(BVH::normals[1])) / r.dir.dot(BVH::normals[1]);
    // double t4 = (-dfar[1] - r.origin.dot(BVH::normals[1])) / r.dir.dot(BVH::normals[1]);
    // double t5 = (-dnear[2] - r.origin.dot(BVH::normals[2])) / r.dir.dot(BVH::normals[2]);
    // double t6 = (-dfar[2] - r.origin.dot(BVH::normals[2])) / r.dir.dot(BVH::normals[2]);
    // double t7 = (-dnear[3] - r.origin.dot(BVH::normals[3])) / r.dir.dot(BVH::normals[3]);
    // double t8 = (-dfar[3] - r.origin.dot(BVH::normals[3])) / r.dir.dot(BVH::normals[3]);
    // double t9 = (-dnear[4] - r.origin.dot(BVH::normals[4])) / r.dir.dot(BVH::normals[4]);
    // double t10 = (-dfar[4] - r.origin.dot(BVH::normals[4])) / r.dir.dot(BVH::normals[4]);
    // double t11 = (-dnear[5] - r.origin.dot(BVH::normals[5])) / r.dir.dot(BVH::normals[5]);
    // double t12 = (-dfar[5] - r.origin.dot(BVH::normals[5])) / r.dir.dot(BVH::normals[5]);
    // double t13 = (-dnear[6] - r.origin.dot(BVH::normals[6])) / r.dir.dot(BVH::normals[6]);
    // double t14 = (-dfar[6] - r.origin.dot(BVH::normals[6])) / r.dir.dot(BVH::normals[6]);


    // double tmin = fmax(fmax(fmin(t1, t2), fmin(t3, t4)), fmin(t5, t6)); // max t near 
    // tmin = fmax(tmin, fmin(t7, t8));
    // tmin = fmax(tmin, fmin(t9, t10));
    // tmin = fmax(tmin, fmin(t11, t12));
    // tmin = fmax(tmin, fmin(t13, t14));
    
    // double tmax = fmin(fmin(fmax(t1, t2), fmax(t3, t4)), fmax(t5, t6)); // min t far
    // tmax = fmin(tmax, fmax(t7, t8));
    // tmax = fmin(tmax, fmax(t8, t10));
    // tmax = fmin(tmax, fmax(t11, t12));
    // tmax = fmin(tmax, fmax(t13, t14));
    // // if tmax < 0, ray (line) is intersecting AABB, but whole AABB is behing us
    // if (tmax < 0){
    //     return 0;
    // }

    // // if tmin > tmax, ray doesn't intersect AABB
    // if (tmin > tmax){
    //     return 0;
    // }

    // return tmin;


    double tmin = -inf;
    double tmax = inf;
    for (uint8_t i = 0; i < BVH::slabCount; ++i){
        double tNear = (-dnear[i] - r.origin.dot(BVH::normals[i])) / r.dir.dot(BVH::normals[i]);
        double tFar = (-dfar[i] - r.origin.dot(BVH::normals[i])) / r.dir.dot(BVH::normals[i]);

        if (tNear > tFar){
            double tmp = tFar;
            tFar = tNear;
            tNear = tmp;
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


