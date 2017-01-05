#pragma once
#include "vec.h"

class Object;
class Transform {
public:
    Transform *parent;
    std::vector<Transform*> children;
    
    Object *object;

    mat4 translate;
    Quaternion rotation;
    mat4 scale;

    void addObject(Object*);

    Transform* addChild(vec3&);

    Transform* addChild(Transform *child);

    void removeChild(Transform*);        

    vec3 getPosition();

    vec3 getScale();

    mat4 getTransformMatrix();

    void setTranslate(double tx, double ty, double tz);

    void setScale(double sx, double sy, double sz);

    void setRotation(Quaternion q);

    void move(const double tx, const double ty, const double tz);

    double getRotateX();

    double getRotateY();

    double getRotateZ();

    void rotateX(double rx);

    void rotateY(double ry);

    void rotateZ(double rz);

    Transform();

    ~Transform();
    
};