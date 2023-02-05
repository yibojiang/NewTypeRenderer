#pragma once

#include <cassert>
#include "math/Matrix.h"
#include "utility/Log.h"

class TestCase
{
    virtual void Run() = 0;
};

class MathTest : public TestCase
{
public:
    virtual void Run() override
    {
        using namespace new_type_renderer;

        // Test Matrix Transpose
        {
            Matrix4x4 mat = Matrix4x4::FromTranslate(4.0f, 5.0f, 1.0f);
            const Matrix4x4 result = mat.Transposed();
            const Matrix4x4 expected(
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                4.0f, 5.0f, 1.0f, 1.0f
            );
            assert(result == expected);
        }

        // Test Translation Matrix
        {
            Vector4 pos(2, 4, 3, 1);
            const Matrix4x4 translate = Matrix4x4::FromTranslate(1, 5, 1);
            const Vector4 result = translate * pos;
            const Vector4 expected(3, 9, 4, 1);
            assert(result == expected);
        }
        
    }
};
