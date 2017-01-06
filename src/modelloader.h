#pragma once
#include <QDir>
class ObjLoader
{
public:

    ObjLoader(){};
    ~ObjLoader(){};

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
