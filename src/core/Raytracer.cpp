#include "raytracer.h"
#include "utility/Log.h"
#include "utility/ModelLoader.h"
#include "utility/Random.h"

#include "BVH.h"
#include "math/Quaternion.h"

namespace new_type_renderer
{
    inline Matrix3x3 setCamera(Vector3 ro, Vector3 ta, Vector3 up)
    {
        Vector3 cw = (ta - ro).Normalized();
        // Vector3 cp = Vector3(sin(cr), cos(cr), 0.0);
        Vector3 cp = up.Normalized();
        Vector3 cu = cw.Cross(cp).Normalized();
        Vector3 cv = cu.Cross(cw).Normalized();
        return Matrix3x3(cu, cv, cw);
    }

    // inline float GGX_Distribution(Vector3 n, Vector3 h, float alpha){
    //     float NoH = n.Dot(h);
    //     float alpha2 = alpha * alpha;
    //     float NoH2 = NoH * NoH;
    //     float den = NoH2 * alpha2 + (1 - NoH2);
    //     return (chiGGX(NoH) * alpha2) / ( M_PI * den * den );
    // }

    inline float GGX_PartialGeometryTerm(Vector3 v, Vector3 n, Vector3 h, float alpha)
    {
        float VoH2 = Saturate(v.Dot(h));
        float chi = ChiGGX(Saturate(VoH2 / v.Dot(n)));
        VoH2 = VoH2 * VoH2;
        float tan2 = (1 - VoH2) / VoH2;
        return (chi * 2) / (1 + sqrt(1 + alpha * alpha * tan2));
    }


    inline void ons(const Vector3& v1, Vector3& v2, Vector3& v3)
    {
        if (fabs(v1.x) > fabs(v1.y))
        {
            // project to the y = 0 plane and construct a Normalized orthogonal vector in this plane
            float invLen = 1.f / sqrtf(v1.x * v1.x + v1.z * v1.z);
            v2 = Vector3(-v1.z * invLen, 0.0f, v1.x * invLen);
        }
        else
        {
            // project to the x = 0 plane and construct a Normalized orthogonal vector in this plane
            float invLen = 1.0f / sqrtf(v1.y * v1.y + v1.z * v1.z);
            v2 = Vector3(0.0f, v1.z * invLen, -v1.y * invLen);
        }
        v3 = v1.Cross(v2);
    }


    inline double fract(double f)
    {
        return f = f - static_cast<long>(f);
    }

    inline Vector3 cosineSampleHemisphere(double u1, double u2)
    {
        const double r = sqrt(u1);
        const double theta = 2 * M_PI * u2;
        const double x = r * cos(theta);
        const double y = r * sin(theta);
        return Vector3(x, y, sqrt(fmax(0.0f, 1 - u1)));
    }

    inline Vector3 uniformSampleHemisphere(double u1, double u2)
    {
        const double r = sqrt(1.0f - u1 * u1);
        const double phi = 2 * M_PI * u2;
        return Vector3(cos(phi) * r, sin(phi) * r, u1);
    }

    inline Vector3 gammaCorrect(Vector3& v)
    {
        return Vector3(pow(v.x, 1 / 2.2), pow(v.y, 1 / 2.2), pow(v.z, 1 / 2.2));
    }

    Color Raytracer::tracing(Ray& ray, int depth, int E = 1)
    {
        // Intersection intersection = scene.Intersect(ray);
        Intersection intersection = bvh.Intersect(ray);

        Color ambColor(0, 0, 0);
        if (!intersection.object)
        {
            ambColor = getEnvColor(ray.dir);


            return ambColor;
        }
        Object* obj = intersection.object;
        Vector3 hit = ray.origin + ray.dir * intersection.t;
        Vector3 N = obj->getNormal(hit);
        Vector3 nl = N.Dot(ray.dir) < 0 ? N : N * -1;


        Color albedo = obj->getMaterial()->getDiffuseColor(ray.uv);

        Color refractColor = obj->getMaterial()->getRefractColor(ray.uv);
        Color reflectColor = obj->getMaterial()->getReflectColor(ray.uv);

#ifdef RUSSIAN_ROULETTE_TERMINATION
        // Russian roulette termination.
        double p = albedo.x > albedo.y && albedo.x > albedo.z ? albedo.x : albedo.y > albedo.z ? albedo.y : albedo.z; // max refl
        double p1 = refractColor.x > refractColor.y && refractColor.x > refractColor.z ? refractColor.x : refractColor.y > refractColor.z ? refractColor.y : refractColor.z;
        double p2 = reflectColor.x > reflectColor.y && reflectColor.x > reflectColor.z ? reflectColor.x : reflectColor.y > reflectColor.z ? reflectColor.y : reflectColor.z; // max refl
        p = fmax(p, fmax(p1, p2));
        if (++depth > 5) {
            if (drand48() < p * 0.9) { // Multiply by 0.9 to avoid infinite loop with colours of 1.0
                albedo = albedo * (0.9 / p);
                refractColor = refractColor * (0.9 / p);
                reflectColor = reflectColor * (0.9 / p);
            }
            else {
                return obj->getMaterial()->getEmission() * E;
            }
        }
#else

        if (++depth > 5)
        {
            return obj->getMaterial()->getEmission() * E;
        }
#endif

        double randomVal = Random01();
        if (randomVal <= obj->getMaterial()->refract)
        {
            // Vector3 reflectColor = obj->getMaterial()->getReflectColor(ray.uv);


            Vector3 refl = ray.dir - N * 2 * N.Dot(ray.dir);
            Ray reflRay(hit, refl); // Ideal dielectric REFRACTION
            bool into = N.Dot(nl) > 0; // Ray from outside going in?
            double nc = 1, nt = obj->getMaterial()->ior, nnt = into ? nc / nt : nt / nc, ddn = ray.dir.Dot(nl), cos2t;
            if ((cos2t = 1 - nnt * nnt * (1 - ddn * ddn)) < 0)
            {
                // Total internal reflection
                return obj->getMaterial()->getEmission() + albedo * tracing(reflRay, depth);
            }
            Vector3 tdir = (ray.dir * nnt - N * ((into ? 1 : -1) * (ddn * nnt + sqrt(cos2t)))).Normalize();

            Ray refrRay(hit, tdir);
            double a = nt - nc;
            double b = nt + nc;
            double R0 = a * a / (b * b);
            double c = 1 - (into ? -ddn : tdir.Dot(N));
            double Re = R0 + (1 - R0) * c * c * c * c * c;
            double Tr = 1 - Re;
            double P = .25 + .5 * Re;
            double RP = Re / P;
            double TP = Tr / (1 - P);

            return obj->getMaterial()->getEmission() + refractColor * (depth > 2
                                                                           ? (Random01() < P
                                                                                  ? // Russian roulette
                                                                                  tracing(reflRay, depth) * RP
                                                                                  : tracing(refrRay, depth) * TP)
                                                                           : tracing(reflRay, depth) * Re + tracing(
                                                                               refrRay, depth) * Tr);
        }
        if (randomVal <= obj->getMaterial()->refract + obj->getMaterial()->reflection)
        {
#ifdef COOK_TORRANCE
            float roughness = obj->getMaterial()->roughness;
            Vector3 viewVector = -ray.dir;
            // Important sample
            // float rand1 = drand48();
            // float tan_theta = roughness * sqrt(rand1/(1 - rand1));
            float tan_theta = sqrt(-roughness * roughness * log(1 - Random01()));
            // float theta = atan(sqrt(-roughness*roughness*log(1-drand48())));
            float theta = atan(tan_theta);
            float sin_theta = sin(theta);
            float phi = 2 * M_PI * Random01();
            float x = sin_theta * cos(phi);
            float y = cos(theta);
            float z = sin(phi) * sin_theta;

            Vector3 w = N.Normalize();
            Vector3 u = ((fabs(w.x) > 0.1 ? Vector3(0, 1, 0) : Vector3(1, 0, 0)).Cross(w)).Normalize();
            Vector3 v = w.Cross(u).Normalize();

            Vector3 m(
                x * u.x + y * w.x + z * v.x,
                x * u.y + y * w.y + z * v.y,
                x * u.z + y * w.z + z * v.z);
            m.Normalize();

            Vector3 sampleVector = ray.dir.Reflect(m);
            Vector3 halfVector = (sampleVector + viewVector).Normalize();

            Ray reflRay(hit, sampleVector);
            // Calculate fresnel with Fresnel_Schlick approixmation
            float F0 = obj->getMaterial()->F0;
            float fresnel = F0 + (1 - F0) * pow((1 - sampleVector.Dot(halfVector)), 5);

            // Geometry term GGX Distribution
            // float geometry = GGX_PartialGeometryTerm(viewVector, N, halfVector, roughness)
            //  * GGX_PartialGeometryTerm(sampleVector, N, halfVector, roughness);
            float NdotH = (N.Dot(halfVector));
            float NdotL = (N.Dot(sampleVector));
            float VdotH = (viewVector.Dot(halfVector));
            float NdotV = (N.Dot(viewVector));
            float geometry = fmin(1, fmin(2 * NdotH * NdotV / VdotH, 2 * NdotH * NdotL / VdotH));

            // Calculate the Cook-Torrance denominator
            float denominator = 4 * NdotV;
            Color specularColor = reflectColor * Saturate(fresnel * geometry / denominator);

            return obj->getMaterial()->getEmission() + specularColor * tracing(reflRay, depth);


#else

            Vector3 refl = ray.dir - N * 2 * N.Dot(ray.dir);
            Vector3 view = -ray.dir;
            Vector3 half = (view + refl).Normalize();
            float NdotH = saturate(N.Dot(half));
            float NdotL = saturate(N.Dot(refl));
            Ray reflRay(hit, refl);
            float glossy = obj->getMaterial()->glossy;
            // float factor = (glossy+1)/(2*M_PI); original phong
            // float factor = (glossy+2)/(2*M_PI);
            float factor = (glossy + 2) * (glossy + 4) / (8 * M_PI * (pow(2, -glossy * 0.5) + glossy));
            // return obj->getMaterial()->getEmission() + reflectColor * tracing(reflRay, depth);

            return obj->getMaterial()->getEmission() + reflectColor * factor * pow(NdotH, glossy) * NdotL * tracing(reflRay, depth);
#endif
        }
        // cos weighted sample
        double r1 = 2 * M_PI * Random01();
        double r2 = Random01();
        double rad = sqrt(r2);
        Vector3 w = nl;
        Vector3 u = ((fabs(w.x) > 0.1 ? Vector3(0, 1, 0) : Vector3(1, 0, 0)).Cross(w)).Normalize();
        Vector3 v = w.Cross(u);
        Vector3 d = (u * cos(r1) * rad + v * sin(r1) * rad + w * sqrt(1 - r2)).Normalize();

        Ray reflRay(hit, d);
        // Sphere Area light sample
        // Explicit light sample. Only support for sphere light
#ifdef EXPLICIT_LIGHT_SAMPLE
        Color e;
        for (unsigned int i = 0; i < scene.lights.size(); i++)
        {
            Object* light = scene.lights[i];
            Vector3 sw = light->getCentriod() - hit;
            Vector3 su = ((fabs(sw.x) > .1 ? Vector3(0, 1, 0) : Vector3(1, 0, 0)).Cross(sw)).Normalize();
            Vector3 sv = sw.Cross(su);

            // Sphere light
            auto s = static_cast<Sphere*>(light);
            double cos_a_max = sqrt(1 - s->rad * s->rad / (hit - s->getCentriod()).Dot(hit - s->getCentriod()));
            double eps1 = Random01(), eps2 = Random01();
            double cos_a = 1 - eps1 + eps1 * cos_a_max;
            double sin_a = sqrt(1 - cos_a * cos_a);
            double phi = 2 * M_PI * eps2;
            Vector3 l = su * cos(phi) * sin_a + sv * sin(phi) * sin_a + sw * cos_a;
            l.Normalize();

            Ray shadowRay(hit, l);
            Intersection shadow = bvh.Intersect(shadowRay);
            // Intersection shadow = scene.Intersect(shadowRay);

            if (shadow.object && shadow.object == light)
            {
                double omega = 2 * M_PI * (1 - cos_a_max);
                e = e + albedo * M_1_PI * (light->getMaterial()->getEmission() * l.Dot(nl) * omega); // 1/pi for brdf
            }
        }

        return obj->getMaterial()->getEmission() * E + e + albedo * (tracing(reflRay, depth, 0));
#else

            return obj->getMaterial()->getEmission() + albedo * (tracing(reflRay, depth));

#endif
    }

    void Raytracer::unloadScene()
    {
        scene.destroyScene();
        bvh.Destroy();
    }

    void Raytracer::setupScene(const std::string& in_scene_path)
    {
        scenePath = in_scene_path;
        ObjLoader loader;
        std::ifstream t(scenePath);
        std::string json((std::istreambuf_iterator<char>(t)),
                         std::istreambuf_iterator<char>());


        rapidjson::Document document;
        if (document.Parse(json.c_str()).HasParseError())
        {
            LOG_ERR("can't parse the scene");
            return;
        }

        scene.root = new SceneNode();
        if (document.HasMember("camera"))
        {
            rapidjson::Value& camera = document["camera"];
            scene.fov = camera["fov"].GetFloat() * M_PI / 180;
            scene.near = 1.0f / tan(scene.fov * 0.5f);
            const rapidjson::Value& position = document["camera"]["transform"]["position"];
            const rapidjson::Value& target = document["camera"]["transform"]["target"];
            const rapidjson::Value& up = document["camera"]["transform"]["up"];
            scene.ro = Vector3(position[0].GetFloat(), position[1].GetFloat(), position[2].GetFloat());
            scene.ta = Vector3(target[0].GetFloat(), target[1].GetFloat(), target[2].GetFloat());
            scene.up = Vector3(up[0].GetFloat(), up[1].GetFloat(), up[2].GetFloat());
            scene.ca = setCamera(scene.ro, scene.ta, scene.up);

            if (camera.HasMember("focusOn"))
            {
                scene.focusOn = camera["focusOn"].GetBool();
            }
            else
            {
                scene.focusOn = false;
            }
            if (camera.HasMember("focalLength"))
            {
                scene.focalLength = camera["focalLength"].GetFloat();
                // qDebug() << "focal length" << scene.focalLength;
            }

            if (camera.HasMember("aperture"))
            {
                scene.aperture = camera["aperture"].GetFloat();
            }
            else
            {
                scene.aperture = 1;
            }
        }

        if (document.HasMember("envlight"))
        {
            rapidjson::Value& envLight = document["envlight"];
            scene.LoadHdri(envLight["hdri"].GetString());
            scene.envLightIntense = envLight["intense"].GetFloat();
            scene.envLightExp = envLight["exp"].GetFloat();
            if (envLight.HasMember("rotate"))
            {
                scene.envRotate = envLight["rotate"].GetFloat() / 180.0 * M_PI;
            }
        }
        if (document.HasMember("primitives"))
        {
            rapidjson::Value& primitives = document["primitives"];
            rapidjson::Value& materials = document["materials"];

            for (rapidjson::SizeType i = 0; i < primitives.Size(); ++i)
            {
                rapidjson::Value& pos = primitives[i]["transform"]["position"];
                rapidjson::Value& scl = primitives[i]["transform"]["scale"];
                rapidjson::Value& rot = primitives[i]["transform"]["rotation"];

                std::string materialName = primitives[i]["material"].GetString();

                Log::Print(Info, materialName.c_str());
                auto material = new Material();
                if (materials.HasMember(materialName.c_str()))
                {
                    rapidjson::Value& mat = materials[materialName.c_str()];
                    if (mat.HasMember("diffuse"))
                    {
                        material->diffuse = mat["diffuse"].GetFloat();
                    }

                    if (mat.HasMember("specular"))
                    {
                        material->specular = mat["specular"].GetFloat();
                    }

                    if (mat.HasMember("reflection"))
                    {
                        material->reflection = mat["reflection"].GetFloat();
                    }

                    if (mat.HasMember("roughness"))
                    {
                        material->roughness = mat["roughness"].GetFloat();
                    }

                    if (mat.HasMember("glossy"))
                    {
                        material->glossy = mat["glossy"].GetFloat();
                    }

                    if (mat.HasMember("useBackground"))
                    {
                        material->useBackground = mat["useBackground"].GetBool();
                    }


                    if (mat.HasMember("metallic"))
                    {
                        material->metallic = mat["metallic"].GetFloat();
                    }

                    if (mat.HasMember("diffuseRoughness"))
                    {
                        material->diffuseRoughness = mat["diffuseRoughness"].GetFloat();
                    }

                    if (mat.HasMember("emission"))
                    {
                        material->emission = mat["emission"].GetFloat();
                    }

                    if (mat.HasMember("ior"))
                    {
                        material->ior = mat["ior"].GetFloat();
                    }

                    if (mat.HasMember("refract"))
                    {
                        material->refract = mat["refract"].GetFloat();
                    }

                    if (mat.HasMember("diffuseColor"))
                    {
                        material->diffuseColor = Color(mat["diffuseColor"][0].GetFloat(),
                                                         mat["diffuseColor"][1].GetFloat(),
                                                         mat["diffuseColor"][2].GetFloat());
                    }

                    if (mat.HasMember("reflectColor"))
                    {
                        material->reflectColor = Color(mat["reflectColor"][0].GetFloat(),
                                                         mat["reflectColor"][1].GetFloat(),
                                                         mat["reflectColor"][2].GetFloat());
                    }

                    if (mat.HasMember("refractColor"))
                    {
                        material->refractColor = Color(mat["refractColor"][0].GetFloat(),
                                                         mat["refractColor"][1].GetFloat(),
                                                         mat["refractColor"][2].GetFloat());
                    }

                    if (mat.HasMember("emissionColor"))
                    {
                        material->setEmission(Color(mat["emissionColor"][0].GetFloat(),
                                                      mat["emissionColor"][1].GetFloat(),
                                                      mat["emissionColor"][2].GetFloat()));
                    }

                    if (mat.HasMember("diffuseTexture"))
                    {
                        material->setDiffuseTexture(mat["diffuseTexture"].GetString());
                    }
                }
                else
                {
                    LOG_ERR("can't find material ", materialName.c_str());
                }

                shared_ptr<Object> obj;
                std::string ptype = primitives[i]["type"].GetString();

                if (ptype == "box")
                {
                    obj = make_shared<Box>(Vector3{ 1.0f, 1.0f, 1.0f });
                }
                else if (ptype == "sphere")
                {
                    obj = make_shared<Sphere>(1.0f);
                }
                else if (ptype == "mesh")
                {
                    auto mesh = make_shared<Mesh>();
                    mesh->name = "mesh";
                    std::string modelName = primitives[i]["path"].GetString();
                    loader.LoadModel(modelName, mesh.get(), material);
                    obj = mesh;
                }
                else
                {
                    LOG_ERR("error");
                    obj = make_shared<Object>();
                }

                obj->setMaterial(material);


                auto node = make_shared<SceneNode>();
                node->AddObject(obj);

                node->transform.SetLocation(Vector3{ pos[0].GetFloat(), pos[1].GetFloat(), pos[2].GetFloat() });
                node->transform.SetScale(Vector3{ scl[0].GetFloat(), scl[1].GetFloat(), scl[2].GetFloat() });
                auto orientation = Quaternion(rot[0].GetFloat() * M_PI / 180, rot[1].GetFloat() * M_PI / 180,
                                              rot[2].GetFloat() * M_PI / 180);
                node->transform.SetOrientation(orientation);
                obj->name = primitives[i]["name"].GetString();
                scene.root->AddChild(node);
                LOG_INFO("add ", obj->name.c_str(), " to the scene");
            }
        }
        // brdf test
        // float r = 16;
        // int NUM_BALL_ROWS = 8;
        // int NUM_BALLS_PER_ROW = 5;
        // for (int i = 0; i < NUM_BALL_ROWS; ++i){
        //     // float xoff = float(NUM_BALL_ROWS) * 2.1 * r * (float(j)/float(NUM_BALL_ROWS-1) - .5);
        //     for (int j = 0; j < NUM_BALLS_PER_ROW; ++j){
        //         Material *material = new Material();

        //         // material->roughness = saturate(pow( (i*1.0/NUM_BALL_ROWS), 2));
        //         material->roughness = saturate(pow( (i*1.0/NUM_BALL_ROWS), 1));

        //         material->metallic = saturate(j*1.0/NUM_BALLS_PER_ROW);
        //         // material->reflection = material->metallic;
        //         // material->diffuse = 1 - material->reflection;
        //         material->reflection = 1;
        //         // material->diffuse = 1 - material->F0;
        //         material->refract = 0;
        //         // qDebug() << "diffuse" << material->diffuse;
        //         // material->metallic = 0;
        //         material->ior = 1.4;
        //         // material->reflectColor = Vector3(1,1,1);
        //         material->reflectColor = Vector3(1,1,1);
        //         // material->reflectColor = Vector3(1., 0.35, 0.5);
        //         // material->diffuseColor = Vector3(1., 0.35, 0.5);
        //         material->diffuseColor = Vector3(1,1,1);


        //         Object *obj = (Object*)new Sphere(1.0);
        //         obj->setMaterial(material);
        //         // obj->name = "ball_" + std::to_string(i) + '_' + std::to_string(j);


        //         qDebug() << obj->name.c_str() << material->F0;
        //         SceneNode *xform = new SceneNode(obj);
        //         xform->setTranslate((r + 25) * i, 17, (r + 25) * j);
        //         xform->setScale(r,r,r);
        //         scene.root->addChild(xform);
        //     }
        // }

        scene.updateTransform(scene.root, Matrix4x4());
        bvh.Setup(scene);
    }

    Raytracer::Raytracer(unsigned _width, unsigned _height, int _samples)
    {
        width = _width;
        height = _height;
        samples = _samples;

        std::string base_path = "";
        // std::string name = "/scene/empty.json";
        // std::string name = "/scene/sportcar.json";
        // std::string name = "/scene/plane.json";
        std::string name = "/scene/cornellbox.json";
        // std::string name = "/scene/sponza.json";
        std::string fullpath = base_path + name;
        setupScene(fullpath);
    }

    void Raytracer::scaleCamera(float scl)
    {
        scl = fmax(-0.99, scl);
        scl = -scl;
        // scene.cameraScale = scene.cameraScale + scl;
        Vector3 dir = scene.ro - scene.ta;
        // float scale = dir.Length() + scl;
        // scale = scale + scl;
        // Matrix4x4 m = Matrix4x4(scale, 0, 0,   1,
        //               0, scale, 0,   1,
        //               0, 0,   scale, 1,
        //               0, 0,   0,   1);

        // Vector4 ro = m * Vector4(scene.ro, 1);

        scene.ro = scene.ta + dir * (1 + scl);
        // scene.ro = Vector3(ro.x, ro.y, ro.z);
        // scene.ta = Vector3(ta.x, ta.y, ta.z);
        scene.ca = setCamera(scene.ro, scene.ta, scene.up);
    }

    void Raytracer::rotateCamera(float x, float y, float z)
    {
        Quaternion rot(x, y, z);
        Matrix4x4 m = rot.ToMatrix();

        Vector4 ro = m * Vector4(scene.ro, 1);
        scene.ro = Vector3(ro.x, ro.y, ro.z);

        Vector4 ta = m * Vector4(scene.ta, 1);
        scene.ta = Vector3(ta.x, ta.y, ta.z);
        scene.ca = setCamera(scene.ro, scene.ta, scene.up);
    }

    void Raytracer::moveCamera(float x, float y)
    {
        // Matrix4x4 m = Matrix4x4();
        Vector3 move(-x, y, 0);
        Vector3 cw = (scene.ta - scene.ro).Normalize();
        auto up = Vector3(0, 1, 0);
        Vector3 cp = up.Normalized();
        Vector3 cu = cw.Cross(cp).Normalized();
        Vector3 cv = cu.Cross(cw).Normalized();
        auto m = Matrix3x3(cu, cv, cw);

        move = m * move;
        scene.ta = scene.ta + move;
        scene.ro = scene.ro + move;
        scene.ca = setCamera(scene.ro, scene.ta, scene.up);
    }

    Color Raytracer::getEnvColor(const Vector3& dir) const
    {
        Color ambColor(0, 0, 0);
        if (scene.hasHdri)
        {
            Matrix3x3 m(cos(scene.envRotate), 0, sin(scene.envRotate),
                        0, 1, 0,
                        sin(scene.envRotate), 0, -cos(scene.envRotate));
            Vector3 newdir = m * dir;
            // hdri  
            HDRImage hdri = scene.hdri;
            double u = atan2(-newdir.x, newdir.z) / (2.0 * M_PI) + 0.5;
            double v = asin(-newdir.y) / M_PI + 0.5;
            int hdrx = u * (hdri.width - 1);
            int hdry = v * (hdri.height - 1);
            double r = hdri.colors[hdry * hdri.width * 3 + hdrx * 3];
            double g = hdri.colors[hdry * hdri.width * 3 + hdrx * 3 + 1];
            double b = hdri.colors[hdry * hdri.width * 3 + hdrx * 3 + 2];
            r = pow(r, 0.44);
            g = pow(g, 0.44);
            b = pow(b, 0.44);
            r = scene.envLightIntense * pow(r, scene.envLightExp);
            g = scene.envLightIntense * pow(g, scene.envLightExp);
            b = scene.envLightIntense * pow(b, scene.envLightExp);
            // return Vector3(r,g,b);
            ambColor = Color(r, g, b);
        }
        return ambColor;
    }

    void Raytracer::testPixel(int x, int y)
    {
        float near = scene.near;
        Matrix3x3 ca = scene.ca;
        Vector3 ro = scene.ro;
        double u = x * 1.0 / width;
        double v = (height - y) * 1.0 / height;
        u = (u * 2.0 - 1.0);
        v = (v * 2.0 - 1.0);
        // u = u * width/height;
        v = v * height / width;
        Vector3 rd = ca * (Vector3(u, v, near)).Normalize();
        Ray ray(ro, rd);


        testRaytracing(ray, 0);

        // ambColor = getEnvColor(ray.dir);
        // if (intersection.object) {
        // }
    }

    void Raytracer::testRaytracing(Ray& ray, int depth)
    {
        if (++depth > 2)
        {
            return;
        }

        Intersection intersection = bvh.Intersect(ray);

        if (!intersection.object)
        {
            return;
        }
        Object* obj = intersection.object;
        LOG_INFO("  hit:", obj->name.c_str());
        Vector3 hit = ray.origin + ray.dir * intersection.t;
        Vector3 N = obj->getNormal(hit);
        Vector3 nl = N.Dot(ray.dir) < 0 ? N : N * -1;

        double r1 = 2 * M_PI * Random01();
        double r2 = Random01();
        double rad = sqrt(r2);
        Vector3 w = nl;
        Vector3 u = ((fabs(w.x) > 0.1 ? Vector3(0, 1, 0) : Vector3(1, 0, 0)).Cross(w)).Normalize();
        Vector3 v = w.Cross(u);
        Vector3 d = (u * cos(r1) * rad + v * sin(r1) * rad + w * sqrt(1 - r2)).Normalize();

        Ray reflRay(hit, d);
        testRaytracing(reflRay, depth);
    }

    void Raytracer::renderDirect(double& time, Image& directImage, Image& normalImage, Image& boundingBoxImage)
    {
        float near = scene.near;
        Matrix3x3 ca = scene.ca;
        Vector3 ro = scene.ro;
        Color normalColor;
        Color directColor;
        Color boundingBoxColor;
        Color ambColor(0.15, 0.15, 0.15);

        // Vector3 ambColor(0,0,0);
        // Vector3 lig = Vector3(-1, -3, -1.5).Normalize();
        Vector3 pointLig(50, 78, 60);

        for (unsigned int i = 0; i < scene.lights.size(); i++)
        {
            Object* light = scene.lights[i];
            pointLig = light->getCentriod();
            break;
        }

#pragma omp parallel for schedule(dynamic, 1)  private(directColor, normalColor, boundingBoxColor, ambColor)    // OpenMP

        for (unsigned short i = 0; i < height; ++i)
        {
            for (unsigned short j = 0; j < width; ++j)
            {
                double u = j * 1.0 / width;
                double v = (height - i) * 1.0 / height;
                u = (u * 2.0 - 1.0);
                v = (v * 2.0 - 1.0);
                // u = u * width/height;
                v = v * height / width;
                Vector3 rd = ca * (Vector3(u, v, near)).Normalize();
                normalColor = Color(0, 0, 0);
                directColor = Color(0, 0, 0);
                boundingBoxColor = Color(0, 0, 0);

                Ray ray(ro, rd);
                // 

#ifdef WIREFRAME_ON
// Intersection intersectionBox = bvh.IntersectBoundingBox(Ray(ro, rd));
                Intersection intersectionBox = bvh.IntersectBVH(ray);
                if (intersectionBox.t > eps && intersectionBox.t < inf) {
                    boundingBoxColor = Vector3(0, 1, 0);
                }
#endif


                Intersection intersection = bvh.Intersect(ray);
                ambColor = getEnvColor(ray.dir);
                if (intersection.object)
                {
                    Object* obj = intersection.object;
                    // Vector3 f = obj->getMaterial()->getDiffuseColor(ray.uv);
                    // Vector3 hit = ro + rd * intersection.t;
                    // Vector3 N = obj->getNormal(hit);
                    Vector3 N = ray.normal;
                    // N = ray.dir.Dot(N) < 0 ? N : -N;
                    normalColor = Color((N.x + 1) * 0.5, (N.y + 1) * 0.5, (N.z + 1) * 0.25 + 0.5) * 255;
                    // normalColor = obj->c * 255;

                    // Vector3 ld = (pointLig - hit).Normalize();

                    Color albedo;
                    if (obj->getMaterial()->useBackground)
                    {
                        // albedo = getEnvColor(ray.dir)/fmax(ld.Dot(N), 0);
                        albedo = getEnvColor(ray.dir);
                    }
                    else
                    {
                        albedo = obj->getMaterial()->getDiffuseColor(ray.uv);
                    }
                    // Vector3 diffuseColor =  albedo * fmax(ld.Dot(N), 0) * obj->getMaterial()->diffuse;

                    // Vector3 reflect = ray.dir.reflect(N);
                    // Vector3 specularColor = obj->getMaterial()->getReflectColor(ray.uv) * this->getEnvColor(reflect) * pow(fmax(reflect.Dot(ld), 0), 10) * obj->getMaterial()->reflection;

                    // Ray shadowRay(hit, ld);
                    // Intersection shadow = bvh.Intersect(shadowRay);
                    // double distToLight = (pointLig - hit).Length();
                    // if (shadow.object && shadow.t <= distToLight){
                    //     directColor = directColor * clamp(3.8 * shadow.t/(distToLight), 0.0, 1.0); 
                    //     // directColor = Vector3();
                    // }
                    // directColor = diffuseColor + specularColor;
                    directColor = albedo * N.Dot(-ray.dir);
                }
                else
                {
                    directColor = ambColor;
                }
                // directColor = directColor + ambColor;

                boundingBoxColor = boundingBoxColor * 255;
                directColor = directColor * 255;
                directColor = Clamp(directColor, Color(0, 0, 0), Color(255, 255, 255));
                boundingBoxColor = (directColor + boundingBoxColor);
                boundingBoxColor = Clamp(boundingBoxColor, Color(0, 0, 0), Color(255, 255, 255));
                boundingBoxImage.SetPixel(j, i, Color(boundingBoxColor.x, boundingBoxColor.y, boundingBoxColor.z));
                normalImage.SetPixel(j, i, Color(normalColor.x, normalColor.y, normalColor.z));
                directImage.SetPixel(j, i, Color(directColor.x, directColor.y, directColor.z));
            }
        }
    }

    void Raytracer::renderIndirectProgressive(Color* colorArray, bool& abort, bool& restart, int& samples)
    {
        Color color(0, 0, 0);
        Color radiance(0, 0, 0);

        scene.focalLength = (scene.ta - scene.ro).Length();

        // ratio of original/new aperture (>1: smaller view angle, <1: larger view angle)
        double aperture = 0.5135 / scene.aperture;

        // Vector3 dir_norm = Vector3(0, -0.042612, -1).Normalize();
        Vector3 dir_norm = (scene.ta - scene.ro).Normalize();
        double L = scene.near;
        double L_new = aperture * L;
        double L_diff = L - L_new;
        Vector3 cam_shift = dir_norm * (L_diff);
        if (L_diff < 0)
        {
            cam_shift = cam_shift * 1.5;
        }

        L = L_new;
        auto camera = Ray(scene.ro + cam_shift, dir_norm);
        // Cross product gets the vector perpendicular to cx and the "gaze" direction
        auto cx = Vector3((width * 1.0) / height, 0, 0);
        Vector3 rd = (scene.ta - scene.ro).Normalize();
        Vector3 cy = (cx.Cross(rd)).Normalize();


#pragma omp parallel for schedule(dynamic, 1) private(color, radiance)       // OpenMP
        for (unsigned short i = 0; i < height; ++i)
        {
            this->progress = 100. * i / (height - 1);
            // qDebug() << "Rendering " << "spp:" << (samples + 1) * 4 << " " << 100.*i / (height - 1) << '%';

            for (unsigned short j = 0; j < width; ++j)
            {
                color = colorArray[i * width + j];
                radiance = Color();

                // super samples
                for (int sy = 0; sy < 2; ++sy)
                {
                    // 2x2 subpixel rows
                    for (int sx = 0; sx < 2; ++sx)
                    {
                        // 2x2 subpixel cols
                        if (abort)
                        {
                            break;
                        }

                        if (restart)
                        {
                            break;
                        }

                        // tent filter
                        double r1 = 2 * Random01(), dx = r1 < 1 ? sqrt(r1) - 1 : 1 - sqrt(2 - r1);
                        double r2 = 2 * Random01(), dy = r2 < 1 ? sqrt(r2) - 1 : 1 - sqrt(2 - r2);
                        double u = (j + (sy - 0.5 + dy * 0.5) * 0.5) / width;
                        double v = (height - (i + (sx - 0.5 + dx * 0.5) * 0.5)) / height;
                        u = (u * 2.0 - 1.0);
                        v = (v * 2.0 - 1.0);
                        // u = u * width/height;
                        v = v * height / width;

                        Vector3 rd = scene.ca * Vector3(u, v, scene.near);
                        // Vector3 rd = dir.Normalized();
                        // if (i == 0 && j == 0){
                        //     // qDebug() << "dir "<< dir;
                        //     qDebug() << "rd "<< rd;
                        //     qDebug()<<"camera shift: " << cam_shift;
                        //     qDebug()<<"L: " << L;
                        //     qDebug()<<"cx: " << cx;
                        //     qDebug()<<"cy: " << cy;
                        //     // qDebug() << "near" << scene.near * L;
                        // }
                        // Vector3 rd = scene.ca * (Vector3(u, v, scene.near)).Normalize();

                        Ray primiaryRay(scene.ro, rd);

                        // If we're actually using depth of field, we need to modify the camera ray to account for that
                        if (scene.focusOn)
                        {
                            Vector3 fp = (camera.origin + rd * L) + rd.Normalized() * scene.focalLength;
                            // Get a pixel point and new ray rdection to calculate where the rays should Intersect
                            // Vector3 del_x = (cx * dx * L / float(width));
                            // Vector3 del_y = (cy * dy * L / float(height));
                            Vector3 del_x = cx * dx * L;
                            Vector3 del_y = cy * dy * L;
                            Vector3 point = camera.origin + rd * L;
                            point = point + del_x + del_y;
                            rd = (fp - point).Normalize();
                            primiaryRay = Ray(point, rd);
                        }
                        else
                        {
                            primiaryRay = Ray(scene.ro, rd.Normalized());
                        }

                        radiance = radiance + tracing(primiaryRay, 0) * 0.25;
                    }
                }

                // Vector3 laverage += radiance
                // //Ld - this part of the code is the same for both versions
                // float lum = Dot(rgb, Vector3(0.2126f, 0.7152f, 0.0722f));
                // float L = (scale / averageLum) * radiance;
                // float Ld = (L * (1.0 + L / lumwhite2)) / (1.0 + L);
                // //first
                // Vector3 xyY = RGBtoxyY(rgb);
                // xyY.z *= Ld;


                // rgb = xyYtoRGB(xyY);
                //second
                // rgb = (rgb / radiance) * Ld;
                // float luma = Dot(color, Vector3(0.2126, 0.7152, 0.0722));
                // float toneMappedLuma = luma / (1.0 + luma);
                // color *= toneMappedLuma / luma;
                // color = pow(color, Vector3(1.0 / gamma));

                // clamp
                color = (color * samples + Color(Clamp01(radiance.x), Clamp01(radiance.y), Clamp01(radiance.z))) * (1.0 / (
                    samples + 1));
                colorArray[i * width + j] = color;
            }
        }
    }

    // Vector3 Raytracer::toneMapping(Vector3 &cradiance)const{

    // }

    void Raytracer::renderIndirect(double& time, Image& image)
    {
        int samps = samples / 4;
        isRendering = true;
        Color r(0, 0, 0);
        Vector3 raw(0, 0, 0);
#pragma omp parallel for schedule(dynamic, 1) private(r)       // OpenMP
        for (unsigned short i = 0; i < height; ++i)
        {
            this->progress = 100. * i / (height - 1);
            // fprintf(stderr, "\rRendering (%d spp) %5.2f%%", samps * gridSize * gridSize, 100.*i / (height - 1));
            LOG_INFO("Rendering ", "spp:", samps * 4, " ", 100. * i / (height - 1), '%');
            for (unsigned short j = 0; j < width; ++j)
            {
                Vector3 color;
                for (int sy = 0; sy < 2; ++sy)
                {
                    // 2x2 subpixel rows
                    for (int sx = 0; sx < 2; ++sx)
                    {
                        // 2x2 subpixel cols
                        r = Color();
                        for (int s = 0; s < samps; ++s)
                        {
                            double r1 = 2 * Random01(), dx = r1 < 1 ? sqrt(r1) - 1 : 1 - sqrt(2 - r1);
                            double r2 = 2 * Random01(), dy = r2 < 1 ? sqrt(r2) - 1 : 1 - sqrt(2 - r2);
                            double u = (j + (sy - 0.5 + dy * 0.5) * 0.5) / width;
                            double v = (height - (i + (sx - 0.5 + dx * 0.5) * 0.5)) / height;
                            u = (u * 2.0 - 1.0);
                            v = (v * 2.0 - 1.0);
                            u = u * width / height;
                            Vector3 rd = scene.ca * (Vector3(u, v, scene.near)).Normalize();

                            Ray primiaryRay(scene.ro, rd);
                            r = r + tracing(primiaryRay, 0) * (1.0f / samps);
                        }
                        color = color + Vector3(Clamp01(r.x), Clamp01(r.y), Clamp01(r.z)) * .25;
                    }
                }

                raw = color * 255;
                image.SetPixel(j, i, Color(raw.x, raw.y, raw.z));
            }
        }

        isRendering = false;
    }

    void Raytracer::setResolution(const int& in_width, const int& in_height)
    {
        width = in_width;
        height = in_height;
    }

    Raytracer::~Raytracer()
    {
    }
}
