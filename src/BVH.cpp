#include "BVH.h"
#include "raytracer.h"
Extents::Extents(){
    for (uint8_t i = 0; i < SLABCOUNT; ++i){
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
    for (int i = 0; i < SLABCOUNT; ++i){
        dnear[i] = fmin(extents.dnear[i], dnear[i]);
        dfar[i] = fmax(extents.dfar[i], dfar[i]);
    }
}

double Extents::intersect(const Ray &r) const{ // returns distance, 0 if nohit    
    
    double tmin = -inf;
    double tmax = inf;
    for (uint8_t i = 0; i < SLABCOUNT; ++i){
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

    if (tmin < 0){
        return tmax;
    }
    
    return tmin;
    
}

double Extents::intersectWireframe(const Ray &r) const{ // returns distance, 0 if nohit    
    
    double tmin = -inf;
    double tmax = inf;
    for (uint8_t i = 0; i < SLABCOUNT; ++i){
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
    double width = 0.4;
    int count = 0;
    double t = tmin;
    vec3 hit = r.origin + r.dir * t;
    for (int i = 0; i < SLABCOUNT; ++i) {
        if ( fabs(-dnear[i] - hit.dot(BVH::normals[i])) < width){
            count ++;
        }
        else if ( fabs(-dfar[i] - hit.dot(BVH::normals[i])) < width){
            count ++;
        }
    }   

    if (count > 1){
        return t;
    }

    count = 0;
    t = tmax;
    hit = r.origin + r.dir * t;
    for (int i = 0; i < SLABCOUNT; ++i) {
        if ( fabs(-dnear[i] - hit.dot(BVH::normals[i])) < width){
            count ++;
        }
        else if ( fabs(-dfar[i] - hit.dot(BVH::normals[i])) < width){
            count ++;
        }

    }

    if (count > 1){
        return t;
    }
    else{
        return 0;
    }
    
}


BVH::BVH(){

}

void BVH::setup(Scene &scene){
    struct timeval start, end;
    gettimeofday(&start, NULL);
    qDebug() << "building bvh...";

    this->scene = &scene;
    Extents sceneExtents;
    for (uint32_t i = 0; i < scene.objects.size(); ++i) {
        scene.objects[i]->computebounds();
        Extents e = scene.objects[i]->getBounds();
        sceneExtents.extendBy(e);
    }
    
    octree.extents = sceneExtents;
    octree.depth = 0;
    // Construct bvh hierarchy.
    for (uint32_t i = 0; i < scene.objects.size(); ++i){
    // for (uint32_t i = 0; i < 2; ++i){
        // qDebug() << "add to bvh" << scene.objects[i]->name.c_str();
        octree.addObject(scene.objects[i]);
        // qDebug();
    }

    gettimeofday(&end, NULL);
    double time = ((end.tv_sec  - start.tv_sec) * 1000000u + end.tv_usec - start.tv_usec) / 1.e6;
    qDebug() << "build bvh done." << " time: " << time;
    octree.isLeaf = false;
    // octree.traverse();
}



OctreeNode::OctreeNode(OctreeNode *parent){
    this->object = nullptr;
    // this->extents = nullptr;
    this->parent = parent;
    isLeaf = true;
    for (int i = 0; i < 8; ++i){
        children[i] = nullptr;
    }

}

OctreeNode::OctreeNode(){
    // OctreeNode(NULL);
    this->object = nullptr;
    // this->extents = nullptr;
    this->parent = nullptr;
    isLeaf = true;
    for (int i = 0; i < 8; ++i){
        children[i] = nullptr;
    }
}

OctreeNode::~OctreeNode(){
    
}

void OctreeNode::traverse(){
    // qDebug() << "children i:"
    
    if (isLeaf){
        return;
    }
    
    for (int i = 0; i < 8; ++i){
        
        if (children[i]){
            qDebug() << "depth: " << depth << "cid:" << i;
            children[i]->traverse();  

            // debugInfo += "\n";  
        }
        
    }

    
}

void OctreeNode::addObject(Object *obj){
    // qDebug() << "addObject: " << obj->name.c_str() << "depth: " << depth;
    // qDebug() << "leaf" << isLeaf;
    // Extents *e = obj->computebounds();
    // Extents *e = new Extents(*obj->getBounds());
    Extents e = obj->getBounds();
    vec3 pos = e.getCentriod() - this->extents.getCentriod();
    // qDebug() << "obj cernter" << e.getCentriod();
    // qDebug() << "extents center" << this->extents.getCentriod();

    int childIdx = 0;

    if (depth >5){
        return ;
    }

    if (pos.x >= 0 && pos.y >= 0 && pos.z >= 0){
        childIdx = 0;  
    }
    else if (pos.x < 0 && pos.y >= 0 && pos.z >= 0){
        childIdx = 1;
    }
    else if (pos.x < 0 && pos.y < 0 && pos.z >= 0){
        childIdx = 2;
    }
    else if (pos.x >= 0 && pos.y < 0 && pos.z >= 0){
        childIdx = 3;
    }
    else if (pos.x >= 0 && pos.y >= 0 && pos.z < 0){
        childIdx = 4;
    }
    else if (pos.x < 0 && pos.y >= 0 && pos.z < 0){
        childIdx = 5;
    }
    else if (pos.x < 0 && pos.y < 0 && pos.z < 0){
        childIdx = 6;
    }
    else if (pos.x >= 0 && pos.y < 0 && pos.z < 0){
        childIdx = 7;
    }

    // qDebug() << "add to child" << childIdx;
    if (!this->children[childIdx]){
        // qDebug() << "new child at " << depth << childIdx;;
        this->children[childIdx] = new OctreeNode(this);
        this->children[childIdx]->depth = depth + 1;
        this->children[childIdx]->object = obj;
        this->children[childIdx]->extents = e;
        this->children[childIdx]->isLeaf = true;
        
    }
    else{
        // qDebug() << "occupied, at " << depth << childIdx;
        // qDebug() << "extent center" << this->children[childIdx]->extents.getCentriod();
        this->children[childIdx]->extents.extendBy(e);
        // qDebug() << "after extent center" << this->children[childIdx]->extents.getCentriod();
        this->children[childIdx]->isLeaf = false;
        
        // qDebug() << "create new children";
        
        Object* childObj = this->children[childIdx]->object;
        if (childObj){
            // qDebug() << "*****************existing: " << childObj->name.c_str();
            this->children[childIdx]->object = nullptr;
            this->children[childIdx]->addObject(childObj);            
        }

        this->children[childIdx]->addObject(obj);
        
        
    }
    
}

BVH::~BVH(){
    for (uint32_t i = 0; i < extentsList.size(); ++i) {
        delete extentsList[i];
    }
}

// Return wireframe for all the boundingbox. (for Debugging)
void OctreeNode::intersectTestWireframe(const Ray &r, Intersection &intersection) const{
    double t = this->extents.intersectWireframe(r);

    // Hit the bounding box.
    if (t > eps && t < intersection.t){
        intersection.t = t;
    }

    if (!this->isLeaf){
        for (int i = 0; i < 8; ++i){
            if (this->children[i]){
                this->children[i]->intersectTestWireframe(r, intersection);
            }            
        }
    }
}


void OctreeNode::intersectTest(const Ray &r, Intersection &intersection) const{
    double test = this->extents.intersect(r);

    // Hit the bounding box.
    if (test > eps){
        if (this->isLeaf){
            double t = this->object->intersect(r);
            if (t > eps &&  t < intersection.t){
                 t = this->object->intersect(r);
                 intersection.object = this->object;
                 intersection.t = t;
            }
        }
        else{
            for (int i = 0; i < 8; ++i){
                if (this->children[i]){
                    this->children[i]->intersectTest(r, intersection);
                }            
            }
        }
    }
}

Intersection BVH::intersectBoundingBox(const Ray& ray) const{
    Intersection closestIntersection;
    for (uint8_t i = 0; i < scene->objects.size(); ++i){
        // double t = scene->objects[i]->getBounds().intersectWireframe(ray);
        double t = scene->objects[i]->getBounds().intersect(ray);
        if (t > eps && t < closestIntersection.t) {
            closestIntersection.t = t;
            closestIntersection.object = scene->objects[i];
        }
    }
    return closestIntersection;
}

Intersection BVH::intersectBVH(const Ray& ray) const{
    // std::priority_queue<OctreeNode> closeNode;
    Intersection closestIntersection;
    octree.intersectTestWireframe(ray, closestIntersection);
    return closestIntersection;   
}


Intersection BVH::intersect(const Ray& ray) const{
    // std::priority_queue<OctreeNode> closeNode;
    Intersection closestIntersection;
    octree.intersectTest(ray, closestIntersection);
    return closestIntersection;   
}



#if SLABCOUNT == 7
const vec3 BVH::normals[SLABCOUNT] = {
    vec3(1, 0, 0),
    vec3(0, 1, 0),
    vec3(0, 0, 1),
    vec3(sqrt(3.0)/3.0, sqrt(3.0)/3.0, sqrt(3.0)/3.0),
    vec3(-sqrt(3.0)/3.0, sqrt(3.0)/3.0, sqrt(3.0)/3.0),
    vec3(-sqrt(3.0)/3.0, -sqrt(3.0)/3.0, sqrt(3.0)/3.0),
    vec3(sqrt(3.0)/3.0, -sqrt(3.0)/3.0, sqrt(3.0)/3.0)

};
#endif

#if SLABCOUNT == 3
const vec3 BVH::normals[SLABCOUNT] = {
    vec3(1, 0, 0),
    vec3(0, 1, 0),
    vec3(0, 0, 1)
};
#endif
