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
        bool LoadModel(std::string name, shared_ptr<Mesh>& mesh, Material* default_material)
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

            std::vector<Material*> materials;
            // Load materials/textures from obj
            // TODO: Only texture is loaded at the moment, need to implement material types and colours
            for (unsigned int i = 0; i < rawMaterial.size(); i++)
            {
                std::string texture_path = "/textures/";
                auto material = new Material();
                material->diffuseColor = Color(rawMaterial[i].diffuse[0], rawMaterial[i].diffuse[1],
                                                 rawMaterial[i].diffuse[2]);

                material->glossy = rawMaterial[i].shininess;
                material->roughness = sqrt(2.0 / (2.0 + rawMaterial[i].shininess));

                material->ior = rawMaterial[i].ior;
                material->emissionColor = Color(rawMaterial[i].emission[0], rawMaterial[i].emission[1],
                                                  rawMaterial[i].emission[2]);
                if (material->emissionColor.Length() > 0)
                {
                    material->emission = 100;
                }
                material->reflectColor = Color(rawMaterial[i].specular[0], rawMaterial[i].specular[1],
                                                 rawMaterial[i].specular[2]);
                material->refract = 1 - rawMaterial[i].dissolve;
                material->metallic = material->reflectColor.Length() / sqrt(3.0);

                if (material->diffuseColor.Length() == 0)
                {
                    material->diffuse = 0;
                }
                else
                {
                    material->diffuse = 1;
                }


                if (material->reflectColor.Length() == 0)
                {
                    material->reflection = 0;
                }
                else
                {
                    material->reflection = 1;
                }

                material->metallic = fmin(material->metallic, 1);
                material->metallic = fmax(material->metallic, 0);
                material->metallic = 1;

                if (!rawMaterial[i].diffuse_texname.empty())
                {
                    texture_path = "/textures/";
                    if (rawMaterial[i].diffuse_texname[0] == '/')
                    {
                        texture_path = rawMaterial[i].diffuse_texname;
                    }
                    texture_path = texture_path + rawMaterial[i].diffuse_texname;
                    LOG_INFO("diffuse texture: %s", texture_path.c_str());

                    material->setDiffuseTexture(texture_path);
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

                material->init();
                LOG_INFO("%s diffuse: %f", rawMaterial[i].name.c_str(), material->diffuse);
                LOG_INFO("%s reflection: %f", rawMaterial[i].name.c_str(), material->reflection);
                LOG_INFO("%s refract %f", rawMaterial[i].name.c_str(), material->refract);
                LOG_INFO("%s metallic: %f", rawMaterial[i].name.c_str(), material->metallic);
                LOG_INFO("%s roughness: %f", rawMaterial[i].name.c_str(), material->roughness);
                LOG_INFO("%s ior: %f", rawMaterial[i].name.c_str(), material->ior);
                LOG_INFO("%s specularColor: %f", rawMaterial[i].name.c_str(), material->reflectColor);
                materials.push_back(material);
            }

            LOG_INFO("material loaded: %d", materials.size());
            // Load triangles from obj
            for (unsigned int i = 0; i < rawShape.size(); i++)
            {
                auto& positions = rawShape[i].mesh.positions;
                for (int j = 0; j < positions.size() / 3; j++)
                {
                    mesh->m_Positions.push_back(Vector3{ positions[j * 3], positions[j * 3 + 1], positions[j * 3 + 2] });
                    mesh->m_VertexNormals.push_back(Vector3{});
                }

                size_t indicesSize = rawShape[i].mesh.indices.size() / 3;
                for (size_t f = 0; f < indicesSize; f++)
                {
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

                    mesh->m_Indices.push_back(3 * f);
                    mesh->m_Indices.push_back(3 * f + 1);
                    mesh->m_Indices.push_back(3 * f + 2);

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
                        face->SetMaterial(default_material);
                    }
                }
                mesh->SmoothVertexNormal();
            }

            return true;
        }
    };
}
