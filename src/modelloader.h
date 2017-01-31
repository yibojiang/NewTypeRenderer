#pragma once
#include "tiny_obj_loader.h"
#include <QDir>

class ObjLoader
{
public:

    ObjLoader(){};
    ~ObjLoader(){};

    bool loadModel(std::string _name, Mesh *mesh, Material *defaultMaterial){

        QString path = QDir::currentPath();
        // _name = "/models/" + _name;
        // std::string modelPath = path.toUtf8().constData() + _name;
        std::vector<tinyobj::shape_t> rawShape;
        std::vector<tinyobj::material_t> rawMaterial;
        
        

        std::string mtlbasepath = path.toUtf8().constData();
        mtlbasepath += "/textures/";
        std::string modelPath = path.toUtf8().constData();
        modelPath += "/models/" + _name;

        qDebug() << "open" << modelPath.c_str();

        tinyobj::LoadObj(rawShape, rawMaterial, modelPath.c_str(), mtlbasepath.c_str());
        qDebug() << rawShape[0].name.c_str();
        // qDebug() << "loaded";

        tinyobj::mesh_t rawMesh = rawShape[0].mesh;
        qDebug() << "position size: " << rawMesh.positions.size() / 3;
        qDebug() << "indices size: " << rawMesh.indices.size();
        qDebug() << "uv size: " << rawMesh.texcoords.size() / 2;
        qDebug() << "normals size: " << rawMesh.normals.size() / 3;

        std::vector<Material*> materials;

        

        qDebug() << "loading material...";

        qDebug() << "material size: " << rawMaterial.size();
        // Load materials/textures from obj
        // TODO: Only texture is loaded at the moment, need to implement material types and colours
        for (unsigned int i=0; i<rawMaterial.size(); i++) {
            std::string texture_path = "/textures/";
            Material *material= new Material();
            material->diffuseColor = vec3(rawMaterial[i].diffuse[0], rawMaterial[i].diffuse[1], rawMaterial[i].diffuse[2]);
            
            material->glossy = rawMaterial[i].shininess;
            material->roughness = sqrt(2.0/(2.0+rawMaterial[i].shininess));
            
            material->ior = rawMaterial[i].ior;
            material->emissionColor = vec3(rawMaterial[i].emission[0], rawMaterial[i].emission[1], rawMaterial[i].emission[2]);
            if (material->emissionColor.length()>0){
                material->emission = 100;
            }
            material->reflectColor = vec3(rawMaterial[i].specular[0], rawMaterial[i].specular[1], rawMaterial[i].specular[2]);
            // material->refract = rawMaterial[i].dissolve < 1 ? 1-rawMaterial[i].dissolve:0;
            material->refract = 1 - rawMaterial[i].dissolve;
            material->metallic = material->reflectColor.length()/sqrt(3.0);

            if (material->diffuseColor.length() == 0){
                material->diffuse = 0;
            }
            else{
                material->diffuse = 1;    
            }
            

            if (material->reflectColor.length() == 0){
                material->reflection = 0;
            }
            else{
                material->reflection = 1;    
            }
            
            // material->diffuseColor.normalize();
            // material->reflectColor.normalize();

            // material->metallic = std::clamp(material->metallic, 0, 1);

            material->metallic = fmin(material->metallic, 1);
            material->metallic = fmax(material->metallic, 0);
            material->metallic = 1;
            // material->metallic = 0;
            // material->metallic = 1;
            // material->reflectColor = vec3(1,1,1);

            
            
            // material->diffuse = 1;

            if (!rawMaterial[i].diffuse_texname.empty()){
                texture_path = "/textures/";
                if (rawMaterial[i].diffuse_texname[0] == '/'){
                    texture_path = rawMaterial[i].diffuse_texname;
                } 
                texture_path = texture_path + rawMaterial[i].diffuse_texname;
                qDebug() << "diffuse texture:" << texture_path.c_str();

                
                
                // qDebug() << "diffuse color" << material->diffuseColor;
                material->setDiffuseTexture(texture_path);
            }
            
            if (!rawMaterial[i].specular_texname.empty()){
                texture_path = "/textures/";
                if (rawMaterial[i].specular_texname[0] == '/'){
                    texture_path = rawMaterial[i].specular_texname;
                }    
                texture_path = texture_path + rawMaterial[i].specular_texname;
                qDebug() << "specular texture:" << texture_path.c_str();
            }
            
            if (!rawMaterial[i].normal_texname.empty()){
                texture_path = "/textures/";
                if (rawMaterial[i].normal_texname[0] == '/'){
                    texture_path = rawMaterial[i].normal_texname;
                } 
                texture_path = texture_path + rawMaterial[i].normal_texname;
                qDebug() << "normal texture:" << texture_path.c_str();
                
                
            }
            
            material->init();
            qDebug() << rawMaterial[i].name.c_str() << "diffuse"<< material->diffuse;
            qDebug() << rawMaterial[i].name.c_str() << "reflection"<< material->reflection;
            qDebug() << rawMaterial[i].name.c_str() << "refract"<< material->refract;
            qDebug() << rawMaterial[i].name.c_str() << "metallic"<< material->metallic;
            qDebug() << rawMaterial[i].name.c_str() << "roughness"<< material->roughness;
            qDebug() << rawMaterial[i].name.c_str() << "ior"<< material->ior;
            qDebug() << rawMaterial[i].name.c_str() << "specularColor"<< material->reflectColor;
            materials.push_back(material);

        }

        qDebug() << "material loaded." << materials.size();
        // Load triangles from obj
        for (unsigned int i = 0; i < rawShape.size(); i++) {
            size_t indicesSize = rawShape[i].mesh.indices.size() / 3;
            for (size_t f = 0; f < indicesSize; f++) {

                // Triangle vertex coordinates
                vec3 p1 = vec3( 
                        rawShape[i].mesh.positions[ rawShape[i].mesh.indices[3*f] * 3     ],
                        rawShape[i].mesh.positions[ rawShape[i].mesh.indices[3*f] * 3 + 1 ],
                        rawShape[i].mesh.positions[ rawShape[i].mesh.indices[3*f] * 3 + 2 ]
                );

                vec3 p2 = vec3(
                        rawShape[i].mesh.positions[ rawShape[i].mesh.indices[3*f + 1] * 3     ],
                        rawShape[i].mesh.positions[ rawShape[i].mesh.indices[3*f + 1] * 3 + 1 ],
                        rawShape[i].mesh.positions[ rawShape[i].mesh.indices[3*f + 1] * 3 + 2 ]
                );

                vec3 p3 = vec3(
                        rawShape[i].mesh.positions[ rawShape[i].mesh.indices[3*f + 2] * 3     ],
                        rawShape[i].mesh.positions[ rawShape[i].mesh.indices[3*f + 2] * 3 + 1 ],
                        rawShape[i].mesh.positions[ rawShape[i].mesh.indices[3*f + 2] * 3 + 2 ]
                );

                vec2 uv1, uv2, uv3;

                //Attempt to load triangle texture coordinates
                if (rawShape[i].mesh.indices[3 * f + 2] * 2 + 1 < rawShape[i].mesh.texcoords.size()) {
                    uv1 = vec2(
                            rawShape[i].mesh.texcoords[rawShape[i].mesh.indices[3 * f] * 2],
                            rawShape[i].mesh.texcoords[rawShape[i].mesh.indices[3 * f] * 2 + 1]
                    );

                    uv2 = vec2(
                            rawShape[i].mesh.texcoords[rawShape[i].mesh.indices[3 * f + 1] * 2],
                            rawShape[i].mesh.texcoords[rawShape[i].mesh.indices[3 * f + 1] * 2 + 1]
                    );

                    uv3 = vec2(
                            rawShape[i].mesh.texcoords[rawShape[i].mesh.indices[3 * f + 2] * 2],
                            rawShape[i].mesh.texcoords[rawShape[i].mesh.indices[3 * f + 2] * 2 + 1]
                    );
                }
                else {
                    uv1=vec2();
                    uv2=vec2();
                    uv3=vec2();
                }

                vec3 n1 = vec3( 
                        rawShape[i].mesh.normals[ rawShape[i].mesh.indices[3*f] * 3     ],
                        rawShape[i].mesh.normals[ rawShape[i].mesh.indices[3*f] * 3 + 1 ],
                        rawShape[i].mesh.normals[ rawShape[i].mesh.indices[3*f] * 3 + 2 ]
                );

                vec3 n2 = vec3(
                        rawShape[i].mesh.normals[ rawShape[i].mesh.indices[3*f + 1] * 3     ],
                        rawShape[i].mesh.normals[ rawShape[i].mesh.indices[3*f + 1] * 3 + 1 ],
                        rawShape[i].mesh.normals[ rawShape[i].mesh.indices[3*f + 1] * 3 + 2 ]
                );

                vec3 n3 = vec3(
                        rawShape[i].mesh.normals[ rawShape[i].mesh.indices[3*f + 2] * 3     ],
                        rawShape[i].mesh.normals[ rawShape[i].mesh.indices[3*f + 2] * 3 + 1 ],
                        rawShape[i].mesh.normals[ rawShape[i].mesh.indices[3*f + 2] * 3 + 2 ]
                );

                Triangle *face = new Triangle(p1, p2, p3);
                // face -> setupVertices(p1, p2, p3);
                face -> setupUVs(uv1, uv2, uv3);
                face -> setupNormals(n1, n2, n3);    
                mesh->addFace(face);

                // int materialIdx = rawShape[i].mesh.material_ids[f];
                // if (!rawMaterial[materialIdx].diffuse_texname.empty()){
                //     std::string texture_path = "";
                //     if (rawMaterial[materialIdx].diffuse_texname[0] == '/'){
                //         texture_path = rawMaterial[materialIdx].diffuse_texname;
                //     } 
                //     texture_path = "/textures/" + rawMaterial[materialIdx].diffuse_texname;
                //     qDebug() << "texture:" << texture_path.c_str();
                //     Material *material = new Material();
                //     material->diffuse = 1;
                //     material->setDiffuseTexture(texture_path);    
                //     mesh->setMaterial(material);
                //     // materials.push_back(material);
                // }
                unsigned long materialIdx= rawShape[i].mesh.material_ids[f];
                if (materialIdx < materials.size()){
                    face->setMaterial(materials[materialIdx]);
                }
                else{
                    face->setMaterial(defaultMaterial);
                    
                }
            }


        }
        // qDebug() << "loaded all.";

        // for (unsigned int i = 0; i < rawMesh.indices.size(); ++i) {   
        //     int idx = rawMesh.indices[i];
        //     qDebug() << idx;
        //     vec3 p1, p2, p3;
        //     vec3 n1, n2, n3;
        //     vec2 uv1, uv2, uv3;
        //     // qDebug() << rawMesh.positions[i];
        //     if (idx+6 < rawMesh.positions.size()){
        //         p1 = vec3(rawMesh.positions[idx], rawMesh.positions[idx+1], rawMesh.positions[idx+2]);
        //         p2 = vec3(rawMesh.positions[idx+2], rawMesh.positions[idx+3], rawMesh.positions[idx+4]);
        //         p3 = vec3(rawMesh.positions[idx+4], rawMesh.positions[idx+5], rawMesh.positions[idx+6]);
        //     }
            // qDebug() << p1 << p2 << p3;

            // if (vid+8 < rawMesh.normals.size()){
            //     n1 = vec3(rawMesh.normals[vid], rawMesh.normals[vid+1], rawMesh.normals[vid+2]); 
            //     n2 = vec3(rawMesh.normals[vid+3], rawMesh.normals[vid+4], rawMesh.normals[vid+5]);
            //     n3 = vec3(rawMesh.normals[vid+6], rawMesh.normals[vid+7], rawMesh.normals[vid+8]);
            // }

            // if (uvid+2 < rawMesh.texcoords.size()){
            //     uv1 = vec2(rawMesh.texcoords[uvid], rawMesh.texcoords[uvid+1]);
            //     uv2 = vec2(rawMesh.texcoords[uvid+2], rawMesh.texcoords[uvid+3]);
            //     uv3 = vec2(rawMesh.texcoords[uvid+4], rawMesh.texcoords[uvid+5]);
            // }

        //     Face *face = new Face();
        //     face -> setupVertices(p1, p2, p3);
        //     face -> setupUVs(uv1, uv2, uv3);
        //     // face -> setupNormals(n1, n2, n2);    
        //     mesh->addFace(face);
        // }
        return true;
    }

    // Deprecated
    // bool loadObj(std::string _name, Mesh *mesh){
    //     // QString path = QCoreApplication::applicationDirPath();
    //     QString path = QDir::currentPath();
    //     _name = "/models/" + _name;
    //     std::string fullpath = path.toUtf8().constData() + _name;
    //     qDebug() << "open" << fullpath.c_str();

    //     // return true;
    //     // QByteArray ba = path.toLatin1();
    //     // const char *c_str2 = ba.data();
    //     FILE *file = fopen( fullpath.c_str(), "r");
    //     if( file == NULL ){
    //         qDebug() << "Impossible to open the file !\n" << fullpath.c_str();
    //         return false;
    //     }

    //     std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
    //     std::vector<vec3> temp_vertices;
    //     std::vector<vec2> temp_uvs;
    //     std::vector<vec3> temp_normals;

    //     while(1){
            
    //         char lineHeader[128];

    //         // read the first word of the line
    //         int res = fscanf(file, "%s", lineHeader);
    //         // qDebug() << res;
    //         if (res == EOF){
    //             // qDebug() << "break";   
    //             break; // EOF = End Of File. Quit the loop.
    //         }
    //         // qDebug() << res;
    //         // else : parse lineHeader
    //         if ( strcmp( lineHeader, "v" ) == 0 ){
    //             vec3 vertex;
    //             fscanf(file, "%lf %lf %lf\n", &vertex.x, &vertex.y, &vertex.z );
    //             // qDebug() << "vertex: " << vertex.x << ',' << vertex.y << ',' << vertex.z;
    //             temp_vertices.push_back(vertex);

    //         }
    //         else if ( strcmp( lineHeader, "vt" ) == 0 ){
    //             vec2 uv;
    //             fscanf(file, "%lf %lf\n", &uv.x, &uv.y );
    //             temp_uvs.push_back(uv);

    //         }
    //         else if ( strcmp( lineHeader, "vn" ) == 0 ){
    //             vec3 normal;
    //             fscanf(file, "%lf %lf %lf\n", &normal.x, &normal.y, &normal.z );
    //             temp_normals.push_back(normal);

    //         }
    //         else if ( strcmp( lineHeader, "f" ) == 0 ){
    //             std::string vertex1, vertex2, vertex3;
    //             unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
    //             int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
    //             if (matches != 9){
    //                 qDebug() << "9 File can't be read by our simple parser : ( Try exporting with other options\n";
    //                 return false;
    //             }
    //             else{
    //                 Face *face = new Face();
    //                 face -> setupVertices(temp_vertices[vertexIndex[0]-1], temp_vertices[vertexIndex[1]-1], temp_vertices[vertexIndex[2]-1]);
    //                 face -> setupUVs(temp_uvs[uvIndex[0]-1], temp_uvs[uvIndex[1]-1], temp_uvs[uvIndex[2]-1]);
    //                 face -> setupNormals(temp_normals[normalIndex[0]-1], temp_normals[normalIndex[1]-1], temp_normals[normalIndex[2]-1]);    
    //                 mesh->addFace(face);
    //             }


    //             // int matches = fscanf(file, "%d %d %d\n", &vertexIndex[0], &vertexIndex[1], &vertexIndex[2]);
    //             // if (matches != 3){
    //             //     qDebug() << matches <<" File can't be read by our simple parser : ( Try exporting with other options\n";
    //             //     return false;
    //             // }
    //             // else{
    //             //     Face *face = new Face();
    //             //     face -> setupVertices(temp_vertices[vertexIndex[0]-1], temp_vertices[vertexIndex[1]-1], temp_vertices[vertexIndex[2]-1]);    
    //             //     mesh->addFace(face);
    //             // }
                
                
             
    //         }
    //     }


    //     // delete file;
    //     return true;
    // }
    
};
