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
        octree.addObject(scene.objects[i]);
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
        qDebug() << "leaf: " << this->object->name.c_str();
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

// TODO: bug exiests when two geometry share the same bounds.
void OctreeNode::addObject(Object *obj){
    if (depth > 90){
        qDebug() << "addObject: " << obj->name.c_str() << "depth: " << depth;     
    }
    
    // qDebug() << "leaf" << isLeaf;
    // Extents *e = obj->computebounds();
    // Extents *e = new Extents(*obj->getBounds());
    Extents e = obj->getBounds();
    vec3 pos = e.getCentriod() - this->extents.getCentriod();
    // vec3 pos = obj->getCentriod() - this->extents.getCentriod();
    // qDebug() << obj->name.c_str() << " center: " << obj->getCentriod();
    
    // qDebug() << "extents center" << this->extents.getCentriod();
    if (depth > 90) qDebug() << "pos: " << pos;

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

    if (depth > 90){
        qDebug() << "available size" << vec.size();
    }

    if (pos.z > 0){
        vec.erase(std::remove(vec.begin(), vec.end(), 4), vec.end());
        vec.erase(std::remove(vec.begin(), vec.end(), 5), vec.end());
        vec.erase(std::remove(vec.begin(), vec.end(), 6), vec.end());
        vec.erase(std::remove(vec.begin(), vec.end(), 7), vec.end());    
        if (depth > 90)  qDebug() << "remove -z: " << vec.size();
        
    }
    else if (pos.z < 0){
        vec.erase(std::remove(vec.begin(), vec.end(), 0), vec.end());
        vec.erase(std::remove(vec.begin(), vec.end(), 1), vec.end());
        vec.erase(std::remove(vec.begin(), vec.end(), 2), vec.end());
        vec.erase(std::remove(vec.begin(), vec.end(), 3), vec.end());       
        if (depth > 90)  qDebug() << "remove +z: " << vec.size();
    }

    if (pos.x > 0){
        vec.erase(std::remove(vec.begin(), vec.end(), 1), vec.end());
        vec.erase(std::remove(vec.begin(), vec.end(), 2), vec.end());
        vec.erase(std::remove(vec.begin(), vec.end(), 5), vec.end());
        vec.erase(std::remove(vec.begin(), vec.end(), 7), vec.end()); 
        if (depth > 90)  qDebug() << "remove -x: " << vec.size();   
    }
    else if (pos.x < 0){
        vec.erase(std::remove(vec.begin(), vec.end(), 0), vec.end());
        vec.erase(std::remove(vec.begin(), vec.end(), 3), vec.end());
        vec.erase(std::remove(vec.begin(), vec.end(), 4), vec.end());
        vec.erase(std::remove(vec.begin(), vec.end(), 6), vec.end());
        if (depth > 90)  qDebug() << "remove +x: " << vec.size();   
    }

    if (pos.y > 0){
        vec.erase(std::remove(vec.begin(), vec.end(), 2), vec.end());
        vec.erase(std::remove(vec.begin(), vec.end(), 3), vec.end());
        vec.erase(std::remove(vec.begin(), vec.end(), 6), vec.end());
        vec.erase(std::remove(vec.begin(), vec.end(), 7), vec.end());  
        if (depth > 90)  qDebug() << "remove -y: " << vec.size();  
    }
    else if (pos.y < 0){
        vec.erase(std::remove(vec.begin(), vec.end(), 1), vec.end());
        vec.erase(std::remove(vec.begin(), vec.end(), 0), vec.end());
        vec.erase(std::remove(vec.begin(), vec.end(), 5), vec.end());
        vec.erase(std::remove(vec.begin(), vec.end(), 4), vec.end()); 
        if (depth > 90)  qDebug() << "remove +y: " << vec.size();   
    }
    
    if (depth > 90) qDebug() << "after available size" << vec.size();
    
    
    for (unsigned int i = 0; i < vec.size(); ++i){
        childIdx = vec[i];

        if (depth > 90) qDebug() << "pos: " << pos << "available: " << vec[i];
        
        if (!this->children[vec[i]]){
            childIdx = vec[i];
            break;
        }
    }       

    if (depth > 100){   
        return;
    }

    if (childIdx == -1){
        qDebug() << "pos" << pos <<" child: " << childIdx;
        qDebug() << "error idx";
        return;
    }


    

    // qDebug() << "add to child" << childIdx;
    if (!this->children[childIdx]){
        qDebug() << "new child at " << depth << childIdx;
        this->children[childIdx] = new OctreeNode(this);
        this->children[childIdx]->depth = depth + 1;
        this->children[childIdx]->object = obj;
        this->children[childIdx]->extents = e;
        this->children[childIdx]->isLeaf = true;
        
    }
    else{

        
        Object* childObj = this->children[childIdx]->object;
        this->children[childIdx]->isLeaf = false;
        if (depth > 90){
            qDebug() << "occupied, at " << depth << childIdx;
            qDebug() << "extent center" << this->children[childIdx]->extents.getCentriod();
        }

        // qDebug() << "extent center" << this->children[childIdx]->extents.getCentriod();
        this->children[childIdx]->extents.extendBy(e);

        if (depth > 90) qDebug() << "after extent center" << this->children[childIdx]->extents.getCentriod();
        

        this->children[childIdx]->addObject(obj);
        if (childObj){
            qDebug() << "create new children";
            if (depth > 90) qDebug() << "*****************existing: " << childObj->name.c_str();
            
            this->children[childIdx]->object = nullptr;
            this->children[childIdx]->addObject(childObj); 
            
        }
        
        
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
