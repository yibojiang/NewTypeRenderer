#pragma once

#include "core/Color.h"
#include "loadpng/lodepng.h"
#include "math/Vector.h"
#include "utility/Log.h"


namespace new_type_renderer
{
    class Texture
    {
    public:
        Texture() {}

        void loadImage(const std::string& filename) {
            image = std::vector<unsigned char>();
            unsigned error = lodepng::decode(image, width, height, filename.c_str());
            if (error) {
                loaded = false;
                LOG_INFO("%s", filename.c_str());
            }
            else {
                loaded = true;
                LOG_INFO("%s", filename.c_str());
            }
        }

        Color getColor4(const Vector2& uv) {
            if (!loaded)
                return (Vector4(1, 0, 1, 1));

            int x = (fmod(fabs(uv.x), 1.0)) * (width - 1);
            int y = (1. - fmod(fabs(uv.y), 1.0)) * (height - 1);
            double r, g, b, a;
            r = (float)image.at(y * width * 4 + x * 4) / 255.;
            g = (float)image.at(y * width * 4 + x * 4 + 1) / 255.;
            b = (float)image.at(y * width * 4 + x * 4 + 2) / 255.;
            a = (float)image.at(y * width * 4 + x * 4 + 3) / 255.;
            return Vector4(r, g, b, a);
        }

        Vector3 getColor3(const Vector2& uv) {
            if (!loaded)
                return (Vector3(1, 0, 1));

            int x = (fmod(fabs(uv.x), 1.0)) * (width - 1);
            int y = (1. - fmod(fabs(uv.y), 1.0)) * (height - 1);
            float r, g, b;
            r = (float)image.at(y * width * 4 + x * 4) / 255.;
            g = (float)image.at(y * width * 4 + x * 4 + 1) / 255.;
            b = (float)image.at(y * width * 4 + x * 4 + 2) / 255.;
            return Vector3(r, g, b);
        }

    public:
        unsigned height;

        unsigned width;

        bool loaded;

        std::vector<unsigned char> image;
    };

}
