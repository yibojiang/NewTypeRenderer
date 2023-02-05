#pragma once
#include <cstdlib>

namespace new_type_renderer
{
    inline float Random01()
    {
        return rand() / static_cast<float>(RAND_MAX);
    }
}

