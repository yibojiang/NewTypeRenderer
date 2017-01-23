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

vec3 Extents::getCentriod() const{
    #if SLABCOUNT == 3
    return vec3(-(dnear[0] + dfar[0]) * 0.5, -(dnear[1] + dfar[1]) * 0.5, -(dnear[2] + dfar[2]) * 0.5);
    #endif
    #if SLABCOUNT == 7
    vec3 c1 = vec3(-(dnear[0] + dfar[0]) * 0.5, -(dnear[1] + dfar[1]) * 0.5, -(dnear[2] + dfar[2]) * 0.5);
    vec3 c2 = vec3(-(dnear[3] + dfar[3]) * 0.5, -(dnear[4] + dfar[4]) * 0.5, -(dnear[5] + dfar[5]) * 0.5);
    
    return (c1 + c2)  * 0.5;
    #endif
    
}

vec3 Extents::getBoundMin() const{
    return vec3(-dfar[0], -dfar[1], -dfar[2]);
}

vec3 Extents::getBoundMax() const{
    return vec3(-dnear[0], -dnear[1], -dnear[2]);
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
            std::swap(tFar, tNear);
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

    if (tmin <= 0){
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
            // double tmp = tFar;
            // tFar = tNear;
            // tNear = tmp;
            std::swap(tFar, tNear);
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

        if (count > 1){
            return t;
        }
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

        if (count > 1){
            return t;
        }

    }

    
    return 0;
    
    
}


BVH::BVH(){

}
int OctreeNode::maxDepth = 0;

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
    
    octree = new OctreeNode();
    octree->extents = sceneExtents;
    octree->depth = 0;
    // octree->centroid = sceneExtents.getCentriod();
    octree->boundMin = sceneExtents.getBoundMin();
    octree->boundMax = sceneExtents.getBoundMax();
    qDebug() << "min: " << octree->boundMin;
    qDebug() << "max: " << octree->boundMax;
    qDebug() << "centroid: " << sceneExtents.getCentriod();
    // Construct bvh hierarchy.
    for (uint32_t i = 0; i < scene.objects.size(); ++i){
        octree->addObject(scene.objects[i]);
    }
    octree->isLeaf = false;
    octree->computeExetents();

    gettimeofday(&end, NULL);
    double time = ((end.tv_sec  - start.tv_sec) * 1000000u + end.tv_usec - start.tv_usec) / 1.e6;
    qDebug() << "build bvh done." << " time: " << time;
    qDebug() << "max depth: " << OctreeNode::maxDepth;
    
    // octree.traverse();
}



OctreeNode::OctreeNode(OctreeNode *parent){
    this->parent = parent;
    isLeaf = true;
    for (int i = 0; i < 8; ++i){
        children[i] = nullptr;
    }

}

OctreeNode::OctreeNode(){
    this->parent = nullptr;
    isLeaf = true;
    for (int i = 0; i < 8; ++i){
        children[i] = nullptr;
    }
}

void OctreeNode::destroyAllNodes(){
    for (int i = 0; i < 8; ++i){
        if (this->children[i]){
            this->children[i]->destroyAllNodes();    
        }
    }

    delete this;
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
            // qDebug() << "depth: " << depth << "cid:" << i;
            children[i]->traverse();  

            // debugInfo += "\n";  
        }
        
    }

    
}

// friend bool Extents::operator==(Extents a, const Extents& b){
//     return false;
// }
Extents OctreeNode::computeExetents(){
    // qDebug() << this -> depth;
    if (this->isLeaf){
        // qDebug() << "leaf node" << this->object->name.c_str();
        for (unsigned int i = 0; i < this->objects.size(); ++i){
            Extents e = this->objects[i]->getBounds();
            this->extents.extendBy(e);
        }
        // this->extents = this->object->getBounds();
        return this->extents;
    }
    else{
        for (unsigned int i = 0; i < 8; ++i){
            if (this->children[i]){
                // qDebug() << "child: " << i;
                Extents e = this->children[i]->computeExetents();
                this->extents.extendBy(e);
            }
        }    
        // qDebug() << -this->extents.dnear[0] << -this->extents.dnear[1] << -this->extents.dnear[2];

        return this->extents;
    }
    
}

void OctreeNode::addObject(Object *obj){
    int debugDepth = 90;
    // qDebug() << "debug depth" << debugDepth;
    if (depth > debugDepth) qDebug() << "=======addObject: " << obj->name.c_str() << "depth: " << depth;     

    // if (depth >= 20){
        // qDebug() << "_______________________________________too deep, terminated";
    //     return;
    // }

    if (depth > OctreeNode::maxDepth){
        OctreeNode::maxDepth = depth;
    }

    Extents e = obj->getBounds();
    // vec3 pos = e.getCentriod() - this->extents.getCentriod();
    vec3 center = (boundMin + boundMax) * 0.5;
    // vec3 pos = e.getCentriod() - center;
    vec3 pos = obj->getCentriod() - center;

    
    int childIdx = -1;

    std::vector<int> vec;
    vec.push_back(0);
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    vec.push_back(4);
    vec.push_back(5);
    vec.push_back(6);
    vec.push_back(7);


    if (pos.z > 0){
        vec.erase(std::remove(vec.begin(), vec.end(), 2), vec.end());
        vec.erase(std::remove(vec.begin(), vec.end(), 3), vec.end());
        vec.erase(std::remove(vec.begin(), vec.end(), 6), vec.end());
        vec.erase(std::remove(vec.begin(), vec.end(), 7), vec.end());    
    }
    else if (pos.z < 0){
        vec.erase(std::remove(vec.begin(), vec.end(), 0), vec.end());
        vec.erase(std::remove(vec.begin(), vec.end(), 1), vec.end());
        vec.erase(std::remove(vec.begin(), vec.end(), 5), vec.end());
        vec.erase(std::remove(vec.begin(), vec.end(), 4), vec.end());       
    }

    if (pos.x > 0){
        vec.erase(std::remove(vec.begin(), vec.end(), 1), vec.end());
        vec.erase(std::remove(vec.begin(), vec.end(), 2), vec.end());
        vec.erase(std::remove(vec.begin(), vec.end(), 5), vec.end());
        vec.erase(std::remove(vec.begin(), vec.end(), 6), vec.end()); 
          
    }
    else if (pos.x < 0){
        vec.erase(std::remove(vec.begin(), vec.end(), 0), vec.end());
        vec.erase(std::remove(vec.begin(), vec.end(), 3), vec.end());
        vec.erase(std::remove(vec.begin(), vec.end(), 4), vec.end());
        vec.erase(std::remove(vec.begin(), vec.end(), 7), vec.end());        
    }

    if (pos.y > 0){
        vec.erase(std::remove(vec.begin(), vec.end(), 4), vec.end());
        vec.erase(std::remove(vec.begin(), vec.end(), 5), vec.end());
        vec.erase(std::remove(vec.begin(), vec.end(), 6), vec.end());
        vec.erase(std::remove(vec.begin(), vec.end(), 7), vec.end());  
    }
    else if (pos.y < 0){
        vec.erase(std::remove(vec.begin(), vec.end(), 0), vec.end());
        vec.erase(std::remove(vec.begin(), vec.end(), 1), vec.end());
        vec.erase(std::remove(vec.begin(), vec.end(), 2), vec.end());
        vec.erase(std::remove(vec.begin(), vec.end(), 3), vec.end()); 
    }
    
    // if (depth > debugDepth) qDebug() << "available size" << vec.size();
    for (unsigned int i = 0; i < vec.size(); ++i){
        childIdx = vec[i];

        if (depth > debugDepth) qDebug() << "centroid" << e.getCentriod() << "dir: " << pos << "available at: " << vec[i];
        
        if (!this->children[vec[i]]){
            childIdx = vec[i];
            break;
        }
    }       

    // qDebug() << "add to child" << childIdx;
    if (!this->children[childIdx]){
        if (depth > debugDepth) qDebug() << "new child at " << depth << childIdx;
        // qDebug() << "new child at " << depth << childIdx;
        this->children[childIdx] = new OctreeNode(this);
        this->children[childIdx]->depth = depth + 1;
        // this->children[childIdx]->object = obj;
        this->children[childIdx]->objects.push_back(obj);
        this->children[childIdx]->isLeaf = true;

        // TODO: caculate the child node bounding min and boundg max
        if (childIdx == 0){
            this->children[childIdx]->boundMin = center;
            this->children[childIdx]->boundMax = this->boundMax;
        }
        else if (childIdx == 1){
            this->children[childIdx]->boundMin = vec3(boundMin.x, center.y, center.z);
            this->children[childIdx]->boundMax = vec3(center.x, boundMax.y, boundMax.z);
        }
        else if (childIdx == 2){
            this->children[childIdx]->boundMin = vec3(boundMin.x, center.y, boundMin.z);
            this->children[childIdx]->boundMax = vec3(center.x, boundMax.y, center.z);
        }
        else if (childIdx == 3){
            this->children[childIdx]->boundMin = vec3(center.x, center.y, boundMin.z);
            this->children[childIdx]->boundMax = vec3(boundMax.x, boundMax.y, center.z);
        }
        else if (childIdx == 4){
            this->children[childIdx]->boundMin = vec3(center.x, boundMin.y, center.z);
            this->children[childIdx]->boundMax = vec3(boundMax.x, center.y, boundMax.z);
        }
        else if (childIdx == 5){
            this->children[childIdx]->boundMin = vec3(boundMin.x, boundMin.y, center.z);
            this->children[childIdx]->boundMax = vec3(center.x, center.y, boundMax.z);
        }
        else if (childIdx == 6){
            this->children[childIdx]->boundMin = boundMin;
            this->children[childIdx]->boundMax = center;
        }
        else if (childIdx == 7){
            this->children[childIdx]->boundMin = vec3(center.x, boundMin.y, boundMin.z);
            this->children[childIdx]->boundMax = vec3(boundMax.x, center.y, center.z);
        }
        
        if (depth > debugDepth) qDebug() << "child min" << this->children[childIdx]->boundMin;
        if (depth > debugDepth) qDebug() << "child max" << this->children[childIdx]->boundMax;
    }
    else{
        // if it is a leaf node
        if (children[childIdx]->isLeaf){
            if (depth >= 50){
               this->children[childIdx]->objects.push_back(obj);
            }
            else{
                // Object* childObj = this->children[childIdx]->object;   
                Object* childObj = this->children[childIdx]->objects[0];   
                if (depth > debugDepth) qDebug() << "*****************existing: " << childObj->name.c_str() << " split into new child node";
                this->children[childIdx]->addObject(obj);
                // this->children[childIdx]->object = nullptr;
                this->children[childIdx]->objects.clear();
                this->children[childIdx]->addObject(childObj); 
                this->children[childIdx]->isLeaf = false;    
            }
            

        }
        else{
            this->children[childIdx]->addObject(obj);
        }

        
    }
    
}

BVH::~BVH(){
    for (uint32_t i = 0; i < extentsList.size(); ++i) {
        delete extentsList[i];
    }
}

void BVH::destroy(){
    octree->destroyAllNodes();
}

// Return wireframe for all the boundingbox. (for Debugging)
void OctreeNode::intersectTestWireframe(const Ray &r, Intersection &intersection) const{

    if (this->depth > 4){
        return;
    }

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


void OctreeNode::intersectTest(Ray &r, Intersection &intersection) const{
    double test = this->extents.intersect(r);

    // Hit the bounding box.
    if (test > eps){
        if (this->isLeaf){
            for (unsigned int i = 0; i < this->objects.size(); ++i){
                double t = this->objects[i]->intersect(r);
                if (t > eps &&  t < intersection.t){
                     t = this->objects[i]->intersect(r);
                     intersection.object = this->objects[i];
                     intersection.t = t;
                }    
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
        double t = scene->objects[i]->getBounds().intersectWireframe(ray);
        // double t = scene->objects[i]->getBounds().intersect(ray);
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
    octree->intersectTestWireframe(ray, closestIntersection);
    return closestIntersection;   
}


Intersection BVH::intersect(Ray& ray) const{
    // std::priority_queue<OctreeNode> closeNode;
    // qDebug() << "BVH::intersect";
    Intersection closestIntersection;
    octree->intersectTest(ray, closestIntersection);
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
