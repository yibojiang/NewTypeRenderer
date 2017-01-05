

// #ifndef RAYTRACER_H
#include "raytracer.h"
// #endif



inline mat3 setCamera( vec3 ro, vec3 ta, float cr )
{
  vec3 cw = (ta - ro).normalized();
  vec3 cp = vec3(sin(cr), cos(cr), 0.0);
  vec3 cu = cw.cross(cp).normalized();
  vec3 cv = cu.cross(cw).normalized();
  return mat3( cu, cv, cw );
}

inline vec3 abs(vec3 &v){
    return vec3(fabs(v.x), fabs(v.y), fabs(v.z));
}

inline double clamp(double v, double a, double b){
    if (v < a) return a;
    if (v > b) return b;
    return v;
}

inline vec3 clamp(vec3 &v, vec3 &a, vec3 &b){
    return vec3(clamp(v.x, a.x, b.x), clamp(v.y, a.y, b.y), clamp(v.z, a.z, b.z));
}

inline vec3 clamp(vec3 &v, vec3 a, vec3 b){ return vec3(clamp(v.x, a.x, b.x), clamp(v.y, a.y, b.y), clamp(v.z, a.z, b.z)); }

inline void ons(const vec3& v1, vec3& v2, vec3& v3) {
    if (fabs(v1.x) > fabs(v1.y)) {
        // project to the y = 0 plane and construct a normalized orthogonal vector in this plane
        float invLen = 1.f / sqrtf(v1.x * v1.x + v1.z * v1.z);
        v2 = vec3(-v1.z * invLen, 0.0f, v1.x * invLen);
    } else {
        // project to the x = 0 plane and construct a normalized orthogonal vector in this plane
        float invLen = 1.0f / sqrtf(v1.y * v1.y + v1.z * v1.z);
        v2 = vec3(0.0f, v1.z * invLen, -v1.y * invLen);
    }
    v3 = v1.cross(v2);
}


inline vec3 cosineSampleHemisphere(double u1, double u2){
    const double r = sqrt(u1);
    const double theta = 2 * M_PI * u2;
    const double x = r * cos(theta);
    const double y = r * sin(theta);
    return vec3(x, y, sqrt(fmax(0.0f, 1 - u1)));
}

inline vec3 uniformSampleHemisphere(double u1, double u2){
    const double r = sqrt(1.0f - u1 * u1);
    const double phi = 2 * M_PI * u2;
    return vec3(cos(phi) * r, sin(phi) * r, u1);
}

inline vec3 gammaCorrect(vec3 &v) { 
    return vec3(pow(v.x, 1 / 2.2), pow(v.y, 1 / 2.2), pow(v.z, 1 / 2.2)); 
}

inline double clamp(double x) { return x < 0 ? 0 : x > 1 ? 1 : x; }


// vec3 Raytracer::tracingNormal(const Ray &ray){
//     Intersection intersection = scene.intersect(ray);
//     if (!intersection.object) return vec3(0);
//     const Object &obj = *intersection.object;
//     vec3 hit = ray.origin + ray.dir * intersection.t;
//     vec3 N = obj.getNormal(hit);
//     return vec3((N.x + 1)*0.5, (N.y + 1)*0.5, (N.z+1) * 0.25 + 0.5 );
// }

vec3 Raytracer::tracing(const Ray &ray, int depth, unsigned short *Xi){
   
    Intersection intersection = scene.intersect(ray);
    if (!intersection.object) return vec3(0);

    const Object &obj = *intersection.object;
    vec3 hit = ray.origin + ray.dir * intersection.t;
    vec3 N = obj.getNormal(hit);
    // vec3 nl = N.dot(ray.dir) < 0 ? N: N * -1;
    vec3 f = obj.getDiffuse();
    
     // X: -1 to +1 :  Red: 0 to 255
     //  Y: -1 to +1 :  Green: 0 to 255
     //  Z: 0 to -1 :  Blue: 128 to 255
    
    // return vec3((N.x + 1)*0.5, (N.y + 1)*0.5, (N.z+1) * 0.25 + 0.5 );
    // return f;
    // Russian roulette: starting at depth 5, each recursive step will stop with a probability of 0.1
    if (++depth > 5){
        // double rrStopProbability = f.x > f.y && f.x > f.z ? f.x : f.y > f.z ? f.y : f.z; // max refl
        const double rrStopProbability = 0.1;
        if (erand48(Xi) < rrStopProbability){
            f = f * (1 / rrStopProbability); 
        } 
        else{
            return obj.getEmission(); //R.R.  
        }
    }
    

    if (obj.getReflectionType() == DIFF) {
        vec3 randd = cosineSampleHemisphere(erand48(Xi), erand48(Xi));
        vec3 rotatedDir;
        vec3 rotX, rotY;
        ons(N, rotX, rotY);
        rotatedDir.x = vec3(rotX.x, rotY.x, N.x).dot(randd);
        rotatedDir.y = vec3(rotX.y, rotY.y, N.y).dot(randd);
        rotatedDir.z = vec3(rotX.z, rotY.z, N.z).dot(randd);
        vec3 d = rotatedDir; // Normalized
        // vec3 d = N + uniformSampleHemisphere(erand48(Xi), erand48(Xi));
        double cost = d.dot(N);
        return obj.getEmission() + f * cost * tracing(Ray(hit, d), depth, Xi);

        // double r1 = 2 * M_PI * erand48(Xi);
        // double r2 = 2 * erand48(Xi);
        // double r2s = sqrt(r2);
        // vec3 w = N; 
        // vec3 u = ((fabs(w.x) > -.1 ? vec3(0, 1, 0) : vec3(1, 0, 0)).cross(w)).normalize();
        // vec3 v = w.cross(u);
        // vec3 d = (u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1 - r2)).normalize();
        // return obj.e + f * tracing(Ray(hit, d), depth, Xi);

        // Loop over any lights
        // vec3 e;
        // for (int i = 0; i < numSphere; i++) {
        //   const Sphere &s = spheres[i];
        //   if (s.e.x <= 0 && s.e.y <= 0 && s.e.z <= 0) continue; // skip non-lights

        //   vec3 sw = s.p - hit, su = ((fabs(sw.x) > .1 ? vec3(0, 1) : vec3(1)).cross(sw)).normalize(), sv = sw.cross(su);
        //   double cos_a_max = sqrt(1 - s.rad * s.rad / (hit - s.p).dot(hit - s.p));
        //   double eps1 = erand48(Xi), eps2 = erand48(Xi);
        //   double cos_a = 1 - eps1 + eps1 * cos_a_max;
        //   double sin_a = sqrt(1 - cos_a * cos_a);
        //   double phi = 2 * M_PI * eps2;
        //   vec3 l = su * cos(phi) * sin_a + sv * sin(phi) * sin_a + sw * cos_a;
        //   l.normalize();
        //   if (intersect(Ray(hit, l), t, id) && id == i) { // shadow ray
        //     double omega = 2 * M_PI * (1 - cos_a_max);
        //     e = e + f * (s.e * l.dot(nl) * omega) * M_1_PI; // 1/pi for brdf
        //   }
        // }
        
        // return obj.e * E + e + f * (tracing(Ray(hit, d), depth, Xi, 0));
    }
    if (obj.getReflectionType() == SPEC){            // Ideal SPECULAR reflection
        double cost = ray.dir.dot(N);
        // vec3 refl = (ray.dir - N*(cost*2)).normalize();
        vec3 refl = (ray.dir.reflect(N) * cost).normalize();
        return obj.getEmission() + f * tracing(Ray(hit, refl), depth, Xi);
    }
    
    if (obj.getReflectionType() == REFR){
        double n = 1.5;
        double R0 = (1.0-n)/(1.0+n);
        R0 = R0*R0;
        if(N.dot(ray.dir)>0) { // we're inside the medium
            N = N*-1;
            n = 1/n;
        }
        n=1/n;
        double cost1 = (N.dot(ray.dir))*-1; // cosine of theta_1
        double cost2 = 1.0 - n*n*(1.0-cost1*cost1); // cosine of theta_2
        double Rprob = R0 + (1.0-R0) * pow(1.0 - cost1, 5.0); // Schlick-approximation
        vec3 d;
        if (cost2 > 0 && erand48(Xi) > Rprob) { // refraction direction
            d = ((ray.dir*n)+(N*(n*cost1-sqrt(cost2)))).normalize();
        }
        else { // reflection direction
            d = (ray.dir+N*(cost1*2)).normalize();
        }

        // ray.dir = d.normalized();
        // vec3 tmp;
        return obj.getEmission() +  f * 1.15 * tracing(Ray(hit, d), depth, Xi);
        // clr = clr + tmp * 1.15 * rrFactor;
    }
    
    // return vec3(1);
    return vec3();
    
}

Raytracer::Raytracer(unsigned _width, unsigned _height, int _samples){
    width = _width;
    height = _height;
    samples = _samples;

    Mesh *mesh = new Mesh();
    ObjLoader *loader = new ObjLoader();
    // loader->loadObj("teapot.obj", mesh);
    // loader->loadObj("rifle.obj", mesh);
    loader->loadObj("cube.obj", mesh);
    
    delete loader;
    qDebug() << "Object Loaded";

    // for (uint32_t i = 0; i < mesh->triangles.size(); ++i){
    //     qDebug() << i << ":" << mesh->triangles[i]->p1;
    //     qDebug() << i << ":" << mesh->triangles[i]->p2;
    //     qDebug() << i << ":"  << mesh->triangles[i]->p3;
    //     qDebug() << "normal:"  << mesh->triangles[i]->normal;
    //     qDebug() << "";
    // }
    // mesh->scale(0.8, 0.8, 0.8);

    mat4 m1(1, 0, 0, 1,
            0, 1, 0, 1,
            0, 0, 1, 1,
            0, 0, 0 , 1);

    mat4 m2(1, 0, 0, -2,
            0, 2, 0, 3,
            0, 0, 1, 1,
            0, 0, 0 , 1);

    vec4 dir(1, 0, 0, 0);
    Transform t1 = Transform();
    t1.setTranslate(4, 2, 1);
    t1.move(1, 1, 1);
    t1.rotateX(M_PI/6);
    t1.rotateY(M_PI/6);
    t1.rotateZ(M_PI/6);
    t1.setScale(2, 3, 2);
    // t1.rotateY(M_PI/6);
    // qDebug() << t1.rotation.toMatrix();

    
    // vec4 dir2 = t1.getTransformMatrix() * dir;
    // if (dir2.x < eps){
    //     dir2.x = 0;
    // }
    // qDebug() << "dir:" << dir2;
    qDebug() << "pos:" << t1.getPosition();
    qDebug() << "scale:" << t1.getScale();
    qDebug() << "rotation:" << t1.getRotateX() << t1.getRotateY() << t1.getRotateZ();

    
    // mesh->scale(0.5, 0.5, 0.5);
    // mesh->rotateY(M_PI*0.2); 
    // mesh->translate(45, 20, 70);
    // qDebug() << "vertices count: " << mesh->faces.size();


    // qDebug() << "after obj pos:" << tobj->getPosition();
    // Transform *t1 = new Transform();
    // t1->setPosition(0, 0, 0);
    // qDebug() << "position: " << t1->position;
    // t1->translate(10,10,10);
    // qDebug() << "position: " << t1->position;
    // mesh->add(new Triangle(vec3(30, 20, 60), vec3(50, 50, 60),  vec3(80, 10, 80),       vec3(), vec3(1, 1, 1)*.999, DIFF));
    // mesh->add(new Triangle(vec3(30, 20, 60), vec3(50, 50, 60),  vec3(80, 10, 80),       vec3(), vec3(1, 1, 1)*.999, DIFF));

    // for (uint32_t i = 0; i < mesh->triangles.size(); ++i){
    //     qDebug() << i << ":" << mesh->triangles[i]->p1;
    //     qDebug() << i << ":" << mesh->triangles[i]->p2;
    //     qDebug() << i << ":"  << mesh->triangles[i]->p3;
    //     qDebug() << "normal:"  << mesh->triangles[i]->normal;
    //     qDebug() << "";
    // }

    scene.add((Object*)new Plane(vec3(1, 0, 0), 0,       vec3(), vec3(.75, .25, .25), DIFF)); //Left
    scene.add((Object*)new Plane(vec3(-1, 0, 0), 99,       vec3(), vec3(.25, .25, .75), DIFF)); //Right
    scene.add((Object*)new Plane(vec3(0, 1, 0), 0,       vec3(), vec3(.75, .75, .75), DIFF)); //Bottom
    scene.add((Object*)new Plane(vec3(0, 0, 1), 0,       vec3(), vec3(.25, .75, .25), DIFF)); //Front
    scene.add((Object*)new Plane(vec3(0, 0, -1), 296,       vec3(), vec3(.75, .75, .75), DIFF)); // Back
    scene.add((Object*)new Plane(vec3(0, -1, 0), 81.6,       vec3(), vec3(.75, .25, .75), DIFF)); //Ceil
    scene.add((Object*)new AABBox(vec3(50, 81, 60), vec3(50, 0.1, 50),       vec3(12, 12, 12), vec3(), DIFF)); //Glas
    scene.addMesh(mesh);

    

    // scene.add((Object*)new Sphere(16.5, vec3(73, 16.5, 78),       vec3(), vec3(1, 1, 1)*.999, SPEC)); //Glas
    // scene.add((Object*)new AABBox(vec3(80, 25, 120), vec3(10, 50, 40),       vec3(), vec3(1, 1, 1)*.999, SPEC)); //Glas
    // scene.add((Object*)new AABBox(vec3(20, 18, 80), vec3(36, 36, 36),       vec3(), vec3(1, 1, 1)*.999, SPEC)); //Glas
    // scene.add((Object*)new Triangle(vec3(30, 20, 60), vec3(50, 50, 60),  vec3(80, 10, 80),       vec3(), vec3(1, 1, 1)*.999, DIFF)); 
    scene.fov = M_PI/3; // hotirzontal fov 60
    scene.ro = vec3(50, 52, 295.6);
    scene.ta = scene.ro + vec3(0, -0.042612, -1).normalize(); 
    scene.ca = setCamera(scene.ro, scene.ta, 0.0);
    scene.near = 2.0/tan(scene.fov/2);
}

vec3 Raytracer::render_pixel(unsigned short i, unsigned short j, unsigned short *Xi){
    float near = scene.near;
    const int gridSize = 2;
    int samps = samples / (gridSize * gridSize);
    mat3 ca = scene.ca;
    Ray ray(scene.ro);

    double ratio = width / height;
    // unsigned short Xi[3] = {0, 0, i*i * i};
    // unsigned short Xi[3] = {0, 0, j*j * j};
    vec3 color(0,0,0);
    #pragma omp parallel for
    for (int sy = 0; sy < gridSize; ++sy) { // 2x2 subpixel rows
        for (int sx = 0; sx < gridSize; ++sx) { // 2x2 subpixel cols
            vec3 r(0,0,0);
            for (int s = 0; s < samps; s++) {
                double r1 = 2 * erand48(Xi), dx = r1 < 1 ? sqrt(r1) - 1 : 1 - sqrt(2 - r1);
                double r2 = 2 * erand48(Xi), dy = r2 < 1 ? sqrt(r2) - 1 : 1 - sqrt(2 - r2);
                // std::cout << sy - 0.5 + dy * 0.5 << std::endl;
                double u = (j + (sy - 0.5 + dy * 0.5) * 0.5  ) / width;
                double v = (height - (i - (sx - 0.5 + dx * 0.5) * 0.5 ) ) / height;
                u = (u * 2.0 - 1.0);
                v = (v * 2.0 - 1.0);
                u = u * ratio;
                // ray.dir = ca * (vec3(u, v, near)).normalize();
                // r = r + tracing(ray, 0, Xi) * (1.0 / samps);   
                vec3 d = ca * (vec3(u, v, near)).normalize();
                r = r + tracing(Ray(ray.origin, d), 0, Xi) * (1.0 / samps);
            }
            color = color + vec3(clamp(r.x), clamp(r.y), clamp(r.z)) * 0.25;
        }
    }

    // Post process effects.
    return gammaCorrect(color) * 255;
}

void Raytracer::renderDirect(double &time, QImage &directImage, QImage &normalImage) {
    struct timeval start, end;
    gettimeofday(&start, NULL);
    float near = scene.near;
    mat3 ca = scene.ca;
    vec3 ro(50, 52, 295.6);
    vec3 normalColor;
    vec3 directColor;

    // vec3 lig = vec3(-1, -3, -1.5).normalize();
    vec3 pointLig = vec3(50 , 78, 60);


    // #pragma omp parallel for schedule(dynamic, 1)  private(directColor, normalColor)    // OpenMP
    for (unsigned short i = 0; i < height; ++i){
        for (unsigned short j = 0; j < width; ++j){
            double u = j * 1.0 / width;
            double v = (height - i) * 1.0 / height;
            u = (u * 2.0 - 1.0);
            v = (v * 2.0 - 1.0);
            u = u * width/height;
            vec3 rd = ca * (vec3(u, v, near)).normalize();
            
            Intersection intersection = scene.intersect(Ray(ro, rd));
            if (intersection.object) {
                const Object &obj = *intersection.object;
                vec3 hit = ro + rd * intersection.t;
                vec3 N = obj.getNormal(hit);
                normalColor = vec3((N.x + 1)*0.5, (N.y + 1)*0.5, (N.z+1) * 0.25 + 0.5) * 255;
                // normalColor = obj.c * 255;
                vec3 ld = (pointLig - hit).normalize();
                directColor = obj.getDiffuse() * fmax(ld.dot(N), 0) * 255;
                // directColor = obj.getDiffuse() * 255;

                // for (int k = 0; k < scene.objects.size(); ++k) {
                //     const Object &ligObj = *scene.objects[k];
                //     if (ligObj.getEmission().x + ligObj.getEmission().y + ligObj.getEmission().z > 0) {
                //         vec3 ld = (ligObj. - hit).normalize();
                //         Intersection shadow = scene.intersect(Ray(hit + ld, ld);
                //         if (*shadow.object == ligObj){
                            
                //         }   
                //         else{
                //             directColor = directColor * 0.2; 
                //         }     
                //     }
                // }
                // Shadow
                Intersection shadow = scene.intersect(Ray(hit + ld*0.01, ld));
                double distToLight = (pointLig - hit).length();
                if (shadow.object && shadow.t <= distToLight){
                    directColor = directColor * clamp(3.8 * shadow.t/(distToLight), 0.0, 1.0); 
                }

            }

            
            normalImage.setPixel(j, i, qRgb(normalColor.x, normalColor.y, normalColor.z));
            directImage.setPixel(j, i, qRgb(directColor.x, directColor.y, directColor.z));
            
        }
    }
    gettimeofday(&end, NULL);
    time = ((end.tv_sec  - start.tv_sec) * 1000000u + end.tv_usec - start.tv_usec) / 1.e6;
 
}

QImage Raytracer::render(double &time) {
    struct timeval start, end;
    gettimeofday(&start, NULL);

    float near = scene.near;
    const int gridSize = 2;
    int samps = samples / (gridSize * gridSize);
    mat3 ca = scene.ca;
    vec3 ro(50, 52, 295.6);
    // vec3 *c = new vec3[width * height];
    
    QImage image(width, height, QImage::Format_RGB32);
    
    vec3 r(0,0,0);
    #pragma omp parallel for schedule(dynamic, 1) private(r)       // OpenMP
    for (unsigned short i = 0; i < height; ++i){
        // fprintf(stderr, "\rRendering (%d spp) %5.2f%%", samps * gridSize * gridSize, 100.*i / (height - 1));
        qDebug() << "Rendering " << "spp:" <<samps * gridSize * gridSize << " " << 100.*i / (height - 1) << '%';
        unsigned short Xi[3] = {0, 0, i*i * i};
        for (unsigned short j = 0; j < width; ++j){
            // unsigned int idx = i*width + j;
            vec3 color;
            for (int sy = 0; sy < 2; ++sy) { // 2x2 subpixel rows
                for (int sx = 0; sx < 2; ++sx) { // 2x2 subpixel cols
                    r = vec3();

                    for (int s = 0; s < samps; ++s) {

                        double r1 = 2 * erand48(Xi), dx = r1 < 1 ? sqrt(r1) - 1 : 1 - sqrt(2 - r1);
                        double r2 = 2 * erand48(Xi), dy = r2 < 1 ? sqrt(r2) - 1 : 1 - sqrt(2 - r2);
                        double u = (j + (sy - 0.5 + dy * 0.5) * 0.5  ) / width;
                        double v = (height - (i + (sx - 0.5 + dx * 0.5) * 0.5 ) ) / height;
                        u = (u * 2.0 - 1.0);
                        v = (v * 2.0 - 1.0);
                        u = u * width/height;
                        vec3 rd = ca * (vec3(u, v, near)).normalize();
                        r = r + tracing(Ray(ro, rd), 0, Xi) * (1.0 / samps);
                    }
                    
                }

                

                color = color + vec3(clamp(r.x), clamp(r.y), clamp(r.z)) * .25;

                vec3 correct = gammaCorrect(color) * 255;

                
                // QRgb value = qRgb(correct.x, correct.y, correct.z); // 0xffbd9527
                image.setPixel(j, i, qRgb(correct.x, correct.y, correct.z));

                // value = qRgb(122, 163, 39); // 0xff7aa327
                // image.setPixel(0, 1, value);
                // image.setPixel(1, 0, value);

                // value = qRgb(237, 187, 51); // 0xffedba31
                // image.setPixel(2, 1, value);
            }

            // Post process effects.
        }
    }


    

    // for (int i = 0; i < width * height; i++){
    //     c[i] = gammaCorrect(c[i]) * 255;
    // }
    // FILE *f = fopen("test_image.ppm", "w");         // Write image to PPM file.
    // fprintf(f, "P3\n%d %d\n%d\n", w, h, 255);
    // for (int i = 0; i < w * h; i++)
    //     fprintf(f, "%d %d %d ", int(c[i].x), int(c[i].y), int(c[i].z));

    gettimeofday(&end, NULL);
    time = ((end.tv_sec  - start.tv_sec) * 1000000u + end.tv_usec - start.tv_usec) / 1.e6;

    qDebug() << "Render time: " << time;

    // return c;
    return image;
}

Raytracer::~Raytracer(){
    
}