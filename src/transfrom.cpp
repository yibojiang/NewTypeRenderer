#include "transform.h"
#include "primitive.h"

vec3 Transform::getPosition(){
    vec4 p(0, 0, 0, 1);
    p = translate * p;
    return vec3(p.x, p.y, p.z);
}

vec3 Transform::getScale(){
    vec4 s(1, 1, 1, 1);
    s = scale * s;
    return vec3(s.x, s.y, s.z);   
}

mat4 Transform::getTransformMatrix(){
    return translate * rotation.toMatrix() * scale;
}

void Transform::setTranslate(double tx, double ty, double tz){
    translate = mat4(1, 0, 0, tx,
                     0, 1, 0, ty,
                     0, 0, 1, tz,
                     0, 0, 0 , 1);
}

void Transform::setScale(double sx, double sy, double sz){
    scale = mat4(sx, 0,  0,  0,
                 0,  sy, 0,  0,
                 0,  0,  sz, 0,
                 0,  0,  0,  1);
}

void Transform::setRotation(Quaternion q){
    rotation = q;
}

void Transform::move(const double tx, const double ty, const double tz){
    mat4 mt(1, 0, 0, tx,
            0, 1, 0, ty,
            0, 0, 1, tz,
            0, 0, 0, 1);
    translate = mt * translate;
}

double Transform::getRotateX(){
    double ysqr = rotation.y * rotation.y;
    // roll (x-axis rotation)
    double t0 = +2.0f * (rotation.w * rotation.x + rotation.y * rotation.z);
    double t1 = +1.0f - 2.0f * (rotation.x * rotation.x + ysqr);
    return atan2(t0, t1);
}

double Transform::getRotateY(){
    // pitch (y-axis rotation)
    double t2 = +2.0f * (rotation.w * rotation.y - rotation.z * rotation.x);
    t2 = t2 > 1.0f ? 1.0f : t2;
    t2 = t2 < -1.0f ? -1.0f : t2;
    return asin(t2);
}

double Transform::getRotateZ(){
    double ysqr = rotation.y * rotation.y;
    // yaw (z-axis rotation)
    double t3 = +2.0f * (rotation.w * rotation.z + rotation.x *rotation.y);
    double t4 = +1.0f - 2.0f * (ysqr + rotation.z * rotation.z);  
    return atan2(t3, t4);
}

void Transform::rotateX(double rx){
    Quaternion q(vec3(1, 0, 0), rx);
    rotation = q * rotation;
}

void Transform::rotateY(double ry){
    Quaternion q(vec3(0, 1, 0), ry);
    rotation = q * rotation;
}

void Transform::rotateZ(double rz){
    Quaternion q(vec3(0, 0, 1), rz);
    rotation = q * rotation;
}

Transform::Transform() {
    translate = mat4 (1, 0, 0, 0,
                    0, 1, 0, 0,
                    0, 0, 1, 0,
                    0, 0, 0, 1);
    rotation = Quaternion(0, 0, 0, 1);
    scale = mat4(1, 0,  0,  0,
                 0,  1, 0,  0,
                 0,  0,  1, 0,
                 0,  0,  0,  1);
    parent = nullptr;
    object = nullptr;
    // children = new std::vector<Transform*>();
}

Transform* Transform::addChild(vec3& pos){
    Transform *child = new Transform();
    child->setTranslate(pos.x, pos.y, pos.z);
    children.push_back(child);
    return child;
}

Transform* Transform::addChild(Transform *child){
    children.push_back(child);
    return child;
}

void Transform::addObject(Object* obj){
    object = obj;
    // object->updateTransform(*this);
}

void Transform::removeChild(Transform* child){
    children.erase(std::remove(children.begin(), children.end(), child), children.end());
    delete child;
}

Transform::~Transform() {}
