#pragma once

#include "loadpng/lodepng.h"
#include "basic/Color.h"
#include "math/Vector.h"
#include "utility/Log.h"
#include <cassert>

namespace new_type_renderer
{
    class Texture
    {
    public:
        Texture() {}

        void LoadImage(const std::string& filename) {
            m_Image = std::vector<unsigned char>();
            unsigned error = lodepng::decode(m_Image, m_Width, m_Height, filename.c_str());

            if (error) {
                m_Loaded = false;
                assert(error == 0, "Fail to load texture: %s", filename.c_str());
            }
            else {
                m_Loaded = true;
                LOG_INFO("%s", filename.c_str());
            }
        }

        Color GetColor(const Vector2& uv)
        {
            if (!m_Loaded)
                return (Vector4(1, 0, 1, 1));

            int x = (fmod(fabs(uv.x), 1.0)) * (m_Width - 1);
            int y = (1. - fmod(fabs(uv.y), 1.0)) * (m_Height - 1);
            float r, g, b, a;
            r = m_Image.at(y * m_Width * 4 + x * 4) / 255.0f;
            g = m_Image.at(y * m_Width * 4 + x * 4 + 1) / 255.0f;
            b = m_Image.at(y * m_Width * 4 + x * 4 + 2) / 255.0f;
            a = m_Image.at(y * m_Width * 4 + x * 4 + 3) / 255.0f;
            return Color(r, g, b, a);
        }

        const unsigned char* GetImage() const
        {
            return m_Image.data();
        }

        void FlipImage()
        {
            unsigned int rowSize = m_Width * 4;
            unsigned char* row = new unsigned char[rowSize];
            unsigned int halfRows = m_Height / 2;

            for (int i = 0; i < halfRows; i++) {
                memcpy(row, &m_Image[i * rowSize], rowSize);
                memcpy(&m_Image[i * rowSize], &m_Image[(m_Height - i - 1) * rowSize], rowSize);
                memcpy(&m_Image[(m_Height - i - 1) * rowSize], row, rowSize);
            }

            delete[] row;
        }

        unsigned int GetHeight() const
        {
            return m_Height;
        }

        unsigned int GetWidth() const
        {
            return m_Width;
        }

    public:
        unsigned int m_Height;

        unsigned int m_Width;

        bool m_Loaded;

        std::vector<unsigned char> m_Image;
    };

}
