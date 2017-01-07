#include "BVH.h"
#include "raytracer.h"
Extents::Extents(){
    for (uint8_t i = 0; i < BVH::slabCount; ++i){
        dnear[i] = inf;
        dfar[i] = -inf;
    }
}

Extents::~Extents(){

}

vec3 Extents::getCentriod(){
    return vec3(-(dnear[0] + dfar[0]) * 0.5, -(dnear[1] + dfar[1]) * 0.5, -(dnear[2] + dfar[2]) * 0.5);
}

void Extents::extendBy(Extents &extents){
    for (int i = 0; i < BVH::slabCount; ++i){
        dnear[i] = fmin(extents.dnear[i], dnear[i]);
        dfar[i] = fmax(extents.dfar[i], dfar[i]);
    }
}

double Extents::intersect(const Ray &r) { // returns distance, 0 if nohit    
    
    double tmin = -inf;
    double tmax = inf;
    for (uint8_t i = 0; i < BVH::slabCount; ++i){
        double tNear = (-dnear[i] - r.origin.dot(BVH::normals[i])) / r.dir.dot(BVH::normals[i]);
        double tFar = (-dfar[i] - r.origin.dot(BVH::normals[i])) / r.dir.dot(BVH::normals[i]);

        // Swap near and far t.
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


BVH::BVH(){

}

void BVH::setup(Scene &scene){
    this->scene = &scene;
    Extents *sceneExtents = new Extents();
    for (uint32_t i = 0; i < scene.objects.size(); ++i) {
        Extents e = *scene.objects[i]->computebounds();
        sceneExtents->extendBy(e);

        // Extents *e2 = scene.objects[i]->computebounds();
        // qDebug() << "centroid: " << e2->getCentriod();
        // extentsList.push_back(e2);
    }
    // extentsList.push_back(sceneExtents);
    
    
    // qDebug() << "sceneExtents: " << sceneExtents->getCentriod();
    // vec3 sceneCenter = sceneExtents->getCentriod();
    octree.extents = sceneExtents;
    // for (int i = 0; i < 8; ++i){
    //     octree.children[i] = new OctreeNode();
    // }

    for (uint32_t i = 0; i < scene.objects.size(); ++i){
        octree.addObject(scene.objects[i]);
    }
}



OctreeNode::OctreeNode(OctreeNode *parent){
    this->object = nullptr;
    this->extents = nullptr;
    this->parent = parent;
    for (int i = 0; i < 8; ++i){
        children[i] = nullptr;
    }
}

OctreeNode::OctreeNode(){
    // OctreeNode(NULL);
    this->object = nullptr;
    this->extents = nullptr;
    this->parent = nullptr;
    for (int i = 0; i < 8; ++i){
        children[i] = nullptr;
    }
}

OctreeNode::~OctreeNode(){
    
}

void OctreeNode::Traverse(){

}

void OctreeNode::addObject(Object *obj){
    Extents *e = obj->computebounds();
    vec3 pos = e->getCentriod() - this->extents->getCentriod();
    int childIdx = 0;
    if (pos.x > 0 && pos.y > 0 && pos.z > 0){
        childIdx = 0;  
    }
    else if (pos.x < 0 && pos.y > 0 && pos.z > 0){
        childIdx = 1;
    }
    else if (pos.x < 0 && pos.y < 0 && pos.z > 0){
        childIdx = 2;
    }
    else if (pos.x > 0 && pos.y < 0 && pos.z > 0){
        childIdx = 3;
    }
    else if (pos.x > 0 && pos.y > 0 && pos.z < 0){
        childIdx = 4;
    }
    else if (pos.x < 0 && pos.y > 0 && pos.z < 0){
        childIdx = 5;
    }
    else if (pos.x < 0 && pos.y < 0 && pos.z < 0){
        childIdx = 6;
    }
    else if (pos.x > 0 && pos.y < 0 && pos.z < 0){
        childIdx = 7;
    }

    if (!this->children[childIdx]){
        this->children[childIdx] = new OctreeNode();
        this->children[childIdx]->object = obj;
        this->children[childIdx]->extents = obj->computebounds();
    }
    else{
        this->children[childIdx]->addObject(obj);
    }
    
}

BVH::~BVH(){
    for (uint32_t i = 0; i < extentsList.size(); ++i) {
        delete extentsList[i];
    }
}

Intersection BVH::intersect(const Ray& ray) const{
    Intersection closestIntersection;
    for (uint32_t i = 0; i < extentsList.size(); ++i) {
        double t = extentsList[i]->intersect(ray);
        if (t > eps && t < closestIntersection.t) {
            closestIntersection.t = t;

            // closestIntersection.object = scene->objects[i];
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

