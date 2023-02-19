#include "Image.h"
#include <fstream>

namespace new_type_renderer
{
    Image::Image(int w, int h) : m_Width(w), m_Height(h)
    {
        m_Pixels = new Color[w * h]();
    }

    Image::~Image()
    {
        delete[] m_Pixels;
    }

    void Image::SetPixel(int x, int y, const Color& color)
    {
        assert(x < m_Width);
        assert(y < m_Height);
        assert(y * m_Width + x < m_Width * m_Height);
        m_Pixels[y * m_Width + x] = color;
    }

    void Image::WriteImage(const char* fileName) const
    {
        ofstream fout{ fileName, ios::binary };
        fout << "P3\n" << m_Width << " " << m_Height << "\n" << 255 << endl;
        for (int i = 0; i < m_Height * m_Width; i++)
        {
            Color& color = m_Pixels[i];
            int R = color.x;
            int G = color.y;
            int B = color.z;
            fout << R << " " << G << " " << B << " ";
        }

        fout.close();
    }

    int Image::GetWidth() const
    {
        return m_Width;
    }

    int Image::GetHeight() const
    {
        return m_Height;
    }

    Color* Image::GetData()
    {
        return m_Pixels;
    }
}

