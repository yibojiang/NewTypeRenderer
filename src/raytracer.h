#pragma once

#include <math.h>   
#include <iostream>
// #include <fstream>
#include <vector>
#include "sys/time.h"
#include <QImage>
#include <QDebug>
#include <QCoreApplication>
#include <QDir>


const double inf=1e9;
const double eps=1e-6;



struct vec2 {
    double x, y;
    vec2(double _x=0, double _y=0){ x = _x; y = _y; }
    vec2 operator+(const vec2 &b) const{ return vec2(x + b.x, y + b.y); }
    vec2 operator-(const vec2 &b) const{ return vec2(x - b.x, y - b.y); }
    vec2 operator*(const vec2 &b) const{ return vec2(x * b.x, y * b.y); }
    vec2 operator*(const double b) const{ return vec2(x * b, y * b); }
    vec2 operator/(const double b) const{ return vec2(x / b, y / b); }
    double dot(const vec2 &b) const { return x * b.x + y * b.y; }
    double length() const { return sqrt(x * x + y * y); }
    vec2& normalize() { return *this = *this/length(); }
    vec2 normalized() { return vec2(*this/length()); }
    vec2 reflect(vec2 &normal) const { return normal * (dot(normal) * 2) - *this; }
};

struct vec3 {
    double x, y, z;
    // vec3(vec4 _v) { x = _v.x; y = _v.y; z = _v.z; }
    vec3(double _x=0, double _y=0, double _z=0) { x = _x; y =_y; z = _z; }
    vec3 operator+(const vec3 &b) const { return vec3(x + b.x, y + b.y, z + b.z); }
    vec3 operator-(const vec3 &b) const { return vec3(x - b.x, y - b.y, z - b.z); }
    vec3 operator*(const vec3 &b) const { return vec3(x * b.x, y * b.y, z * b.z ); }
    vec3 operator*(const double b) const { return vec3(x * b, y * b, z * b ); }
    vec3 operator/(const double b) const { return vec3(x / b, y / b, z / b ); }
    bool operator==(const vec3 &b) const { return (x == b.x && y == b.y && z == b.z); }
    vec3 operator-() const { return vec3(-x, -y, -z); }
    // vec3 mult(const vec3 &b) const { return vec3(x * b.x, y * b.y, z * b.z ); }
    double dot(const vec3 &b) const { return x * b.x + y * b.y + z * b.z; }
    double length() const{ return sqrt(x * x + y * y + z * z); }
    vec3& normalize(){ return *this = *this/length(); }
    vec3 normalized() { return vec3(*this/length()); }
    vec3 cross(vec3 &b) const { return vec3(y * b.z - z * b.y, z * b.x - x * b.z, x * b.y - y * b.x); }
    vec3 reflect(vec3 &normal) const { return normal * (dot(normal) * 2) - *this; }
    friend std::ostream &operator<< (std::ostream &stream, const vec3 &v) {
        stream << '(' << v.x << ',' << v.y << ',' << v.z << ')';
        return stream;
    }

    friend QDebug operator<< (QDebug stream, const vec3 &p) {
        stream << p.x << ',' << p.y << ',' << p.z;
        return stream;
    }
};

struct vec4 {
    double x, y, z, w;
    vec4( double _x=0, double _y=0, double _z=0) { x = _x; y = _y; z = _z; w = 1.0; }
    vec4( vec3 _v, double w) { x = _v.x; y = _v.y; z = _v.z; w = 1.0; }
    vec4( double _x, double _y, double _z, double _w) { x = _x; y = _y; z = _z; w = _w == 0.0 ? 0.0 : 1.0; }
    vec4 operator+(const vec4 &b) const { return vec4(x + b.x, y + b.y, z + b.z); }
    vec4 operator-(const vec4 &b) const { return vec4(x - b.x, y - b.y, z - b.z); }
    vec4 operator*(const double b) const { return vec4(x * b, y * b, z * b); }
    vec4 operator/(const double b) const { return vec4(x / b, y / b, z / b); }
    double dot(const vec4 &b) const { return x * b.x + y * b.y + z * b.z; }
    double length() const { return sqrt(x * x + y * y + z * z); }
    vec4& normalize() { return *this = *this/length(); }
    vec4 normalized() { return vec4(*this/length()); }
    vec4 cross(vec4 &b) const { return vec4(y * b.z - z * b.y, z * b.x - x * b.z, x * b.y - y * b.x); }
    vec4 reflect(vec4 &normal) const { return normal * (dot(normal) * 2) - *this; }
    friend QDebug operator<< (QDebug stream, const vec4 &p) {
        stream << p.x << ',' << p.y << ',' << p.z << ',' << p.w;
        return stream;
    }
};


struct mat3{
    double cols[3][3];
    mat3(){}
    mat3(
        const double &m00, const double &m10, const double &m20, 
        const double &m01, const double &m11, const double &m21, 
        const double &m02, const double &m12, const double &m22
    ){
        cols[0][0] = m00; cols[1][0] = m01; cols[2][0] = m02; 
        cols[0][1] = m10; cols[1][1] = m11; cols[2][1] = m12; 
        cols[0][2] = m20; cols[1][2] = m21; cols[2][2] = m22; 
    }

    mat3(const vec3 &uu, const vec3 &vv, const vec3 &ww){

        cols[0][0] = uu.x;
        cols[1][0] = uu.y;
        cols[2][0] = uu.z;

        cols[0][1] = vv.x;
        cols[1][1] = vv.y;
        cols[2][1] = vv.z;

        cols[0][2] = ww.x;
        cols[1][2] = ww.y;
        cols[2][2] = ww.z;
    }

    vec3 operator*(vec3 &b){
        vec3 c;
        c.x = b.x * cols[0][0] + b.y * cols[0][1] + b.z * cols[0][2];
        c.y = b.x * cols[1][0] + b.y * cols[1][1] + b.z * cols[1][2];
        c.z = b.x * cols[2][0] + b.y * cols[2][1] + b.z * cols[2][2];

        return c;
    }

    mat3 operator*(mat3 &b){
        mat3 c;
        for (int i = 0; i < 3; ++i){
            for (int j = 0; j < 3; ++j){
                double sum(0);
                for (int k = 0; k < 3; ++k){
                    sum+= cols[i][j] *  b.cols[k][j]; 
                }

                c.cols[i][j] = sum;
            }
        }
        return c;
    }

    friend std::ostream &operator<< (std::ostream &stream, const mat3 &a) {
        stream << '[';
        for (int i = 0; i < 3; ++i){
            for (int j = 0; j < 3; ++j){
                stream << a.cols[i][j] << ((i * j == 4)? ']' : ',');
            }
            stream << std::endl;
        }
        return stream;
    }
};

struct mat4{
  double cols[4][4];
    mat4(){}
    mat4(
        const double &m00, const double &m10, const double &m20, const double &m30, 
        const double &m01, const double &m11, const double &m21, const double &m31, 
        const double &m02, const double &m12, const double &m22, const double &m32, 
        const double &m03, const double &m13, const double &m23, const double &m33
    ){
        cols[0][0] = m00; cols[1][0] = m01; cols[2][0] = m02; cols[3][0] = m03; 
        cols[0][1] = m10; cols[1][1] = m11; cols[2][1] = m12; cols[3][1] = m13; 
        cols[0][2] = m20; cols[1][2] = m21; cols[2][2] = m22; cols[3][2] = m23; 
        cols[0][3] = m30; cols[1][3] = m31; cols[2][3] = m32; cols[3][3] = m33; 
    }


    // vec4 operator*(vec4 &b){
    friend vec4 operator*(mat4 a, const vec4& b){
        vec4 c;
        c.x = b.x * a.cols[0][0] + b.y * a.cols[0][1] + b.z * a.cols[0][2] + b.w * a.cols[0][3];
        c.y = b.x * a.cols[1][0] + b.y * a.cols[1][1] + b.z * a.cols[1][2] + b.w * a.cols[1][3];
        c.z = b.x * a.cols[2][0] + b.y * a.cols[2][1] + b.z * a.cols[2][2] + b.w * a.cols[2][3];
        c.w = b.x * a.cols[3][0] + b.y * a.cols[3][1] + b.z * a.cols[3][2] + b.w * a.cols[3][3];
        
        return c;
    }

    friend mat4 operator+(mat4 a, const mat4& b){
        mat4 c;
        for (int i = 0; i < 4; ++i){
            for (int j = 0; j < 4; ++j){
                c.cols[i][j] = a.cols[i][j] + b.cols[i][j];
            }
        }
        return c;
    }

    // mat4 operator*(mat4 &b){
    friend mat4 operator*(mat4 a, const mat4& b){
        mat4 c;
        for (int i = 0; i < 4; ++i){
            for (int j = 0; j < 4; ++j){
                double sum(0);
                for (int k = 0; k < 4; ++k){
                    sum+= a.cols[i][k] *  b.cols[k][j]; 
                }

                c.cols[i][j] = sum;
            }
        }
        return c;
    }

    // mat4& reverse() {
    //     return *this;
    // }

    friend std::ostream &operator<< (std::ostream &stream, const mat4 &a) {
        stream << '[';
        for (int i = 0; i < 4; ++i){
            for (int j = 0; j < 4; ++j){
                stream << a.cols[i][j] << ((i * j == 9)? ']' : ',');
            }
            stream << std::endl;
        }
        return stream;
    }  

    friend QDebug operator<< (QDebug stream, const mat4 &a) {
        stream << '[';
        for (int i = 0; i < 4; ++i){
            for (int j = 0; j < 4; ++j){
                stream << a.cols[i][j] << ((i * j == 9)? ']' : ',');
            }
            stream << "\n";
        }
        return stream;
    }  
};



struct Ray {
    vec3 origin;
    vec3 dir;
    Ray(vec3 ro) { origin = ro; dir = vec3(1,0,0); }
    Ray(vec3 ro, vec3 rd){ origin = ro; dir = rd; }
};

enum Refl_t { DIFF, SPEC, REFR };  // material types, used in radiance()




struct Quaternion{
    double x, y, z, w;
    Quaternion() :x(0), y(0), z(0), w(1) {}
    Quaternion(double _x, double _y, double _z, double _w) 
        :x(_x), y(_y), z(_z), w(_w){}
    
    // Axis had to be normalized.
    Quaternion(const vec3 axis, const double angle){
      double half_angle = angle / 2.0;
      x = axis.x * sin(half_angle);
      y = axis.y * sin(half_angle);
      z = axis.z * sin(half_angle);
      w = cos(half_angle);
    }

    Quaternion invertQuaternion(const Quaternion &q){
      return Quaternion(-q.x, -q.y, -q.z, q.w);
    }

    friend Quaternion operator*(Quaternion q1, const Quaternion& q2){
        Quaternion qr;
        qr.x = (q1.w * q2.x) + (q1.x * q2.w) + (q1.y * q2.z) - (q1.z * q2.y);
        qr.y = (q1.w * q2.y) - (q1.x * q2.z) + (q1.y * q2.w) + (q1.z * q2.x);
        qr.z = (q1.w * q2.z) + (q1.x * q2.y) - (q1.y * q2.x) + (q1.z * q2.w);
        qr.w = (q1.w * q2.w) - (q1.x * q2.x) - (q1.y * q2.y) - (q1.z * q2.z);
        return qr;
    }

    mat4 toMatrix(){
        return mat4(
        1 - 2*y*y - 2*z*z,   2*x*y - 2*z*w,   2*x*z + 2*y*w,       0,
        2*x*y + 2*z*w,         1 - 2*x*x - 2*z*z,   2*y*z - 2*x*w,       0,
        2*x*z - 2*y*w,         2*y*z + 2*x*w,   1 - 2*x*x - 2*y*y,   0,
        0,                         0,                   0,                       1);
    }

    friend QDebug operator<< (QDebug stream, const Quaternion &a) {
        stream << a.x << ',' << a.y << ',' << a.z << ',' << a.w;
        return stream;
    }
                     
    // vec4 operator*(const vec4 &q2) const{ 
    //     vec4 qr;
    //     qr.x = (w * q2.x) + (x * q2.w) + (y * q2.z) - (z * q2.y);
    //     qr.y = (w * q2.y) - (x * q2.z) + (y * q2.w) + (z * q2.x);
    //     qr.z = (w * q2.z) + (x * q2.y) - (y * q2.x) + (z * q2.w);
    //     qr.w = (w * q2.w) - (x * q2.x) - (y * q2.y) - (z * q2.z);
    //     return qr;
    // }
};
// Quaternion::identity(0,0,0,1);
class Transform {
public:
    mat4 translate;
    Quaternion rotation;
    mat4 scale;

    mat4 getTransformMatrix(){
        // return vec3(position.x, position.y, position.z);
        return translate * scale * rotation.toMatrix();
    }

    void setTranslate(double tx, double ty, double tz){
        translate = mat4(1, 0, 0, tx,
                         0, 1, 0, ty,
                         0, 0, 1, tz,
                         0, 0, 0 , 1);
    }

    void setScale(double sx, double sy, double sz){
        scale = mat4(sx, 0,  0,  0,
                     0,  sy, 0,  0,
                     0,  0,  sz, 0,
                     0,  0,  0,  1);
    }

    void move(const double tx, const double ty, const double tz){
        mat4 mt(1, 0, 0, tx,
                0, 1, 0, ty,
                0, 0, 1, tz,
                0, 0, 0, 1);
        translate = mt * translate;
    }

    void setRotation(Quaternion q){
        rotation = q;
    }

    void rotateX(double rx){
        // mat4 mt(1, 0,       0,          0,
        //         0, cos(rx), -sin(rx),   0, 
        //         0, sin(rx), cos(rx),    0, 
        //         0, 0,       0,          0); 
        Quaternion q(vec3(1, 0, 0), rx);
        rotation = q * rotation;
    }

    void rotateY(double ry){
        // mat4 mt(cos(ry), 0,  -sin(ry),  0,
        //         0,       1,  0,         0,
        //         sin(ry), 0,  cos(ry),   0,
        //         0,       0,  0,         0);
        Quaternion q(vec3(0, -1, 0), ry);
        rotation = q * rotation;
    }

    void rotateZ(double rz){
        // mat4 mt(cos(rz), -sin(rz), 0,   0,
        //         sin(rz), cos(rz),  0,   0,
        //         0,       0,        1,   0,
        //         0,       0,        0,   1);
        Quaternion q(vec3(0, 0, 1), rz);
        rotation = q * rotation;
    }

    Transform() {
        translate = mat4 (1, 0, 0, 0,
                        0, 1, 0, 0,
                        0, 0, 1, 0,
                        0, 0, 0, 1);
        rotation = Quaternion(0, 0, 0, 1);
        scale = mat4(1, 0,  0,  0,
                     0,  1, 0,  0,
                     0,  0,  1, 0,
                     0,  0,  0,  1);
        
        
    }
    ~Transform() {}
};

class Object {
protected:
    vec3 p; 
    vec3 e;
    vec3 c; // position, emission, color
    Refl_t refl; // reflection type (DIFFuse, SPECular, REFRactive)
public:
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

class Intersection {
    public:
    Intersection() { t = inf; object = nullptr; }
    Intersection(double t_, Object* object_) { t = t_; object = object_; }
    operator bool() { return object != nullptr; }
    double t;
    Object* object;
};


class Face
{

public:
    // std::vector<vec3*> vertices;
    // std::vector<vec3*> normals;
    // std::vector<unsigned int*> faces;
    // std::vector<unsigned int*> uvs;
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


class ObjLoader
{
public:

    ObjLoader(){};
    ~ObjLoader(){};

    bool loadObj(std::string _name, Mesh *mesh){
        // QString path = QCoreApplication::applicationDirPath();
        QString path = QDir::currentPath();
        _name = '/' + _name;
        std::string fullpath = path.toUtf8().constData() + _name;
        qDebug() << "open" << fullpath.c_str();

        // return true;
        // QByteArray ba = path.toLatin1();
        // const char *c_str2 = ba.data();
        FILE *file = fopen( fullpath.c_str(), "r");
        if( file == NULL ){
            qDebug() << "Impossible to open the file !\n" << fullpath.c_str();
            return false;
        }

        std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
        std::vector<vec3> temp_vertices;
        std::vector<vec2> temp_uvs;
        std::vector<vec3> temp_normals;

        while(1){
            
            char lineHeader[128];

            // read the first word of the line
            int res = fscanf(file, "%s", lineHeader);
            // qDebug() << res;
            if (res == EOF){
                // qDebug() << "break";   
                break; // EOF = End Of File. Quit the loop.
            }
            // qDebug() << res;
            // else : parse lineHeader
            if ( strcmp( lineHeader, "v" ) == 0 ){
                vec3 vertex;
                fscanf(file, "%lf %lf %lf\n", &vertex.x, &vertex.y, &vertex.z );
                // qDebug() << "vertex: " << vertex.x << ',' << vertex.y << ',' << vertex.z;
                temp_vertices.push_back(vertex);

            }
            else if ( strcmp( lineHeader, "vt" ) == 0 ){
                vec2 uv;
                fscanf(file, "%lf %lf\n", &uv.x, &uv.y );
                temp_uvs.push_back(uv);

            }
            else if ( strcmp( lineHeader, "vn" ) == 0 ){
                vec3 normal;
                fscanf(file, "%lf %lf %lf\n", &normal.x, &normal.y, &normal.z );
                temp_normals.push_back(normal);

            }
            else if ( strcmp( lineHeader, "f" ) == 0 ){
                std::string vertex1, vertex2, vertex3;
                unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
                int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
                if (matches != 9){
                    qDebug() << "9 File can't be read by our simple parser : ( Try exporting with other options\n";
                    return false;
                }
                else{
                    Face *face = new Face();
                    face -> setupVertices(temp_vertices[vertexIndex[0]-1], temp_vertices[vertexIndex[1]-1], temp_vertices[vertexIndex[2]-1]);
                    face -> setupUVs(temp_uvs[uvIndex[0]-1], temp_uvs[uvIndex[1]-1], temp_uvs[uvIndex[2]-1]);
                    face -> setupNormals(temp_normals[normalIndex[0]-1], temp_normals[normalIndex[1]-1], temp_normals[normalIndex[2]-1]);    
                    mesh->addFace(face);
                }


                // int matches = fscanf(file, "%d %d %d\n", &vertexIndex[0], &vertexIndex[1], &vertexIndex[2]);
                // if (matches != 3){
                //     qDebug() << matches <<" File can't be read by our simple parser : ( Try exporting with other options\n";
                //     return false;
                // }
                // else{
                //     Face *face = new Face();
                //     face -> setupVertices(temp_vertices[vertexIndex[0]-1], temp_vertices[vertexIndex[1]-1], temp_vertices[vertexIndex[2]-1]);    
                //     mesh->addFace(face);
                // }
                
                
             
            }
        }


        // delete file;
        return true;
    }
    
};

class Scene
{
public:
    std::vector<Object*> objects;
    Scene(){}
    void add(Object* object) {
        objects.push_back(object);
    }

    void addMesh(Mesh* mesh) {
        // for (uint32_t i = 0; i < mesh->triangles.size(); ++i) {
        //     add((Object*)mesh->triangles[i]);
        // }

        for (uint32_t i = 0; i < mesh->faces.size(); ++i) {
            Triangle *triangle = new Triangle(mesh->faces[i]->v1, mesh->faces[i]->v2, mesh->faces[i]->v3);
            add((Object*)triangle);
        }
    }

    Intersection intersect(const Ray& ray) const {
        Intersection closestIntersection;
        // intersect all objects, one after the other
        // for (std::vector<Object*>::iterator it = objects.begin(); it != objects.end(); ++it){
        for (uint32_t i = 0; i < objects.size(); ++i) {
            double t = objects[i]->intersect(ray);
            // std::cout << t << std::endl;
            // double t = (*it)->intersect(ray);
            if (t > eps && t < closestIntersection.t) {
                closestIntersection.t = t;
                closestIntersection.object = objects[i];
            }
        }
        return closestIntersection;
    }
    ~Scene(){
        // printf("destroy scene\n");
        for (uint32_t i = 0; i < objects.size(); ++i) {
            delete objects[i];
        }
    }

    mat3 ca;
    vec3 ro;
    vec3 ta;
    float fov;
    float near;    
};



class Raytracer{
public:
    Scene scene;
    int samples;
    Raytracer(unsigned width, unsigned height,int _samples);
    ~Raytracer();
    vec3 tracing(const Ray &ray, int depth, unsigned short *Xi);
    vec3 render_pixel(unsigned short i, unsigned short j, unsigned short *Xi);
    QImage render(double &time) ;
    void renderDirect(double &time, QImage &directImage, QImage &normalImage);
    unsigned short width;
    unsigned short height;
};
