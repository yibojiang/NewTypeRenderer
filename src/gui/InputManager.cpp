#include "InputManager.h"

#include <cassert>

namespace new_type_renderer
{
    double InputManager::m_MousePosX{ 0.0f };

    double InputManager::m_MousePosY{ 0.0f };

    double InputManager::m_LastMousePositionX{ 0.0f };

    double InputManager::m_LastMousePositionY{ 0.0f };

    int InputManager::m_MouseStates[NUM_MOUSE_BUTTON];

    bool InputManager::IsMousePressed(const int buttonId)
    {
        assert(buttonId < NUM_MOUSE_BUTTON);
        return InputManager::m_MouseStates[buttonId];
    }
}