#pragma once
#include "transform.h"
const double inf=1e9;
const double eps=1e-6;

struct Ray {
    vec3 origin;
    vec3 dir;
    Ray(vec3 ro) { origin = ro; dir = vec3(1,0,0); }
    Ray(vec3 ro, vec3 rd){ origin = ro; dir = rd; }
};


enum Refl_t { DIFF, SPEC, REFR };  // material types, used in radiance()
class Object {
protected:
    vec3 p; 
    vec3 e;
    vec3 c; // position, emission, color
    Refl_t refl; // reflection type (DIFFuse, SPECular, REFRactive)
public:
    Transform transform;
    Object(){}
    virtual ~Object(){}
    
    virtual vec3 getNormal(const vec3 &) const{ return vec3(1);}
    virtual double intersect(const Ray &){ return 0;}

    virtual vec3 getDiffuse() const{
        return c;
    }

    virtual vec3 getEmission() const{
        return e;
    }

    virtual Refl_t getReflectionType() const{
        return refl;
    }
    // virtual vec3 debug(vec3 _pos) const{return vec3(0);}
};



class Plane: public Object{
private:
    vec3 normal;
public:
    
    double off;
    Plane(vec3 _nor, double _off, vec3 _e, vec3 _c, Refl_t _refl){
        normal = _nor.normalize();
        off = _off;
        e = _e;
        c = _c;
        refl = _refl;
    }

    Plane(vec3 _nor, double _off) { off = _off; normal = _nor; }
    double intersect(const Ray &r) {
        return ( -r.origin.dot(normal) - off) / (normal).dot(r.dir);
    }

    vec3 getNormal(const vec3 &) const{
        return normal;
    }

    // virtual vec3 debug(vec3 _pos) const{return vec3(1);}
};

class Sphere: public Object {
public:
    double rad;
    vec3 center;

    Sphere(double _rad, vec3 _p, vec3 _e, vec3 _c, Refl_t _refl){
        rad = _rad;
        center = _p;
        e = _e;
        c = _c;
        refl = _refl;
    }

    Sphere(double _rad, vec3 _pos) { rad = _rad; center = _pos; }
    double intersect(const Ray &r) { // returns distance, 0 if nohit
        vec3 op = center - r.origin; // Solve t^2*d.d + 2*t*(o-p).d + (o-p).(o-p)-R^2 = 0
        double t, eps = 1e-4, b = op.dot(r.dir), det = b * b - op.dot(op) + rad * rad;
        if (det < 0) return 0; else det = sqrt(det);
        return (t = b - det) > eps ? t : ((t = b + det) > eps ? t : 0);
    }

    vec3 getNormal(const vec3 &_pos) const{
        return (_pos - center)/rad;
    }

    // virtual vec3 debug(vec3 _pos) const{return vec3(1);}
};


class AABBox: public Object{ 
public: 
    vec3 bounds[2]; 
    vec3 center;
    vec3 size;
    AABBox(const vec3 &_center, const vec3 &_size, vec3 _e, vec3 _c, Refl_t _refl) { 
        center = _center;
        size = _size;
        bounds[0] = center - _size * 0.5, bounds[1] = center + _size * 0.5; 
        e = _e;
        c = _c;
        refl = _refl;
    }

    double intersect(const Ray &r) { // returns distance, 0 if nohit
        double t;
        vec3 dirfrac;
        dirfrac.x = 1.0 / r.dir.x;
        dirfrac.y = 1.0 / r.dir.y;
        dirfrac.z = 1.0 / r.dir.z;
        // lb is the corner of AABB with minimal coordinates - left bottom, rt is maximal corner
        // r.org is origin of ray
        double t1 = (bounds[0].x - r.origin.x)*dirfrac.x;
        double t2 = (bounds[1].x - r.origin.x)*dirfrac.x;
        double t3 = (bounds[0].y - r.origin.y)*dirfrac.y;
        double t4 = (bounds[1].y - r.origin.y)*dirfrac.y;
        double t5 = (bounds[0].z - r.origin.z)*dirfrac.z;
        double t6 = (bounds[1].z - r.origin.z)*dirfrac.z;

        double tmin = fmax(fmax(fmin(t1, t2), fmin(t3, t4)), fmin(t5, t6));
        double tmax = fmin(fmin(fmax(t1, t2), fmax(t3, t4)), fmax(t5, t6));

        // if tmax < 0, ray (line) is intersecting AABB, but whole AABB is behing us
        if (tmax < 0)
        {
            t = tmax;
            return 0;
        }

        // if tmin > tmax, ray doesn't intersect AABB
        if (tmin > tmax)
        {
            t = tmax;
            return 0;
        }

        t = tmin;
        return t;
    } 
    
    vec3 getNormal(const vec3 &_pos) const{
        vec3 normal;
        vec3 localPoint = _pos - center;
        float min = std::numeric_limits<float>::max();
        float distance = fabs(size.x - fabs(localPoint.x));
        if (distance < min) {
            min = distance;
            normal = vec3(1, 0, 0);
            if (localPoint.x < 0){
                normal = normal * -1;
            }
            // normal *= SIGN(localPoint.x);
        }
        distance = fabs(size.y - fabs(localPoint.y));
        if (distance < min) {
            min = distance;
            normal = vec3(0, 1, 0);
            if (localPoint.y < 0){
                normal = normal * -1;
            }
            // normal *= SIGN(localPoint.y);
        }
        distance = fabs(size.z - fabs(localPoint.z));
        if (distance < min) { 
            min = distance; 
            normal = vec3(0, 0, 1);
            if (localPoint.z < 0){
                normal = normal * -1;
            }
        } 
        return normal;
       
    }
};
 
class Triangle : public Object{
private:
    
    vec3 n1, n2, n3;
    vec2 uv1, uv2, uv3;

public:
    vec3 normal, u, v;
    vec3 p1, p2, p3;


    Triangle(vec3 _p1, vec3 _p2, vec3 _p3, vec3 _e=vec3(0,0,0), vec3 _c=vec3(1,1,1), Refl_t _refl=DIFF) : p1(_p1), p2(_p2), p3(_p3) {
        e = _e;
        c = _c;
        refl = _refl;
        u = p2 - p1;
        v = p3 - p1;
        normal = u.cross(v).normalize();              // cross product
    }
    ~Triangle(){

    }

    void setNormals(vec3 _n1, vec3 _n2, vec3 _n3) {
        // normal = _normal.normalize();
        n1 = _n1.normalize();
        n2 = _n2.normalize();
        n3 = _n3.normalize();
    }

    void setUVs(vec2 _uv1, vec2 _uv2, vec2 _uv3){
        uv1 = _uv1;
        uv2 = _uv2;
        uv3 = _uv3;
    }

    vec3 getNormal(const vec3 &) const{    
        return normal;
        // return n1;
    }

    double intersect(const Ray &r) { // returns distance, 0 if nohit
        
        // vec3 nl = r.dir.dot(normal) > 0 ? normal : normal * -1;
        // double dist = -r.origin.dot(nl) + center.dot(nl);
        // double tt = dist / r.dir.dot(nl);
        
        if (r.dir.dot(normal) == 0){
            return 0;
        }

        double dn = r.dir.dot(normal);
        if (normal == vec3(0,0,0)){             // triangle is degenerate
            return 0; 
        }

        // if (fabs(dn) < eps) {     // ray is  parallel to triangle plane
        //     // if (a == 0)                 // ray lies in triangle plane
        //     //     return 2;
        //     // else return 0;              // ray disjoint from plane
        //     return 0;
        // }

        vec3 center = (p1 + p2 + p3) / 3;
        double dist = -r.origin.dot(normal) + center.dot(normal);
        double tt = dist / dn;
        vec3 hit = r.origin + r.dir * tt;

        u = p2 - p1;
        v = p3 - p1;

         // is I inside T?
        double    uu, uv, vv, wu, wv, D;
        uu = u.dot(u);
        uv = u.dot(v);
        vv = v.dot(v);
        
        vec3 w = hit - p1;
        wu = w.dot(u);
        wv = w.dot(v);
        D = uv * uv - uu * vv;

        // get and test parametric coords
        double s, t;
        s = (uv * wv - vv * wu) / D;
        if (s < 0.0 || s > 1.0)         // I is outside T
            return 0;

        t = (uv * wu - uu * wv) / D;
        if (t < 0.0 || (s + t) > 1.0)  // I is outside T
            return 0;

        return tt;
    }
    
};

class Face
{

public:
    vec3 v1, v2, v3;
    vec3 n1, n2, n3;
    vec2 uv1, uv2, uv3;

    void setupUVs(const vec2 _uv1,const vec2 _uv2,const vec2 _uv3){
        uv1 = _uv1;
        uv2 = _uv2;
        uv3 = _uv3;
    }

    void setupVertices(const vec3 _v1,const vec3 _v2,const vec3 _v3){
        v1 = _v1;
        v2 = _v2;
        v3 = _v3;
    }

    void setupNormals(const vec3 _n1,const vec3 _n2,const vec3 _n3){
        n1 = _n1;
        n2 = _n2;
        n3 = _n3;
    }


    Face() {};
    ~Face() {};
    
};

class Mesh: public Object{

public:
    std::vector<Face*> faces;
    
    Mesh() {

    }

    ~Mesh(){
        for (uint32_t i = 0; i < faces.size(); ++i){
            delete faces[i];
        }
    }

    void addFace(Face* face){
        faces.push_back(face);
    }

    void translate(double tx, double ty, double tz){
        vec3 tr = vec3(tx, ty, tz);
        for (uint32_t i = 0; i < faces.size(); ++i){
            faces[i]->v1 = faces[i]->v1 + tr;
            faces[i]->v2 = faces[i]->v2 + tr;
            faces[i]->v3 = faces[i]->v3 + tr;
        }
    } 

    void rotateX(double rx){
        mat3 mt(1, 0,       0,
                0, cos(rx), -sin(rx), 
                0, sin(rx), cos(rx));

        for (uint32_t i = 0; i < faces.size(); ++i){
            faces[i]->v1 = mt * faces[i]->v1;
            faces[i]->v2 = mt * faces[i]->v2;
            faces[i]->v3 = mt * faces[i]->v3;

            faces[i]->n1 = mt * faces[i]->n1;
            faces[i]->n2 = mt * faces[i]->n2;
            faces[i]->n3 = mt * faces[i]->n3;
        }
    }

    void rotateY(double rx){
        mat3 mt(cos(rx), 0, -sin(rx), 
                0,       1, 0, 
                sin(rx), 0, cos(rx));

        for (uint32_t i = 0; i < faces.size(); ++i){
            faces[i]->v1 = mt * faces[i]->v1;
            faces[i]->v2 = mt * faces[i]->v2;
            faces[i]->v3 = mt * faces[i]->v3;

            faces[i]->n1 = mt * faces[i]->n1;
            faces[i]->n2 = mt * faces[i]->n2;
            faces[i]->n3 = mt * faces[i]->n3;
        }
    }

    void rotateZ(double rx){
        mat3 mt(cos(rx), -sin(rx), 0,
                sin(rx), cos(rx), 0,
                0, 0,       1 );

        for (uint32_t i = 0; i < faces.size(); ++i){
            faces[i]->v1 = mt * faces[i]->v1;
            faces[i]->v2 = mt * faces[i]->v2;
            faces[i]->v3 = mt * faces[i]->v3;

            faces[i]->n1 = mt * faces[i]->n1;
            faces[i]->n2 = mt * faces[i]->n2;
            faces[i]->n3 = mt * faces[i]->n3;
        }
    }

    void scale(double sx, double sy, double sz){
        mat3 mt(sx, 0, 0, 
            0, sy, 0, 
            0, 0, sz );
        
        for (uint32_t i = 0; i < faces.size(); ++i){
            faces[i]->v1 = mt * faces[i]->v1;
            faces[i]->v2 = mt * faces[i]->v2;
            faces[i]->v3 = mt * faces[i]->v3;
        }
    }
    
};