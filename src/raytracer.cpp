
// #ifndef RAYTRACER_H
#include "raytracer.h"
// #endif

inline mat3 setCamera( vec3 ro, vec3 ta, vec3 up )
{
  vec3 cw = (ta - ro).normalized();
  // vec3 cp = vec3(sin(cr), cos(cr), 0.0);
  vec3 cp = up.normalized();
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

inline double fract(double f){
    return f = f-(long)f;
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


vec3 Raytracer::tracing(Ray &ray, int depth, int E = 1){
    // Intersection intersection = scene.intersect(ray);
    Intersection intersection = bvh.intersect(ray);
    if (!intersection.object){

        if (scene.hasHdri){
            // hdri  
            HDRImage hdri = scene.hdri;
            // float u = (atan( sp.x, sp.z)) / ( 2.0 * M_PI ) + 0.5;
            // float v = asin( sp.y ) / M_PI + 0.5;
            double u = atan2(ray.dir.x, ray.dir.z) / ( 2.0 * M_PI ) + 0.5;
            double v = asin(ray.dir.y) / M_PI + 0.5;
            int hdrx = u* (hdri.width-1);
            int hdry = v* (hdri.height-1);
            double r = hdri.colors[hdry*hdri.width*3 + hdrx*3];
            double g = hdri.colors[hdry*hdri.width*3 + hdrx*3 + 1];
            double b = hdri.colors[hdry*hdri.width*3 + hdrx*3 + 2];
            r = pow(r, 0.44);
            g = pow(g, 0.44);    
            b = pow(b, 0.44);
            r = scene.envLightIntense * pow(r, scene.envLightExp);
            g = scene.envLightIntense * pow(g, scene.envLightExp);
            b = scene.envLightIntense * pow(b, scene.envLightExp);
            return vec3(r,g,b);
        }
        else{
            return vec3();
        }
        
    }
    Object *obj = intersection.object;
    vec3 hit = ray.origin + ray.dir * intersection.t;

    vec3 N = obj->getNormal(hit);
    vec3 nl = N.dot(ray.dir) < 0 ? N: N * -1;


    vec3 f = obj->getMaterial()->getDiffuseColor(ray.uv);
    vec3 reflectColor = obj->getMaterial()->getReflectColor(ray.uv);

    // Russian roulette: starting at depth 5, each recursive step will stop with a probability of p.
    double p = f.x > f.y && f.x > f.z ? f.x : f.y > f.z ? f.y : f.z; // max refl

    // Russian roulette termination.
    if (++depth>5){
        if (drand48()<p*0.9) { // Multiply by 0.9 to avoid infinite loop with colours of 1.0
            f=f*(0.9/p);
        }
        else {
            return obj->getMaterial()->getEmission() * E;
        }
    }

    double randomVal = drand48();
    if (randomVal <= obj->getMaterial()->refract){
        vec3 refl = ray.dir - N * 2 * N.dot(ray.dir);
        // float roughness = obj->getMaterial()->roughness;
        // vec3 rnddir = roughness > 0 ? vec3(drand48()-0.5, drand48()-0.5, drand48()-0.5).normalize()*roughness : vec3(0);
        // refl = refl + rnddir * roughness;


        Ray reflRay(hit, refl); // Ideal dielectric REFRACTION
        bool into = N.dot(nl) > 0;              // Ray from outside going in?
        double nc = 1, nt = obj->getMaterial()->ior, nnt = into ? nc / nt : nt / nc, ddn = ray.dir.dot(nl), cos2t;
        if ((cos2t = 1 - nnt * nnt * (1 - ddn * ddn)) < 0) // Total internal reflection
            return obj->getMaterial()->getEmission() + f * tracing(reflRay, depth);
        vec3 tdir = (ray.dir * nnt - N * ((into ? 1 : -1) * (ddn * nnt + sqrt(cos2t)))).normalize();

        Ray refrRay(hit, tdir);
        double a = nt - nc, b = nt + nc, R0 = a * a / (b * b), c = 1 - (into ? -ddn : tdir.dot(N));
        double Re = R0 + (1 - R0) * c * c * c * c * c, Tr = 1 - Re, P = .25 + .5 * Re, RP = Re / P, TP = Tr / (1 - P);
        return obj->getMaterial()->getEmission() + f * (depth > 2 ? (drand48() < P ? // Russian roulette
                            tracing(reflRay, depth) * RP : tracing(refrRay, depth) * TP) :
                            tracing(reflRay, depth) * Re + tracing(refrRay, depth) * Tr);
    }
    else if (randomVal <= obj->getMaterial()->refract + obj->getMaterial()->reflection ){

        vec3 refl = ray.dir - N * 2 * N.dot(ray.dir);
        // float roughness = obj->getMaterial()->roughness;
        // vec3 rnddir = roughness > 0 ? vec3(drand48()-0.5, drand48()-0.5, drand48()-0.5).normalize()*roughness : vec3(0);
        // refl = refl + rnddir;

        Ray reflRay(hit, refl);
        return obj->getMaterial()->getEmission() + reflectColor * tracing(reflRay, depth);
    }
    else{
        double r1 = 2 * M_PI * drand48(); // random ø angle
        double r2 = drand48(); // random distance from sphere center cos θ
        double r2s = sqrt(r2);
        vec3 w = N;
        vec3 u = ((fabs(w.x) > 0.1 ? vec3(0, 1, 0) : vec3(1, 0, 0)).cross(w)).normalize();
        vec3 v = w.cross(u);  
        // cos weighted estimator
        vec3 d = (u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1 - r2)).normalize();


        Ray reflRay(hit, d);

        // Sphere Area light sample
        // Explicit light sample. Only support for sphere light
        #ifdef EXPLICIT_LIGHT_SAMPLE
        vec3 e;
        for (unsigned int i = 0; i < scene.lights.size(); i++) {
            Object* light = scene.lights[i];
            vec3 sw = light->getCentriod() - hit;
            vec3 su = ((fabs(sw.x) > .1 ? vec3(0, 1, 0) : vec3(1, 0, 0)).cross(sw)).normalize();
            vec3 sv = sw.cross(su);
            
            // Sphere light
            Sphere* s = (Sphere*)light;
            double cos_a_max = sqrt(1 - s->rad * s->rad / (hit - s->getCentriod()).dot(hit - s->getCentriod()));
            double eps1 = drand48(), eps2 = drand48();
            double cos_a = 1 - eps1 + eps1 * cos_a_max;
            double sin_a = sqrt(1 - cos_a * cos_a);
            double phi = 2 * M_PI * eps2;
            vec3 l = su * cos(phi) * sin_a + sv * sin(phi) * sin_a + sw * cos_a;
            l.normalize();

            Ray shadowRay(hit, l);
            Intersection shadow = bvh.intersect(shadowRay);
            
            if (shadow.object && shadow.object == light){
                double omega = 2 * M_PI * (1 - cos_a_max);
                e = e + f * M_1_PI * (light->getMaterial()->getEmission() * l.dot(nl) * omega); // 1/pi for brdf
            }
        }
        return obj->getMaterial()->getEmission() * E + e + f * (tracing(reflRay, depth, 0));
        #else

        return obj->getMaterial()->getEmission() + f * (tracing(reflRay, depth));

        #endif        
    }

    
}

void Raytracer::unloadScene(){
    scene.destroyScene();
    bvh.destroy();
}

void Raytracer::setupScene(const std::string& scenePath){
    this->scenePath = scenePath;
    ObjLoader loader;
    std::ifstream t(scenePath);
    std::string json((std::istreambuf_iterator<char>(t)),
                     std::istreambuf_iterator<char>());


    rapidjson::Document document;
    if (document.Parse(json.c_str()).HasParseError()) {
        qDebug() << "can't parse the scene";
        return;
        // fprintf(stderr, "\nError(offset %u): %s\n", 
        // (unsigned)document.GetErrorOffset(),
        // GetParseError_En(document.GetParseError()));
    }

    scene.root = new Transform();
    if (document.HasMember("camera")){
        rapidjson::Value& camera = document["camera"];
        scene.fov = camera["fov"].GetFloat()*M_PI/180;
        scene.near = 1.0f/tan(scene.fov*0.5f);
        const rapidjson::Value& position = document["camera"]["transform"]["position"];
        const rapidjson::Value& target = document["camera"]["transform"]["target"];
        const rapidjson::Value& up = document["camera"]["transform"]["up"];
        scene.ro = vec3(position[0].GetFloat(), position[1].GetFloat(), position[2].GetFloat());
        scene.ta = vec3(target[0].GetFloat(), target[1].GetFloat(), target[2].GetFloat());
        scene.up = vec3(up[0].GetFloat(), up[1].GetFloat(), up[2].GetFloat()); 
        scene.ca = setCamera(scene.ro, scene.ta, scene.up);

        if (camera.HasMember("focusOn")){
            scene.focusOn = camera["focusOn"].GetBool();
        }
        else{
            scene.focusOn = false;
        }
        if (camera.HasMember("focalLength")){
            scene.focalLength = camera["focalLength"].GetFloat();
            // qDebug() << "focal length" << scene.focalLength;
        }

        if (camera.HasMember("aperture")){
            scene.aperture = camera["aperture"].GetFloat();
        }
        else{
            scene.aperture = 1;
        }


        
    }

    if (document.HasMember("envlight")){
        rapidjson::Value& envLight = document["envlight"];
        scene.LoadHdri(envLight["hdri"].GetString());
        scene.envLightIntense = envLight["intense"].GetFloat();
        scene.envLightExp = envLight["exp"].GetFloat();
    }
    if (document.HasMember("primitives")){

        rapidjson::Value& primitives = document["primitives"];
        rapidjson::Value& materials = document["materials"];

        for (rapidjson::SizeType i = 0; i < primitives.Size(); ++i){
            Object *obj;
            std::string ptype = primitives[i]["type"].GetString();

            if (ptype == "box"){
                obj = (Object*)new Box(vec3(1, 1, 1));
                
            }
            else if (ptype == "sphere"){
                obj = (Object*)new Sphere(1.0);
            }
            else if (ptype == "mesh"){
                Mesh *mesh = new Mesh();
                mesh->name = "mesh";
                std::string modelName = primitives[i]["path"].GetString();
                loader.loadModel(modelName, mesh);
                obj = (Object*)mesh;
            }
            else{
                qDebug() << "error";
                obj = new Object();
            }

            rapidjson::Value& pos = primitives[i]["transform"]["position"];
            rapidjson::Value& scl = primitives[i]["transform"]["scale"];
            rapidjson::Value& rot = primitives[i]["transform"]["rotation"];
            
            std::string materialName = primitives[i]["material"].GetString();

            qDebug() << materialName.c_str();
            Material *material = new Material();
            if (materials.HasMember(materialName.c_str())){
                // qDebug() << materialName.c_str() << "found";
                rapidjson::Value& mat = materials[materialName.c_str()];
                if (mat.HasMember("diffuse")){
                   material->diffuse = mat["diffuse"].GetFloat();
                
                }
                if (mat.HasMember("reflection")){
                    material->reflection = mat["reflection"].GetFloat();
                }
                
                if (mat.HasMember("roughness")){
                   material->roughness = mat["roughness"].GetFloat();
                }

                if (mat.HasMember("diffuseRoughness")){
                   material->diffuseRoughness = mat["diffuseRoughness"].GetFloat();
                }
                
                if (mat.HasMember("emission")){
                    material->emission = mat["emission"].GetFloat();
                }
                
                if (mat.HasMember("ior")){
                    material->ior = mat["ior"].GetFloat();
                }

                if (mat.HasMember("refract")){
                    material->refract = mat["refract"].GetFloat();
                }

                if (mat.HasMember("diffuseColor")){
                    material->diffuseColor = vec3(mat["diffuseColor"][0].GetFloat(), mat["diffuseColor"][1].GetFloat(), mat["diffuseColor"][2].GetFloat());    
                }

                if (mat.HasMember("reflectColor")){
                    material->reflectColor = vec3(mat["reflectColor"][0].GetFloat(), mat["reflectColor"][1].GetFloat(), mat["reflectColor"][2].GetFloat());    
                }

                if (mat.HasMember("emissionColor")){
                    material->setEmission(vec3(mat["emissionColor"][0].GetFloat(), mat["emissionColor"][1].GetFloat(), mat["emissionColor"][2].GetFloat()));    
                }

                if (mat.HasMember("diffuseTexture")){
                    material->setDiffuseTexture(mat["diffuseTexture"].GetString());    
                }
                
            }
            else{
                qDebug() << "can't find material " << materialName.c_str();
            }
            obj->setMaterial(material);

            
            Transform *xform = new Transform(obj);
            xform->setTranslate(pos[0].GetFloat(), pos[1].GetFloat(), pos[2].GetFloat());
            xform->setScale(scl[0].GetFloat(), scl[1].GetFloat(), scl[2].GetFloat());
            Quaternion orientation = Quaternion(rot[0].GetFloat() * M_PI / 180, rot[1].GetFloat() * M_PI/180, rot[2].GetFloat() * M_PI / 180);
            // qDebug() << rot[0].GetFloat()/ 180 * M_PI << ;
            xform->setRotation(orientation);
            
            

            obj->name = primitives[i]["name"].GetString();
            // obj->setMaterial(material.GetString());
            // obj->setDiffuseColor(vec3(color[0].GetFloat(), color[1].GetFloat(), color[2].GetFloat()));
            // obj->setEmissionColor(vec3(emission[0].GetFloat(), emission[1].GetFloat(), emission[2].GetFloat()));

            scene.root->addChild(xform);
            qDebug() << "add " << obj->name.c_str() << " to the scene";

        }
    }

    scene.updateTransform(scene.root, mat4());
    bvh.setup(scene);
}

Raytracer::Raytracer(unsigned _width, unsigned _height, int _samples){
    width = _width;
    height = _height;
    samples = _samples;    

    QString path = QDir::currentPath();
    std::string name = "/scene/caustic.json";
    std::string fullpath = path.toUtf8().constData() + name;
    setupScene(fullpath);
    
}

void Raytracer::rotateCamera(float x, float y, float z){
    Quaternion rot(x, y, z);
    mat4 m = rot.toMatrix();
    
    vec4 ro = m * vec4(scene.ro, 1);
    scene.ro = vec3(ro.x, ro.y, ro.z);

    vec4 ta = m * vec4(scene.ta, 1);
    scene.ta = vec3(ta.x, ta.y, ta.z);    
    scene.ca = setCamera(scene.ro, scene.ta, scene.up);    
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

    for (unsigned int i = 0; i < scene.lights.size(); i++) {
        Object* light = scene.lights[i];
        pointLig = light->getCentriod();
        break;
    }

    #pragma omp parallel for schedule(dynamic, 1)  private(directColor, normalColor, boundingBoxColor)    // OpenMP
    for (unsigned short i = 0; i < height; ++i){
        for (unsigned short j = 0; j < width; ++j){
            double u = j * 1.0 / width;
            double v = (height - i) * 1.0 / height;
            u = (u * 2.0 - 1.0);
            v = (v * 2.0 - 1.0);
            // u = u * width/height;
            v = v * height/width;
            vec3 rd = ca * (vec3(u, v, near)).normalize();
            normalColor = vec3(0,0,0);
            directColor = vec3(0,0,0);
            boundingBoxColor = vec3(0,0,0);

            Ray ray(ro, rd);
            // 

            #ifdef WIREFRAME_ON
            // Intersection intersectionBox = bvh.intersectBoundingBox(Ray(ro, rd));
            Intersection intersectionBox = bvh.intersectBVH(ray);
            if (intersectionBox.t > eps && intersectionBox.t < inf){
                boundingBoxColor = vec3(0, 1, 0);
            }
            #endif

            
            Intersection intersection = bvh.intersect(ray);
            if (intersection.object) {
                Object *obj = intersection.object;
                vec3 f = obj->getMaterial()->getDiffuseColor(ray.uv);
                vec3 hit = ro + rd * intersection.t;
                vec3 N = obj->getNormal(hit);
                // N = ray.dir.dot(N) < 0 ? N : -N;
                normalColor = vec3((N.x + 1)*0.5, (N.y + 1)*0.5, (N.z+1) * 0.25 + 0.5) * 255;
                // normalColor = obj->c * 255;

                vec3 ld = (pointLig - hit).normalize();
                directColor = f * fmax(ld.dot(N), 0);
                // directColor = f;
                Ray shadowRay(hit, ld);
                Intersection shadow = bvh.intersect(shadowRay);
                double distToLight = (pointLig - hit).length();
                if (shadow.object && shadow.t <= distToLight){
                    directColor = directColor * clamp(3.8 * shadow.t/(distToLight), 0.0, 1.0); 
                    // directColor = vec3();
                }
                directColor = directColor + ambColor;
                
            }

            boundingBoxColor = boundingBoxColor * 255;
            directColor = directColor * 255;
            directColor = clamp(directColor, vec3(0,0,0), vec3(255,255,255));
            boundingBoxColor = (directColor + boundingBoxColor); 
            boundingBoxColor = clamp(boundingBoxColor, vec3(0,0,0), vec3(255,255,255));
            boundingBoxImage.setPixel(j, i, qRgb(boundingBoxColor.x, boundingBoxColor.y, boundingBoxColor.z));
            normalImage.setPixel(j, i, qRgb(normalColor.x, normalColor.y, normalColor.z));
            directImage.setPixel(j, i, qRgb(directColor.x, directColor.y, directColor.z));
            
        }
    }
    gettimeofday(&end, NULL);
    time = ((end.tv_sec  - start.tv_sec) * 1000000u + end.tv_usec - start.tv_usec) / 1.e6;
 
}

void Raytracer::renderIndirectProgressive(vec3 *colorArray, bool& abort, bool& restart, int &samples) {
    vec3 color(0,0,0);
    vec3 r(0,0,0);

    scene.focalLength = (scene.ta - scene.ro).length();
    
    // ratio of original/new aperture (>1: smaller view angle, <1: larger view angle)
    double aperture = 0.5135 / scene.aperture;
    
    // vec3 dir_norm = vec3(0, -0.042612, -1).normalize();
    vec3 dir_norm = (scene.ta-scene.ro).normalize();
    double L = scene.near;
    double L_new = aperture * L;
    double L_diff = L - L_new;
    vec3 cam_shift = dir_norm * (L_diff);
    if (L_diff < 0){
        cam_shift = cam_shift * 1.5;
    }


    
    L = L_new;
    Ray camera = Ray(scene.ro + cam_shift, dir_norm);
  // Cross product gets the vector perpendicular to cx and the "gaze" direction
    vec3 cx = vec3((width*1.0) / height, 0, 0);
    vec3 rd = (scene.ta - scene.ro).normalize();
    vec3 cy = (cx.cross(rd)).normalize();


    #pragma omp parallel for schedule(dynamic, 1) private(color, r)       // OpenMP
    for (unsigned short i = 0; i < height; ++i){
        this -> progress = 100.*i / (height - 1);
        // qDebug() << "Rendering " << "spp:" << (samples + 1) * 4 << " " << 100.*i / (height - 1) << '%';

        for (unsigned short j = 0; j < width; ++j){
            color = colorArray[i*width+j];
            r = vec3();

            // super samples
            for (int sy = 0; sy < 2; ++sy) { // 2x2 subpixel rows
                for (int sx = 0; sx < 2; ++sx) { // 2x2 subpixel cols
                    if (abort){
                        break;
                    }

                    if (restart){
                        break;
                    }

                    // tent filter
                    double r1 = 2 * drand48(), dx = r1 < 1 ? sqrt(r1) - 1 : 1 - sqrt(2 - r1);
                    double r2 = 2 * drand48(), dy = r2 < 1 ? sqrt(r2) - 1 : 1 - sqrt(2 - r2);
                    double u = (j + (sy - 0.5 + dy * 0.5) * 0.5  ) / width;
                    double v = (height - (i + (sx - 0.5 + dx * 0.5) * 0.5 ) ) / height;
                    u = (u * 2.0 - 1.0);
                    v = (v * 2.0 - 1.0);
                    // u = u * width/height;
                    v = v * height/width;

                    vec3 rd = scene.ca * vec3(u, v, scene.near);
                    // vec3 rd = dir.normalized();
                    if (i == 0 && j == 0){
                        // qDebug() << "dir "<< dir;
                        qDebug() << "rd "<< rd;
                        qDebug()<<"camera shift: " << cam_shift;
                        qDebug()<<"L: " << L;
                        qDebug()<<"cx: " << cx;
                        qDebug()<<"cy: " << cy;
                        // qDebug() << "near" << scene.near * L;
                    }
                    // vec3 rd = scene.ca * (vec3(u, v, scene.near)).normalize();
                    
                    Ray primiaryRay(scene.ro, rd);

                    // If we're actually using depth of field, we need to modify the camera ray to account for that
                    if (scene.focusOn) {
                        vec3 fp = (camera.origin + rd * L) + rd.normalized() * scene.focalLength;
                        // Get a pixel point and new ray rdection to calculate where the rays should intersect
                        // vec3 del_x = (cx * dx * L / float(width));
                        // vec3 del_y = (cy * dy * L / float(height));
                        vec3 del_x = cx * dx * L;
                        vec3 del_y = cy * dy * L;
                        vec3 point = camera.origin + rd * L;
                        point = point + del_x + del_y;
                        rd = (fp - point).normalize();
                        primiaryRay = Ray(point, rd);
                    }
                    else{
                        primiaryRay = Ray(scene.ro, rd.normalized());
                    }

                    // if (scene.focusOn) {
                    //     vec3 fp = (camera.origin) + rd * scene.focalLength;
                    //     // Get a pixel point and new ray direction to calculate where the rays should intersect
                    //     vec3 del_x = cx * dx;
                    //     vec3 del_y = cy * dy;
                    //     vec3 point = camera.origin;
                    //     point = point + del_x + del_y;
                    //     rd = (fp - point).normalize();
                    //     primiaryRay = Ray(camera.origin, rd);
                    // }

                    
                    r = r + tracing(primiaryRay, 0) * 0.25;
                }
            }

            color = (color * samples + vec3(clamp(r.x), clamp(r.y), clamp(r.z))) * (1.0/ (samples + 1));
            colorArray[i*width+j] = color;
        }
    }

    
    
}

void Raytracer::renderIndirect(double &time, QImage &image) {
    struct timeval start, end;
    gettimeofday(&start, NULL);

    

    int samps = samples / 4;
    isRendering = true;
    vec3 r(0,0,0);
    vec3 raw(0,0,0);
    #pragma omp parallel for schedule(dynamic, 1) private(r)       // OpenMP
    for (unsigned short i = 0; i < height; ++i){
        this -> progress = 100.*i / (height - 1);
        // fprintf(stderr, "\rRendering (%d spp) %5.2f%%", samps * gridSize * gridSize, 100.*i / (height - 1));
        qDebug() << "Rendering " << "spp:" <<samps * 4 << " " << 100.*i / (height - 1) << '%';
        for (unsigned short j = 0; j < width; ++j){
            vec3 color;
            for (int sy = 0; sy < 2; ++sy) { // 2x2 subpixel rows
                for (int sx = 0; sx < 2; ++sx) { // 2x2 subpixel cols
                    r = vec3();
                    for (int s = 0; s < samps; ++s) {
                        double r1 = 2 * drand48(), dx = r1 < 1 ? sqrt(r1) - 1 : 1 - sqrt(2 - r1);
                        double r2 = 2 * drand48(), dy = r2 < 1 ? sqrt(r2) - 1 : 1 - sqrt(2 - r2);
                        double u = (j + (sy - 0.5 + dy * 0.5) * 0.5  ) / width;
                        double v = (height - (i + (sx - 0.5 + dx * 0.5) * 0.5 ) ) / height;
                        u = (u * 2.0 - 1.0);
                        v = (v * 2.0 - 1.0);
                        u = u * width/height;
                        vec3 rd = scene.ca * (vec3(u, v, scene.near)).normalize();

                        Ray primiaryRay(scene.ro, rd);
                        r = r + tracing(primiaryRay, 0) * (1.0 / samps);
                    }
                    color = color + vec3(clamp(r.x), clamp(r.y), clamp(r.z)) * .25;
                    
                }
                
            }
            
            raw = color * 255;
            image.setPixel(j, i, qRgb(raw.x, raw.y, raw.z));
        }
    }

    gettimeofday(&end, NULL);
    time = ((end.tv_sec  - start.tv_sec) * 1000000u + end.tv_usec - start.tv_usec) / 1.e6;
    qDebug() << "Render time: " << time;
    isRendering = false;
}

void Raytracer::setResolution(const int &width, const int &height){
    this->width = width;
    this->height = height;
}

Raytracer::~Raytracer(){
    
}

