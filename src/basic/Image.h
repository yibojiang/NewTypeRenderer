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

    private:
        int m_Width;

        int m_Height;

        Color* m_Pixels{ nullptr };

        unsigned char* m_Bytes{ nullptr };
    };
}
