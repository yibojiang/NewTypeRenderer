#pragma once

#include "GLFW/glfw3.h"

#define NUM_MOUSE_BUTTON 8

namespace new_type_renderer
{
    class InputManager
    {
    public:
        static bool IsMousePressed(const int buttonId);

    public:
        static double m_MousePosX;

        static double m_MousePosY;

        static double m_LastMousePositionX;

        static double m_LastMousePositionY;

        static int m_MouseStates[NUM_MOUSE_BUTTON];
    };
}