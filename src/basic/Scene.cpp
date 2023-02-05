#include "Scene.h"

#include <fstream>
#include "rapidjson/document.h"
#include "utility/ModelLoader.h"

namespace new_type_renderer
{
    Scene::Scene()
    {
        m_EnvLightIntense = 1.0f;
        m_EnvLightExp = 1.0f;
        m_HasHdri = false;
    }

    Scene::~Scene()
    {
        DestroyScene();
    }

    bool Scene::LoadFromJson(const std::string& fullPath)
    {
        ObjLoader loader;
        std::ifstream t(fullPath);
        std::string json((std::istreambuf_iterator<char>(t)),
                         std::istreambuf_iterator<char>());


        rapidjson::Document document;
        if (document.Parse(json.c_str()).HasParseError())
        {
            LOG_ERR("can't parse the scene");
            return false;
        }

        m_Root = new SceneNode();
        if (document.HasMember("camera"))
        {
            rapidjson::Value& camera_data = document["camera"];

            camera.m_FOV = camera_data["fov"].GetFloat() * M_PI / 180;
            camera.m_Near = 1.0f / tan(camera.m_FOV * 0.5f);
            const rapidjson::Value& position = camera_data["transform"]["position"];
            const rapidjson::Value& target = camera_data["transform"]["target"];
            const rapidjson::Value& up = camera_data["transform"]["up"];
            auto camPos = Vector3(position[0].GetFloat(), position[1].GetFloat(), position[2].GetFloat());
            auto camLookAt = Vector3(target[0].GetFloat(), target[1].GetFloat(), target[2].GetFloat());
            Matrix4x4 camMatrix = Matrix4x4::CreateViewMatrix(camPos, camLookAt);
            camera.m_CameraMatrix = camMatrix;

            if (camera_data.HasMember("focusOn"))
            {
                camera.m_FocusOn = camera_data["focusOn"].GetBool();
            }
            else
            {
                camera.m_FocusOn = false;
            }
            if (camera_data.HasMember("focalLength"))
            {
                camera.m_FocalLength = camera_data["focalLength"].GetFloat();
            }

            if (camera_data.HasMember("m_Aperture"))
            {
                camera.m_Aperture = camera_data["m_Aperture"].GetFloat();
            }
            else
            {
                camera.m_Aperture = 1.0f;
            }
        }

        if (document.HasMember("envlight"))
        {
            rapidjson::Value& envLight = document["envlight"];
            LoadHdri(envLight["m_HDRI"].GetString());
            m_EnvLightIntense = envLight["intense"].GetFloat();
            m_EnvLightExp = envLight["exp"].GetFloat();
            if (envLight.HasMember("rotate"))
            {
                m_EnvRotate = envLight["rotate"].GetFloat() / 180.0 * M_PI;
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
                    obj = make_shared<Box>(Vector3{1.0f, 1.0f, 1.0f});
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
                    return false;
                }

                obj->SetMaterial(material);


                auto node = make_shared<SceneNode>();
                node->AddObject(obj);

                node->m_Transform.SetLocation(Vector3{pos[0].GetFloat(), pos[1].GetFloat(), pos[2].GetFloat()});
                node->m_Transform.SetScale(Vector3{scl[0].GetFloat(), scl[1].GetFloat(), scl[2].GetFloat()});
                auto orientation = Quaternion(rot[0].GetFloat() * M_PI / 180, rot[1].GetFloat() * M_PI / 180,
                                              rot[2].GetFloat() * M_PI / 180);
                node->m_Transform.SetOrientation(orientation);
                obj->name = primitives[i]["name"].GetString();
                m_Root->AddChild(node);
                LOG_INFO("add ", obj->name.c_str(), " to the scene");
            }
        }

        UpdateTransform(m_Root, Matrix4x4());
        // bvh.Setup(*this);

        return true;
    }

    void Scene::LoadHdri(const std::string& name)
    {
        std::string path = "";
        // std::string name = "/textures/env.hdr";
        std::string fullpath = path + name;
        m_HasHdri = HDRLoader::load(fullpath.c_str(), m_HDRI);
    }

    void Scene::Add(Object* object)
    {
        m_Objects.push_back(object);
    }

    void Scene::AddMesh(Mesh* mesh)
    {
        for (uint32_t i = 0; i < mesh->faces.size(); ++i)
        {
            Triangle* triangle = mesh->faces[i];
            // triangle->setupVertices(mesh->faces[i]->p1, mesh->faces[i]->p2, mesh->faces[i]->p3);
            // Triangle *triangle = new Triangle(mesh->faces[i]->v1, mesh->faces[i]->v2, mesh->faces[i]->v3);
            // Triangle *triangle = new Triangle(mesh->faces[i]->p1, mesh->faces[i]->p2, mesh->faces[i]->p3);
            // if (mesh->GetMaterial()){
            //     triangle->setMaterial(mesh->GetMaterial());    
            // }

            if (!triangle->GetMaterial())
            {
                triangle->SetMaterial(mesh->GetMaterial());
            }

            triangle->name = mesh->name + '_' + std::to_string(i);
            // triangle->setupUVs(mesh->faces[i]->uv1, mesh->faces[i]->uv2, mesh->faces[i]->uv3);
            Add(triangle);
        }
    }

    Intersection Scene::Intersect(Ray& ray) const
    {
        return bvh.Intersect(ray);
        Intersection closestIntersection;
        // Intersect all m_Objects, one after the other
        // for (std::vector<Object*>::iterator it = m_Objects.begin(); it != m_Objects.end(); ++it){
        for (uint32_t i = 0; i < m_Objects.size(); ++i)
        {
            double t = m_Objects[i]->Intersect(ray);
            if (t > FLT_EPSILON && t < closestIntersection.t)
            {
                closestIntersection.t = t;
                closestIntersection.object = m_Objects[i];
            }
        }
        return closestIntersection;
    }

    void Scene::DestroyScene()
    {
        m_Root->RemoveAllChildren();
        for (uint32_t i = 0; i < m_Objects.size(); ++i)
        {
            delete m_Objects[i];
        }

        m_Lights.clear();
        m_Objects.clear();
        bvh.Destroy();
    }

    void Scene::UpdateTransform(SceneNode* scene_node, Matrix4x4 matrix)
    {
        matrix = matrix * scene_node->m_Transform.TransformMatrix();
        auto obj_wekptr = scene_node->GetObject();

        if (auto obt_spt = scene_node->GetObject().lock())
        {
            obt_spt->UpdateTransformMatrix(matrix);

            if (obt_spt->isMesh)
            {
                AddMesh(dynamic_cast<Mesh*>(obt_spt.get()));
            }
            else
            {
                Add(obt_spt.get());
            }

            if (obt_spt->GetMaterial()->getEmission().Length() > FLT_EPSILON)
            {
                m_Lights.push_back(obt_spt.get());
            }
        }

        for (unsigned int i = 0; i < scene_node->GetChildren().size(); ++i)
        {
            UpdateTransform(scene_node->GetChildren()[i].get(), matrix);
        }
    }
}
