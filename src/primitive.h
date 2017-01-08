#pragma once
#include "transform.h"
#include "BVH.h"



struct Ray {
    vec3 origin;
    vec3 dir;
    Ray(vec3 ro) { origin = ro; dir = vec3(1,0,0); }
    Ray(vec3 ro, vec3 rd){ origin = ro; dir = rd; }
};


enum Refl_t { DIFF, SPEC, REFR };  // material types, used in radiance()

class Object {
protected:

    vec3 emission;
    vec3 color;
    Refl_t refl; // reflection type (DIFFuse, SPECular, REFRactive)
    Extents bounds;

public:
    std::string name;
    bool isMesh;
    Object(){}
    virtual ~Object(){}
    virtual vec3 getNormal(const vec3 &) const{ return vec3(1);}
    virtual double intersect(const Ray &){ return 0;}

    virtual vec3 getDiffuse() const{
        return color;
    }

    virtual vec3 getEmission() const{
        return emission;
    }

    virtual Refl_t getReflectionType() const{
        return refl;
    }

    virtual void updateTransformMatrix(const mat4&){

    }

    virtual void computebounds(){
        

    }

    Extents getBounds(){
        return bounds;
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
        emission = _e;
        color = _c;
        refl = _refl;
        isMesh = false;
    }

    Plane(vec3 _nor, double _off) { off = _off; normal = _nor; }
    double intersect(const Ray &r) {
        return (-r.origin.dot(normal) - off) / (normal).dot(r.dir);
    }

    vec3 getNormal(const vec3 &) const{
        return normal;
    }

    void updateTransformMatrix(const mat4& m){
        vec4 normalDir = vec4(normal, 0);
        normalDir = m * normalDir;
        normalDir.normalize();
        normal = vec3(normalDir.x, normalDir.y, normalDir.z);

        vec4 origin = vec4(normal*off, 1);
        origin = m * origin;
        vec3 origin3 = vec3(origin.x, origin.y, origin.z);
        off = origin3.dot(normal);
    }

    // virtual vec3 debug(vec3 _pos) const{return vec3(1);}
};



class Sphere: public Object {
public:
    double rad;
    vec3 center;

    Sphere(double _rad, vec3 _e, vec3 _c, Refl_t _refl){
        rad = _rad;
        emission = _e;
        color = _c;
        refl = _refl;
        isMesh = false;
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

    void updateTransformMatrix(const mat4& m){
        vec4 pos = vec4(center, 1);
        pos = m * pos;
        center = vec3(pos.x, pos.y, pos.z);
    }

    void computebounds(){
        // bounds = new Extents();
        for (uint8_t i = 0; i < SLABCOUNT; ++i){
            vec3 slabN = BVH::normals[i];
            double d = center.dot(slabN);
            bounds.dnear[i] = -rad - d;
            bounds.dfar[i] = rad - d;
        }
    }

    
    // virtual vec3 debug(vec3 _pos) const{return vec3(1);}
};


class Box: public Object{ 
private:
    double dnear[3];
    double dfar[3];
public: 
    vec3 p[8]; 
    vec3 center;
    vec3 size;
    vec3 normals[3];

    Box(const vec3 &_size, vec3 _e, vec3 _c, Refl_t _refl) { 
        size = _size;
        

        emission = _e;
        color = _c;
        refl = _refl;

        normals[0] = vec3(1, 0, 0);
        normals[1] = vec3(0, 1, 0);
        normals[2] = vec3(0, 0, 1);

        vec3 half_size = size * 0.5;
        p[0] = center + vec3(half_size.x, half_size.y, half_size.z);
        p[1] = center + vec3(-half_size.x, half_size.y, half_size.z);
        p[2] = center + vec3(half_size.x, -half_size.y, half_size.z);
        p[3] = center + vec3(half_size.x, half_size.y, -half_size.z);
        p[4] = center + vec3(-half_size.x, -half_size.y, half_size.z);
        p[5] = center + vec3(half_size.x, -half_size.y, -half_size.z);
        p[6] = center + vec3(-half_size.x, half_size.y, -half_size.z);
        p[7] = center + vec3(-half_size.x, -half_size.y, -half_size.z);
        updatePlane();

        isMesh = false;
        
    }

    void updatePlane(){
        
        for (int i = 0; i < 3; ++i){
            dnear[i] = inf;
            dfar[i] = -inf;
            
            for (int j = 0; j < 8; ++j){
                double d = -p[j].dot(normals[i]);
                if (d > dfar[i]){
                    dfar[i] = d;
                }

                if (d < dnear[i]){
                    dnear[i] = d;
                }
            }

        }
    }

    // AABB
    // double intersect(const Ray &r) { // returns distance, 0 if nohit
    //     vec3 invdir(1.0 / r.dir.x, 1.0 / r.dir.y, 1.0 / r.dir.z);
        
    //     // lb is the corner of AABB with minimal coordinates - left bottom, rt is maximal corner
    //     // r.org is origin of ray
    //     double t1 = (bounds[0].x - r.origin.x) * invdir.x;
    //     double t2 = (bounds[1].x - r.origin.x) * invdir.x;
    //     double t3 = (bounds[0].y - r.origin.y) * invdir.y;
    //     double t4 = (bounds[1].y - r.origin.y) * invdir.y;
    //     double t5 = (bounds[0].z - r.origin.z) * invdir.z;
    //     double t6 = (bounds[1].z - r.origin.z) * invdir.z;

    //     double tmin = fmax(fmax(fmin(t1, t2), fmin(t3, t4)), fmin(t5, t6)); // max tnear 
    //     double tmax = fmin(fmin(fmax(t1, t2), fmax(t3, t4)), fmax(t5, t6)); // min tfar

    //     // if tmax < 0, ray (line) is intersecting AABB, but whole AABB is behing us
    //     if (tmax < 0){
    //         return 0;
    //     }

    //     // if tmin > tmax, ray doesn't intersect AABB
    //     if (tmin > tmax){
    //         return 0;
    //     }
    //     return tmin;
    // } 


    double intersect(const Ray &r) { // returns distance, 0 if nohit
        double t1 = (-dnear[0] - r.origin.dot(normals[0])) / r.dir.dot(normals[0]);
        double t2 = (-dfar[0] - r.origin.dot(normals[0])) / r.dir.dot(normals[0]);
        double t3 = (-dnear[1] - r.origin.dot(normals[1])) / r.dir.dot(normals[1]);
        double t4 = (-dfar[1] - r.origin.dot(normals[1])) / r.dir.dot(normals[1]);
        double t5 = (-dnear[2] - r.origin.dot(normals[2])) / r.dir.dot(normals[2]);
        double t6 = (-dfar[2] - r.origin.dot(normals[2])) / r.dir.dot(normals[2]);
        double tmin = fmax(fmax(fmin(t1, t2), fmin(t3, t4)), fmin(t5, t6)); // max t near 
        double tmax = fmin(fmin(fmax(t1, t2), fmax(t3, t4)), fmax(t5, t6)); // min t far
        // if tmax < 0, ray (line) is intersecting AABB, but whole AABB is behing us
        if (tmax < 0){
            return 0;
        }

        // if tmin > tmax, ray doesn't intersect AABB
        if (tmin > tmax){
            return 0;
        }

        return tmin;
    } 
    
    vec3 getNormal(const vec3 &_pos) const{
        for (int i = 0; i < 3; ++i){
            if (fabs((_pos.dot(normals[i]) + dnear[i])) < eps){
                return normals[i];
            }

            if (fabs((_pos.dot(normals[i]) + dfar[i])) < eps){
                return -normals[i];
            }

            if (fabs((_pos.dot(-normals[i]) + dnear[i])) < eps){
                return -normals[i];
            }

            if (fabs((_pos.dot(-normals[i]) + dfar[i])) < eps){
                return normals[i];
            }
        }
        return vec3();
    }

    void updateTransformMatrix(const mat4& m){
        for (int i = 0; i < 3; ++i){
            // vec4 n = m*  vec4(normals[i], 0);
            normals[i] = m * normals[i];   
        }

        for (int i = 0; i < 8; ++i){
            vec4 p4 = vec4(p[i], 1);
            p4 = m * p4;
            p[i] = vec3(p4.x, p4.y, p4.z);
        }

        // size = m * size;
        vec4 center4 = m * vec4(center, 1);
        center = vec3(center4.x, center4.y, center4.z);
        updatePlane();
    }

    void computebounds(){

        for (uint8_t i = 0; i < SLABCOUNT; ++i){
            bounds.dnear[i] = inf;
            bounds.dfar[i] = -inf;
            for (int j = 0; j < 8; ++j){
                // qDebug()<<p[j];
                double d =  -p[j].dot(BVH::normals[i]);
                if (d < bounds.dnear[i]){
                    bounds.dnear[i] = d;
                }
                if (d > bounds.dfar[i]){
                    bounds.dfar[i] = d;
                }
            }
        }
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
        emission = _e;
        color = _c;
        refl = _refl;
        u = p2 - p1;
        v = p3 - p1;
        normal = u.cross(v).normalize();              // cross product
        isMesh = false;
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

    void computebounds(){
        for (uint8_t i = 0; i < SLABCOUNT; ++i){
            vec3 slabN = BVH::normals[i];
            double d1 =  -p1.dot(slabN);
            double d2 =  -p2.dot(slabN);
            double d3 =  -p3.dot(slabN);
            bounds.dnear[i] = fmin(d1, fmin(d2, d3));
            bounds.dfar[i] = fmax(d1, fmax(d2, d3));
        }
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
        isMesh = true;
    }

    ~Mesh(){
        for (uint32_t i = 0; i < faces.size(); ++i){
            delete faces[i];
        }
    }

    void updateTransformMatrix(const mat4& m){
        // qDebug()<<"update mesh matrix";
        for (uint32_t i = 0; i < faces.size(); ++i){
            vec4 v1(faces[i]->v1, 1.0);
            vec4 v2(faces[i]->v2, 1.0);
            vec4 v3(faces[i]->v3, 1.0);
            // qDebug() << v1 << v2 << v3;
            v1 = m * v1;
            v2 = m * v2;
            v3 = m * v3;
            // qDebug() << "after: " << v1 << v2 << v3;
            faces[i]->v1 = vec3(v1.x, v1.y, v1.z);
            faces[i]->v2 = vec3(v2.x, v2.y, v2.z);
            faces[i]->v3 = vec3(v3.x, v3.y, v3.z);

        }
    }

    void addFace(Face* face){
        faces.push_back(face);
    }

};