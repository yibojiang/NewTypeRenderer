#include "raytracer.h"
#include "utility/Log.h"
#include "utility/ModelLoader.h"
#include "math/Common.h"

#include "BVH.h"
#include "basic/Color.h"
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

    Color Raytracer::Tracing(Ray& ray, int depth, int E = 1)
    {
        // Intersection intersection = m_Scene.Intersect(ray);
        Intersection intersection = m_Scene.Intersect(ray);

        Color ambColor(0, 0, 0);
        if (intersection.m_HitObject.expired() == true)
        {
            ambColor = GetEnvColor(ray.dir);


            return ambColor;
        }
        auto obj = intersection.m_HitObject.lock();
        Vector3 hit = ray.origin + ray.dir * intersection.m_Distance;
        Vector3 N = obj->GetHitNormal(hit);
        Vector3 nl = N.Dot(ray.dir) < 0 ? N : N * -1;


        Color albedo = obj->GetMaterial()->GetDiffuseColor(intersection.m_UV);

        Color refractColor = obj->GetMaterial()->GetRefractColor(intersection.m_UV);
        Color reflectColor = obj->GetMaterial()->GetReflectColor(intersection.m_UV);

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
                return obj->GetMaterial()->getEmission() * E;
            }
        }
#else

        if (++depth > 5)
        {
            return obj->GetMaterial()->GetEmission() * E;
        }
#endif

        double randomVal = Random01();
        if (randomVal <= obj->GetMaterial()->m_Refract)
        {
            // Vector3 reflectColor = obj->GetMaterial()->getReflectColor(ray.uv);


            Vector3 refl = ray.dir - N * 2 * N.Dot(ray.dir);
            Ray reflRay(hit, refl); // Ideal dielectric REFRACTION
            bool into = N.Dot(nl) > 0; // Ray from outside going in?
            double nc = 1, nt = obj->GetMaterial()->m_IOR, nnt = into ? nc / nt : nt / nc, ddn = ray.dir.Dot(nl), cos2t;
            if ((cos2t = 1 - nnt * nnt * (1 - ddn * ddn)) < 0)
            {
                // Total internal reflection
                return obj->GetMaterial()->GetEmission() + albedo * Tracing(reflRay, depth);
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

            return obj->GetMaterial()->GetEmission() + refractColor * (depth > 2
                                                                           ? (Random01() < P
                                                                                  ? // Russian roulette
                                                                                  Tracing(reflRay, depth) * RP
                                                                                  : Tracing(refrRay, depth) * TP)
                                                                           : Tracing(reflRay, depth) * Re + Tracing(
                                                                               refrRay, depth) * Tr);
        }
        if (randomVal <= obj->GetMaterial()->m_Refract + obj->GetMaterial()->m_Reflection)
        {
#ifdef COOK_TORRANCE
            float roughness = obj->GetMaterial()->m_Roughness;
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
            float F0 = obj->GetMaterial()->m_F0;
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

            return obj->GetMaterial()->GetEmission() + specularColor * Tracing(reflRay, depth);


#else

            Vector3 refl = ray.dir - N * 2 * N.Dot(ray.dir);
            Vector3 view = -ray.dir;
            Vector3 half = (view + refl).Normalize();
            float NdotH = saturate(N.Dot(half));
            float NdotL = saturate(N.Dot(refl));
            Ray reflRay(hit, refl);
            float glossy = obj->GetMaterial()->glossy;
            // float factor = (glossy+1)/(2*M_PI); original phong
            // float factor = (glossy+2)/(2*M_PI);
            float factor = (glossy + 2) * (glossy + 4) / (8 * M_PI * (pow(2, -glossy * 0.5) + glossy));
            // return obj->GetMaterial()->getEmission() + reflectColor * Tracing(reflRay, depth);

            return obj->GetMaterial()->getEmission() + reflectColor * factor * pow(NdotH, glossy) * NdotL * Tracing(reflRay, depth);
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
        for (unsigned int i = 0; i < m_Scene.m_Lights.size(); i++)
        {
            auto light = m_Scene.m_Lights[i];
            Vector3 sw = light->GetCentriod() - hit;
            Vector3 su = ((fabs(sw.x) > .1 ? Vector3(0, 1, 0) : Vector3(1, 0, 0)).Cross(sw)).Normalize();
            Vector3 sv = sw.Cross(su);

            // Sphere light
            auto s = dynamic_pointer_cast<Sphere>(light);
            double cos_a_max = sqrt(1 - s->rad * s->rad / (hit - s->GetCentriod()).Dot(hit - s->GetCentriod()));
            double eps1 = Random01(), eps2 = Random01();
            double cos_a = 1 - eps1 + eps1 * cos_a_max;
            double sin_a = sqrt(1 - cos_a * cos_a);
            double phi = 2 * M_PI * eps2;
            Vector3 l = su * cos(phi) * sin_a + sv * sin(phi) * sin_a + sw * cos_a;
            l.Normalize();

            Ray shadowRay(hit, l);
            Intersection shadow = m_Scene.Intersect(shadowRay);
            // Intersection shadow = m_Scene.Intersect(shadowRay);

            if (shadow.m_HitObject.expired() == false && shadow.m_HitObject.lock() == light)
            {
                double omega = 2 * M_PI * (1 - cos_a_max);
                e = e + albedo * M_1_PI * (light->GetMaterial()->GetEmission() * l.Dot(nl) * omega); // 1/pi for brdf
            }
        }

        return obj->GetMaterial()->GetEmission() * E + e + albedo * (Tracing(reflRay, depth, 0));
#else

            return obj->GetMaterial()->getEmission() + albedo * (Tracing(reflRay, depth));

#endif
    }

    Raytracer::Raytracer(unsigned _width, unsigned _height, int _samples)
    {
        m_Width = _width;
        m_Height = _height;
        m_Samples = _samples;

        std::string base_path = "";
        std::string name = "/scene/cornellbox.json";
        std::string fullpath = base_path + name;
        m_Scene.LoadFromJson(fullpath);
    }

    Color Raytracer::GetEnvColor(const Vector3& dir) const
    {
        Color ambColor(0, 0, 0);
        if (m_Scene.m_HasHdri)
        {
            Matrix3x3 m(cos(m_Scene.m_EnvRotate), 0, sin(m_Scene.m_EnvRotate),
                        0, 1, 0,
                        sin(m_Scene.m_EnvRotate), 0, -cos(m_Scene.m_EnvRotate));
            Vector3 newdir = m * dir;
            // m_HDRI  
            HDRImage hdri = m_Scene.m_HDRI;
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
            r = m_Scene.m_EnvLightIntense * pow(r, m_Scene.m_EnvLightExp);
            g = m_Scene.m_EnvLightIntense * pow(g, m_Scene.m_EnvLightExp);
            b = m_Scene.m_EnvLightIntense * pow(b, m_Scene.m_EnvLightExp);
            // return Vector3(r,g,b);
            ambColor = Color(r, g, b);
        }
        return ambColor;
    }

    void Raytracer::TestPixel(int x, int y)
    {
        Matrix4x4 viewMatrix = m_Scene.m_Camera.GetViewMatrix();
        Vector3 ro = m_Scene.m_Camera.m_Location;
        double u = x * 1.0 / m_Width;
        double v = (m_Height - y) * 1.0 / m_Height;
        u = (u * 2.0 - 1.0);
        v = (v * 2.0 - 1.0);
        v = v * m_Height / m_Width;
        Vector3 rd = viewMatrix * (Vector3(u, v, m_Scene.m_Camera.m_Near)).Normalize();
        Ray ray(ro, rd);
        TestRaytracing(ray, 0);
    }

    void Raytracer::TestRaytracing(Ray& ray, int depth)
    {
        if (++depth > 2)
        {
            return;
        }

        Intersection intersection = m_Scene.Intersect(ray);

        if (intersection.m_HitObject.expired())
        {
            return;
        }
        auto obj = intersection.m_HitObject.lock();
        LOG_INFO("  hit:", obj->name.c_str());
        Vector3 hit = ray.origin + ray.dir * intersection.m_Distance;
        Vector3 N = obj->GetHitNormal(hit);
        Vector3 nl = N.Dot(ray.dir) < 0 ? N : N * -1;

        double r1 = 2 * M_PI * Random01();
        double r2 = Random01();
        double rad = sqrt(r2);
        Vector3 w = nl;
        Vector3 u = ((fabs(w.x) > 0.1 ? Vector3(0, 1, 0) : Vector3(1, 0, 0)).Cross(w)).Normalize();
        Vector3 v = w.Cross(u);
        Vector3 d = (u * cos(r1) * rad + v * sin(r1) * rad + w * sqrt(1 - r2)).Normalize();

        Ray reflRay(hit, d);
        TestRaytracing(reflRay, depth);
    }

    void Raytracer::RenderDirect(float& time, Image& directImage, Image& normalImage, Image& boundingBoxImage)
    {
        Matrix4x4 viewMatrix = m_Scene.m_Camera.GetViewMatrix();
        Vector3 ro = m_Scene.m_Camera.GetLocation();
        Color normalColor;
        Color directColor;
        Color boundingBoxColor;
        Color ambColor(0.15, 0.15, 0.15);

        // Vector3 ambColor(0,0,0);
        // Vector3 lig = Vector3(-1, -3, -1.5).Normalize();
        Vector3 pointLig(50, 78, 60);

        for (unsigned int i = 0; i < m_Scene.m_Lights.size(); i++)
        {
            auto light = m_Scene.m_Lights[i];
            pointLig = light->GetCentriod();
            break;
        }

#pragma omp parallel for schedule(dynamic, 1)  private(directColor, normalColor, boundingBoxColor, ambColor)    // OpenMP

        for (unsigned short i = 0; i < m_Height; ++i)
        {
            for (unsigned short j = 0; j < m_Width; ++j)
            {
                double u = j * 1.0 / m_Width;
                double v = (m_Height - i) * 1.0 / m_Height;
                u = (u * 2.0 - 1.0);
                v = (v * 2.0 - 1.0);
                // u = u * m_Width/m_Height;
                v = v * m_Height / m_Width;
                Vector3 rd = viewMatrix * (Vector3(u, v, m_Scene.m_Camera.m_Near)).Normalize();
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
                Intersection intersection = m_Scene.Intersect(ray);
                ambColor = GetEnvColor(ray.dir);
                if (intersection.m_HitObject.expired() == false)
                {
                    auto obj = intersection.m_HitObject.lock();
                    // Vector3 f = obj->GetMaterial()->getDiffuseColor(ray.uv);
                    // Vector3 hit = ro + rd * intersection.t;
                    // Vector3 N = obj->GetNormal(hit);
                    Vector3 N = intersection.m_Normal;
                    // N = ray.dir.Dot(N) < 0 ? N : -N;
                    normalColor = Color((N.x + 1) * 0.5, (N.y + 1) * 0.5, (N.z + 1) * 0.25 + 0.5) * 255;
                    // normalColor = obj->c * 255;

                    // Vector3 ld = (pointLig - hit).Normalize();

                    Color albedo;
                    if (obj->GetMaterial()->m_UseBackground)
                    {
                        // albedo = GetEnvColor(ray.dir)/fmax(ld.Dot(N), 0);
                        albedo = GetEnvColor(ray.dir);
                    }
                    else
                    {
                        albedo = obj->GetMaterial()->GetDiffuseColor(intersection.m_UV);
                    }
                    // Vector3 diffuseColor =  albedo * fmax(ld.Dot(N), 0) * obj->GetMaterial()->diffuse;

                    // Vector3 reflect = ray.dir.reflect(N);
                    // Vector3 specularColor = obj->GetMaterial()->getReflectColor(ray.uv) * this->GetEnvColor(reflect) * pow(fmax(reflect.Dot(ld), 0), 10) * obj->GetMaterial()->reflection;

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

    void Raytracer::RenderIndirectProgressive(Color* colorArray, bool& abort, bool& restart, int& samples)
    {
        Color color(0, 0, 0);
        Color radiance(0, 0, 0);

        Camera& camera = m_Scene.m_Camera;
        // camera.m_FocalLength = (camera.ta - camera.ro).Length();

        // ratio of original/new m_Aperture (>1: smaller view angle, <1: larger view angle)
        double aperture = 0.5135 / camera.m_Aperture;

        // Vector3 dir_norm = Vector3(0, -0.042612, -1).Normalize();
        Vector3 dir_norm = (m_Scene.m_Camera.GetLookAt() - m_Scene.m_Camera.GetLocation()).Normalize();
        double L = m_Scene.m_Camera.m_Near;
        double L_new = aperture * L;
        double L_diff = L - L_new;
        Vector3 cam_shift = dir_norm * (L_diff);
        if (L_diff < 0)
        {
            cam_shift = cam_shift * 1.5;
        }

        L = L_new;
        auto camera_ray = Ray(m_Scene.m_Camera.GetLocation() + cam_shift, dir_norm);
        // Cross product gets the vector perpendicular to cx and the "gaze" direction
        auto cx = Vector3((m_Width * 1.0) / m_Height, 0, 0);
        Vector3 rd = (m_Scene.m_Camera.GetLookAt() - m_Scene.m_Camera.GetLocation()).Normalize();
        Vector3 cy = (cx.Cross(rd)).Normalize();
        Matrix4x4 viewMatrix = m_Scene.m_Camera.GetViewMatrix();

#pragma omp parallel for schedule(dynamic, 1) private(color, radiance)       // OpenMP
        for (unsigned short i = 0; i < m_Height; ++i)
        {
            this->m_Progress = 100. * i / (m_Height - 1);

            for (unsigned short j = 0; j < m_Width; ++j)
            {
                color = colorArray[i * m_Width + j];
                radiance = Color();

                // super m_Samples
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
                        float r1 = 2 * Random01(), dx = r1 < 1 ? sqrt(r1) - 1 : 1 - sqrt(2 - r1);
                        float r2 = 2 * Random01(), dy = r2 < 1 ? sqrt(r2) - 1 : 1 - sqrt(2 - r2);
                        float u = (j + (sy - 0.5 + dy * 0.5) * 0.5) / m_Width;
                        float v = (m_Height - (i + (sx - 0.5 + dx * 0.5) * 0.5)) / m_Height;
                        u = (u * 2.0 - 1.0);
                        v = (v * 2.0 - 1.0);
                        // u = u * m_Width/m_Height;
                        v = v * m_Height / m_Width;

                        Vector3 rd = viewMatrix * Vector3(u, v, m_Scene.m_Camera.m_Near);
                        // Vector3 rd = dir.Normalized();
                        // if (i == 0 && j == 0){
                        //     // qDebug() << "dir "<< dir;
                        //     qDebug() << "rd "<< rd;
                        //     qDebug()<<"camera shift: " << cam_shift;
                        //     qDebug()<<"L: " << L;
                        //     qDebug()<<"cx: " << cx;
                        //     qDebug()<<"cy: " << cy;
                        //     // qDebug() << "m_Near" << m_Scene.m_Near * L;
                        // }
                        // Vector3 rd = m_Scene.ca * (Vector3(u, v, m_Scene.m_Near)).Normalize();

                        Ray primiaryRay(m_Scene.m_Camera.GetLocation(), rd);

                        // If we're actually using depth of field, we need to modify the camera ray to account for that
                        if (camera.m_FocusOn)
                        {
                            Vector3 fp = (camera_ray.origin + rd * L) + rd.Normalized() * camera.m_FocalLength;
                            // Get a pixel point and new ray rdection to calculate where the rays should Intersect
                            // Vector3 del_x = (cx * dx * L / float(m_Width));
                            // Vector3 del_y = (cy * dy * L / float(m_Height));
                            Vector3 del_x = cx * dx * L;
                            Vector3 del_y = cy * dy * L;
                            Vector3 point = camera_ray.origin + rd * L;
                            point = point + del_x + del_y;
                            rd = (fp - point).Normalize();
                            primiaryRay = Ray(point, rd);
                        }
                        else
                        {
                            primiaryRay = Ray(m_Scene.m_Camera.GetLocation(), rd.Normalized());
                        }

                        radiance = radiance + Tracing(primiaryRay, 0) * 0.25;
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
                colorArray[i * m_Width + j] = color;
            }
        }
    }

    // Vector3 Raytracer::ToneMapping(Vector3 &cradiance)const{

    // }

    void Raytracer::RenderIndirect(float& time, Image& image)
    {
        Matrix4x4 viewMatrix = m_Scene.m_Camera.GetViewMatrix();
        int samps = m_Samples / 4;
        is_rendering = true;
        Color r(0, 0, 0);
        Vector3 raw(0, 0, 0);
#pragma omp parallel for schedule(dynamic, 1) private(r)       // OpenMP
        for (unsigned short i = 0; i < m_Height; ++i)
        {
            this->m_Progress = 100. * i / (m_Height - 1);
            // fprintf(stderr, "\rRendering (%d spp) %5.2f%%", samps * gridSize * gridSize, 100.*i / (m_Height - 1));
            LOG_INFO("Rendering ", "spp:", samps * 4, " ", 100. * i / (m_Height - 1), '%');
            for (unsigned short j = 0; j < m_Width; ++j)
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
                            double u = (j + (sy - 0.5 + dy * 0.5) * 0.5) / m_Width;
                            double v = (m_Height - (i + (sx - 0.5 + dx * 0.5) * 0.5)) / m_Height;
                            u = (u * 2.0 - 1.0);
                            v = (v * 2.0 - 1.0);
                            u = u * m_Width / m_Height;
                            Vector3 rd = viewMatrix * (Vector3(u, v, m_Scene.m_Camera.m_Near)).Normalize();

                            Ray primiaryRay(m_Scene.m_Camera.GetLocation(), rd);
                            r = r + Tracing(primiaryRay, 0) * (1.0f / samps);
                        }
                        color = color + Vector3(Clamp01(r.x), Clamp01(r.y), Clamp01(r.z)) * .25;
                    }
                }

                raw = color * 255;
                image.SetPixel(j, i, Color(raw.x, raw.y, raw.z));
            }
        }

        is_rendering = false;
    }

    void Raytracer::SetResolution(const int& in_width, const int& in_height)
    {
        m_Width = in_width;
        m_Height = in_height;
    }

    Raytracer::~Raytracer()
    {
    }
}
