#pragma once
#include <cassert>

#include "basic/material.h"
#include "basic/Mesh.h"
#include "basic/primitive.h"
#include "thirdparty/tiny_obj_loader/tiny_obj_loader.h"
#include "utility/Log.h"

namespace new_type_renderer
{
    class ObjLoader
    {
    public:
        bool LoadModel(std::string name, shared_ptr<Mesh>& mesh, shared_ptr<Material>& defalutMaterial)
        {
            std::vector<tinyobj::shape_t> rawShape;
            std::vector<tinyobj::material_t> rawMaterial;
            std::string mtlbasepath = "textures/" + name;
            std::string modelPath = "models/" + name;

            LOG_INFO("open %s", modelPath.c_str());
            LoadObj(rawShape, rawMaterial, modelPath.c_str(), mtlbasepath.c_str());

            assert(rawShape.size() > 0);

            LOG_INFO(rawShape[0].name.c_str());

            tinyobj::mesh_t rawMesh = rawShape[0].mesh;

            LOG_INFO("position size: %d", rawMesh.positions.size() / 3);
            LOG_INFO("indices size: %d", rawMesh.indices.size());
            LOG_INFO("uv size: %d", rawMesh.texcoords.size() / 2);
            LOG_INFO("normals size: %d", rawMesh.normals.size() / 3);
            LOG_INFO("loading material...");
            LOG_INFO("material size: %d", rawMaterial.size());

            std::vector<shared_ptr<Material>> materials;
            // Load materials/textures from obj
            // TODO: Only texture is loaded at the moment, need to implement material types and colours
            for (unsigned int i = 0; i < rawMaterial.size(); i++)
            {
                std::string texture_path = "/textures/";
                auto material = make_shared<Material>();
                material->m_DiffuseColor = Color(rawMaterial[i].diffuse[0], rawMaterial[i].diffuse[1],
                                                 rawMaterial[i].diffuse[2]);

                material->m_Glossy = rawMaterial[i].shininess;
                material->m_Roughness = sqrt(2.0 / (2.0 + rawMaterial[i].shininess));

                material->m_IOR = rawMaterial[i].ior;
                material->m_EmissionColor = Color(rawMaterial[i].emission[0], rawMaterial[i].emission[1],
                                                  rawMaterial[i].emission[2]);
                if (material->m_EmissionColor.Length() > 0)
                {
                    material->m_Emission = 100;
                }
                material->m_ReflectColor = Color(rawMaterial[i].specular[0], rawMaterial[i].specular[1],
                                                 rawMaterial[i].specular[2]);
                material->m_Refract = 1 - rawMaterial[i].dissolve;
                material->m_Metallic = material->m_ReflectColor.Length() / sqrt(3.0);

                if (material->m_DiffuseColor.Length() == 0)
                {
                    material->m_Diffuse = 0;
                }
                else
                {
                    material->m_Diffuse = 1;
                }


                if (material->m_ReflectColor.Length() == 0)
                {
                    material->m_Reflection = 0;
                }
                else
                {
                    material->m_Reflection = 1;
                }

                material->m_Metallic = fmin(material->m_Metallic, 1);
                material->m_Metallic = fmax(material->m_Metallic, 0);
                material->m_Metallic = 1;

                if (!rawMaterial[i].diffuse_texname.empty())
                {
                    texture_path = "/textures/";
                    if (rawMaterial[i].diffuse_texname[0] == '/')
                    {
                        texture_path = rawMaterial[i].diffuse_texname;
                    }
                    texture_path = texture_path + rawMaterial[i].diffuse_texname;
                    LOG_INFO("diffuse texture: %s", texture_path.c_str());

                    material->SetDiffuseTexture(texture_path);
                }

                if (!rawMaterial[i].specular_texname.empty())
                {
                    texture_path = "/textures/";
                    if (rawMaterial[i].specular_texname[0] == '/')
                    {
                        texture_path = rawMaterial[i].specular_texname;
                    }
                    texture_path = texture_path + rawMaterial[i].specular_texname;
                    LOG_INFO("specular texture: %s", texture_path.c_str());
                }

                if (!rawMaterial[i].normal_texname.empty())
                {
                    texture_path = "/textures/";
                    if (rawMaterial[i].normal_texname[0] == '/')
                    {
                        texture_path = rawMaterial[i].normal_texname;
                    }
                    texture_path = texture_path + rawMaterial[i].normal_texname;
                    LOG_INFO("normal texture: %s", texture_path.c_str());
                }

                material->Init();
                LOG_INFO("%s diffuse: %f", rawMaterial[i].name.c_str(), material->m_Diffuse);
                LOG_INFO("%s reflection: %f", rawMaterial[i].name.c_str(), material->m_Reflection);
                LOG_INFO("%s refract %f", rawMaterial[i].name.c_str(), material->m_Refract);
                LOG_INFO("%s metallic: %f", rawMaterial[i].name.c_str(), material->m_Metallic);
                LOG_INFO("%s roughness: %f", rawMaterial[i].name.c_str(), material->m_Roughness);
                LOG_INFO("%s ior: %f", rawMaterial[i].name.c_str(), material->m_IOR);
                LOG_INFO("%s specularColor: %f", rawMaterial[i].name.c_str(), material->m_ReflectColor);
                materials.push_back(material);
            }

            LOG_INFO("material loaded: %d", materials.size());
            // Load triangles from obj
            for (unsigned int i = 0; i < rawShape.size(); i++)
            {
                auto& positions = rawShape[i].mesh.positions;
                size_t positionsSize = rawShape[i].mesh.positions.size() / 3;
                for (int j = 0; j < positionsSize; j++)
                {
                    mesh->m_Positions.push_back(Vector3{ positions[j * 3], positions[j * 3 + 1], positions[j * 3 + 2] });
                    mesh->m_VertexNormals.push_back(Vector3{});
                }

                size_t indicesSize = rawShape[i].mesh.indices.size() / 3;
                for (size_t f = 0; f < indicesSize; f++)
                {
                    mesh->m_Indices.push_back(rawShape[i].mesh.indices[3 * f]);
                    mesh->m_Indices.push_back(rawShape[i].mesh.indices[3 * f + 1]);
                    mesh->m_Indices.push_back(rawShape[i].mesh.indices[3 * f + 2]);

                    // Triangle vertex coordinates
                    auto p1 = Vector3(
                        rawShape[i].mesh.positions[rawShape[i].mesh.indices[3 * f] * 3],
                        rawShape[i].mesh.positions[rawShape[i].mesh.indices[3 * f] * 3 + 1],
                        rawShape[i].mesh.positions[rawShape[i].mesh.indices[3 * f] * 3 + 2]
                    );

                    auto p2 = Vector3(
                        rawShape[i].mesh.positions[rawShape[i].mesh.indices[3 * f + 1] * 3],
                        rawShape[i].mesh.positions[rawShape[i].mesh.indices[3 * f + 1] * 3 + 1],
                        rawShape[i].mesh.positions[rawShape[i].mesh.indices[3 * f + 1] * 3 + 2]
                    );

                    auto p3 = Vector3(
                        rawShape[i].mesh.positions[rawShape[i].mesh.indices[3 * f + 2] * 3],
                        rawShape[i].mesh.positions[rawShape[i].mesh.indices[3 * f + 2] * 3 + 1],
                        rawShape[i].mesh.positions[rawShape[i].mesh.indices[3 * f + 2] * 3 + 2]
                    );

                    Vector2 uv1, uv2, uv3;

                    //Attempt to load triangle texture coordinates
                    if (rawShape[i].mesh.indices[3 * f + 2] * 2 + 1 < rawShape[i].mesh.texcoords.size())
                    {
                        uv1 = Vector2(
                            rawShape[i].mesh.texcoords[rawShape[i].mesh.indices[3 * f] * 2],
                            rawShape[i].mesh.texcoords[rawShape[i].mesh.indices[3 * f] * 2 + 1]
                        );

                        uv2 = Vector2(
                            rawShape[i].mesh.texcoords[rawShape[i].mesh.indices[3 * f + 1] * 2],
                            rawShape[i].mesh.texcoords[rawShape[i].mesh.indices[3 * f + 1] * 2 + 1]
                        );

                        uv3 = Vector2(
                            rawShape[i].mesh.texcoords[rawShape[i].mesh.indices[3 * f + 2] * 2],
                            rawShape[i].mesh.texcoords[rawShape[i].mesh.indices[3 * f + 2] * 2 + 1]
                        );
                    }
                    else
                    {
                        uv1 = Vector2();
                        uv2 = Vector2();
                        uv3 = Vector2();
                    }

                    auto n1 = Vector3(
                        rawShape[i].mesh.normals[rawShape[i].mesh.indices[3 * f] * 3],
                        rawShape[i].mesh.normals[rawShape[i].mesh.indices[3 * f] * 3 + 1],
                        rawShape[i].mesh.normals[rawShape[i].mesh.indices[3 * f] * 3 + 2]
                    );

                    auto n2 = Vector3(
                        rawShape[i].mesh.normals[rawShape[i].mesh.indices[3 * f + 1] * 3],
                        rawShape[i].mesh.normals[rawShape[i].mesh.indices[3 * f + 1] * 3 + 1],
                        rawShape[i].mesh.normals[rawShape[i].mesh.indices[3 * f + 1] * 3 + 2]
                    );

                    auto n3 = Vector3(
                        rawShape[i].mesh.normals[rawShape[i].mesh.indices[3 * f + 2] * 3],
                        rawShape[i].mesh.normals[rawShape[i].mesh.indices[3 * f + 2] * 3 + 1],
                        rawShape[i].mesh.normals[rawShape[i].mesh.indices[3 * f + 2] * 3 + 2]
                    );

                    auto face = make_shared<MeshFace>(
                        mesh,
                        rawShape[i].mesh.indices[3 * f],
                        rawShape[i].mesh.indices[3 * f + 1],
                        rawShape[i].mesh.indices[3 * f + 2]);
                    
                    face->SetupUVs(uv1, uv2, uv3);

                    // skip update vertex normal on the mesh, as obj file doesn't normally have the normal infomation
                    // face->SetVertexNormals(n1, n2, n3);
                    mesh->AddFace(face);

                    unsigned long materialIdx = rawShape[i].mesh.material_ids[f];
                    if (materialIdx < materials.size())
                    {
                        face->SetMaterial(materials[materialIdx]);
                    }
                    else
                    {
                        face->SetMaterial(defalutMaterial);
                    }
                }
                mesh->SmoothVertexNormal();
            }

            return true;
        }
    };
}
