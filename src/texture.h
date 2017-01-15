#pragma once
#include "lodepng.h"
#include <QDir>
class Texture
{
public:
    Texture(){}
    ~Texture(){}
    unsigned height;
    unsigned width;
    bool loaded;
    std::vector<unsigned char> image;

    void loadImage(const std::string& name){
        // Load file and decode image.
        
        QString path = QDir::currentPath();
        std::string filename = path.toUtf8().constData() + name;
        image = std::vector<unsigned char>();
        unsigned error = lodepng::decode(image, width, height ,filename.c_str());
        if (error) {
            loaded = false;
            qDebug() << "image "<< filename.c_str() <<"not loaded";
        }
        else{
            loaded = true;
            qDebug() << "image "<< filename.c_str() <<" loaded";
            // qDebug() << width << "x" << height;
        }
        

    }

    vec4 getColor4(const vec2& uv){
        if (!loaded)
            return (vec4(1,0,1,1));

        int x = (fmod(fabs(uv.x), 1.0)) * (width-1);
        int y = (1.-fmod(fabs(uv.y), 1.0)) * (height-1);
        double r, g, b, a;
        r = (double)image.at(y*width*4 + x*4    )/255.;
        g = (double)image.at(y*width*4 + x*4 + 1)/255.;
        b = (double)image.at(y*width*4 + x*4 + 2)/255.;
        a = (double)image.at(y*width*4 + x*4 + 3)/255.;
        return vec4(r, g, b, a);
    }

    vec3 getColor3(const vec2& uv){
        if (!loaded)
            return (vec3(1,0,1));

        int x = (fmod(fabs(uv.x), 1.0)) * (width-1);
        int y = (1.-fmod(fabs(uv.y), 1.0)) * (height-1);
        double r, g, b;
        r = (double)image.at(y*width*4 + x*4    )/255.;
        g = (double)image.at(y*width*4 + x*4 + 1)/255.;
        b = (double)image.at(y*width*4 + x*4 + 2)/255.;
        return vec3(r, g, b);
    }
};