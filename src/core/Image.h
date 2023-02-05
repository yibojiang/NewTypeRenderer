#pragma once
#include "Color.h"

namespace new_type_renderer
{
    struct Image
    {
    public:
        Image(int w, int h) : width(w), height(h)
        {
            pixels = new Color(w * h);
        }

        void SetPixel(int i, int j, const Color& color)
        {
            assert(i <= height);
            assert(j <= width);
            pixels[i * height + j] = color;
        }

    private:
        int width;
        int height;

        Color* pixels;
    };
}
