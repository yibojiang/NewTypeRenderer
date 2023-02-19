#include "Image.h"
#include <fstream>

namespace new_type_renderer
{
    Image::Image(int w, int h) : m_Width(w), m_Height(h)
    {
        m_Bytes = new unsigned char[w * h * 4]();
    }

    Image::~Image()
    {
        delete[] m_Bytes;
    }

    void Image::SetPixel(int x, int y, const Color& color)
    {
        assert(x < m_Width);
        assert(y < m_Height);
        assert(y * m_Width + x < m_Width * m_Height);
        m_Bytes[(y * m_Width + x) * 4] = static_cast<unsigned char>(color.x);
        m_Bytes[(y * m_Width + x) * 4 + 1] = static_cast<unsigned char>(color.y);
        m_Bytes[(y * m_Width + x) * 4 + 2] = static_cast<unsigned char>(color.z);
        m_Bytes[(y * m_Width + x) * 4 + 3] = 255;
    }

    void Image::WriteImage(const char* fileName) const
    {
        ofstream fout{ fileName, ios::binary };
        fout << "P3\n" << m_Width << " " << m_Height << "\n" << 255 << endl;
        for (int i = 0; i < m_Height * m_Width; i += 4)
        {
            int R = m_Bytes[i];
            int G = m_Bytes[i + 1];
            int B = m_Bytes[i + 2];
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

    unsigned char* Image::GetByteData()
    {
        return m_Bytes;
    }
}

