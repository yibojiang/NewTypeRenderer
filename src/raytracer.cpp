

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


vec3 Raytracer::tracing(const Ray &ray, int depth, unsigned short *Xi){
    // Intersection intersection = bvh.intersectBoundingBox(ray);
    // Intersection intersection = scene.intersect(ray);
    Intersection intersection = bvh.intersect(ray);
    
    if (!intersection.object) return vec3(0);


    const Object &obj = *intersection.object;
    vec3 hit = ray.origin + ray.dir * intersection.t;

    vec3 N = obj.getNormal(hit);
    vec3 nl = N.dot(ray.dir) < 0 ? N: N * -1;
    vec3 f = obj.getDiffuse();
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
        double r1 = 2 * M_PI * erand48(Xi);
        double r2 = erand48(Xi);
        double r2s = sqrt(r2);
        vec3 w = N;
        vec3 u = ((fabs(w.x) > .1 ? vec3(0, 1, 0) : vec3(1, 0, 0)).cross(w)).normalize();
        vec3 v = w.cross(u);
        vec3 d = (u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1 - r2)).normalize();
        return obj.getEmission() + f*(tracing(Ray(hit, d), depth, Xi));
    }
    if (obj.getReflectionType() == SPEC){            // Ideal SPECULAR reflection
        // r=d−2(d⋅n)n
        vec3 refl = ray.dir - N * 2 * N.dot(ray.dir);
        return obj.getEmission() + f * tracing(Ray(hit, refl), depth, Xi);
    }
    
    // if (obj.getReflectionType() == REFR){

    // }
      Ray reflRay(hit, ray.dir - N * 2 * N.dot(ray.dir)); // Ideal dielectric REFRACTION
      bool into = N.dot(nl) > 0;              // Ray from outside going in?
      double nc = 1, nt = 1.5, nnt = into ? nc / nt : nt / nc, ddn = ray.dir.dot(nl), cos2t;
      if ((cos2t = 1 - nnt * nnt * (1 - ddn * ddn)) < 0) // Total internal reflection
        return obj.getEmission() + f * tracing(reflRay, depth, Xi);
      vec3 tdir = (ray.dir * nnt - N * ((into ? 1 : -1) * (ddn * nnt + sqrt(cos2t)))).normalize();
      double a = nt - nc, b = nt + nc, R0 = a * a / (b * b), c = 1 - (into ? -ddn : tdir.dot(N));
      double Re = R0 + (1 - R0) * c * c * c * c * c, Tr = 1 - Re, P = .25 + .5 * Re, RP = Re / P, TP = Tr / (1 - P);
      return obj.getEmission() + f * (depth > 2 ? (erand48(Xi) < P ? // Russian roulette
                            tracing(reflRay, depth, Xi) * RP : tracing(Ray(hit, tdir), depth, Xi) * TP) :
                            tracing(reflRay, depth, Xi) * Re + tracing(Ray(hit, tdir), depth, Xi) * Tr);
    // {
    //     double n = 1.5;
    //     double R0 = (1.0-n)/(1.0+n);
    //     R0 = R0*R0;
    //     if(N.dot(ray.dir)>0) { // we're inside the medium
    //         N = N*-1;
    //         n = 1/n;
    //     }
    //     n=1/n;
    //     double cost1 = (N.dot(ray.dir))*-1; // cosine of theta_1
    //     double cost2 = 1.0 - n*n*(1.0-cost1*cost1); // cosine of theta_2
    //     double Rprob = R0 + (1.0-R0) * pow(1.0 - cost1, 5.0); // Schlick-approximation
    //     vec3 d;
    //     if (cost2 > 0 && erand48(Xi) > Rprob) { // refraction direction
    //         d = ((ray.dir*n)+(N*(n*cost1-sqrt(cost2)))).normalize();
    //     }
    //     else { // reflection direction
    //         d = ray.dir.reflect(N);
    //     }

    //     return obj.getEmission() +  f * tracing(Ray(hit, d), depth, Xi);
    // }
    
    // return vec3(1);
    return vec3();
    
}

void Raytracer::setupScene(){
    
    
    ObjLoader loader;
    // loader.loadObj("rifle.obj", mesh);
    // loader->loadObj("cube.obj", mesh);
    // loader->loadObj("sponza.obj", mesh);
    // loader.loadModel("sponza.obj", mesh);
    // loader.loadModel("sponza.obj", mesh);
    // loader.loadModel("rifle.obj", mesh);

    
    
    // delete loader;
    qDebug() << "Object Loaded";

    scene.fov = M_PI/3; // hotirzontal fov 60
    // float rtY = 1.5* M_PI;
    // float camDist = 250;
    // scene.ro = vec3(50 + camDist * cos(rtY), 60, 50 -camDist * sin(rtY));
    // scene.ta = vec3(50, 50, 50);
    // scene.ca = setCamera(scene.ro, scene.ta, 0.0);
    
    scene.near = 2.0f/tan(scene.fov*0.5f);
    rotateCamera(0, 0.5 * M_PI, 0);

    
    
    Object *light = (Object*)new Box(vec3(40, 0.1, 40),       vec3(9, 9, 9), vec3(), DIFF);
    light->name = "light";
    Transform *lightxform = new Transform(light);
    lightxform->setTranslate(50, 99, 60);
    scene.root->addChild(lightxform);


    Mesh *mesh = new Mesh();
    mesh->name = "mesh";
    loader.loadModel("rifle.obj", mesh);
    mesh->setMaterial(SPEC);
    Transform *meshxform = new Transform(mesh);
    meshxform->rotateY(M_PI*0.3);
    meshxform->setTranslate(50, 50, 50);
    meshxform->setScale(0.8, 0.8, 0.8);
    scene.root->addChild(meshxform);


    // for (int i = 0; i < 3; ++i){
    //     for (int j = 0; j < 3; ++j){
    //         for (int k = 0; k < 3; ++k){
    //         // Object *cube = new Box(vec3(20, 20, 20), vec3(), vec3(1, 1, 1)*.999, DIFF);
    //         // Transform *t = new Transform();
    //         // t->addObject((Object*)cube);
    //         // t->setScale(1, 0.5, 1);
    //         // // t->rotateY(M_PI/6);
    //         // t->setTranslate(i * 30, j*30, 50);
    //         // scene.root->addChild(t); 

    //             Object *sphere = new Sphere(10.0, vec3(), vec3(1, 1, 1)*.999, DIFF);
    //             sphere->name = "sphere" + std::to_string(i + j * 10 + k * 100);
    //             Transform *t = new Transform(sphere);
    //             t->setScale(1, 0.5, 1);
    //             t->setTranslate(i * 30 + 20, j*30 + 25,  k * 30);
    //             scene.root->addChild(t);
    //         }
    //     }
    // }

    Object *floor = (Object*)new Box(vec3(150, 0.1, 300),       vec3(), vec3(.75, .75, .75), DIFF);
    floor->name = "floor";
    Transform *xform = new Transform(floor);
    xform->setTranslate(50, 0, 0);
    scene.root->addChild(xform);


    Object *left = (Object*)new Box(vec3(0.1, 300, 300),       vec3(), vec3(.75, .25, .25), DIFF);
    left->name = "left";
    Transform *xform1 = new Transform(left);
    xform1->setTranslate(0, 150, 0);
    scene.root->addChild(xform1);


    Object *right = (Object*)new Box(vec3(0.1, 100, 300),       vec3(), vec3(.25, .75, .25), DIFF);
    right->name = "right";
    Transform *xform2 = new Transform(right);
    xform2->setTranslate(100, 50, 0);
    scene.root->addChild(xform2);


    Object *ceil = (Object*)new Box(vec3(100, 0.1, 300),       vec3(), vec3(.75, .75, .75), DIFF);
    ceil->name = "ceil";
    Transform *xform3 = new Transform(ceil);
    xform3->setTranslate(50, 100, 0);
    scene.root->addChild(xform3);

    Object *front = (Object*)new Box(vec3(100, 100, 0.1),       vec3(), vec3(.75, .75, .75), DIFF);
    front->name = "front";
    Transform *xform4 = new Transform(front);
    xform4->setTranslate(50, 50, -150);
    scene.root->addChild(xform4);

    Object *back = (Object*)new Box(vec3(100, 100, 0.1),       vec3(), vec3(1,1,1), DIFF);
    back->name = "back";
    Transform *xform5 = new Transform(back);
    xform5->setTranslate(50, 50, 150);
    scene.root->addChild(xform5);

}

Raytracer::Raytracer(unsigned _width, unsigned _height, int _samples){
    width = _width;
    height = _height;
    samples = _samples;

    setupScene();
    scene.updateTransform(scene.root, mat4());
    // scene.add((Object*)new Plane(vec3(1, 0, 0), 0,       vec3(), vec3(.75, .25, .25), DIFF)); //Left
    // scene.add((Object*)new Plane(vec3(-1, 0, 0), 99,       vec3(), vec3(.25, .25, .75), DIFF)); //Right
    // scene.add((Object*)new Plane(vec3(0, 1, 0), 0,       vec3(), vec3(.75, .75, .75), DIFF)); //Bottom
    // scene.add((Object*)new Plane(vec3(0, 0, 1), 0,       vec3(), vec3(.25, .75, .25), DIFF)); //Front
    // scene.add((Object*)new Plane(vec3(0, 0, -1), 296,       vec3(), vec3(.75, .75, .75), DIFF)); // Back
    // scene.add((Object*)new Plane(vec3(0, -1, 0), 81.6,       vec3(), vec3(.75, .25, .75), DIFF)); //Ceil
    // scene.add((Object*)light); //Glas
    // scene.addMesh(mesh);
    // scene.add((Object*)new Sphere(16.5, vec3(0, 0, 0),       vec3(), vec3(1, 1, 1)*.999, SPEC)); //Glas
    // scene.add((Object*)new Sphere(16.5, vec3(50, 0, 90),       vec3(), vec3(1, 1, 1)*.999, SPEC)); //Glas
    // scene.add((Object*)new Sphere(16.5, vec3(73, 16.5, 78),       vec3(), vec3(1, 1, 1)*.999, SPEC)); //Glas
    // scene.add((Object*)new Sphere(16.5, vec3(20, 16.5, 90),       vec3(), vec3(1, 1, 1)*.999, REFR)); //Glas
    // scene.add((Object*)cube); //Glas
    // scene.add((Object*)new Triangle(vec3(30, 20, 60), vec3(50, 50, 60),  vec3(80, 10, 60),       vec3(), vec3(1, 1, 1)*.999, DIFF)); 
    

    bvh.setup(scene);
}

void Raytracer::rotateCamera(float x, float y, float z){
    float camDist = 200;
    scene.ro = vec3(50 + camDist * cos(y), 50, 50 -camDist * sin(y));
    scene.ta = vec3(50, 50, 50);
    scene.ca = setCamera(scene.ro, scene.ta, 0.0);
    
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

void Raytracer::renderDirect(double &time, QImage &directImage, QImage &normalImage, QImage &boundingBoxImage) {
    struct timeval start, end;
    gettimeofday(&start, NULL);
    float near = scene.near;
    mat3 ca = scene.ca;
    vec3 ro = scene.ro;
    vec3 normalColor;
    vec3 directColor;
    vec3 boundingBoxColor;
    vec3 ambColor(0.15, 0.15, 0.15);
    // vec3 lig = vec3(-1, -3, -1.5).normalize();
    vec3 pointLig(50 , 78, 60);


    #pragma omp parallel for schedule(dynamic, 1)  private(directColor, normalColor, boundingBoxColor)    // OpenMP
    for (unsigned short i = 0; i < height; ++i){
        for (unsigned short j = 0; j < width; ++j){
            double u = j * 1.0 / width;
            double v = (height - i) * 1.0 / height;
            u = (u * 2.0 - 1.0);
            v = (v * 2.0 - 1.0);
            u = u * width/height;
            vec3 rd = ca * (vec3(u, v, near)).normalize();
            normalColor = vec3(0,0,0);
            directColor = vec3(0,0,0);
            boundingBoxColor = vec3(0,0,0);


            // Intersection intersectionBox = bvh.intersectBoundingBox(Ray(ro, rd));

            #ifdef WIREFRAME_ON
            Intersection intersectionBox = bvh.intersectBVH(Ray(ro, rd));
            if (intersectionBox.t > eps && intersectionBox.t < inf){
                boundingBoxColor = vec3(0, 1, 0);
            }
            #endif

            
            Intersection intersection = bvh.intersect(Ray(ro, rd));
            if (intersection.object) {
                const Object &obj = *intersection.object;
                vec3 hit = ro + rd * intersection.t;
                vec3 N = obj.getNormal(hit);
                normalColor = vec3((N.x + 1)*0.5, (N.y + 1)*0.5, (N.z+1) * 0.25 + 0.5) * 255;
                // normalColor = obj.c * 255;
                vec3 ld = (pointLig - hit).normalize();
                directColor = obj.getDiffuse() * fmax(ld.dot(N), 0);


                Intersection shadow = bvh.intersect(Ray(hit, ld));
                double distToLight = (pointLig - hit).length();
                if (shadow.object && shadow.t <= distToLight){
                    directColor = directColor * clamp(3.8 * shadow.t/(distToLight), 0.0, 1.0); 
                    // directColor = vec3();
                }
                directColor = directColor + ambColor;
                
            }

            boundingBoxColor = boundingBoxColor * 255;
            directColor = directColor * 255;
            directColor = (directColor + boundingBoxColor); 
            directColor = clamp(directColor, vec3(0,0,0), vec3(255,255,255));
            boundingBoxImage.setPixel(j, i, qRgb(boundingBoxColor.x, boundingBoxColor.y, boundingBoxColor.z));
            normalImage.setPixel(j, i, qRgb(normalColor.x, normalColor.y, normalColor.z));
            directImage.setPixel(j, i, qRgb(directColor.x, directColor.y, directColor.z));
            
        }
    }
    gettimeofday(&end, NULL);
    time = ((end.tv_sec  - start.tv_sec) * 1000000u + end.tv_usec - start.tv_usec) / 1.e6;
 
}

void Raytracer::renderIndirect(double &time, QImage &image) {
    struct timeval start, end;
    gettimeofday(&start, NULL);

    float near = scene.near;
    const int gridSize = 2;
    int samps = samples / (gridSize * gridSize);
    mat3 ca = scene.ca;
    vec3 ro = scene.ro;

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
                
            }
            // vec3 correct = gammaCorrect(color) * 255;
            vec3 raw = color * 255;
            // QRgb value = qRgb(correct.x, correct.y, correct.z); // 0xffbd9527
            // image.setPixel(j, i, qRgb(correct.x, correct.y, correct.z));
            image.setPixel(j, i, qRgb(raw.x, raw.y, raw.z));
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

    
}

void Raytracer::setResolution(const int &width, const int &height){
    this->width = width;
    this->height = height;
}

Raytracer::~Raytracer(){
    
}

