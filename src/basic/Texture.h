#pragma once

#include "loadpng/lodepng.h"
#include "math/Vector.h"
#include "utility/Log.h"


namespace new_type_renderer
{
    class Texture
    {
    public:
        Texture() {}
        ~Texture() {}
        unsigned height;
        unsigned width;
        bool loaded;
        std::vector<unsigned char> image;

        void loadImage(const std::string& filename) {
            image = std::vector<unsigned char>();
            unsigned error = lodepng::decode(image, width, height, filename.c_str());
            if (error) {
                loaded = false;
                Log::Print(LogLevel::Error, filename.c_str());
            }
            else {
                loaded = true;
                Log::Print(LogLevel::Info, filename.c_str());
            }


        }

        Vector4 getColor4(const Vector2& uv) {
            if (!loaded)
                return (Vector4(1, 0, 1, 1));

            int x = (fmod(fabs(uv.x), 1.0)) * (width - 1);
            int y = (1. - fmod(fabs(uv.y), 1.0)) * (height - 1);
            double r, g, b, a;
            r = (double)image.at(y * width * 4 + x * 4) / 255.;
            g = (double)image.at(y * width * 4 + x * 4 + 1) / 255.;
            b = (double)image.at(y * width * 4 + x * 4 + 2) / 255.;
            a = (double)image.at(y * width * 4 + x * 4 + 3) / 255.;
            return Vector4(r, g, b, a);
        }

        Vector3 getColor3(const Vector2& uv) {
            if (!loaded)
                return (Vector3(1, 0, 1));

            int x = (fmod(fabs(uv.x), 1.0)) * (width - 1);
            int y = (1. - fmod(fabs(uv.y), 1.0)) * (height - 1);
            double r, g, b;
            r = (double)image.at(y * width * 4 + x * 4) / 255.;
            g = (double)image.at(y * width * 4 + x * 4 + 1) / 255.;
            b = (double)image.at(y * width * 4 + x * 4 + 2) / 255.;
            return Vector3(r, g, b);
        }
    };

}
