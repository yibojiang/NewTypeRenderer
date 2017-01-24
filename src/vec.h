#pragma once
#include <math.h>
#include <QDebug>
#include <iostream>
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
    // vec2& operator=(const vec2 &b) { this.x = b.x; this.y = b.y; return *this;}
    friend QDebug operator<< (QDebug stream, const vec2 &p) {
        stream << p.x << ',' << p.y;
        return stream;
    }
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
    vec3 reflect(vec3 &normal) const { return *this - normal * (dot(normal) * 2); }
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
    vec4( vec3 _v, double _w) { x = _v.x; y = _v.y; z = _v.z; w = _w == 0.0 ? 0.0 : 1.0;}
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

    friend vec3 operator*(mat3 a, const vec3& b){
        vec3 c;
        c.x = b.x * a.cols[0][0] + b.y * a.cols[0][1] + b.z * a.cols[0][2];
        c.y = b.x * a.cols[1][0] + b.y * a.cols[1][1] + b.z * a.cols[1][2];
        c.z = b.x * a.cols[2][0] + b.y * a.cols[2][1] + b.z * a.cols[2][2];

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
    mat4(){
        cols[0][0] = 1; cols[1][0] = 0; cols[2][0] = 0; cols[3][0] = 0; 
        cols[0][1] = 0; cols[1][1] = 1; cols[2][1] = 0; cols[3][1] = 0; 
        cols[0][2] = 0; cols[1][2] = 0; cols[2][2] = 1; cols[3][2] = 0; 
        cols[0][3] = 0; cols[1][3] = 0; cols[2][3] = 0; cols[3][3] = 1;   
    }
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

    friend vec3 operator*(mat4 a, const vec3& b){
        vec3 c;
        c.x = b.x * a.cols[0][0] + b.y * a.cols[0][1] + b.z * a.cols[0][2];
        c.y = b.x * a.cols[1][0] + b.y * a.cols[1][1] + b.z * a.cols[1][2];
        c.z = b.x * a.cols[2][0] + b.y * a.cols[2][1] + b.z * a.cols[2][2];
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

struct Quaternion{
    double x, y, z, w;
    Quaternion() :x(0), y(0), z(0), w(1) {}
    Quaternion(double qx, double qy, double qz, double qw) 
        :x(qx), y(qy), z(qz), w(qw){}
    
    // Axis had to be normalized.
    Quaternion(const vec3 axis, const double angle){
      double half_angle = angle / 2.0;
      x = axis.x * sin(half_angle);
      y = axis.y * sin(half_angle);
      z = axis.z * sin(half_angle);
      w = cos(half_angle);
    }

    Quaternion(double roll, double pitch, double yaw){
        double t0 = cos(yaw * 0.5f);
        double t1 = sin(yaw * 0.5f);
        double t2 = cos(roll * 0.5f);
        double t3 = sin(roll * 0.5f);
        double t4 = cos(pitch * 0.5f);
        double t5 = sin(pitch * 0.5f);
        w = t0 * t2 * t4 + t1 * t3 * t5;
        x = t0 * t3 * t4 - t1 * t2 * t5;
        y = t0 * t2 * t5 + t1 * t3 * t4;
        z = t1 * t2 * t4 - t0 * t3 * t5;
    }

    friend Quaternion operator*(Quaternion q1, const Quaternion& q2){
        Quaternion qr;
        qr.x = (q1.w * q2.x) + (q1.x * q2.w) + (q1.y * q2.z) - (q1.z * q2.y);
        qr.y = (q1.w * q2.y) - (q1.x * q2.z) + (q1.y * q2.w) + (q1.z * q2.x);
        qr.z = (q1.w * q2.z) + (q1.x * q2.y) - (q1.y * q2.x) + (q1.z * q2.w);
        qr.w = (q1.w * q2.w) - (q1.x * q2.x) - (q1.y * q2.y) - (q1.z * q2.z);
        return qr;
    }

    void toEulerAngle(double& roll, double& pitch, double& yaw)
    {
        double ysqr = y * y;

        // roll (x-axis rotation)
        double t0 = +2.0f * (w * x + y * z);
        double t1 = +1.0f - 2.0f * (x * x + ysqr);
        roll = atan2(t0, t1);

        // pitch (y-axis rotation)
        double t2 = +2.0f * (w * y - z * x);
        t2 = t2 > 1.0f ? 1.0f : t2;
        t2 = t2 < -1.0f ? -1.0f : t2;
        pitch = asin(t2);

        // yaw (z-axis rotation)
        double t3 = +2.0f * (w * z + x *y);
        double t4 = +1.0f - 2.0f * (ysqr + z * z);  
        yaw = atan2(t3, t4);
    }

    mat4 toMatrix(){
        return mat4(
        1 - 2*y*y - 2*z*z,     2*x*y - 2*z*w,       2*x*z + 2*y*w,       0,
        2*x*y + 2*z*w,         1 - 2*x*x - 2*z*z,   2*y*z - 2*x*w,       0,
        2*x*z - 2*y*w,         2*y*z + 2*x*w,       1 - 2*x*x - 2*y*y,   0,
        0,                     0,                   0,                   1);
    }

    friend QDebug operator<< (QDebug stream, const Quaternion &a) {
        stream << a.x << ',' << a.y << ',' << a.z << ',' << a.w;
        return stream;
    }
};