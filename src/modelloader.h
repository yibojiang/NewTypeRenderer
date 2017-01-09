#pragma once
#include "tiny_obj_loader.h"
#include <QDir>

class ObjLoader
{
public:

    ObjLoader(){};
    ~ObjLoader(){};

    bool loadModel(std::string _name, Mesh *mesh){

        QString path = QDir::currentPath();
        _name = "/models/" + _name;
        std::string fullpath = path.toUtf8().constData() + _name;
        std::vector<tinyobj::shape_t> rawShape;
        std::vector<tinyobj::material_t> rawMaterial;

        qDebug() << "open" << fullpath.c_str();

        tinyobj::LoadObj(rawShape, rawMaterial, fullpath.c_str());
        qDebug() << rawShape[0].name.c_str();
        // qDebug() << "loaded";

        tinyobj::mesh_t rawMesh = rawShape[0].mesh;
        qDebug() << "position size: " << rawMesh.positions.size() / 3;
        qDebug() << "indices size: " << rawMesh.indices.size();
        qDebug() << "uv size: " << rawMesh.texcoords.size() / 2;
        qDebug() << "normals size: " << rawMesh.normals.size() / 3;

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

                Face *face = new Face();
                face -> setupVertices(p1, p2, p3);
                face -> setupUVs(uv1, uv2, uv3);
                // face -> setupNormals(n1, n2, n2);    
                mesh->addFace(face);
            }
        }

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
    bool loadObj(std::string _name, Mesh *mesh){
        // QString path = QCoreApplication::applicationDirPath();
        QString path = QDir::currentPath();
        _name = "/models/" + _name;
        std::string fullpath = path.toUtf8().constData() + _name;
        qDebug() << "open" << fullpath.c_str();

        // return true;
        // QByteArray ba = path.toLatin1();
        // const char *c_str2 = ba.data();
        FILE *file = fopen( fullpath.c_str(), "r");
        if( file == NULL ){
            qDebug() << "Impossible to open the file !\n" << fullpath.c_str();
            return false;
        }

        std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
        std::vector<vec3> temp_vertices;
        std::vector<vec2> temp_uvs;
        std::vector<vec3> temp_normals;

        while(1){
            
            char lineHeader[128];

            // read the first word of the line
            int res = fscanf(file, "%s", lineHeader);
            // qDebug() << res;
            if (res == EOF){
                // qDebug() << "break";   
                break; // EOF = End Of File. Quit the loop.
            }
            // qDebug() << res;
            // else : parse lineHeader
            if ( strcmp( lineHeader, "v" ) == 0 ){
                vec3 vertex;
                fscanf(file, "%lf %lf %lf\n", &vertex.x, &vertex.y, &vertex.z );
                // qDebug() << "vertex: " << vertex.x << ',' << vertex.y << ',' << vertex.z;
                temp_vertices.push_back(vertex);

            }
            else if ( strcmp( lineHeader, "vt" ) == 0 ){
                vec2 uv;
                fscanf(file, "%lf %lf\n", &uv.x, &uv.y );
                temp_uvs.push_back(uv);

            }
            else if ( strcmp( lineHeader, "vn" ) == 0 ){
                vec3 normal;
                fscanf(file, "%lf %lf %lf\n", &normal.x, &normal.y, &normal.z );
                temp_normals.push_back(normal);

            }
            else if ( strcmp( lineHeader, "f" ) == 0 ){
                std::string vertex1, vertex2, vertex3;
                unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
                int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
                if (matches != 9){
                    qDebug() << "9 File can't be read by our simple parser : ( Try exporting with other options\n";
                    return false;
                }
                else{
                    Face *face = new Face();
                    face -> setupVertices(temp_vertices[vertexIndex[0]-1], temp_vertices[vertexIndex[1]-1], temp_vertices[vertexIndex[2]-1]);
                    face -> setupUVs(temp_uvs[uvIndex[0]-1], temp_uvs[uvIndex[1]-1], temp_uvs[uvIndex[2]-1]);
                    face -> setupNormals(temp_normals[normalIndex[0]-1], temp_normals[normalIndex[1]-1], temp_normals[normalIndex[2]-1]);    
                    mesh->addFace(face);
                }


                // int matches = fscanf(file, "%d %d %d\n", &vertexIndex[0], &vertexIndex[1], &vertexIndex[2]);
                // if (matches != 3){
                //     qDebug() << matches <<" File can't be read by our simple parser : ( Try exporting with other options\n";
                //     return false;
                // }
                // else{
                //     Face *face = new Face();
                //     face -> setupVertices(temp_vertices[vertexIndex[0]-1], temp_vertices[vertexIndex[1]-1], temp_vertices[vertexIndex[2]-1]);    
                //     mesh->addFace(face);
                // }
                
                
             
            }
        }


        // delete file;
        return true;
    }
    
};
