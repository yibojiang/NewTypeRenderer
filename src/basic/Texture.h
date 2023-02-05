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

        void LoadImage(const std::string& filename) {
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

        Color GetColor(const Vector2& uv) {
            if (!loaded)
                return (Vector4(1, 0, 1, 1));

            int x = (fmod(fabs(uv.x), 1.0)) * (width - 1);
            int y = (1. - fmod(fabs(uv.y), 1.0)) * (height - 1);
            float r, g, b, a;
            r = image.at(y * width * 4 + x * 4) / 255.0f;
            g = image.at(y * width * 4 + x * 4 + 1) / 255.0f;
            b = image.at(y * width * 4 + x * 4 + 2) / 255.0f;
            a = image.at(y * width * 4 + x * 4 + 3) / 255.0f;
            return Color(r, g, b, a);
        }

    public:
        unsigned height;

        unsigned width;

        bool loaded;

        std::vector<unsigned char> image;
    };

}
