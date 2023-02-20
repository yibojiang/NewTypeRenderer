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
        m_Root = make_shared<SceneNode>();
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
            __debugbreak();
            return false;
        }

        if (document.HasMember("camera"))
        {
            rapidjson::Value& camera_data = document["camera"];

            m_Camera.m_FOV = camera_data["fov"].GetFloat();
            const rapidjson::Value& position = camera_data["transform"]["position"];
            const rapidjson::Value& target = camera_data["transform"]["target"];
            const rapidjson::Value& up = camera_data["transform"]["up"];
            m_Camera.m_Location = Vector3(position[0].GetFloat(), position[1].GetFloat(), position[2].GetFloat());
            m_Camera.m_LookAt = Vector3(target[0].GetFloat(), target[1].GetFloat(), target[2].GetFloat());

            if (camera_data.HasMember("focusOn"))
            {
                m_Camera.m_FocusOn = camera_data["focusOn"].GetBool();
            }
            else
            {
                m_Camera.m_FocusOn = false;
            }
            if (camera_data.HasMember("focalLength"))
            {
                m_Camera.m_FocalLength = camera_data["focalLength"].GetFloat();
            }

            if (camera_data.HasMember("aperture"))
            {
                m_Camera.m_Aperture = camera_data["aperture"].GetFloat();
            }
            else
            {
                m_Camera.m_Aperture = 1.0f;
            }
        }

        if (document.HasMember("envlight"))
        {
            rapidjson::Value& envLight = document["envlight"];
            LoadHdri(envLight["HDRI"].GetString());
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
            auto defaultShader = make_shared<Shader>();
            for (rapidjson::SizeType i = 0; i < primitives.Size(); ++i)
            {
                rapidjson::Value& pos = primitives[i]["transform"]["position"];
                rapidjson::Value& scl = primitives[i]["transform"]["scale"];
                rapidjson::Value& rot = primitives[i]["transform"]["rotation"];

                std::string materialName = primitives[i]["material"].GetString();

                Log::Print(Info, materialName.c_str());
                auto material = make_shared<Material>();
                material->SetShader(defaultShader);
                if (materials.HasMember(materialName.c_str()))
                {
                    rapidjson::Value& mat = materials[materialName.c_str()];
                    if (mat.HasMember("diffuse"))
                    {
                        material->m_Diffuse = mat["diffuse"].GetFloat();
                    }

                    if (mat.HasMember("specular"))
                    {
                        material->m_Specular = mat["specular"].GetFloat();
                    }

                    if (mat.HasMember("reflection"))
                    {
                        material->m_Reflection = mat["reflection"].GetFloat();
                    }

                    if (mat.HasMember("roughness"))
                    {
                        material->m_Roughness = mat["roughness"].GetFloat();
                    }

                    if (mat.HasMember("glossy"))
                    {
                        material->m_Glossy = mat["glossy"].GetFloat();
                    }

                    if (mat.HasMember("useBackground"))
                    {
                        material->m_UseBackground = mat["useBackground"].GetBool();
                    }


                    if (mat.HasMember("metallic"))
                    {
                        material->m_Metallic = mat["metallic"].GetFloat();
                    }

                    if (mat.HasMember("diffuseRoughness"))
                    {
                        material->m_DiffuseRoughness = mat["diffuseRoughness"].GetFloat();
                    }

                    if (mat.HasMember("emission"))
                    {
                        material->m_Emission = mat["emission"].GetFloat();
                    }

                    if (mat.HasMember("ior"))
                    {
                        material->m_IOR = mat["ior"].GetFloat();
                    }

                    if (mat.HasMember("refract"))
                    {
                        material->m_Refract = mat["refract"].GetFloat();
                    }

                    if (mat.HasMember("diffuseColor"))
                    {
                        material->m_DiffuseColor = Color(mat["diffuseColor"][0].GetFloat(),
                                                       mat["diffuseColor"][1].GetFloat(),
                                                       mat["diffuseColor"][2].GetFloat());
                    }

                    if (mat.HasMember("reflectColor"))
                    {
                        material->m_ReflectColor = Color(mat["reflectColor"][0].GetFloat(),
                                                       mat["reflectColor"][1].GetFloat(),
                                                       mat["reflectColor"][2].GetFloat());
                    }

                    if (mat.HasMember("refractColor"))
                    {
                        material->m_RefractColor = Color(mat["refractColor"][0].GetFloat(),
                                                       mat["refractColor"][1].GetFloat(),
                                                       mat["refractColor"][2].GetFloat());
                    }

                    if (mat.HasMember("emissionColor"))
                    {
                        material->SetEmission(Color(mat["emissionColor"][0].GetFloat(),
                                                    mat["emissionColor"][1].GetFloat(),
                                                    mat["emissionColor"][2].GetFloat()));
                    }

                    if (mat.HasMember("diffuseTexture"))
                    {
                        material->SetDiffuseTexture(mat["diffuseTexture"].GetString());
                    }
                }
                else
                {
                    LOG_ERR("can't find material ", materialName.c_str());
                }

                shared_ptr<Shape> obj;
                std::string ptype = primitives[i]["type"].GetString();

                if (ptype == "box")
                {
                    auto box = make_shared<Box>(Vector3{1.0f, 1.0f, 1.0f});
                    obj = box;
                }
                else if (ptype == "sphere")
                {
                    auto sphere = make_shared<Sphere>(1.0f);
                    obj = std::move(sphere);
                }
                else if (ptype == "mesh")
                {
                    auto mesh = make_shared<Mesh>();
                    mesh->name = "mesh";
                    std::string modelName = primitives[i]["path"].GetString();
                    loader.LoadModel(modelName, mesh, material);
                    obj = mesh;
                }
                else
                {
                    LOG_ERR("Unknown mesh type.");
                    return false;
                }

                obj->SetMaterial(material);
                auto node = make_shared<SceneNode>(m_Root);
                node->m_Transform.SetLocation(Vector3{ pos[0].GetFloat(), pos[1].GetFloat(), pos[2].GetFloat() });
                node->m_Transform.SetScale(Vector3{ scl[0].GetFloat(), scl[1].GetFloat(), scl[2].GetFloat() });
                node->m_Transform.SetOrientation(
                    Quaternion{ ToRadian(
                        rot[0].GetFloat()),
                        ToRadian(rot[1].GetFloat()),
                        ToRadian(rot[2].GetFloat())
                    }
                );

                obj->name = primitives[i]["name"].GetString();
                m_Root->AddChild(node);
                node->AddObject(obj);
                LOG_INFO("add %s to the scene", obj->name.c_str());
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
        m_HasHdri = HDRLoader::load(fullpath.c_str(), *m_HDRI);
    }

    void Scene::Add(shared_ptr<Shape>& object)
    {
        m_Shapes.push_back(object);
    }

    void Scene::AddMesh(shared_ptr<Mesh>& mesh)
    {
        for (uint32_t i = 0; i < mesh->m_Faces.size(); ++i)
        {
            auto face = mesh->m_Faces[i];

            if (auto& material = face->GetMaterial())
            {
                face->SetMaterial(material);
            }

            face->name = mesh->name + '_' + std::to_string(i);
            auto obj = dynamic_pointer_cast<Shape>(face);
            Add(obj);
        }
    }

    Intersection Scene::Intersect(Ray& ray) const
    {
        // return bvh.Intersect(ray);
        Intersection closestIntersection;
        // Intersect all m_Objects, one after the other
        // for (std::vector<Object*>::iterator it = m_Objects.begin(); it != m_Objects.end(); ++it){
        for (uint32_t i = 0; i < m_Shapes.size(); ++i)
        {
            Intersection intersection = m_Shapes[i]->Intersect(ray);
            double hitDistance = intersection.GetHitDistance();
            if (hitDistance > FLT_EPSILON && hitDistance < closestIntersection.m_Distance)
            {
                closestIntersection.m_Distance = hitDistance;
                closestIntersection.m_HitObject = m_Shapes[i];
            }
        }
        return closestIntersection;
    }

    void Scene::DestroyScene()
    {
        m_Root->RemoveAllChildren();
        m_Lights.clear();
        m_Shapes.clear();
    }

    void Scene::UpdateTransform(shared_ptr<SceneNode>& sceneNode, Matrix4x4 matrix)
    {
        matrix = matrix * sceneNode->m_Transform.TransformMatrix();
        if (auto& obj = sceneNode->GetObject())
        {
            obj->UpdateTransformMatrix(matrix);

            if (std::shared_ptr<Mesh> mesh = std::dynamic_pointer_cast<Mesh>(obj))
            {
                AddMesh(mesh);
            }
            else
            {
                Add(obj);
            }

            if (obj->GetMaterial()->GetEmission().Length() > FLT_EPSILON)
            {
                m_Lights.push_back(obj);
            }
        }

        for (unsigned int i = 0; i < sceneNode->GetChildren().size(); ++i)
        {
            UpdateTransform(sceneNode->GetChildren()[i], matrix);
        }
    }
}
