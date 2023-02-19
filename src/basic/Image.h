#pragma once
#include <cassert>

#include "Color.h"

#include <iostream>

namespace new_type_renderer
{
    using std::ofstream;
    using std::endl;

    struct Image
    {
    public:
        Image(int w, int h);

        ~Image();

        void SetPixel(int x, int y, const Color& color);

        void WriteImage(const char* fileName) const;

        int GetWidth() const;

        int GetHeight() const;

        Color* GetData();

        unsigned char* GetByteData();

        void Empty()
        {
            memset(m_Bytes, 0, sizeof(unsigned char) * m_Width * m_Height * 4);
        }

        void Resize(int width, int height)
        {
            m_Width = width;
            m_Height = height;
            delete m_Bytes;
            m_Bytes = new unsigned char[m_Width * m_Height * 4]();
        }

    private:
        int m_Width;

        int m_Height;

        unsigned char* m_Bytes{ nullptr };
    };
}
