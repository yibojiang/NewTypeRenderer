#pragma once
#include "Color.h"

namespace new_type_renderer
{
    struct Image
    {
    public:
        Image(int w, int h) : m_Width(w), m_Height(h)
        {
            m_Pixels = new Color(w * h);
        }

        void SetPixel(int i, int j, const Color& color)
        {
            assert(i <= m_Height);
            assert(j <= m_Width);
            m_Pixels[i * m_Height + j] = color;
        }

    private:
        int m_Width;
        int m_Height;

        Color* m_Pixels;
    };
}
